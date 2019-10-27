#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "headers.h"
#include "config.h"
#include "server.h"
#include "systools.h"
#include "session_ftp.h"

int  main(int argc, const char *argv[]){
    
    // check_permission(); //Check the root permission

    // set_sig_chld(); //set a sig_chld function to deal with the zombie process (僵死进程)

	int port = 21;
	int pos = 1;
	while (pos<argc){
		if (pos+1==argc) break;
		if (strcmp(argv[pos], "-port")==0){
			port = atoi(argv[pos+1]);
		}		
		pos += 2;
	}

    int listenfd, connfd;		//监听socket和连接socket不一样，后者用于数据传输
    struct sockaddr_in addr;
	pid_t pid;

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
	addr.sin_port = htons(port);
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

	// int ii=0;
	// printf("Scan1\n");
	// scanf("%d", ii);

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

		//每当用户连接上，就fork一个子进程
        if((pid = fork()) == -1)
            ERR_EXIT("fork");
        else if(pid == 0) //子进程
        {
            printf("This is the child thread, pid = %d\n", pid);
            close(listenfd);

            sess.connfd = connfd;
            session_begin(&sess);
            //这里保证每次成功执行后退出循环
            printf("Exit the child thread\n");
            exit(EXIT_SUCCESS);
        }
        else // 原始进程
        {
            printf("This is  the father thread, pid = %d\n", pid);
            //pid_to_ip
            // add_pid_ip_to_hash(pid, ip);
            close(connfd);
            printf("End the father thread\n");
        }

		/*
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
		*/

    }

    return 0;
}



/*
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

	// int ii=0;
	// printf("Scan1\n");
	// scanf("%d", ii);

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
*/