//  main.cpp
//  实验3
//  Created by 张凯 on 2022/6/1.
//  Copyright © 2022 张凯. All rights reserved.

#include<iostream>
#include<fstream>
#include<queue>
#include<vector>
using namespace std;
class Banker{
public:
    Banker(int n,int m, vector<int> Available,vector<vector<int>> Max,vector<vector<int>> Allocation,vector<vector<int>> Need);
    // 主要函数:处理请求
    void Request(int id,vector<int>R);
    // 安全性算法函数
    bool SafeAlgorithm();
    // 终端打印输出结果的函数
    void Ans();
private:
    const int M = 100;
    //全局变量
    int n = 0; //线程数
    int m = 0; //资源数
    //安全序列
    vector<int>Safe;
    // Available 资源向量
    vector<int> Available;
    // Max 最大需求矩阵
    vector<vector<int>> Max;
    // Allocation 分配矩阵
    vector<vector<int>> Allocation;
    // Need 需求矩阵
    vector<vector<int>> Need;
    // Work 动态可分配资源
    vector<vector<int>> Work;
    // Work+Allocation Matrix
    vector<vector<int>> Work_Allocation;
    // Finish 是否成功分配
    vector<bool> Finish;
};
Banker::Banker(int n,int m,vector<int> Available,vector<vector<int>> Max,vector<vector<int>> Allocation,vector<vector<int>> Need){
    this->n = n;
    this->m = m;
    this->Available = Available;
    this->Max = Max;
    this->Allocation = Allocation;
    this->Need = Need;
    vector<bool>tmp(n,false);
    this->Finish = tmp;
}
void Banker::Request(int id,vector<int>R){
    bool flag = true;
    for (int i=0;i<R.size();i++) {
        if(this->Need[id][i]<R[i]){
            flag = false;
            cout<<"Request[i]>Need[i],请求向量不正确,程序错误返回 "<<endl;
            return;
        }
    }
    for (int i=0;i<R.size();i++) {
        if(this->Available[i]<R[i]){
            flag = false;
            cout<<"Request[i]>Available[i],请求向量无法满足 "<<"线程:"<<id<<" 等待"<<endl;
            return;
        }
    }
    if(flag){
        //试分配
        cout<<"开始试分配..."<<endl;
        for(int i=0;i<R.size();i++){
            this->Available[i] -= R[i];
            this->Allocation[id][i] += R[i];
            this->Need[id][i] -= R[i];
        }
        if (SafeAlgorithm()) {
            cout<<"系统新状态安全,分配完成！"<<endl;
            //输出程序运行结果步骤
            this->Ans();
        }
        else{
            cout<<"安全性算法检查失败！"<<endl;
            flag = false;
            this->Ans();
            //恢复原状态,进程等待
            for(int i=0;i<R.size();i++){
                this->Available[i] += R[i];
                this->Allocation[id][i] -= R[i];
                this->Need[id][i] += R[i];
            }
            cout<<"恢复原状态,进程等待"<<endl;
            return;
        }
    }
}
bool Banker::SafeAlgorithm(){
    vector<int> work = this->Available;
    vector<int> work_allocation = work;
    vector<bool> finish(this->n,false);
    while(this->Safe.size()<n){
        bool select = false;
        int select_id=0;
        for (int i=0; i<n; i++) {
            if(finish[i]) continue;
            select = true;
            for (int j=0; j<work.size(); j++) {
                if(work[j]<this->Need[i][j]){
                    select = false;
                    break;
                }
            }
            if(select){
                select_id = i;
                break;
            }
        }
        if(!select)
            return false;
        //可分配给此进程
        else{
            //加入安全序列
            finish[select_id] = true;
            this->Finish[select_id] = true;
            this->Safe.push_back(select_id);
            this->Work.push_back(work);
            for (int j=0; j<work.size(); j++)
                work_allocation[j] += this->Allocation[select_id][j];
            this->Work_Allocation.push_back(work_allocation);
            //更新work向量
            work = work_allocation;
        }
    }
    return true;
}
void Banker::Ans(){
    cout<<"算法执行流程:"<<endl;
    cout<<"试分配后Available变量: ";
    for(auto x:this->Available)
        cout<<x<<" ";
    cout<<endl;
    cout<<"进程号"<<"\t"<<"|"<<"\t"<<"Work"<<"\t"<<"|"<<"\t"<<"Need"<<"\t"<<"|"<<"\t"<<"Allocation"<<"\t"<<"|"<<"\t"<<"Work + Allocation" <<"\t"<<"|"<<"\t" << "Finish"<<endl;
    for (int i=0; i<this->Safe.size(); i++) {
        auto id = this->Safe[i];
        cout<<id<<"     \t"<<"|"<<"\t";
        for(auto x:this->Work[i])
            cout<<x<<" ";
        cout<<"\t"<<"|"<<"\t";
        for(auto x:this->Need[id])
            cout<<x<<" ";
        cout<<"\t"<<"|"<<"\t";
        for(auto x:this->Allocation[id])
            cout<<x<<" ";
        cout<<"   \t"<<"|"<<"\t    ";
        for(auto x:this->Work_Allocation[i])
            cout<<x<<" ";
        cout<<"      \t"<<"|"<<"\t";
        cout<<"True ";
        cout<<endl;
    }
    for (int i=0; i<this->n; i++) {
        if(Finish[i]) continue;
        auto id = i;
        cout<<id<<"     \t"<<"|"<<"\t";
        for(int j=0;j<this->Work.size();j++)
            cout<<"  ";
        cout<<"      \t"<<"|"<<"\t";
        for(auto x:this->Need[id])
            cout<<x<<" ";
        cout<<"\t"<<"|"<<"\t";
        for(auto x:this->Allocation[id])
            cout<<x<<" ";
        cout<<"   \t"<<"|"<<"\t    ";
        for(int j=0;j<this->Work_Allocation.size();j++)
            cout<<"  ";
        cout<<"            \t"<<"|"<<"\t";
        cout<<"False";
        cout<<endl;
    }
}

int main(){
    //设置全局变量
    int n = 5; //线程数
    int m = 3; //资源数
    // Available 资源向量
    vector<int> Available = {2,3,0};
    // Max 最大需求矩阵
    vector<vector<int>> Max = {{7,5,3},{3,2,2},{9,0,2},{2,2,2},{4,3,3}};
    // Allocation 分配矩阵
    vector<vector<int>> Allocation = {{0,1,0},{2,0,0},{3,0,2},{2,1,1,},{0,0,2}};
    // Need 需求矩阵
    vector<vector<int>> Need = {{7,4,3},{1,2,2},{6,0,0},{0,1,1},{4,3,1}};
    Banker banker(n,m,Available,Max,Allocation,Need);
    vector<int> R = {0,2,0};
    banker.Request(0,R);
}
