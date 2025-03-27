#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <fmt/format.h>
#include <string.h>

struct socket_address_fatptr{
  struct sockaddr *m_addr;
  socklen_t m_addrlen;
};

struct address_resolved_entry{
  struct addrinfo *m_curr = nullptr;

  socket_address_fatptr get_address()const{
    return {m_curr->ai_addr,m_curr->ai_addrlen};
  }

  int create_socket() const{
    int sockfd = socket(m_curr->ai_family,m_curr->ai_socktype,m_curr->ai_protocol);
    if(sockfd == -1){
      fmt::println("socket:{}",strerror(errno));
      throw;
    }
    return sockfd;
  }


};

struct address_resolver
{
  /* data */
  struct addrinfo *m_head = nullptr;

  void resolve(std::string const &name,std::string const &service){
    int err = getaddrinfo(name.c_str(),service.c_str(),NULL,&m_head);
    if(err != 0){
      fmt::println("getaddrinfo:{} {}",gai_strerror(err),err);
      throw;
    }
  }
  address_resolved_entry get_first_entry(){
    return {m_head};
  }

  address_resolver() = default;

  address_resolver(address_resolver &&that):m_head(that.m_head){
    that.m_head = nullptr;
  }

  ~address_resolver(){
    if(m_head){
      freeaddrinfo(m_head);
    }
  }
};


int main(){
    setlocale(LC_ALL,"zh_CN.UTF-8");
    address_resolver resolver;
    auto entry = resolver.resolve("127.0.0.0","80");
    int sockfd = entry


}