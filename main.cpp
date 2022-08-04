#include <cstdlib>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <time.h>
#define BUFFSIZE 2048
#define MAXLINK 2048
using namespace std;

int sockfd, connfd;    // 定义服务端套接字和客户端套接字
enum METHOD
{
    NONE=0,
    GET,
    POST,
    DELETE,
    PUT
};

void stop_server_running(int p)
{
    close(sockfd);
    printf("Close Server\n");
    exit(0);
}

int random_port()
{
    srand((unsigned)time(NULL));
    return rand() % 65536;
}

// GET / HTTP/1.1
// Host: 192.168.88.108:16555
// User-Agent: curl/7.61.1
// Accept: */*
int parse_request_line(char *text, METHOD *m_method, char *response)
{
    char *m_url = strpbrk(text, " \t");
    if (!m_url)
    {
        return -1;
    }
    *m_url++ = '\0';
    // get request method
    if (strcasecmp(text, "GET") == 0)
    {
        *m_method = GET;
    } else if (strcasecmp(text, "POST") == 0)
    {
        *m_method = POST;
    } else if (strcasecmp(text, "DELETE") == 0)
    {
        *m_method = DELETE;
    } else if (strcasecmp(text, "PUT") == 0)
    {
        *m_method = PUT;
    } else {
        return -1;
    }
    // get http version
    if (strcasecmp(m_url, "http") != 0) {
        cout << "get error" << endl;
        return -1;
    }
    char *m_vershon = strpbrk(m_url, " \t");
    *m_vershon++ = '\0';
    cout << m_vershon << endl;
    return 0;
}

int main()
{
    METHOD m_method;
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
    int port_number = random_port();
    servaddr.sin_port = htons(port_number);
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

    printf("Listening on 127.0.0.1:%d...\n", port_number);
    while (true)
    {
        signal(SIGINT, stop_server_running);    // 这句用于在输入Ctrl+C的时候关闭服务器
        connfd = accept(sockfd, NULL, NULL);
        if (-1 == connfd)
        {
            printf("Accept error(%d): %s\n", errno, strerror(errno));
            return -1;
        }
        bzero(buff, BUFFSIZE);
        recv(connfd, buff, BUFFSIZE - 1, 0);
        printf("%s\n", buff);
        char text[BUFFSIZE]; // 拷贝数组
        char response[BUFFSIZE];
        copy(begin(buff), end(buff), begin(text));
        if (parse_request_line(text, &m_method, response) == -1) {
            printf("parsing request line error.");
            continue;
        }
        send(connfd, response, strlen(response), 0);
        close(connfd);
    }
    return 0;
}