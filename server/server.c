#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "config.h"
#include "server.h"
#include "systools.h"
#include "session_ftp.h"

int  main(int argc, const char *argv[]){
    
    // check_permission(); //Check the root permission

    // set_sig_chld(); //set a sig_chld function to deal with the zombie process (僵死进程)

    int listenfd, connfd;		//监听socket和连接socket不一样，后者用于数据传输
    struct sockaddr_in addr;

    //创建socket - 创建监听fd
	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}else{
        printf("Secceed socketfd: %d\n", listenfd);
    }

    //设置本机的ip和port
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = 6789;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);	//监听"0.0.0.0"

	//将本机的ip和port与socket绑定
	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}else{
        printf("Secceed bind\n");
    }

 	//开始监听socket
	if (listen(listenfd, 10) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 1;
	} else{
        printf("Begin listen\n");
    }

    Session_t sess;
    session_init(&sess);

    printf("Begin the loop\n");
    while(1){
 		//等待client的连接 -- 阻塞函数
		if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			// continue;
            break;
		}else{
			printf("Connected!connfd=%d\n", connfd);
		}

        sess.connfd = connfd;
        
        session_begin(&sess);
        printf("End one loop\n");

    }
    return 0;
}