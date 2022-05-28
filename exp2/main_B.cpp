//  main.cpp
//  操作系统大作业实验2-B
//  Created by 张凯 on 2022/5/24.
//  Copyright © 2022 张凯. All rights reserved.

#include <iostream>
#include <cstring>
#include <vector>
#include <mutex>
#include <thread>
#include <fstream>
#include <algorithm>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <queue>
#include <condition_variable>
using namespace std;

const int Amount = 1000000;
const int Max_thread_num = 20;

//信号量的实现
class Semaphore
{
public:
    bool flag = true;
    Semaphore(int count = 0) : count(count) {}
    // V操作，唤醒
    void V()
    {
        std::unique_lock<std::mutex> unique(mt);
        ++count;
        if (count <= 0)
            cond.notify_one();
    }
    // P操作，阻塞
    void P()
    {
        std::unique_lock<std::mutex> unique(mt);
        --count;
        while (count < 0 && flag)
            cond.wait(unique);
    }
    void getcount()
    {
        cout << this->count << endl;
    }

private:
    int count;
    mutex mt;
    condition_variable cond;
};

int data[Amount];
int thread_num = 0;
int sorted_num = 0;

//互斥锁
std::mutex print_mutex;       //多线程打印互斥信号量
std::mutex thread_open_mutex; //创建新进程锁
std::mutex sorted_num_mutex;  //已排序个数修改锁
std::mutex request_queue;     //申请进入队列锁

//二叉树节点队列
queue<pair<int, int>> unsorted;

//队列长度信号量
Semaphore queue_length(0); //队列长度信号量,如果小于等于0,当前排序进程应该阻塞

//总线程
vector<std::thread> Thread;

void Producer(int *nums, int L, int R)
{
    auto ref = nums[L], i = L, j = R;
    if (L == R)
    {
        sorted_num_mutex.lock();
        sorted_num++;
        sorted_num_mutex.unlock();
    }
    else if (R - L <= 1000)
    {
        request_queue.lock();
        unsorted.push(make_pair(L, R));
        //释放队列资源 V操作
        queue_length.V();
        request_queue.unlock();
    }
    else
    {
        while (i != j)
        {
            while (nums[j] >= ref && j > i)
                j--;
            while (nums[i] <= ref && j > i)
                i++;
            if (i < j)
                swap(nums[i], nums[j]);
        }
        swap(nums[L], nums[i]);
        sorted_num_mutex.lock();
        sorted_num++;
        sorted_num_mutex.unlock();
        Producer(nums, L, i - 1);
        Producer(nums, i + 1, R);
    }
}

// quicksort algorithm
void quicksort(int *nums, int L, int R)
{
    auto ref = nums[L], i = L, j = R;
    if (L < R)
    {
        while (i != j)
        {
            while (nums[j] >= ref && j > i)
                j--;
            while (nums[i] <= ref && j > i)
                i++;
            if (i < j)
                swap(nums[i], nums[j]);
        }
        swap(nums[L], nums[i]);
        quicksort(nums, L, i - 1);
        quicksort(nums, i + 1, R);
    }
}

void Sort(int *nums, int pid)
{
    while (sorted_num < Amount)
    {
        if (sorted_num >= Amount)
            break;
        //申请队列元素 P操作
        // cout << "Pid:" << pid << " 现在的数量" << sorted_num << " 要开始申请P操作了" << endl;
        queue_length.P();
        if (sorted_num >= Amount)
            break;
        request_queue.lock();
        pair<int, int> T = unsorted.front();
        unsorted.pop();

        sorted_num_mutex.lock();
        sorted_num += T.second - T.first + 1;
        cout << sorted_num << endl;
        sorted_num_mutex.unlock();
        quicksort(nums, T.first, T.second);
        request_queue.unlock();
        print_mutex.lock();
        cout << "Pid:" << pid << " 排序完的数量:" << sorted_num << endl;
        print_mutex.unlock();
    }
}

int main()
{
    //二进制文件
    fstream data_in, data_out;
    data_in.open("data.dat", ios_base::out | ios_base::binary);
    data_out.open("out.dat", ios_base::out | ios_base::binary);
    if (!data_in.is_open())
    {
        std::cerr << "Opening file error!" << endl;
        exit(0);
    }
    std::srand(unsigned(time(nullptr)));
    for (int i = 0; i < Amount; i++)
        data[i] = rand() % 1000000;
    data_in.write((char *)data, Amount * sizeof(int));
    data_in.close();
    data_out.close();

    //保存TXT文件
    ofstream data_txt("data.txt");
    for (int i = 0; i < Amount; i++)
    {
        data_txt << data[i] << " ";
        if (i % 10 == 9)
            data_txt << endl;
    }
    data_txt.close();
    cout << "随机数生成保存完毕,开始创建文件映射..." << endl;
    //创建文件映射
    auto fd = open("data.dat", O_RDONLY);
    auto len = lseek(fd, 0, SEEK_END);
    //建立内存映射
    char *buffer = (char *)mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    //创建输出文件映射,复制原文件内容
    auto fd_out = open("out.dat", O_RDWR);

    lseek(fd_out, len - 1, SEEK_END);
    write(fd_out, "", 1);

    int *nums = (int *)mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd_out, 0);
    close(fd_out);

    //复制
    memcpy(nums, buffer, len);
    //解除映射
    munmap(buffer, len);

    //创建Producer 线程
    thread_open_mutex.lock();
    Thread.push_back(std::thread(Producer, nums, 0, Amount - 1));
    thread_open_mutex.unlock();
    Thread[0].join();
    //创建20个快速排序进程
    for (int i = 0; i < Max_thread_num; i++)
    {
        thread_open_mutex.lock();
        Thread.push_back(std::thread(Sort, nums, i + 1));
        print_mutex.lock();
        cout << "Thread num:" << Thread.size() << endl;
        print_mutex.unlock();
        thread_open_mutex.unlock();
    }

    while (sorted_num < Amount);

    //保存TXT文件
    ifstream F("out.dat", ios::binary | ios::in);
    F.read((char *)data, Amount * sizeof(int));

    ofstream out_txt("out.txt");
    for (int i = 0; i < Amount; i++)
    {
        out_txt << data[i] << " ";
        if (i % 10 == 9)
            out_txt << endl;
    }
    out_txt.close();
    munmap(nums, len);
    cout << "一切顺利!" << endl;
    queue_length.flag = false;
    //销毁线程
    for (int i = 1; i < Thread.size(); i++)
        Thread[i].join();
    //解除映射
    return 0;
}
