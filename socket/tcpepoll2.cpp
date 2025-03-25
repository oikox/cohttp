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

int setnoblocking(int fd){
  int flags;

  //Get FD status;
  if(flags=fcntl(fd,F_GETFL,0) == -1){
    flags = 0;
  }
  return fcntl(fd,F_SETFL,flags | O_NONBLOCK);
}

int main(int argc,char *argv[]){
  if(argc != 2){
    cout << "argu not match, ./demo port" << endl;
    return -1;
  }
  int listensock = initserver(atoi(argv[1]));
  cout << "listensock = " << listensock << endl;

  setnoblocking(listensock);

  //创建epoll句柄
  int epollfd = epoll_create(1);

  //为服务端的listensock准备读事件
  epoll_event ev;
  ev.data.fd = listensock;
  ev.events = EPOLLIN;

  //ev.events = EPOLLIN | EPOLLET; //ET边缘模式

  epoll_ctl(epollfd,EPOLL_CTL_ADD,listensock,&ev); //把需要监视的socket加入epollfd中

  epoll_event evs[100]; //存放epoll返回的事件
  
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

        while(true){
          struct sockaddr_in client;
          socklen_t len = sizeof(client);
          int clientsock = accept(listensock,(struct sockaddr*)&client,&len);
          if((clientsock < 0 && errno == EAGAIN))break;
          cout << "ACCEPT, CLIENT SOCK = "<< clientsock <<endl;
          static int k = 0;
          printf("this is %d connect\n",++k);
          
          setnoblocking(clientsock);

          //为新客户端准备读事件,加入epoll中
          ev.data.fd = clientsock;
          //ev.events = EPOLLIN;
          ev.events = EPOLLOUT | EPOLLET;
          //ev.events = EPOLLIN | EPOLLET;
          epoll_ctl(epollfd,EPOLL_CTL_ADD,clientsock,&ev);
        }
      }
      else{
        //如果是有报文或者连接已断开
        /*
        char buffer[1024];
        memset(buffer,0,sizeof(buffer));
        int readn; //每次调用recv的返回值
        char* ptr = buffer; //buffer的位置指针
        while(true){
          if((readn = recv(evs[ii].data.fd,ptr,5,0))<=0){
            if((readn <0) && (errno == EAGAIN)){
              //数据被读取完毕,则发回报文
              send(evs[ii].data.fd,buffer,strlen(buffer),0);
              cout << "recv: " << buffer << endl;
            }
            else{
              //连接断开
              cout << "disconnected,eventfd = " << evs[ii].data.fd << endl;
              close(evs[ii].data.fd);
            }
            break;
          }
          else{
            ptr += readn; //buffer指针后移
          }
        }
        */
       cout << "ET WRITE EVENT" << endl;
       for(int k=0;k<1000000;k++){
        if(send(ev.data.fd,"aaaaaaaaaaaaaaaaaaaavgbbbbbbbbbbbb",30,0)<=0){
          if(errno == EAGAIN){
            cout << "SEND CACHE FULL" << endl;
            break;
          }
        }
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