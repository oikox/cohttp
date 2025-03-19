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

//�Լ����Ŵ��
//�Լ����Ŵ��
//�Լ����Ŵ��

int initserver(int port);

int main(int argc,char *argv[]){
  if(argc != 2){
    cout << "argu not match, ./demo port" << endl;
    return -1;
  }
  int listensock = initserver(atoi(argv[1]));
  cout << "listensock = " << listensock << endl;

  pollfd fds[1024];

  //��ʼ������,poll�����fd = -1;
  for(int ii=0;ii<1024;ii++){
    fds[ii].fd = -1;
  }

  fds[listensock].fd = listensock;
  fds[listensock].events = POLLIN; 
  //POLLIN������¼�,POLLOUT����д�¼�
  //pollfds�������÷�,��һ���������±��socketһһ��Ӧ,�ڶ����Ǵ��±�0��ʼ����socket.�ڶ��ֿռ������ʸߵ�Ч�ʽϵ�

  int maxfd = listensock;

  while(true){
    int infds = poll(fds,maxfd+1,10000); //������������ms,��ʱ�¼�

    if(infds < 0){
      perror("poll()");
      break;
    }
    if(infds == 0){
      perror("timeout");
      continue;
    }

    for(int eventfd=0;eventfd<=maxfd;eventfd++){
      if(fds[eventfd].fd == -1)continue; //-1����

      if((fds[eventfd].revents & POLLIN) == 0)continue; //���û�ж��¼�

      //�����listen�¼�
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