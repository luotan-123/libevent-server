#include <cstdio>
#include "CommonHead.h"
#include "Lock.h"

int main()
{
	printf("+++++++++++++++==\n");
	CSignedLock lock;
	CSignedLockObject testLock(&lock, false);

	testLock.Lock();
	testLock.Lock();
	printf("+++++++++++++++==\n");

	testLock.UnLock();
	testLock.UnLock();

	int    socket_fd, connect_fd;
	struct sockaddr_in     servaddr;
	char    buff[4096];
	int     n;
	//初始化Socket  
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
		exit(0);
	}
	//初始化  
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址。  
	servaddr.sin_port = htons(6666);//设置的端口为DEFAULT_PORT  

	//将本地地址绑定到所创建的套接字上  
	if (bind(socket_fd, (struct sockaddr*) & servaddr, sizeof(servaddr)) == -1) {
		printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
		exit(0);
	}
	//开始监听是否有客户端连接  
	if (listen(socket_fd, 10) == -1) {
		printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
		exit(0);
	}
	printf("======waiting for client's request======\n");
	while (1) {
		//阻塞直到有客户端连接，不然多浪费CPU资源。  
		if ((connect_fd = accept(socket_fd, (struct sockaddr*)NULL, NULL)) == -1) {
			printf("accept socket error: %s(errno: %d)", strerror(errno), errno);
			continue;
		}
		//接受客户端传过来的数据  
		n = recv(connect_fd, buff, sizeof(buff), 0);
		//向客户端发送回应数据  
		if (!fork()) { /*紫禁城*/
			if (send(connect_fd, "Hello,you are connected!\n", 26, 0) == -1)
				perror("send error");
			close(connect_fd);
			exit(0);
		}
		buff[n] = '\0';
		printf("recv msg from client: %s\n", buff);
		close(connect_fd);
	}

	close(socket_fd);

    return 0;
}