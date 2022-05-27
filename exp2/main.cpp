//  main.cpp
//  操作系统大作业实验2
//  Created by 张凯 on 2022/5/24.
//  Copyright © 2022 张凯. All rights reserved.

#include <iostream>
#include <vector>
#include <mutex>
#include <fstream>
#include <algorithm>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <queue>
#include <condition_variable>

using namespace std;

const int Amount = 1000000;
const int Buffer_size = 4000000;
const int Max_thread_num = 20;

//信号量的实现
class Semaphore
{
public:
    Semaphore(int count=0) : count(count) {}
    //V操作，唤醒
    void V()
    {
        std::unique_lock<std::mutex> unique(mt);
        ++count;
        if (count <= 0)
            cond.notify_one();
    }
    //P操作，阻塞
    void P()
    {
        std::unique_lock<std::mutex> unique(mt);
        --count;
        if (count < 0)
            cond.wait(unique);
    }
    void getcount(){
        cout<<this->count<<endl;
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
std::mutex print_mutex;  //多线程打印互斥信号量
std::mutex thread_open_mutex; //创建新进程锁
std::mutex sorted_num_mutex; //已排序个数修改锁
std::mutex request_queue; //申请进入队列锁

//二叉树节点队列
queue<pair<int, int>> unsorted;

//队列长度信号量
Semaphore queue_length(0); //队列长度信号量,如果小于等于0,当前排序进程应该阻塞

// quicksort algorithm
void quicksort(int *nums,int L,int R){
    auto ref = nums[L],i = L,j = R;
    if(L<R){
        while(i!=j){
            while(nums[j]>=ref && j>i)
                j--;
            while(nums[i]<=ref && j>i)
                i++;
            if(i<j)
                swap(nums[i], nums[j]);
        }
        swap(nums[L], nums[i]);
        quicksort(nums, L, i-1);
        quicksort(nums, i+1, R);
    }
}

void Sort(int *nums,pair<int, int>LR){
    int L = LR.first;
    int R = LR.second;
    while (sorted_num<Amount) {
        if(L==R){
            //此进程运行结束,可以从队列中取出未排序的序列->复用当前进程,不必重新关闭和开启
            sorted_num_mutex.lock();
            sorted_num ++;
            sorted_num_mutex.unlock();
            if(sorted_num==Amount) break;
            else{
                queue_length.P();
                request_queue.lock();
                pair<int, int>T = unsorted.front();
                unsorted.pop();
                print_mutex.lock();
                cout<<"out node:"<<"(L,R) = ("<<T.first<<","<<T.second<<")"<<endl;
                print_mutex.unlock();
                L = T.first;
                R = T.second;
            }
            continue;
        }
        if(L<R){
            print_mutex.lock();
            cout<<"分割序列:"<<"(L,R) = ("<<L<<","<<R<<")"<<endl;
            print_mutex.unlock();
        }
        if(R-L<=1000){
            quicksort(nums, L, R);
            sorted_num_mutex.lock();
            sorted_num = sorted_num + R-L+1;
            print_mutex.lock();
            cout<<"already sorted num:"<<sorted_num<<endl;
            print_mutex.unlock();
            sorted_num_mutex.unlock();
            //此进程运行结束,可以从队列中取出未排序的序列->复用当前进程,不必重新关闭和开启
            if(sorted_num==Amount) break;
            else{
                //占用元素,判断队列是否有元素 P操作
                queue_length.P();
                request_queue.lock();
                pair<int, int> T = unsorted.front();
                unsorted.pop();
                print_mutex.lock();
                cout<<"out node:"<<"(L,R) = ("<<T.first<<","<<T.second<<")"<<endl;
                print_mutex.unlock();
                request_queue.unlock();
                L = T.first;
                R = T.second;
            }
        }
        else if
        
    }
    
    
}




int main() {
    //二进制文件
    fstream data_in,data_out;
    data_in.open("data.mat",ios_base::out|ios_base::binary);
    data_out.open("out.mat",ios_base::out|ios_base::binary);
    if(!data_in.is_open()){
        std::cerr<<"Opening file error!"<<endl;
        exit(0);
    }
    std::srand(unsigned(time(nullptr)));
    for(int i=0;i<Amount;i++)
        data[i] = rand();
    data_in.write((char*)data, Amount*sizeof(int));
    data_in.close();
    data_out.close();
    
    //保存TXT文件
    ofstream data_txt("data.txt");
    for (int i=0; i<Amount; i++) {
        data_txt<<data[i]<<" ";
        if(i%10==9)
            data_txt<<endl;
    }
    cout<<"随机数生成保存完毕,开始创建文件映射..."<<endl;
    //创建文件映射
    auto fd = open("data.mat", O_RDONLY);
    
    auto len = lseek(fd, 0, SEEK_END);
    //建立内存映射
    char *buffer = (char *)mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    
    //创建输出文件映射,复制原文件内容
    auto fd_out = open("out.mat",O_RDWR);
    
    lseek(fd_out,len-1,SEEK_END);
    write(fd_out, "", 1);
    cout<<len<<endl;
    int *nums = (int *)mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_SHARED, fd_out, 0);
    close(fd_out);
    
    //复制
    memcpy(nums, buffer, len);
    //解除映射
    munmap(buffer, len);
    
    
    //进入快速排序进程
    
    
    
    

    
    //解除映射
    munmap(nums, len);
    
    
    cout<<"一切顺利!"<<endl;
    return 0;
}
