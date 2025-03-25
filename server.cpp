#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <fmt/format.h>
#include <string.h>

int main(){
    setlocale(LC_ALL,"zh_CN.UTF-8");
    struct addrinfo *addrinfo;
    int res = getaddrinfo("localhost","80",NULL,&addrinfo);
    if(res != 0){
        perror("getaddrinfo");
        return -1;
    }
    
    int sockfd = socket(-1,addrinfo->ai_socktype,addrinfo->ai_protocol);
    if(sockfd == -1){
        fmt::println("errno: {}",strerror(errno));
        return -1;
    }


}