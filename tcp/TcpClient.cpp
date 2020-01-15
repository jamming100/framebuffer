#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <strings.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

#define SERVER_PORT 9999
#define SERVER_IP "127.0.0.1"

int main(int argc,char *argv[])
{
    if(argc != 2)
    {
        printf("usage :client IP\n");
        return 1;
    }
    char *str = argv[1];
    char buf[1024];
    memset(buf,'\0',sizeof(buf));

    struct sockaddr_in serverAddr;
    int socketFd = socket(AF_INET,SOCK_STREAM,0);

    bzero(&serverAddr,sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET,SERVER_IP,&serverAddr.sin_addr);
    serverAddr.sin_port = htons(SERVER_PORT);

    int ret = connect(socketFd,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
    if(ret < 0)
    {
        printf("connect failed...,errno is %d,errstring is %s\n",errno,strerror(errno));
    }
    printf("connect is success...\n");
    while(1)
    {
        printf("client :#");
        fgets(buf,sizeof(buf),stdin);
        buf[strlen(buf)-1] = '\0';
        write(socketFd,buf,sizeof(buf));
        if(strncasecmp(buf,"quit",4) == 0)
        {
            printf("quit\n");
            break;
        }
        printf("please wait...\n");
        read(socketFd,buf,sizeof(buf));
        printf("server :$ %s\n",buf);
    }
    close(socketFd);
    return 0;
}
