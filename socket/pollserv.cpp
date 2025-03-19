/*
?* ��������tcppoll.cpp���˳���������ʾ����pollģ��ʵ������ͨѶ�ķ���ˡ�
?* ���ߣ������
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>

// ��ʼ������˵ļ����˿ڡ�
int initserver(int port);

int main(int argc,char *argv[])
{
    if (argc != 2) { printf("usage: ./tcppoll port\n"); return -1; }

    // ��ʼ����������ڼ�����socket��
    int listensock = initserver(atoi(argv[1]));
    printf("listensock=%d\n",listensock);

    if (listensock < 0) { printf("initserver() failed.\n"); return -1; }

    pollfd fds[2048];                 // fds�����Ҫ���ӵ�socket��

    // ��ʼ�����飬��ȫ����socket����Ϊ-1����������е�socket��ֵΪ-1����ô��poll����������
    for (int ii=0;ii<2048;ii++)             
        fds[ii].fd=-1;

    // ������poll����listensock���¼���
    fds[listensock].fd=listensock;
    fds[listensock].events=POLLIN;// POLLIN��ʾ���¼���POLLOUT��ʾд�¼���
    // fds[listensock].events=POLLIN|POLLOUT;

    int maxfd=listensock;// fds��������Ҫ���ӵ�socket��ʵ�ʴ�С��

    while (true)// �¼�ѭ����
    {
        // ����poll() �ȴ��¼��ķ�����������Щsocket�������¼�)��
        int infds=poll(fds,maxfd+1,10000);// ��ʱʱ��Ϊ10�롣

        // ���infds<0����ʾ����poll()ʧ�ܡ�
        if (infds < 0)
        {
            perror("poll() failed"); break;
        }

        // ���infds==0����ʾpoll()��ʱ��
        if (infds == 0)
        {
            printf("poll() timeout.\n"); continue;
        }

        // ���infds>0����ʾ���¼�������infds������ѷ����¼��ĸ�����
        for (int eventfd=0;eventfd<=maxfd;eventfd++)
        {
            if (fds[eventfd].fd<0) continue;// ���fdΪ������������

            if ((fds[eventfd].revents&POLLIN)==0) continue;// ���û�ж��¼���continue

            // ��������¼�����listensock����ʾ�����Ӷ��������Ѿ�׼���õ�socket�����µĿͻ����������ˣ���
            if (eventfd==listensock)
            {
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int clientsock = accept(listensock,(struct sockaddr*)&client,&len);
                if (clientsock < 0) { perror("accept() failed"); continue; }

                printf ("accept client(socket=%d) ok.\n",clientsock);

                // �޸�fds������clientsockλ�õ�Ԫ�ء�
                fds[clientsock].fd=clientsock;
                fds[clientsock].events=POLLIN;

                if (maxfd<clientsock) maxfd=clientsock;// ����maxfd��ֵ��
            }
            else
            {
                // ����ǿͻ������ӵ�socke���¼�����ʾ�б��ķ������˻��������ѶϿ���

                char buffer[1024]; // ��Ŵӿͻ��˶�ȡ�����ݡ�
                memset(buffer,0,sizeof(buffer));
                if (recv(eventfd,buffer,sizeof(buffer),0)<=0)
                {
                    // ����ͻ��˵������ѶϿ���
                    printf("client(eventfd=%d) disconnected.\n",eventfd);

                    close(eventfd);// �رտͻ��˵�socket��
                    fds[eventfd].fd=-1;// �޸�fds������clientsockλ�õ�Ԫ�أ���Ϊ-1��poll�����Ը�Ԫ�ء�
         
                    // ���¼���maxfd��ֵ��ע�⣬ֻ�е�eventfd==maxfdʱ����Ҫ���㡣
                    if (eventfd == maxfd)
                    {
                        for (int ii=maxfd;ii>0;ii--)// �Ӻ�����ǰ�ҡ�
                        {
                            if (fds[ii].fd!=-1)
                            {
                                maxfd = ii; break;
                            }
                        }
                    }
                }
                else
                {
                    // ����ͻ����б��ķ�������
                    printf("recv(eventfd=%d):%s\n",eventfd,buffer);

                    send(eventfd,buffer,strlen(buffer),0);
                }
            }
        }
    }

    return 0;
}

// ��ʼ������˵ļ����˿ڡ�
int initserver(int port)
{
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock < 0)
    {
        perror("socket() failed"); return -1;
    }

    int opt = 1; unsigned int len = sizeof(opt);
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,len);

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(sock,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0 )
    {
        perror("bind() failed"); close(sock); return -1;
    }

    if (listen(sock,5) != 0 )
    {
        perror("listen() failed"); close(sock); return -1;
    }

    return sock;
}