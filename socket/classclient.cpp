#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define MAXLEN 1024

using namespace std;

class ctcpclient{
    public:
        int m_clientfd;
        string m_ip;
        unsigned short m_port;
        
        ctcpclient():m_clientfd(-1){}
        ~ctcpclient(){close();}

        bool connect(const string &in_ip,unsigned short in_port);
        bool send(const string &buffer);
        bool recv(string &buffer,const size_t maxlen);
        bool close(){
            if(m_clientfd == -1){
                return false;
              }
                ::close(m_clientfd);
                m_clientfd = -1; //初始状态是-1,代表未连接
                return true;
            }
        
};

bool ctcpclient::connect(const string &in_ip,unsigned short in_port){
    if(m_clientfd != -1)return false;
    m_ip = in_ip;
    m_port = in_port;

    m_clientfd = socket(AF_INET,SOCK_STREAM,0);
    if(m_clientfd == -1){
      perror("socket failed!");
      return false;
    }

    struct hostent *h; //用于存放服务器地址的数据结构
    if((h = gethostbyname(m_ip.c_str())) == 0){
      cout << "gethostbyname failed!\n" << endl;
      //close(m_clientfd);
      m_clientfd = -1;
      return false; 
    }
    struct sockaddr_in servaddr;
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    memcpy(&servaddr.sin_addr,h->h_addr_list[0],h->h_length); //h_addr?指定服务器的IP地址
    servaddr.sin_port = htons(m_port); //指定服务器的端口

    if(::connect(m_clientfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) != 0){
      perror("connect failed!");
      //close(m_clientfd);
      m_clientfd = -1;
      return false;
    }
    return true;
}

bool ctcpclient::send(const string &buffer){
    if(m_clientfd == -1)return false;
      if(::send(m_clientfd,buffer.data(),buffer.size(),0)<=0){
        return false;
      }

      return true;
}

bool ctcpclient::recv(string &buffer,const size_t maxlen){
    buffer.clear();
    buffer.resize(maxlen);
    int readn = ::recv(m_clientfd,&buffer[0],buffer.size(),0);
    if(readn <= 0){
        buffer.clear();
        return false;
    }
    buffer.resize(readn);
    return true;
}

int main(int argc,char *argv[]){
    if(argc != 3){
      cout << "arguments failed!\n" << endl;
      return -1;
    }

    ctcpclient tcpclient;
    if(tcpclient.connect(argv[1],atoi(argv[2])) == false){
        perror("connect failed!");
        return -1;
    }

    cout << "CONNECT TO SERVER!" << endl;

    //return 0;

    string buffer;
    for(int ii = 0; ii<10; ii++){
      int iret;
      cin >> buffer ;
      //buffer = "this is " + to_string(ii+1) + " message";
      if(tcpclient.send(buffer) == false){
        perror("send failed!");
        break;
      }
      cout << "send to serv: " << buffer << endl;

      //再接受服务器端的回应
      if((iret = tcpclient.recv(buffer,MAXLEN))<=0){
        cout << "iret =" << iret << endl;
        break;
      }
      cout << "recv from serv :" << buffer << endl;
      sleep(1);
    }

    //close(tcpclient.m_clientfd);
    return 0;
}