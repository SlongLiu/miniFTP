#include "session_ftp.h"
#include "strtools.h"
#include "execute_func.h"
#include "priv_sock.h"
#include "headers.h"
#include "handle_ftp.h"

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <memory.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

void session_init(Session_t* sess){
    memset(sess->command, 0, sizeof(sess->command));
    memset(sess->com, 0, sizeof(sess->com));
    memset(sess->args, 0, sizeof(sess->args));

    sess->islogin = 0; //是否登录 0否 1是
    memset(sess->user, 0, sizeof(sess->user));
    sess->p_addr = NULL;
    
    sess->connfd = -1;
    sess->nobody_fd = -1;
    sess->proto_fd = -1;
    sess->listen_fd = -1;

    sess->is_translating_data = 1;
    sess->restart_pos = 0;

    sess->rnfr_name = NULL;
}

void session_reset_command(Session_t* sess){
    memset(sess->command, 0, sizeof(sess->command));
    memset(sess->com, 0, sizeof(sess->com));
    memset(sess->args, 0, sizeof(sess->args)); 
}

void session_begin(Session_t* sess){

    printf("Session begin!");
    priv_sock_init(sess);

    pid_t pid;
    if((pid = fork()) == -1)
        ERR_EXIT("fork");
    else if (pid == 0){
        priv_sock_set_proto_context(sess);
        handle_proto(sess);
    }else
    {
        priv_sock_set_nobody_context(sess);
        handle_nobody(sess);
    }
    // // int kkk = 0;
}

/*
读取ftp的数据,返回读取的数据的长度
*/
int receive_ftp(int connfd, char* buf, int maxsize){
    	//榨干socket传来的内容
		// printf("Begin receive ftp\n");
        int p = 0;
		while (1) {
            // printf("Before read: p=%d\t", p);
			int n = read(connfd, buf + p, maxsize - 1 - p);
            // printf("After read: n=%d,p=%d\n", n, p);
			if (n < 0) {
				printf("Error read(): %s(%d)\n", strerror(errno), errno);
				close(connfd);
				return -1;
			} else if (n == 0) {
				break;
			} else {
				p += n;
                // printf("-1- buf:%s", buf);
				if (buf[p - 1] == '\n') {
					break;
				}
			}
		}
    // printf("-1- buf:%s", buf);

    if(*(buf+p-1) != '\0'){
		p += 1;
		*(buf+p-1) = '\0';
	}

    // printf("C->S:%d || %s", p-1, buf);
    
    return p-1;
}


/*
向client返回数据
sess: session
status: 状态码
text: 要返回的字符串
*/
void reply_ftp(Session_t *sess, int status, const char *text){
    char tmp[1024] = { 0 };

    // int len = strlen(text);
    // while(text[len]=='\n' || text[len]=='\r'){
    //     *(text + len) = '\0';
    //     len -= 1;
    // }

    snprintf(tmp, sizeof tmp, "%d %s\r\n", status, text);
    writen(sess->connfd, tmp, strlen(tmp));

    // int len = strlen(tmp);
    // // if(tmp[len] != '\0'){
    // //     len += 1;
    // //     tmp[len] = '\0';
    // // }

    // // print("%d:%s", len, tmp)
    // int p =0;
	// 	while (p < len) {
	// 		int n = write(sess->connfd, tmp + p, len + 1 - p);
	// 		if (n < 0) {
	// 			printf("Error write(): %s(%d)\n", strerror(errno), errno);
	// 			return;
	//  		} else {
	// 			p += n;
	// 		}
	// 	}

    int len = strlen(tmp);
    printf("S->C:");
    printf("%d || %s",len ,tmp);
}


/*
 *功能：发送固定的字节数
 *fd:文件描述符
 *buf:发送数据的缓冲区
 *n:要发送的字节数
 *返回值：成功返回n,失败返回-1
 */
int writen(int fd, const void *buf, int n)
{
    int nleft = n;
    int nwrite;
    char *bufp = (char*)buf;

    while(nleft > 0)
    {
        if((nwrite = write(fd, bufp, nleft)) < 0)//这里
        {
            if (errno == EINTR)
                continue;
            return -1;
        }
        else if (nwrite == 0)
            continue;

        bufp += nwrite;
        nleft -= nwrite;
    }
    return n;
}

/*
 *功能：读取固定的字节数
 *fd:文件描述符
 *buf:接受数据的缓冲区
 *n:要读取的字节数
 *返回值：成功返回n,失败返回-1，读到EOF返回小于n的值
 */
int readn(int fd, void *buf, int n)
{
    int nleft = n;
    int nread;
    char *bufp = (char*)buf;

    while(nleft > 0)
    {
        if((nread = read(fd, bufp, nleft)) < 0)//这里
        {
            if(errno == EINTR)
                continue;
            return -1;
        }
        else if(nread == 0)
            return n - nleft;

        bufp += nread;
        nleft -= nread;
    }
    return n;
}