#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <iostream>
#include <errno.h>

using namespace std;

int initserver(int port);

int main(int argc,char *argv[]){
  if(argc != 2){
    cout << "argu not match, ./demo port" << endl;
    return -1;
  }
  int listensock = initserver(atoi(argv[1]));
  cout << "listensock = " << listensock << endl;

  //创建epoll句柄
  int epollfd = epoll_create(1);

  //为服务端的listensock准备读事件
  epoll_event ev;
  ev.data.fd = listensock;
  ev.events = EPOLLIN;

  epoll_ctl(epollfd,EPOLL_CTL_ADD,listensock,&ev); //把需要监视的socket加入epollfd中

  epoll_event evs[10]; //存放epoll返回的事件
  
  while(true){
    int infds = epoll_wait(epollfd,evs,10,-1);

    
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