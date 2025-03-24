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

  //����epoll���
  int epollfd = epoll_create(1);

  //Ϊ����˵�listensock׼�����¼�
  epoll_event ev;
  ev.data.fd = listensock;
  ev.events = EPOLLIN;

  ev.events = EPOLLIN | EPOLLET; //ET��Եģʽ

  epoll_ctl(epollfd,EPOLL_CTL_ADD,listensock,&ev); //����Ҫ���ӵ�socket����epollfd��

  epoll_event evs[100]; //���epoll���ص��¼�
  
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

    //���infds>0,�����¼�������socket����
    for(int ii=0;ii<infds;ii++){ //����epoll���ص�����evs[]
      if(evs[ii].data.fd == listensock){

        while(true){
          struct sockaddr_in client;
          socklen_t len = sizeof(client);
          int clientsock = accept(listensock,(struct sockaddr*)&client,&len);
          if((clientsock < 0 && errno == EAGAIN))break;
          cout << "ACCEPT, CLIENT SOCK = "<< clientsock <<endl;
          static int k = 0;
          printf("this is %d connect\n",++k);
          
          //setnoblocking(clientsock);

          //Ϊ�¿ͻ���׼�����¼�,����epoll��
          ev.data.fd = clientsock;
          ev.events = EPOLLIN;
          ev.events = EPOLLIN | EPOLLET;
          epoll_ctl(epollfd,EPOLL_CTL_ADD,clientsock,&ev);
        }
      }
      else{
        //������б��Ļ��������ѶϿ�
        char buffer[1024];
        memset(buffer,0,sizeof(buffer));
        if(recv(evs[ii].data.fd,buffer,5,0)<=0){
          cout << "DISCONNECTED, CLIENT SOCK ="<<evs[ii].data.fd << endl;
          close(evs[ii].data.fd);
          //epollfd�����socket�Ͽ�,���Զ�ɾ��,����Ҫ����Ա�ֶ�д����ɾ��
          //epoll_ctl(epollfd,EPOLL_CTL_DEL,evs[ii].data.fd,0);
        }
        else{
          //�б���
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