#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <memory.h>
#include <stdio.h>

int main(int argc, char **argv) {
	int sockfd;
	struct sockaddr_in addr;
	char sentence[8192];
	int len;
	int p;

	//创建socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	//设置目标主机的ip和port
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = 12000;
	if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) {			//转换ip地址:点分十进制-->二进制
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	//连接上目标主机（将socket和目标主机连接）-- 阻塞函数
	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}else{
		printf("Connected!\n");
	}

while(1){
	//获取键盘输入
	fgets(sentence, 4096, stdin);
	len = strlen(sentence);
	// sentence[len] = '\n';
	sentence[len] = '\0';
	printf("Send: len=%d, str=%s", len, sentence);
	
	//把键盘输入写入socket
	p = 0;
	while (p < len) {
		int n = write(sockfd, sentence + p, len + 1 - p);		//write函数不保证所有的数据写完，可能中途退出
		if (n < 0) {
			printf("Error write(): %s(%d)\n", strerror(errno), errno);
			return 1;
 		} else {
			p += n;
		}			
	}

	// printf
	//榨干socket接收到的内容
	p = 0;
	// printf("%d\n", p);
	while (1) {
		// printf("%d\t", p);
		int n = read(sockfd, sentence + p, 8191 - p);
		if (n < 0) {
			printf("Error read(): %s(%d)\n", strerror(errno), errno);	//read不保证一次读完，可能中途退出
			return 1;
		} else if (n == 0) {
			break;
		} else {
			p += n;
			if (sentence[p - 1] == '\0' ) {
				break;
			}
		}
	}
	// printf("\n");

		//socket接收到的字符串并不会添加'\0'
		if(sentence[p-1]!='\0'){
			p += 1;
			sentence[p - 1] = '\0';
		}
		len = p - 1;

	printf("FROM SERVER: len=%d, %s", len, sentence);
}


	close(sockfd);

	return 0;
}
