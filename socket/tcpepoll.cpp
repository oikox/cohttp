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
    
    if(infds<0){
      perror("epoll()");
      break;
    }

    if(infds == 0){
      cout << "timeout" << endl;
      continue;;
    }

    //如果infds>0,代表事件发生的socket数量
    for(int ii=0;ii<infds;ii++){ //遍历epoll返回的数组evs[]
      if(evs[ii].data.fd == listensock){
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int clientsock = accept(listensock,(struct sockaddr*)&client,&len);
        cout << "ACCEPT, CLIENT SOCK = "<< clientsock <<endl;

        //为新客户端准备读事件,加入epoll中
        ev.data.fd = clientsock;
        ev.events = EPOLLIN;
        epoll_ctl(epollfd,EPOLL_CTL_ADD,clientsock,&ev);
      }
      else{
        //如果是有报文或者连接已断开
        char buffer[1024];
        memset(buffer,0,sizeof(buffer));
        if(recv(evs[ii].data.fd,buffer,sizeof(buffer),0)<=0){
          cout << "DISCONNECTED, CLIENT SOCK ="<<evs[ii].data.fd << endl;
          close(evs[ii].data.fd);
          //epollfd中如果socket断开,会自动删除,不需要程序员手动写代码删除
          //epoll_ctl(epollfd,EPOLL_CTL_DEL,evs[ii].data.fd,0);
        }
        else{
          //有报文
          cout << "recv from client: " << buffer << endl;
          string sendback = "recv from serv: " + string(buffer);
          send(evs[ii].data.fd,sendback.c_str(),sendback.size(),0);
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