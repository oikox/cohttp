#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <iostream>
using namespace std;

//自己照着打的
//自己照着打的
//自己照着打的

int initserver(int port);

int main(int argc,char *argv[]){
  if(argc != 2){
    cout << "argu not match, ./demo port" << endl;
    return -1;
  }
  int listensock = initserver(atoi(argv[1]));
  cout << "listensock = " << listensock << endl;

  pollfd fds[1024];

  //初始化数组,poll会忽略fd = -1;
  for(int ii=0;ii<1024;ii++){
    fds[ii].fd = -1;
  }

  fds[listensock].fd = listensock;
  fds[listensock].events = POLLIN; 
  //POLLIN代表读事件,POLLOUT代表写事件
  //pollfds有两种用法,第一种是数组下标和socket一一对应,第二种是从下标0开始填入socket.第二种空间利用率高但效率较低

  int maxfd = listensock;

  while(true){
    int infds = poll(fds,maxfd+1,10000); //第三个参数是ms,超时事件

    if(infds < 0){
      perror("poll()");
      break;
    }
    if(infds == 0){
      perror("timeout");
      continue;
    }

    for(int eventfd=0;eventfd<=maxfd;eventfd++){
      if(fds[eventfd].fd == -1)continue; //-1忽略

      if((fds[eventfd].revents & POLLIN) == 0)continue; //如果没有读事件

      //如果是listen事件
      if(eventfd == listensock){
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int clientsock = accept(listensock,(struct sockaddr*)&client,&len);
        if(clientsock < 0){
          perror("accept()");
          continue;
        }
        cout << "ACCEPT CLIENT SOCKET = " << clientsock << endl;

        fds[clientsock].fd = clientsock;
        fds[clientsock].events = POLLIN;

        if(maxfd < clientsock)maxfd = clientsock;
      }
      else{
        char buffer[1024];
        memset(buffer,0,sizeof(buffer));
        if(recv(eventfd,buffer,sizeof(buffer),0) <= 0){
          cout << "DISCONNECTED,CLIENT SOCKET = " << eventfd << endl;
          close(eventfd);
          fds[eventfd].fd = -1;

          if(eventfd == maxfd){
            for(int ii=maxfd;ii>=0;ii--){
              if(fds[ii].fd != -1){
                maxfd = ii;break;
              }
            }
          }
        }
        else{
          string sendback = "Recv from serv :" + string(buffer);
          cout << "Recv :" << buffer << endl;
          send(eventfd,sendback.c_str(),sendback.size(),0);
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