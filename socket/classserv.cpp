#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#define MAXLEN 1024

using namespace std;

//自己瞎写的。视频里把socket bind listen全部整合为init了

class ctcpserv{
    private:
        int m_listenfd;
        int m_connectfd;
        string m_clientip;
        unsigned short m_port; //服务器的端口

    public:
        ctcpserv():m_listenfd(-1),m_connectfd(-1){
            m_listenfd = socket(AF_INET,SOCK_STREAM,0);
            if(m_listenfd == -1){
                perror("SOCKET FAILED!");
                exit(EXIT_FAILURE);
            }
        }
        ~ctcpserv(){closelisten();closeconnect();}
        
        bool closelisten(){
            if(m_listenfd == -1)return false; //close要分开成两个close函数才行
            ::close(m_listenfd);
            m_listenfd = -1;
            return true;
        }

        bool closeconnect(){
            if(m_connectfd == -1)return false;
            ::close(m_connectfd);
            m_connectfd = -1;
            return true;
        }

        bool bind(unsigned short in_port);
        bool listen(int quenum);
        bool accept();
        bool recv(string &buffer,const size_t maxlen);
        bool send(const string &buffer);

        const string & clientip() const{ //用于外部访问私有成员
            return m_clientip;
        }

};

bool ctcpserv::bind(unsigned short in_port){
    m_port = in_port;

    struct sockaddr_in servaddr;
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //服务端任何网卡的IP都可以用于通讯
    servaddr.sin_port = htons(in_port);
    if(::bind(m_listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) != 0){
      perror("bind failed!");
      return false;
    }
    cout << "BIND SUCCESS!" << endl;
    return true;
}

bool ctcpserv::listen(int quenum){
    if(::listen(m_listenfd,quenum)!=0){
        perror("LISTEN FAILED!");
        m_listenfd = -1;
        return false;
    }
    return true;
}

bool ctcpserv::accept(){ //这个函数可以获取客户端的IP地址存在类里面。后两个参数就不能是0了。
    m_connectfd = ::accept(m_listenfd,0,0);
    if(m_connectfd == -1){
        perror("ACCEPT FAILED!");
        return false;
      }
    cout << "CONNETION ESTABLISHED\n" << endl;
    return true;
}

bool ctcpserv::recv(string &buffer,const size_t maxlen){
    if(m_connectfd == -1)return false;
    buffer.clear();
    buffer.resize(maxlen);
    //cout << "DEBUG" << endl;
    int readn = ::recv(m_connectfd,&buffer[0],buffer.size(),0);
    if(readn <= 0){
        buffer.clear();
        if(readn < 0)perror("RECV FAILED!");
        return false;
    }
    buffer.resize(readn);
    return true;
}

bool ctcpserv::send(const string &buffer){
    if(m_connectfd == -1)return false;
    if(::send(m_connectfd,buffer.data(),buffer.size(),0)<=0){
        return false;
      }

    return true;
}

ctcpserv tcpserv;

void FathEXIT(int sig);
void ChldEXIT(int sig);

void FathEXIT(int sig){
    signal(SIGINT,SIG_IGN); //防止信号处理函数在执行过程中又被打断
    signal(SIGTERM,SIG_IGN);

    cout << "Father EXIT, sig = " << sig << endl;
    kill(0,SIGTERM); //向所有子进程发送SIG = 15 SIGTERM信号,优雅退出

    tcpserv.closelisten(); //释放资源

    exit(0);
}

void ChldEXIT(int sig){
    signal(SIGINT,SIG_IGN); //防止信号处理函数在执行过程中又被打断
    signal(SIGTERM,SIG_IGN);

    cout << "Son EXIT, sig = " << sig << endl;

    tcpserv.closeconnect();

    exit(0);
}

int main(int argc,char *argv[]){
  if(argc != 2){
    cout << "arguments not match !\n" << endl;
    return -1;
  }

  for(int ii=1;ii<=64;ii++)signal(ii,SIG_IGN);
  signal(SIGTERM,FathEXIT); //设置父进程可以触发SIGINT和SIGTERM
  signal(SIGINT,FathEXIT);
  //signal(SIGCHLD, SIG_DFL);  // 恢复默认行为（不忽略）
  //如果不忽略SIGCHLD,会产生僵尸进程,此时需要父进程显式waitpid

  if(tcpserv.bind(atoi(argv[1])) == false)return -1;

  if(tcpserv.listen(5) == false)return -1;
  while(true){
    if(tcpserv.accept() == false)return -1;
    int pid = fork();
    if(pid == -1){ //系统资源不足
        perror("FORK FAILED!");
        return -1;
    }
    if(pid >0){
        cout << "Im father,New Sub pid is " << pid << endl;
        tcpserv.closeconnect(); //父进程不需要连接
        continue;
    }

    cout << "Im son" << endl;
    //子进程需要重新设置信号
    signal(SIGTERM,ChldEXIT);
    signal(SIGINT,SIG_IGN);
    tcpserv.closelisten(); //子进程不需要监听

    string buffer;
    while(true){
        if(tcpserv.recv(buffer,MAXLEN) == false){
            break;
        }
        cout << "recv :" << buffer << endl;

        buffer = "ACK";
        if(tcpserv.send(buffer) == 0){
            perror("send failed");
            return -1;
            }
        cout << "send :" << buffer << endl;
        }

    return 0;
    }
  //return 0;
}