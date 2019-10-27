#include "execute_func.h"
#include "session_ftp.h"
#include "headers.h"
#include "priv_sock.h"
#include "data_trans.h"


void execute_map(Session_t *sess){
    if(strcmp(sess->com,"USER")==0){
        run_user(sess);
    }else if(strcmp(sess->com, "PASS")==0){
        run_pass(sess);
    }else{
        if(sess->islogin == 0){
                reply_ftp(sess, 530, "Please login by using USER your username");
                return;
            }

            if(strcmp(sess->com, "SYST")==0){
                run_syst(sess);
            }else if(strcmp(sess->com, "TYPE")==0){
                run_type(sess);
            }else if (strcmp(sess->com, "QUIT")==0){
                run_quit(sess);
            }else if (strcmp(sess->com, "PORT")==0){
                run_port(sess);
            }else if (strcmp(sess->com, "PASV")==0){
                run_pasv(sess);
            }else if (strcmp(sess->com, "RETR")==0){
                run_retr(sess);
            }else{
                reply_ftp(sess, 504, "Undefined command");
            }
    }
}

void run_user(Session_t *sess){
    if (strcmp(sess->args, "anonymous") == 0){
        strcpy(sess->user, "anonymous");
        reply_ftp(sess, 331, "Guest login ok, please send your complete e-mail address as password.");
    }else{
        strcpy(sess->user, sess->args);
        reply_ftp(sess, 331, "Guest login ok, please send your complete e-mail address as password.");
    }
}

void run_pass(Session_t *sess){
    if(1 || strcmp(sess->user, "anonymous") == 0){
        
        if(strlen(sess->args) == 0){
            sess->islogin = 1;
            reply_ftp(sess, 230, "Welcome to shilong's FTP");
            return;
        }

        char *pchar = strchr (sess->args, '@');
        if (pchar == NULL){
            reply_ftp(sess, 501, "Please use the right email");
            return;
        }else{
            sess->islogin = 1;
            reply_ftp(sess, 230, "Welcome to shilong's FTP");
            // reply_ftp(sess, 230, "-\nWelcome to shilong's FTP\nin Department of IE in THU\n-\n-Guest login OK.");
            // reply_ftp(sess, 230, "");
            // reply_ftp(sess, 230, "");
            // reply_ftp(sess, 230, "-");
            // reply_ftp(sess, 230, "");
        }
    }
}

void run_syst(Session_t *sess){
    // printf("run_syst\n");
    reply_ftp(sess, 215, "UNIX Type: L8");
}

void run_type(Session_t *sess){
    if (strcmp(sess->args, "I")==0){
        reply_ftp(sess, 200, "Type set to I.");
    }else{
        reply_ftp(sess, 501, "Syntax error in parameters or arguments.");
    }
}

void run_quit(Session_t *sess){
    reply_ftp(sess, 221, "Good Bye!");

    sess->islogin = 0;
    memset(sess->user, 0, sizeof(sess->user));

    printf("Before close\n");
    close(sess->connfd);
}

void run_port(Session_t *sess){
    //设置主动工作模式
    //PORT 192,168,44,1,200,174
    unsigned int v[6] = {0};
    sscanf(sess->args, "%u,%u,%u,%u,%u,%u", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]);

    sess->p_addr = (struct sockaddr_in *)malloc(sizeof (struct sockaddr_in));
    memset(sess->p_addr, 0, sizeof(struct sockaddr_in));
    sess->p_addr->sin_family = AF_INET;

    char *p = (char*)&sess->p_addr->sin_port;
    p[0] = v[4];
    p[1] = v[5];

    p = (char*)&sess->p_addr->sin_addr.s_addr;
    p[0] = v[0];
    p[1] = v[1];
    p[2] = v[2];
    p[3] = v[3];

    reply_ftp(sess, 200, "PORT command successful. Consider using PASV.");
}

void run_pasv(Session_t *sess)
{
    char ip[16] = "127.0.0.1";
    // get_local_ip(ip);
    printf("Begin run\n");

    //给nobody发送命令
    priv_sock_send_cmd(sess->proto_fd, PRIV_SOCK_PASV_LISTEN);
    printf("proto-->nobody: Send PRIV_SOCK_PASV_LISTEN\n");
    //接收nobody的应答
    char res = priv_sock_recv_result(sess->proto_fd);
    printf("nobody-->proto: Receive result %d\n", res);
    if(res == PRIV_SOCK_RESULT_BAD)
    {
        reply_ftp(sess, 500, "get listenfd error");
        return;
    }
    //接收port 一般是20
    uint16_t port = priv_sock_recv_int(sess->proto_fd);
    printf("The port: %d\n", port);


    //227 Entering Passive Mode (192,168,44,136,194,6).
    unsigned int v[6];
    sscanf(ip, "%u.%u.%u.%u", &v[0], &v[1], &v[2], &v[3]);
    uint16_t net_endian_port = htons(port); //网络字节序
    unsigned char *p = (unsigned char*)&net_endian_port;
    v[4] = p[0];
    v[5] = p[1];

    char text[1024] = {0};
    snprintf(text, sizeof text, "Entering Passive Mode (%u,%u,%u,%u,%u,%u).", v[0], v[1], v[2], v[3], v[4], v[5]);

    reply_ftp(sess, 227, text);
}

//run_retr 未完成
void run_retr(Session_t *sess){
    //进入数据传输阶段
    sess->is_translating_data = 1;

    //获取data_fd 现在主要发args
    if(get_trans_data_fd(sess) == 0) 
    {
        reply_ftp(sess, 550, "Failed to get_trans_data_fd.");
        return;
    }

    reply_ftp(sess, 150, "Attemp");

    int mark = priv_sock_recv_result(sess->proto_fd);

    if (mark == 1){
        reply_ftp(sess, 226, "Transfer complete.");
    }else{
        reply_ftp(sess, 451, "Sendfile failed.");
    }

    // //226
    // if(flag == 0)
    //     reply_ftp(sess, 226, "Transfer complete.");
    // else if(flag == 1)
    //     reply_ftp(sess, 451, "Sendfile failed.");
    // else if(flag == 2)
    //     reply_ftp(sess, 226, "ABOR successful.");

    //先恢复控制连接的信号
    // setup_signal_alarm_ctrl_fd();
    sess->is_translating_data = 0;    
}

/*
====无用代码仓库===
//open 文件
    int fd = open(sess->args, O_RDONLY);
    if(fd == -1)
    {
        reply_ftp(sess, 550, "Failed to open file.");
        return;
    }

    // //对文件加锁
    // if(lock_file_read(fd) == -1)
    // {
    //     reply_ftp(sess, 550, "Failed to open file.");
    //     return;
    // }

    // //判断是否是普通文件
    struct stat sbuf;
    if(fstat(fd, &sbuf) == -1) //获取文件状态
        ERR_EXIT("fstat");
    // if(!S_ISREG(sbuf.st_mode))
    // {
    //     reply_ftp(sess, 550, "Can only download regular file.");
    //     return;
    // }

    //判断断点续传
    unsigned long filesize = sbuf.st_size;//剩余的文件字节
    int offset = sess->restart_pos;
    if(offset != 0)
    {
        filesize -= offset;
    }

    if(lseek(fd, offset, SEEK_SET) == -1) //lseek用于重新定位文件读写的位移
        ERR_EXIT("lseek");

    //仅有二进制模式
    // char text[1024] = {0};
    // snprintf(text, sizeof text, "Opening Binary mode data connection for %s (%lu bytes).", sess->args, filesize);
    reply_ftp(sess, 150, "zhi you er jin zhi");

    // //记录时间
    // sess->start_time_sec = get_curr_time_sec();
    // sess->start_time_usec = get_curr_time_usec();

    //传输
    int flag = 1; //记录下载的结果
    int nleft = filesize; //剩余字节数
    int block_size = 0; //一次传输的字节数
    const int kSize = 65536;
    while(nleft > 0)
    {
        block_size = (nleft > kSize) ? kSize : nleft;//读取字节数
        //sendfile发生在内核，更加高效
        int nwrite = sendfile(sess->data_fd, fd, NULL, block_size);

        // if(sess->is_receive_abor == 1)
        // {
        //     flag = 2; //ABOR
        //     //426
        //     reply_ftp(sess, FTP_BADSENDNET, "Interupt downloading file.");
        //     sess->is_receive_abor = 0;
        //     break;
        // }

        if(nwrite == -1)
        {
            flag = 1; //错误
            break;
        }
        nleft -= nwrite;

        // //实行限速
        // limit_curr_rate(sess, nwrite, 0);
    }
    if(nleft == 0)
        flag = 0; //正确退出

    // //清理 关闭fd 文件解锁
    // if(unlock_file(fd) == -1)
    //     ERR_EXIT("unlock_file");
    close(fd);
    
    close(sess->data_fd);
    sess->data_fd = -1;
*/