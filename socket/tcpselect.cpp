#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <iostream>
using namespace std;

//自己照着打的,有问题跑不通
//自己照着打的,有问题跑不通
//自己照着打的,有问题跑不通

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

    int maxfd = listensock;

    while(true){
        //select有超时机制,若10s后监视的socket没有任何事件发生,返回超时
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        fd_set tmpfds = readfds;

        int infds = select(maxfd+1,&tmpfds,NULL,NULL,0);

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
        for(int eventfd = 0;eventfd <= maxfd;eventfd++){
            if(FD_ISSET(eventfd,&tmpfds) == 0)continue;

            //监听socket有事件,表示有新来的
            if(eventfd == listensock){
              struct sockaddr_in client;
              socklen_t len = sizeof(client);
              int clientsock = accept(listensock,(struct sockaddr*)&client,&len);
              if(clientsock < 0){
                perror("accpet()");
                continue;
              }
              cout << "accept success, clientsocket is " << clientsock << endl;

              if(maxfd < clientsock) maxfd = clientsock;
            }

            //客户端的socket有事件,即可以读了
            else{
              cout << "debug" << endl;
              char buffer[1024];
              memset(buffer,0,sizeof(buffer));
              if(recv(eventfd,buffer,sizeof(buffer),0) <= 0){
                cout << "clientsocket diconnected" << endl;
                close(eventfd);
                FD_CLR(eventfd,&readfds);

                if(eventfd == maxfd){ //重新计算maxfd的值,从后往前找
                  for(int ii=maxfd;ii>0;ii--){
                    if(FD_ISSET(ii,&readfds)){
                      maxfd = ii;break;
                    }
                  }
                }
              }
              else{
                string sendback = "Recved message: " + string(buffer);
                cout << "Recv :" << buffer << endl;
                //报文再发送回去
                send(eventfd,sendback.c_str(),sendback.size() + strlen(buffer),0);

              }
            }
        }

    }
    return 0;
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