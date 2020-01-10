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
#include <vector>

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

bool isServer = TRUE;
int heartCount = 0;

int splitString(const char*  str, std::vector<std::string>& cmd,const char* seq )
{
    cmd.clear();
    char    tempStr[256];
    strcpy( tempStr, str );
    char*    sub = strtok( (char*)tempStr, seq );
    while( sub != NULL )
    {
        cmd.push_back( sub );
        sub = strtok( NULL, seq );
    }
    return (int)cmd.size();
}


void onProcessMsg(std::vector<std::string>& msg){

    cout << "onProcessMsg msg.size() = " << msg.size() << endl;

    if(msg.size() == 0){
        return;
    }

    for(int i = 0; i < msg.size();i++){
        string str = msg.at(i);
        cout << str << endl;
    }

    int len = 0;
    string cmd = msg.at(0);

    if(cmd.compare("list") == 0){
        len = UART_Send(fd,"[list device1 device2 device3]",strlen("[list device1 device2 device3]"));
    }else if(cmd.compare("start") == 0){
        status = STATUS_START;
        len = UART_Send(fd,"[start ok]",strlen("[start ok]"));
        if(msg.size() == 2){
            cout << "msg.at(1) = " << msg.at(1) << endl;
        }
    }else if(cmd.compare("stop") == 0){
        status = STATUS_STOP;
        len = UART_Send(fd,"[stop ok]",strlen("[stop ok]"));
        if(msg.size() == 2){
            cout << "msg.at(1) = " << msg.at(1) << endl;
        }
    }else if(cmd.compare("exit") == 0){
        status = STATUS_EXIT;
        len = UART_Send(fd,"[exit ok]",strlen("[exit ok]"));
    }
}

void onResult(std::vector<std::string>& msg){

    cout << "onResult msg.size() = " << msg.size() << endl;

    if(msg.size() == 0){
        return;
    }

    for(int i = 0; i < msg.size();i++){
        string str = msg.at(i);
        cout << str << endl;
    }

    int len = 0;
    string cmd = msg.at(0);

    if(cmd.compare("list") == 0){
        for(int i = 0; i < msg.size();i++){
            string str = msg.at(i);
            cout << str << endl;
        }
    }else if(cmd.compare("start") == 0){
        status = STATUS_START;

        if(msg.size() == 2){
            cout << "msg.at(1) = " << msg.at(1) << endl;
        }
    }else if(cmd.compare("stop") == 0){
        status = STATUS_STOP;

        if(msg.size() == 2){
            cout << "msg.at(1) = " << msg.at(1) << endl;
        }
    }else if(cmd.compare("exit") == 0){
        status = STATUS_EXIT;
        if(msg.size() == 2){
            cout << "msg.at(1) = " << msg.at(1) << endl;
        }
    }else if(cmd.compare("heart") == 0){
        cout << "heartCount = " << heartCount << endl;
        heartCount++;
        for(int i = 0; i < msg.size();i++){
            string str = msg.at(i);
            cout << str << endl;
        }
    }
}


void* readUartThread(void * pArg){

    long id  = (long)pArg;
    Log("readThread arg = %d\n",id);

    int len;
    int i = 0;
    string str;

    while (1) //循环读取数据
    {
        int ret = mutex.TryLock();
        if(ret == 0){

startParse: len = UART_Recv(fd, readBUf,sizeof(readBUf));
            if(len > 0)
            {
                readBUf[len] = '\0';
                printf("receive data is %s\n",readBUf);
                str.append(readBUf);

                cout << "start parse:" << str << endl;

                int startIndex = -1;
                int endIndex = -1;
                if(isServer){
                    startIndex = str.find_first_of('{');
                    endIndex = str.find_first_of('}');
                }else{
                    startIndex = str.find_first_of('[');
                    endIndex = str.find_first_of(']');
                }

                if(startIndex >= 0 ){
                    if(endIndex > startIndex){
                        string sub = str.substr(startIndex + 1,(endIndex - startIndex) - 1);
                        cout << "parse msg :"<< sub << endl;
                        str = str.substr(endIndex + 1,str.length() - endIndex);
                        vector<std::string> msg;
                        splitString(sub.c_str(),msg," ");
                        if(isServer){
                            cout << "" << i << " msg :" << msg.at(0) << endl;
                            i++;
                            onProcessMsg(msg);
                        }else{
                            onResult(msg);
                        }

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
        if(isServer){
            cout << "heartCount = " << heartCount << endl;
            heartCount++;
            len = UART_Send(fd,"[heart device1 device2 device3]",strlen("[heart device1 device2 device3]"));
        }
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
    fd = UART_Open(fd,argv[1]); //打开串口，返回文件描述符

   // fd = UART_Open(fd, "/dev/ttyUSB0"); //打开串口，返回文件描述符

    printf("fd= \n",fd);

    do{
        err = UART_Init(fd,9600,0,8,1,'N');  //115200
        printf("Set Port Exactly!\n"); 
        sleep(1);
    }while(FALSE == err || FALSE == fd);    

    createReadThread();

    if(0 == strcmp(argv[2],"0"))    //开发板向pc发送数据的模式
    {   
        isServer = FALSE;
        mutex.TryLock();
        len = UART_Send(fd,"{list}",strlen("{list}"));
        mutex.Unlock();
        for(i = 0;i < 10000;i++)
        {

            fgets(writeBuf,256,stdin);   //输入内容，最大不超过256字节，fgets能吸收回车符，这样pc收到的数据就能自动换行
            mutex.TryLock();

            len = UART_Send(fd,writeBuf,strlen(writeBuf));
            //len = UART_Send(fd,"{list device1 device2 device3}",strlen("{list device1 device2 device3}"));
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
    UART_Close(fd);
}

