# OS

## 操作系统课程期末大作业Lab实验代码仓库

### 操作系统大作业 实验1 进程互斥与同步

Linux 下编译命令-需要链接`pthread`库才能正确编译

```bash
cd exp1
ls
g++ -std=c++11 main.cpp -o mainn -lpthread
./mainn
```



### 操作系统大作业 实验2 高级进程间通信问题

两个实现：传统的版本实现和自己的新方法实现

Linux 下编译命令-需要链接`pthread`库才能正确编译

- 实现A

```bash
cd exp2
ls
g++ -std=c++11 main_A.cpp -o mainn_A -lpthread
./mainn_A
```

- 实现B

```bash
cd exp2
ls
g++ -std=c++11 main_B.cpp -o mainn_B -lpthread
./mainn_B
```



### 操作系统大作业 实验3 处理机调度

银行家算法的模拟与实现

Linux/Mac/Windows均可正常编译

Linux 下编译命令

```bash
cd exp3
ls
g++ -std=c++11 main.cpp -o mainn
./mainn
```

