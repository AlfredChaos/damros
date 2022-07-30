#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <signal.h>
#define BUFFSIZE 2048
#define DEFAULT_PORT 38804    // 指定端口为38804
#define MAXLINK 2048

int sockfd, connfd;    // 定义服务端套接字和客户端套接字
enum METHOD
{
    GET = 0,
    POST,
    DELETE,
    PUT
};

void stopServerRunning(int p)
{
    close(sockfd);
    printf("Close Server\n");
    exit(0);
}

// GET / HTTP/1.1
// Host: 192.168.88.108:16555
// User-Agent: curl/7.61.1
// Accept: */*
int parse_request_line(char buff, METHOD *m_method)
{
    char *m_url = strpbrk(buff, " \t");
    if (!m_url)
    {
        return -1;
    }
    *m_url++ = '\0';
    char *method = buff;
    if (strcasecmp(method, "GET") == 0)
    {
        m_method = GET;
    } else if (strcasecmp(method, "POST") == 0)
    {
        m_method = POST;
    } else if (strcasecmp(method, "DELETE") == 0)
    {
        m_method = DELETE;
    } else if (strcasecmp(method, "PUT") == 0)
    {
        m_method = PUT;
    } else {
        return -1;
    }
    return 0;
}

int main()
{
    METHOD *m_method;
    struct sockaddr_in servaddr;    // 用于存放ip和端口的结构
    char buff[BUFFSIZE];    // 用于收发数据
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd)
    {
        printf("Create socket error(%d): %s\n", errno, strerror(errno));
        return -1;
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(DEFAULT_PORT);
    if (-1 == bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)))
    {
        printf("Bind error(%d): %s\n", errno, strerror(errno));
        return -1;
    }
    if (-1 == listen(sockfd, MAXLINK))
    {
        printf("Listen error(%d): %s\n", errno, strerror(errno));
        return -1;
    }

    printf("Listening...\n");
    while (true)
    {
        signal(SIGINT, stopServerRunning);    // 这句用于在输入Ctrl+C的时候关闭服务器
        connfd = accept(sockfd, NULL, NULL);
        if (-1 == connfd)
        {
            printf("Accept error(%d): %s\n", errno, strerror(errno));
            return -1;
        }
        bzero(buff, BUFFSIZE);
        recv(connfd, buff, BUFFSIZE - 1, 0);
        printf("Recv: %s\n", buff);
        if (parse_request_line(buff, m_method) == -1)
        {
            printf("parse request line error.");
        }
        printf("m_method = %s\n", m_method);
        send(connfd, buff, strlen(buff), 0);
        close(connfd);
    }
    return 0;
}