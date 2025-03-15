#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;

int main(int argc,char *argv[]){
  if(argc != 2){
    cout << "arguments not match !\n" << endl;
    return -1;
  }

  int listenfd = socket(AF_INET,SOCK_STREAM,0);
  if(listenfd ==-1){
    perror("socket failed");
    return -1;
  }

  struct sockaddr_in servaddr;
  memset(&servaddr,0,sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //服务端任何网卡的IP都可以用于通讯
  servaddr.sin_port = htons(atoi(argv[1]));
  if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) != 0){
    perror("bind failed!");
    return -1;
  }

  if(listen(listenfd,5) != 0){
    perror("listen failed!");
    return -1;
  }

  int clientfd = accept(listenfd,0,0);
  if(clientfd == -1){
    perror("accept failed");
    return -1;
  }
  cout << "CONNETION ESTABLISHED\n" << endl;

  char buffer[1024];
  while(1){
    int iret;
    memset(buffer,0,sizeof(buffer));
    if((iret = recv(clientfd,buffer,sizeof(buffer),0)) <= 0){
      cout << "iret = " << iret << endl;
      break;
    }
    cout << "recv :" << buffer << endl;

    strcpy(buffer,"ACK");
    if((iret = send(clientfd,buffer,strlen(buffer),0)) <= 0){
      perror("send failed");
      return -1;
    }
    cout << "send :" << buffer << endl;
  }

  close(listenfd);
  close(clientfd);
  return 0;
}