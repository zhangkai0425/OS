//  main.cpp
//  操作系统大作业实验1
//  Created by 张凯 on 2022/5/9.
//  Copyright © 2022 张凯. All rights reserved.
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <fstream>
#include <unistd.h>
#include <condition_variable>

using namespace std;

//柜台数量和最大顾客数
const int N_Counter = 1;
const int N_Customer = 20;

int customer_number = 0;//顾客总数
int customer_serve = 0; //正在服务顾客序号
int customer_served = 0;//已经服务过的顾客数
int counter_number = 0; //柜台序号

//读取顾客信息数据
struct cus_in{
    int cus_number;//顾客编号
    int time_in;//进入时间
    int time_serve;//服务时间
};

struct cus_out{
    int cus_number;//顾客编号
    int time_in;//进入时间
    int time_serve;//服务时间
    double time_beginserve; //开始服务的时间
    int counter_no; //柜台号
    double time_served; //结束服务时间
};
//自己实现的信号量
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



vector<cus_in>cus_ins;
vector<cus_out>cus_outs;

Semaphore sema_customer(0);                                //同步信号量,确保柜台处于等待状态情况

std::mutex counter_mutex[N_Counter];                        //柜台互斥量,防止一个柜台服务多个顾客
std::mutex customer_mutex;                                  //顾客互斥量,防止多个顾客同时取相同的号
time_t time_begin = time(NULL);
void PVcounter(int id){
    while (true) {
        //等待顾客出现 P操作
        sema_customer.P();
        //占用柜台资源
        counter_mutex[id].lock();
        time_t time_start = time(NULL);
        // cout<<time_start<<endl;
        //模拟服务时间
        int now_serve = customer_serve;
        customer_serve ++;
        cout<<"服务时长:"<<cus_outs[now_serve].time_serve<<" counter id:"<<id<<endl;
        std::this_thread::sleep_for(std::chrono::seconds(cus_outs[now_serve].time_serve));
        time_t time_end = time(NULL);
        cus_outs[now_serve].time_beginserve = time_start - time_begin;
        cus_outs[now_serve].time_served = time_end - time_start;
        cus_outs[now_serve].counter_no = id;
        //此顾客服务结束——>顾客号+1
        //释放柜台资源
        counter_mutex[id].unlock();
        //总服务过的人数+1
        customer_served ++;
        cout<<"服务结束:总服务过的人数:"<<customer_served<<endl;
    }
}
void PVcustomer(int id){
    //模拟睡眠至进入线程的时间
    std::this_thread::sleep_for(std::chrono::seconds(cus_ins[id].time_in));
    cout<<"顾客进入银行 id = " <<cus_ins[id].cus_number<<" 进入时间:"<<cus_ins[id].time_in<<" 需要服务时长:"<<cus_ins[id].time_serve<<endl;
    //占用银行取号机
    customer_mutex.lock();
    cus_out tmp_cus;
    tmp_cus.cus_number = cus_ins[id].cus_number;
    tmp_cus.time_in = cus_ins[id].time_in;
    tmp_cus.time_serve = cus_ins[id].time_serve;
    cus_outs.push_back(tmp_cus);
    //取完号之后释放互斥量
    customer_mutex.unlock();
    //顾客开始等待柜台服务 V操作
    sema_customer.V();
}

int main(){
    ifstream file;
    file.open("input.txt",ios::in);
    if(!file.good()){
        cout<<"Opening file failed:EXIT(0)"<<endl;
        exit(0);
    }
    else
        cout<<"Opening file succeeded!"<<endl;
    cus_in tmp_cus;
    while (!file.eof()) {
        file>>tmp_cus.cus_number>>tmp_cus.time_in>>tmp_cus.time_serve;
        cus_ins.push_back(tmp_cus);
    }
    customer_number = cus_ins.size();
    vector<std::thread> Thread;
    //创建顾客线程
    for(int i=0;i<cus_ins.size();i++)
        Thread.push_back(std::thread(PVcustomer,i));
    //创建柜台线程
    for(int j=0; j<N_Counter;j++)
        Thread.push_back(std::thread(PVcounter,j));
    //等待线程结束,关闭线程
    while(customer_served<customer_number);


    cout<<"---------------------------顾客接待情况表----------------------------"<<endl;
    cout << "顾客编号" << '\t' 
         << "进入时间" << '\t' 
         << "开始时间" << '\t' 
         << "服务时间" << '\t' 
         << "柜台号  " << endl;
    for(auto x:cus_outs)
        cout << x.cus_number << '\t' << '\t' << x.time_in << '\t' << '\t' << x.time_beginserve << '\t' << '\t' << x.time_serve << '\t' << '\t'<<x.counter_no<<endl;

    for (int i = 0; i < Thread.size(); i++)
        Thread[i].~thread();
}
