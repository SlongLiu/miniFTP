#ifndef SESSION_FTP_H
#define SESSION_FTP_H

#define MAX_COMMAND 1024

typedef struct 
{
    /* session */
    char command[MAX_COMMAND]; //指令
    char com[MAX_COMMAND]; //指令码
    char args[MAX_COMMAND]; // 参数
    
    char user[MAX_COMMAND]; // 用户名
    // char pass[MAX_COMMAND];

    int connfd; //客户链接的fd
    int nobody_fd; //nobody进程的fd
    int proto_fd; // proto进程的fd

    int islogin; //用户是否登录 是1 否0

    struct sockaddr_in *p_addr; //port下对方的ip和port
    int listen_fd; //pasv下监听fd
    int data_fd; //用于数据传输fd
    
    int is_translating_data; //是否在数据传输阶段 是1 否0

    long long restart_pos; //传输断点

    char* rnfr_name;

} Session_t;

void session_init(Session_t* sess);
void session_reset_command(Session_t* sess);
void session_begin(Session_t *sess);


void reply_ftp(Session_t *sess, int status, const char *text);
int writen(int fd, const void *buf, int n);
int receive_ftp(int connfd, char* buf, int maxsize);
int readn(int fd, void *buf, int n);
#endif