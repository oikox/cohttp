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
    if(argc != 3){
      cout << "arguments failed!\n" << endl;
      return -1;
    }

    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd == -1){
      perror("socket failed!");
      return -1;
    }

    struct hostent *h; //用于存放服务器地址的数据结构
    if((h = gethostbyname(argv[1])) == 0){
      cout << "gethostbyname failed!\n" << endl;
      close(sockfd);
      return -1; 
    }
    struct sockaddr_in servaddr;
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    memcpy(&servaddr.sin_addr,h->h_addr_list[0],h->h_length); //h_addr?指定服务器的IP地址
    servaddr.sin_port = htons(atoi(argv[2])); //指定服务器的端口

    if(connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) != 0){
      perror("connect failed!");
      close(sockfd);
      return -1;
    }

    //连接建立成功
    char buffer[1024];
    for(int ii = 0; ii<3; ii++){
      int iret;
      memset(buffer,0,sizeof(buffer));
      sprintf(buffer,"the %d message,id is %d",ii+1,ii+1);
      if((iret = send(sockfd,buffer,strlen(buffer),0))<=0){
        perror("send failed!");
        return -1;
      }
      cout << "send to serv: " << buffer << endl;

      memset(buffer,0,sizeof(buffer));

      //再接受服务器端的回应
      if((iret = recv(sockfd,buffer,sizeof(buffer),0))<=0){
        cout << "iret =" << iret << endl;
        //perror("recv failed!");
        break;
      }
      cout << "recv from serv :" << buffer << endl;
      sleep(1);
    }
    close(sockfd);
}