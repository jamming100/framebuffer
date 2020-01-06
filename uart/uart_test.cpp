#include "uart.h"
#include<stdio.h>      /*标准输入输出定义*/    
#include<stdlib.h>     /*标准函数库定义*/    
#include<unistd.h>     /*Unix 标准函数定义*/    
#include<sys/types.h>     
#include<sys/stat.h>       
#include<fcntl.h>      /*文件控制定义*/    
#include<termios.h>    /*PPSIX 终端控制定义*/    
#include<errno.h>      /*错误号定义*/    
#include<string.h>
 
#include <pthread.h>
#include "log.h"
#include "Mutex.h"
#include <string>
#include <iostream>

using namespace std;
pthread_t g_ReadThread = -1;
int fd = -1;           //文件描述符，先定义一个与程序无关的值，防止fd为任意值导致程序出bug

Mutex mutex = Mutex();
char readBUf[256];
char writeBuf[256];

#define STATUS_START 1
#define STATUS_STOP 2
#define STATUS_EXIT 3
int status = 0;

void onProcessMsg(string msg){

    cout << "onProcessMsg : " << msg << endl;

    int len = 0;
    //process msg
    if(msg.compare("list") == 0){
        len = UART0_Send(fd,"device1 device2 device3",strlen("device1 device2 device3"));
    }else if(msg.compare("start") == 0){
        status = STATUS_START;
        len = UART0_Send(fd,"started",strlen("started"));
    }else if(msg.compare("stop") == 0){
        status = STATUS_STOP;
        len = UART0_Send(fd,"stopped",strlen("stopped"));
    }else if(msg.compare("exit") == 0){
        status = STATUS_EXIT;
        len = UART0_Send(fd,"exited",strlen("exited"));
    }
}


void* readUartThread(void * pArg){

    long id  = (long)pArg;
    Log("readThread arg = %d\n",id);

    int len;

   string str;

    while (1) //循环读取数据
    {
        int ret = mutex.TryLock();
        if(ret == 0){

            len = UART0_Recv(fd, readBUf,sizeof(readBUf));
            if(len > 0)
            {
                readBUf[len] = '\0';
                printf("receive data is %s\n",readBUf);
                str.append(readBUf);


                cout << "start parse:" << str << endl;

                startParse:
                int startIndex = str.find_first_of('$');
                int endIndex = str.find_first_of('*');
                if(startIndex >= 0 ){
                    if(endIndex > startIndex){
                        string sub = str.substr(startIndex + 1,(endIndex - startIndex) - 1);
                        cout << "parse msg :"<< sub << endl;
                        str = str.substr(endIndex + 1,str.length() - endIndex);
                        onProcessMsg(sub);
                    }else{
                       str = str.substr(startIndex, str.length() - startIndex);
                       goto startParse;
                    }
                }else{
                    str.clear();
                }

                cout << "after parse:" << str << endl;
            }
            else
            {
                //printf("cannot receive data\n");
            }
            mutex.Unlock();
        }


        if(status == STATUS_EXIT){
            break;
        }
        sleep(1);
    }
}

void createReadThread(){
    pthread_create(&g_ReadThread, NULL, readUartThread, (void *)0);
}

void waitReadThreadExit(){
   pthread_join(g_ReadThread, NULL);
}

int main(int argc, char **argv)    
{

    int err;               //返回调用函数的状态    
    int len;
    int i;    

    for(int i = 0; i < argc; i++){
        printf("arg %d = %s\n",i, argv[i]);
    }

//    if(argc != 3)
//    {
//        write: ./uart /dev/pts/9 0    ;  read: ./uart /dev/pts/9 1
//        printf("Usage: %s /dev/ttySn 0(send data)\1(receive data) \n",argv[0]);
//        printf("open failure : %s\n", strerror(errno));
    
//        return FALSE;
//    }

    // socat  -d  -d  PTY  PTY 创建虚拟串口: /dev/pts/9 dev/pts/10
    fd = UART0_Open(fd,argv[1]); //打开串口，返回文件描述符

   // fd = UART0_Open(fd, "/dev/ttyUSB0"); //打开串口，返回文件描述符

    printf("fd= \n",fd);

    do{
        err = UART0_Init(fd,9600,0,8,1,'N');  //115200
        printf("Set Port Exactly!\n"); 
        sleep(1);
    }while(FALSE == err || FALSE == fd);    

    createReadThread();

    if(0 == strcmp(argv[2],"0"))    //开发板向pc发送数据的模式
    {   
        mutex.TryLock();
        len = UART0_Send(fd,"$list*",strlen("$list*"));
        mutex.Unlock();
        for(i = 0;i < 10;i++)
        {

            fgets(writeBuf,256,stdin);   //输入内容，最大不超过40字节，fgets能吸收回车符，这样pc收到的数据就能自动换行
            mutex.TryLock();

            len = UART0_Send(fd,writeBuf,40);
            if(len > 0)
                printf(" %d time send %d data successful\n",i,len);
            else
                printf("send data failed!\n");

           mutex.Unlock();
           sleep(1);
        }
        status = STATUS_EXIT;
    }


    waitReadThreadExit();
    UART0_Close(fd);
}

