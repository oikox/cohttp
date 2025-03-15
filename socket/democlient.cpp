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

    struct hostent *h; //���ڴ�ŷ�������ַ�����ݽṹ
    if((h = gethostbyname(argv[1])) == 0){
      cout << "gethostbyname failed!\n" << endl;
      close(sockfd);
      return -1; 
    }
    struct sockaddr_in servaddr;
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    memcpy(&servaddr.sin_addr,h->h_addr_list[0],h->h_length); //h_addr?ָ����������IP��ַ
    servaddr.sin_port = htons(atoi(argv[2])); //ָ���������Ķ˿�

    if(connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) != 0){
      perror("connect failed!");
      close(sockfd);
      return -1;
    }

    //���ӽ����ɹ�
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

      //�ٽ��ܷ������˵Ļ�Ӧ
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