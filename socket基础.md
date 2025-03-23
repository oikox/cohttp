# socket基础

## getaddrinfo（）

`getaddrinfo()` 是 Linux 网络编程中用于**解析主机名和服务名**的核心函数，它能自动处理 IPv4/IPv6、TCP/UDP 等协议细节，返回适用于套接字操作的地址信息链表。以下是其详细用法和示例：

```c++
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getaddrinfo(
    const char *node,        // 主机名或 IP 地址（如 "www.example.com" 或 "127.0.0.1"）
    const char *service,     // 服务名或端口号（如 "http" 或 "80"）
    const struct addrinfo *hints,  // 输入参数，指定过滤条件
    struct addrinfo **res    // 输出参数，返回的地址链表
);

void freeaddrinfo(struct addrinfo *res); // 释放链表内存

```

### **参数详解**

1. **`node`**

- 要解析的主机名或 IP 地址。
- 若为 `NULL`，表示解析本地地址（需配合 `AI_PASSIVE` 标志）。

2. **`service`**

- 要解析的服务名（如 `"http"`）或端口号（如 `"8080"`）。
- 若为 `NULL`，表示不解析服务名。

3. **`hints`**

- 输入参数，通过

   

  ```
  struct addrinfo
  ```

   

  指定过滤条件：

  - `ai_family`: 地址族（如 `AF_INET`、`AF_INET6`、`AF_UNSPEC`）。
  - `ai_socktype`: 套接字类型（如 `SOCK_STREAM`、`SOCK_DGRAM`）。
  - `ai_protocol`: 协议（如 `IPPROTO_TCP`、`IPPROTO_UDP`）。
  - `ai_flags`: 控制行为的标志（见下表）。

**常用 `ai_flags` 标志**：

|       标志       |                      说明                       |
| :--------------: | :---------------------------------------------: |
|   `AI_PASSIVE`   | 用于服务器绑定，自动填充本地 IP（`INADDR_ANY`） |
|  `AI_CANONNAME`  |    返回主机的规范名称（`ai_canonname` 字段）    |
| `AI_NUMERICHOST` |   禁止 DNS 查询，`node` 必须是 IP 地址字符串    |
| `AI_NUMERICSERV` |  禁止服务名解析，`service` 必须是端口号字符串   |

4. **`res`**

- 输出参数，返回一个 `struct addrinfo` 链表，包含所有匹配的地址信息。



## addrinfo结构体

`struct addrinfo` 是 Linux 网络编程中用于表示地址信息的核心结构体，通常与 `getaddrinfo()` 和 `freeaddrinfo()` 函数配合使用。它用于存储主机名、服务名解析后的套接字地址信息，支持多种协议（如 IPv4/IPv6）和套接字类型（如 TCP/UDP）。

```c++
#include <netdb.h>

struct addrinfo {
    int              ai_flags;       // 控制地址解析行为的标志
    int              ai_family;      // 地址族（如 AF_INET、AF_INET6）
    int              ai_socktype;    // 套接字类型（如 SOCK_STREAM、SOCK_DGRAM）
    int              ai_protocol;    // 协议（如 IPPROTO_TCP、IPPROTO_UDP）
    socklen_t        ai_addrlen;     // ai_addr 指向的地址结构长度（字节）
    struct sockaddr *ai_addr;        // 指向套接字地址结构的指针
    char            *ai_canonname;   // 主机的规范名称（Canonical Name）
    struct addrinfo *ai_next;        // 指向下一个 addrinfo 的指针（链表结构）
};

```





`getaddrinfo` 的第四个参数是一个 **指向指针的指针**（双重指针）

```c++
int getaddrinfo(const char *node, const char *service,
               const struct addrinfo *hints,
               struct addrinfo **res); // res 是 addrinfo 指针的指针

```



- 函数内部会 **动态分配内存** 并填充 `struct addrinfo` 链表，然后通过 `res` 参数返回链表头指针。
- 为了修改外部指针变量 `res` 的值（使其指向新分配的内存），需要传递 `res` 的地址（即 `&res`）。

假设 `res` 是一个指针变量：

- `res` 本身存储的是一个内存地址（指向 `struct addrinfo`）。
- `&res` 是 `res` 变量的地址（类型为 `struct addrinfo**`）。

通过传递 `&res`，函数可以修改外部的 `res` 指针，使其指向新分配的链表头节点。

### 参数为二级指针的原因

在 C 语言中，`getaddrinfo` 函数的 `res` 参数设计为二级指针（`struct addrinfo **res`）的原因如下：

**核心原因**

1. 修改调用者的指针变量
   - C 语言中函数参数是值传递的，若要在函数内修改**指针变量本身的值**（使其指向新分配的内存），必须传递指针的地址（即二级指针）。
   - 如果 `res` 是一级指针（`struct addrinfo *res`），函数内部只能修改指针指向的内容，但无法让外部的指针变量指向新的内存。





## hostent结构体

`struct hostent` 是 Linux 网络编程中用于表示 **主机信息** 的结构体，通常与 `gethostbyname()` 或 `gethostbyaddr()` 等函数配合使用，用于通过主机名或 IP 地址查询主机的详细信息。以下是它的详细说明和使用方法：

```c++
#include <netdb.h>

struct hostent {
    char  *h_name;       // 主机的正式名称（Official name）
    char **h_aliases;    // 主机的别名列表（以 NULL 结尾）
    int    h_addrtype;   // 地址类型（如 AF_INET 或 AF_INET6）
    int    h_length;     // 地址长度（IPv4 为 4 字节，IPv6 为 16 字节）
    char **h_addr_list;  // 主机的 IP 地址列表（以 NULL 结尾）
};
```

**字段说明**

1. **`h_name`**
   - 主机的正式名称（例如 `www.example.com`）。
   - 如果通过别名查询，此处返回规范名称。
2. **`h_aliases`**
   - 主机的别名列表，以 `NULL` 结尾的字符串数组。
   - 例如，`mail.example.com` 可能是 `www.example.com` 的别名。
3. **`h_addrtype`**
   - 地址类型，表示返回的 IP 地址是 IPv4 还是 IPv6：
     - `AF_INET`：IPv4 地址。
     - `AF_INET6`：IPv6 地址。
4. **`h_length`**
   - 每个 IP 地址的字节长度：
     - IPv4 地址：4 字节。
     - IPv6 地址：16 字节。
5. **`h_addr_list`**
   - 主机的 IP 地址列表，以 `NULL` 结尾的指针数组。
   - 每个元素指向一个二进制格式的 IP 地址（如 `struct in_addr` 或 `struct in6_addr`）。
   - 通过 `inet_ntop()` 函数可将其转换为可读的字符串格式。

**已过时的函数**
`gethostbyname()` 和 `gethostbyaddr()` 是旧版函数，**不支持 IPv6** 且**非线程安全**。现代代码应优先使用 `getaddrinfo()`：





如何将hostent中的addr转化为sockaddr_in中的addr?

```c++
    // 将 h_addr_list[0] 转换为 struct in_addr*
    if (he->h_addr_list[0] != NULL) {
        // 方法 1: 直接指针转换和解引用
        addr.sin_addr = *(struct in_addr *)he->h_addr_list[0];

        // 方法 2: 使用 memcpy（更安全）
        memcpy(&addr.sin_addr.s_addr, he->h_addr_list[0], he->h_length);
    }
```



## sockaddr_in结构体

`sockaddr_in` 是 Linux 网络编程中用于表示 **IPv4 套接字地址** 的结构体，通常用于 TCP/UDP 通信中的地址绑定（`bind()`）、连接（`connect()`）等操作。以下是其详细说明和使用方法：

```c++
#include <netinet/in.h>

struct sockaddr_in {
    sa_family_t    sin_family;   // 地址族（Address Family），固定为 AF_INET（IPv4）
    in_port_t      sin_port;     // 端口号（16位，需用 htons() 转换字节序）
    struct in_addr sin_addr;     // IPv4 地址（32位，需用 inet_pton() 转换格式）
    char           sin_zero[8];  // 填充字段（未使用，通常置零）
};
```





## in_addr结构体

`struct in_addr` 是用于表示 **IPv4 地址** 的结构体，在网络编程中常用于存储和操作 32 位的 IPv4 地址。以下是其详细说明及使用方法：

```c++
#include <netinet/in.h>

struct in_addr {
    in_addr_t s_addr;  // 32 位的 IPv4 地址（网络字节序）
};

```



# 文件操作基础



Linux进程默认打开了三个文件描述符:0-标准输入(键盘)，1-标准输出（显示器)，2-标准错误（显示器)。cin cout cerr。

在/proc/<pid>/fd目录中，存放了每个进程打开的fd。

分配文件描述符的规则是，分配最小的未被分配的数字。

socket和fd是一样的，例如send可以改成write，recv可以改成read



# 封装socket服务器和客户端类

一般来讲，形参都用string&类型，因为string既支持string又支持c_str

如果想要只读效果，加一个const即可。例如const string& buffer



一般来讲，不会直接操作string的内存空间。要操作时，一定要注意

1.不要越界，

2.操作后手动设置数据的大小。



获取string内存对象地址的方法有三种

```c++
string buffer;

&buffer[0];

buffer.c_str();
buffer.data();
//但c_str()和.data()获取到的是const
```



在 C++ 中，**类内声明函数时可以不写形参名称，但必须指定形参类型**；类外定义时需要保持参数列表的 **类型和顺序完全一致**，但可以自由选择形参名称。



子进程只负责连接不需要监听，所以可以关闭listensocket

父进程只负责监听不需要连接，所以可以关闭connectsocket

# 信号处理

信号处理函数中，只能访问全局对象。

## 僵尸进程自动消失的原因

1. **隐式 `SIGCHLD` 处理**

您的代码中有以下关键设置：

```
CPP
for(int ii=1;ii<=64;ii++) signal(ii,SIG_IGN);  // 忽略所有信号
```

尽管后续重新注册了 `SIGTERM` 和 `SIGINT` 的处理函数，但 **`SIGCHLD` 仍被忽略（`SIG_IGN`）**。
当父进程忽略 `SIGCHLD` 时，内核会自动回收子进程的退出状态，无需显式调用 `wait()`。

2. **验证方法**

修改代码以观察 `SIGCHLD` 的默认行为：

```
CPP// 注释掉忽略所有信号的代码
// for(int ii=1;ii<=64;ii++) signal(ii,SIG_IGN); 

// 显式设置 SIGCHLD 的处理方式
signal(SIGCHLD, SIG_DFL);  // 恢复默认行为（不忽略）
```

此时若子进程退出，父进程未调用 `wait()`，僵尸进程将保留。



1. **父进程忽略 `SIGCHLD`**

通过 `signal(ii, SIG_IGN)` 忽略所有信号（包括 `SIGCHLD`），导致：

- 子进程退出时，内核自动回收其资源。
- 父进程无需调用 `wait()`，子进程不会成为僵尸进程。

2. **`SIGCHLD` 的特殊行为**

当 `SIGCHLD` 的处理方式为 `SIG_IGN` 时：

- 子进程终止后立即被内核回收，**不生成僵尸进程**。
- 此行为是 POSIX 标准的一部分（详见 `man 2 signal`）。



显式处理SIGCHLD：

```c++
#include <signal.h>
#include <sys/wait.h>

void sigchld_handler(int sig) {
    // 非阻塞回收所有终止的子进程
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    signal(SIGCHLD, sigchld_handler); // 注册信号处理器

    pid_t pid = fork();
    if (pid == 0) {
        // 子进程执行任务后退出
        exit(0);
    }

    // 父进程继续执行
    while (1) pause(); // 等待信号
    return 0;
}

```



## waitpid（）

`waitpid` 函数是 Unix/Linux 系统中用于 **等待子进程状态变化** 的关键函数，允许父进程以灵活的方式管理子进程的退出。以下是 `waitpid(-1, NULL, WNOHANG)` 的详细解析

```c++
#include <sys/wait.h>

pid_t waitpid(pid_t pid, int *status, int options);

```

**参数解析**

| **参数**  |                           **说明**                           |
| :-------: | :----------------------------------------------------------: |
|   `pid`   | 指定要等待的子进程。取值：<br>- `-1`：等待 **任意子进程**。<br>- `>0`：等待进程ID为 `pid` 的子进程。<br>- `0`：等待与父进程同进程组的任意子进程。<br>- `<-1`：等待进程组ID为 pid绝对值的任何子进程 |
| `status`  | 存储子进程退出状态的指针。若为 `NULL`，表示 **不获取状态信息**（仅回收资源）。 |
| `options` | 控制函数行为的标志位。常用选项：<br>- `WNOHANG`：非阻塞模式，无终止子进程时立即返回 `0`。<br>- `WUNTRACED`：报告已停止的子进程状态。 |

**返回值**

| **返回值** |                          **说明**                           |
| :--------: | :---------------------------------------------------------: |
|    `>0`    |           返回 **终止的子进程PID**，资源已回收。            |
|    `0`     |      在非阻塞模式（`WNOHANG`）下，**没有子进程终止**。      |
|    `-1`    | 出错（如无子进程、信号中断等），通过 `errno` 获取错误原因。 |

1. **非阻塞模式**：
   使用 `WNOHANG` 时，函数不会阻塞父进程，适用于需要 **持续运行的主循环**（如网络服务器）。



使用场景：

1.服务器程序

```c++
void clean_zombies() {
    while (true) {
        pid_t pid = waitpid(-1, NULL, WNOHANG);
        if (pid <= 0) break; // 无更多子进程终止
        printf("回收子进程 PID=%d\n", pid);
    }
}

int main() {
    while (true) {
        // 处理客户端请求
        handle_requests();
        // 非阻塞清理僵尸进程
        clean_zombies();
    }
}

```



2.获取子进程退出状态

```c++
int status;
pid_t pid = waitpid(-1, &status, 0);
if (WIFEXITED(status)) {
    printf("子进程正常退出，状态码=%d\n", WEXITSTATUS(status));
} else if (WIFSIGNALED(status)) {
    printf("子进程被信号终止，信号=%d\n", WTERMSIG(status));
}

```





### waitpid参数的设置

**3. 关键对比**

|  **参数**  |      **目标范围**      |           **风险**           |       **适用场景**       |
| :--------: | :--------------------: | :--------------------------: | :----------------------: |
| `pid = -1` |  当前进程的所有子进程  |              无              | 通用场景，无需关心进程组 |
| `pid = 0`  |  同进程组的所有子进程  | 可能误收同组其他进程的子进程 |   明确控制进程组的场景   |
| `pid > 0`  |   指定 PID 的子进程    |              无              |    精确等待特定子进程    |
| `pid < -1` | 指定进程组的所有子进程 |       需确保进程组存在       |     多进程组管理场景     |

**Q1：`pid = -1` 会误杀其他进程吗？**

- **不会**。`waitpid` 仅用于回收 **已终止的子进程**，不发送任何信号或终止进程。

**Q2：`pid = 0` 会导致问题吗？**

- **可能**。若进程组内有其他无关子进程（如 Shell 作业控制的子进程），`waitpid(0, ...)` 可能意外回收它们。
- **建议**：仅在父子进程明确属于同一进程组时使用。

**Q3：如何安全回收所有子进程？**

- 使用 `while (waitpid(-1, NULL, WNOHANG) > 0);` 循环，非阻塞回收所有终止的子进程。

最佳实践：

1. **通用场景**：优先使用 `waitpid(-1, ...)`，避免进程组干扰。
2. **多进程组场景**：显式设置进程组（`setpgid`），再使用 `waitpid(0, ...)` 或 `waitpid(-1, ...)`。
3. **精准控制**：为每个子进程记录 PID，使用 `waitpid(pid, ...)` 单独回收。



# 三次握手与四次挥手

1)==主动断开的端==在四次挥手后，sOcket的状态为TIME_WAIT，该状态将持续2MSL(30秒/1分钟/2分钟)。MSL(MaximumSegment Lifetime)报文在网络上存在的最长时间，超过这个时间报文将被丢弃。

2)如果是客户端主动断开，TIME_WAIT状态的socket几乎不会造成危害。a)客户端程序的socket很少，服务端程序很多(成千上万）；b）客户端的端口是随机分配的，不存在重用的问题。
3）如果是服务端主动断开，有两方面的危害：a)socket没有立即释放;b）端口号只能在2MSL后才能重用



在服务端程序中,用setsock()函数设置SOCKET属性（一定要放在bind()之前）

```c++
int opt = 1;
setsockopt(m_listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
```



1. `SOL_SOCKET` 表示操作层级在套接字基础层

2. `SO_REUSEADDR` 是地址重用选项

3. `opt=1` 表示启用该选项

4. 作用：允许服务端程序在非正常关闭（如崩溃）后快速重启，避免出现"Address already in use"错误。它让内核立即回收处于TIME_WAIT状态的套接字地址。

   注：在Linux系统上，实际使用中更推荐同时设置 `SO_REUSEPORT`（端口重用）以获得更好的效果，但需要注意不同系统的兼容性。



# Nagle算法

在TCP协议中，无论发送多少数据，都要在数据前面加上协议头，同时，对方收到数据后，也需要回复ACK表示确认。为了尽可能的利用网络带宽，TCP希望每次都能够以MSS(MaximumSegmentSize，最大报文长度】的数据块来发送数据。



Nagle算法就是为了尽可能发送大块的数据，避免网络中充斥着小数据块。
Nagle算法的定义是：任意时刻，最多只能有一个未被确认的小段，小段是指小于MSS的数据块，未被确认是指一个数据块发送出去后，没有收到对端回复的ACK。



举个例子：发送端调用send0函数将一个int型数据（称之为A数据块）写入到socket中，A数据块会被马上发送到接收端，接着，发送端又调用send0函数写入一个int型数据（称之为B数据块，这时候，A块的ACK没有返回（已经存在了一个未被确认的小段)，所以B块不会立即被发送，而是等A块的ACK返回之后（大约40ms）才发送。



TCP协议中不仅仅有Nagle算法，还有一个ACK延迟机制：当接收端收到数据之后，并不会马上向发送端回复ACK，而是延退40ms后再回复，它希望在40ms内接收端会向发送端回复应答数据，这样ACK就可以和应答数据一起发送，把ACK捎带过去。◇

如果TCP连接的一端启用了Nagle算法，另一端启用了ACK延时机制，而发送的数据包又比较小，则可能会出现这样的情况：发送端在等待上一个包的ACK，而接收端正好延退了此ACK，那么这个正要被发送的包就会延退40ms。



解决方案：
开启TCP_NODELAY选项，这个选项的作用就是禁用Nagle算法。
#include<netinet/tcp.h>
//注意，要包含这个头文件。

```c++
int opt=1;
setsockopt(sockfd,IPPROTO_TCP,TCPNODELAY,&opt,sizeof(opt);
```

对时效要求很高的系统中，例如联机游戏。一般会禁用Nagle算法。



# I/O多路复用

代替多进程/多线程



==网络通讯的读事件==：

1. 已连接的队列中有已经准备好的socket（有新的客户端连上来
2. 接收缓存中有数据可以读（对端发送的报文已经到达
3. tcp连接已断开（对端调用close()函数关闭了连接



==网络通讯的写事件==：

发送缓冲区没有满，可以写入数据（可以向对端发送报文



## select模型

```c++
int select(int nfds, fd_set *readfds, fd_set *writefds,
          fd_set *expectfds, struct timeval *timeout);

//nfds是fd的数量

//fd_set实际上是int[32]
//C语言提供4个宏操作位图
void FD_CLR(int fd, fd_set *set);
int FD_ISSET(int fd, fd_set *set);
void FD_SET(int fd, fd_set *set);
void FD_ZERO(fd_set *set);
```





### 写事件

如果tcp的发送缓冲区没满,那么socket连接是可写的.

一般来说,不会被填满.

如果发送的数据量太大,或者网络带宽不够,有可能被填满.

### 水平触发

select是==水平触发==的：

1. select监视的socket如果发生了事件，select会返回（通知应用程序处理事件

（监视写事件会一直返回，一般不关心写事件）

1. 如果事件没有被处理，再次调用select的时候会立即再通知，例如上一次recv没有读取完

### select的缺点

1. 采用轮询方式扫描bitmap,性能随着socket数量增多而下降
2. 每次调用select(),需要拷贝bitmap,因为select会修改bitmap,因此要拷贝副本
3. select()运行在用户态,网络通讯在内核态,bitmap还要拷贝到内核态,开销较大
4. bitmap的大小决定了单个进程/线程处理的SOCKET数量,默认1024宏,可以修改,但是改大了效率更低（轮询



## POLL模型

```c++
int poll(struct pollfd *fds, nfds_t nfds, int timeout);
//nfds是fd的数量
```



```c++
struct pollfd{
    int fd;
    short events;
    short revents;
};
```

第一个成员是需要监视的socket

第二个成员是需要监视的事件

第三个成员是返回的事件。

poll只会修改第三个成员。

传入参数是结构体的指针。



### revents成员



`poll()` 是一个系统调用，用于监视多个文件描述符（文件、套接字等）的状态变化（如可读、可写、异常等）。
它的参数是一个 `struct pollfd` 数组，结构体定义如下：

```c++
struct pollfd {
    int   fd;      // 要监视的文件描述符
    short events;  // 关心的事件（由调用者设置）
    short revents; // 实际发生的事件（由内核填充）
};

```

- **`events`**：由调用者设置，告诉内核“我关心哪些事件”（例如 `POLLIN` 表示关心“是否有数据可读”）。
- **`revents`**：由内核填充，表示“实际发生了哪些事件”。

代码中的 `(fds[eventfd].revents & POLLIN)` 是一个按位与操作，目的是检查 `revents` 中是否包含 `POLLIN` 标志：

- **如果结果 `!= 0`**：表示 `revents` 中存在 `POLLIN` 标志（有数据可读）。
- **如果结果 `== 0`**：表示 `revents` 中没有 `POLLIN` 标志（没有数据可读）。



- **7. 常见误区**

- **为什么不用 `== POLLIN`？**
  因为 `revents` 可能同时发生多个事件（例如 `POLLIN | POLLERR`），按位与操作可以精准提取目标事件。
- **`events` 和 `revents` 的区别**：
  `events` 是“我关心的事件”，由调用者设置；`revents` 是“实际发生的事件”，由内核填充。



### poll的缺点

1. 在程序中,poll的数据结构是结构体数组,传入内核后转换为数组
2. 每调用一次select()需要拷贝两次bitmap,poll拷贝一次结构体数组
3. poll监视的连接数没有1024的限制,但是也是遍历的方法,监视的socket越多,效率越低



## EPOLL模型

```c++
       int epoll_create(int size);
       int epoll_create1(int flags);
```

size参数必须大于0,是什么无所谓。

返回的是一个文件描述符

## epoll_create

```c++
       int epoll_create(int size);
       int epoll_create1(int flags);
```

size参数必须大于0,是什么无所谓。

返回的是一个文件描述符

## struct epoll_event

```c++
       struct epoll_event {
           uint32_t      events;  /* Epoll events */
           epoll_data_t  data;    /* User data variable */
       };

       union epoll_data {
           void     *ptr;
           int       fd;
           uint32_t  u32;
           uint64_t  u64;
       };

       typedef union epoll_data  epoll_data_t;
```

EPOLLIN是读事件,EPOLLOUT是写事件

## epoll_wait

```c++
       int epoll_wait(int epfd, struct epoll_event *events,
                      int maxevents, int timeout);
       int epoll_pwait(int epfd, struct epoll_event *events,
                      int maxevents, int timeout,
                      const sigset_t *_Nullable sigmask);
       int epoll_pwait2(int epfd, struct epoll_event *events,
                      int maxevents, const struct timespec *_Nullable timeout,
                      const sigset_t *_Nullable sigmask);
```

epoll_wait()第一个参数是epoll句柄,第二个是返回数组,第三个是数组大小,第四个是超时时间,-1代表不设置。

## 阻塞与非阻塞I/O

会阻塞的四个函数：

​	connect(),accept(),send(),recv()

在IO复用的模型中,==事件循环不能被阻塞在任何环节==,应该采用非阻塞的IO。

一般将while(true)循环称为事件循环,事件循环不能阻塞在任何环节 

之前的简单epoll模型中,最后一步

```c++
          //有报文
          cout << "recv from client: " << buffer << endl;
          string sendback = "recv from serv: " + string(buffer);
          send(evs[ii].data.fd,sendback.c_str(),sendback.size(),0);
```

send()函数,如果发送缓冲区满了,会阻塞,事件循环就无法监视其他socket,并发消失



# 一个关于recv和string的小问题

```c++
    string buffer;
    if(recv(eventfd,buffer.data(),buffer.size(),0) <= 0){
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
```

1. **缓冲区问题**：

   - 在这段代码中，`buffer.data()` 返回的是 `std::string` 的内部缓冲区指针，而 `buffer.size()` 返回的是当前字符串的大小。如果 `buffer` 是空的（即 `buffer.size()` 为 0），那么 `recv` 将无法接收任何数据，因此可能会返回 0 或 -1。

   - `buffer` 是一个空的 `std::string`，因此 `buffer.size()` 返回 0。
   - `recv(eventfd, buffer.data(), buffer.size(), 0)` 尝试从套接字 `eventfd` 接收数据，但由于 `buffer.size()` 为 0，`recv` 无法接收任何数据，因此返回 0。
   - 因此，`recv` 的返回值小于或等于 0，导致代码认为连接已断开，并关闭了套接字。
