/* 
/*
?* ��������tcpselect.cpp���˳���������ʾ����selectģ��ʵ������ͨѶ�ķ���ˡ�
?* ���ߣ������
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>

// ��ʼ������˵ļ����˿ڡ�
int initserver(int port);

int main(int argc,char *argv[])
{
    if (argc != 2) { printf("usage: ./tcpselect port\n"); return -1; }

    // ��ʼ����������ڼ�����socket��
    int listensock = initserver(atoi(argv[1]));
    printf("listensock=%d\n",listensock);

    if (listensock < 0) { printf("initserver() failed.\n"); return -1; }

    // ���¼���1�������Ӷ��������Ѿ�׼���õ�socket�����µĿͻ����������ˣ���
    //             ? 2�����ջ����������ݿ��Զ����Զ˷��͵ı����ѵ����
    //             ? 3��tcp�����ѶϿ����Զ˵���close()�����ر������ӣ���
    // д�¼������ͻ�����û����������д�����ݣ�������Զ˷��ͱ��ģ���

    fd_set readfds;                         // ��Ҫ���Ӷ��¼���socket�ļ��ϣ���СΪ16�ֽڣ�1024λ����bitmap��  
    FD_ZERO(&readfds);// ��ʼ��readfds����bitmap��ÿһλ����Ϊ0��
    FD_SET(listensock,&readfds);// �ѷ�������ڼ�����socket����readfds��

    int maxfd=listensock;// readfds��socket�����ֵ��

    while (true)// �¼�ѭ����
    {
        // ���ڱ�ʾ��ʱʱ��Ľṹ�塣
        struct timeval timeout;     
        timeout.tv_sec=10;// ��
        timeout.tv_usec=0;// ΢�롣

        fd_set tmpfds=readfds;// ��select()�����У����޸�bitmap�����ԣ�Ҫ��readfds����һ�ݸ�tmpfds���ٰ�tmpfds����select()��

        // ����select() �ȴ��¼��ķ�����������Щsocket�������¼�)��
        int infds=select(maxfd+1,&tmpfds,NULL,NULL,0); 

        // ���infds<0����ʾ����select()ʧ�ܡ�
        if (infds<0)
        {
            perror("select() failed"); break;
        }

        // ���infds==0����ʾselect()��ʱ��
        if (infds==0)
        {
            printf("select() timeout.\n"); continue;
        }

        // ���infds>0����ʾ���¼�������infds������ѷ����¼��ĸ�����
        for (int eventfd=0;eventfd<=maxfd;eventfd++)
        {
            if (FD_ISSET(eventfd,&tmpfds)==0) continue;// ���eventfd��bitmap�еı�־Ϊ0����ʾ��û���¼���continue

            // ��������¼�����listensock����ʾ�����Ӷ��������Ѿ�׼���õ�socket�����µĿͻ����������ˣ���
            if (eventfd==listensock)
            {
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int clientsock = accept(listensock,(struct sockaddr*)&client,&len);
                if (clientsock < 0) { perror("accept() failed"); continue; }

                printf ("accept client(socket=%d) ok.\n",clientsock);

                FD_SET(clientsock,&readfds);// ��bitmap�����������Ŀͻ��˵ı�־λ��Ϊ1��

                if (maxfd<clientsock) maxfd=clientsock;// ����maxfd��ֵ��
            }
            else
            {
                // ����ǿͻ������ӵ�socke���¼�����ʾ���ջ����������ݿ��Զ����Զ˷��͵ı����ѵ���������пͻ����ѶϿ����ӡ�
                char buffer[1024];// ��Ŵӽ��ջ������ж�ȡ�����ݡ�
                memset(buffer,0,sizeof(buffer));
                if (recv(eventfd,buffer,sizeof(buffer),0)<=0)
                {
                    // ����ͻ��˵������ѶϿ���
                    printf("client(eventfd=%d) disconnected.\n",eventfd);

                    close(eventfd);                         // �رտͻ��˵�socket

                    FD_CLR(eventfd,&readfds);     // ��bitmap���ѹرտͻ��˵ı�־λ��ա�
         
                    if (eventfd == maxfd)// ���¼���maxfd��ֵ��ע�⣬ֻ�е�eventfd==maxfdʱ����Ҫ���㡣
                    {
                        for (int ii=maxfd;ii>0;ii--)// �Ӻ�����ǰ�ҡ�
                        {
                            if (FD_ISSET(ii,&readfds))
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

                    // �ѽ��յ��ı�������ԭ�ⲻ���ķ���ȥ��
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