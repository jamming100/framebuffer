#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <cstring>		//c++
#include <arpa/inet.h>	//c++
#define PORT 9999
#define BACKLOG 10	//限制请求连接队列的长度

int main()
{
    int listenSocket = socket(AF_INET,SOCK_STREAM,0);
    if(listenSocket < 0)
    {
        printf("create listenSocketet error,errno is : %d, errstring is :%s\n",errno,strerror(errno));
    }
    struct sockaddr_in  serverAddr;
    struct sockaddr_in  clientAddr;
    bzero(&serverAddr,sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listenSocket,(struct sockaddr *)&serverAddr,sizeof(struct sockaddr_in)) < 0)
    {
        printf("bind error,error code is %d,error string is :%s\n",errno,strerror(errno));
        close(listenSocket);
        return 1;
    }
    if(listen(listenSocket,BACKLOG) < 0)
    {
        printf("listen error,error code is %d,error string is %s\n",errno,strerror(errno));
        close(listenSocket);
        return 2;
    }
    printf("bind and listen success,wait accept...\n");

	bool isShutdown = 0;
    for(;;)
    {
        socklen_t len = 0;
        printf("start accept \n");
        int clientSocket = accept(listenSocket,(struct sockaddr *)&clientAddr,&len);
        if(clientSocket < 0)
        {
            printf("accept error, error is %d,errstring is %s\n",errno,strerror(errno));
            close(listenSocket);
            return 3;
        }
        char buf_ip[INET_ADDRSTRLEN];
        memset(buf_ip,'\0',sizeof(buf_ip));
        inet_ntop(AF_INET,&clientAddr.sin_addr,buf_ip,sizeof(buf_ip));
        printf("get connect,ip is %s,port is %d\n",buf_ip,ntohs(clientAddr.sin_port));
		//if (pthread_create(&connectthread, NULL, start_routine, (void*)arg)) {        
        //    printf("pthread_create() error\n");
        //}
        while(1)
        {
            char buf[1024];
            memset(buf,'\0',sizeof(buf));//跟前面的初始化对比
            int readBytes = read(clientSocket,buf,sizeof(buf));
			printf("readBytes = %d\n",readBytes);
			if(readBytes <= 0){
				break;
			}
            printf("client :# %s\n",buf);

     		if(strncasecmp(buf,"quit",4) == 0)
	        {
				isShutdown = 1;
    	        printf("quit\n");
    	        break;
    	    }

            printf("server :$");
            memset(buf,'\0',sizeof(buf));
            fgets(buf,sizeof(buf),stdin);
            buf[strlen(buf)-1] = '\0';
            int sendBytes = write(clientSocket,buf,strlen(buf)+1);//在这里strlen(buf)+1和sizeof(buf)的区别
            printf("sendBytes = %d\n",sendBytes);
			if(sendBytes <= 0){
				break;
			}
            printf("please wait...\n");
        }
		if(isShutdown){
			break;
		}
	}
    close(listenSocket);
    return 0;
}

