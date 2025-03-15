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
