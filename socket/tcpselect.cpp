#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <iostream>
using namespace std;

int initserver(int port);

int main(int argc,char *argv[]){
    if(argc != 2){
        cout << "argu not match, ./demo port" << endl;
        return -1;
    }
    int listensock = initserver(atoi(argv[1]));
    cout << "listensock = " << listensock << endl;

    //select可以监视三种读事件
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(listensock,&readfds);

    int maxf = listensock;

    while(true){
        //select有超时机制,若10s后监视的socket没有任何事件发生,返回超时
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        fd_set tmpfds = readfds;

        int infds = select(maxf+1,&tmpfds,NULL,NULL,&timeout);

        //如果<0,表示select调用失败
        if(infds < 0){
            perror("select()");
            break;
        }
        if(infds == 0){
            cout << "timeout" << endl;
            continue;
        }
        //如果select有事件发生,返回发生事件的个数,并修改bitmap,未发生事件的被清空
        for(int eventfd = 0;eventfd <= maxf;eventfd++){
            if(FD_ISSET(eventfd,&tmpfds) == 0)continue;
        }

    }

}

int initserver(int port){
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0){
        perror("socket failed");
    }
    int opt = 1;
    unsigned int len = sizeof(opt);
    setsockopt(sock,SOL_SOCKET,SO_REUSEPORT,&opt,len);

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0){
        perror("bind failed");
        close(sock);
        return -1;
    }

    if(listen(sock,5) != 0){
        perror("listen failed");
        close(sock);
        return -1;
    }

    return sock;
}