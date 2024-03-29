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
            }else if (strcmp(sess->com, "STOR")==0){
                run_stor(sess);
            }else if (strcmp(sess->com, "MKD")==0){
                run_mkd(sess);
            }else if (strcmp(sess->com, "CWD")==0){
                run_cwd(sess);
            }else if (strcmp(sess->com, "PWD")==0){
                run_pwd(sess);
            }else if (strcmp(sess->com, "RMD")==0){
                run_rmd(sess);
            }else if (strcmp(sess->com, "RNFR")==0){
                run_rnfr(sess);
            }else if (strcmp(sess->com, "RNTO")==0){
                run_rnto(sess);
            }else if (strcmp(sess->com, "LIST")==0){
                run_list(sess);
            }else if (strcmp(sess->com, "REST")==0){
                run_rest(sess);
            }else if (strcmp(sess->com, "DELE")==0){
                run_dele(sess);
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
    }else{
        reply_ftp(sess, 501, "Incorrect username or password.");
        return;
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
    priv_sock_send_cmd(sess->proto_fd,5);
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

//指示nobody进程发送文件
void run_retr(Session_t *sess){
    //进入数据传输阶段
    sess->is_translating_data = 1;

    //获取data_fd 现在主要发args
    if(get_trans_data_fd(sess) == 0) 
    {
        reply_ftp(sess, 550, "Failed to get_trans_data_fd.");
        return;
    }

    int readMark = priv_sock_recv_int(sess->proto_fd);
    if (readMark == 1){
        reply_ftp(sess, 150, "OK before the transfer");
    }else{
        reply_ftp(sess, 550, "Failed to open file");
        return;
    }
    

    int mark = priv_sock_recv_result(sess->proto_fd);

    if (mark == 1){
        sess->restart_pos = 0;
        reply_ftp(sess, 226, "Transfer complete.");
    }else{
        priv_sock_recv_int(sess->proto_fd);
        reply_ftp(sess, 451, "Sendfile failed.");
    }

    // //226
    // if(flag == 0)
    //     reply_ftp(sess, 226, "Transfer complete.");
    // else if(flag == 1)
    //     reply_ftp(sess, 451, "Sendfile failed.");
    // else if(flag == 2)
    //     reply_ftp(sess, 226, "ABOR successful.");

    sess->is_translating_data = 0;    
}

void run_stor(Session_t *sess){
    //进入数据传输阶段
    sess->is_translating_data = 1;

     //获取data_fd 现在主要发args
    if(get_trans_data_fd(sess) == 0) 
    {
        reply_ftp(sess, 550, "Failed to get_trans_data_fd.");
        return;
    }

    int readMark = priv_sock_recv_int(sess->proto_fd);
    if (readMark == 1){
        reply_ftp(sess, 150, "OK before the transfer");
    }else{
        reply_ftp(sess, 550, "Failed to open file");
        return;
    }

    int mark = priv_sock_recv_result(sess->proto_fd);

     if (mark == 1){
        reply_ftp(sess, 226, "Transfer complete.");
        // sess->restart_pos = 0;
    }else{
        reply_ftp(sess, 451, "Sendfile failed.");
        // sess->restart_pos = priv_sock_recv_int(sess->proto_fd);
    }

    sess->is_translating_data = 0;    
}

void run_mkd(Session_t *sess){

    if(mkdir(sess->args, 0777) == -1){
        reply_ftp(sess, 550, "Create directory failed.");
        return;
    }

    char text[1124] = {0};
    if(sess->args[0] == '/') //绝对路径
    {
        snprintf(text, sizeof text, "%s created.", sess->args);
    }
    else{
        //char *getcwd(char *buf, size_t size);
        char tmp[1024] = {0};
        if(getcwd(tmp, sizeof tmp) == NULL){
            reply_ftp(sess, 550, "Something wrong in getcwd");
            ERR_EXIT("getcwd");
        }
        snprintf(text, sizeof text, "%s/%s created.", tmp, sess->args);
    }

    reply_ftp(sess, 257, text);
}

void run_cwd(Session_t *sess)
{
    if(chdir(sess->args) == -1) {
        //550
        reply_ftp(sess, 550, "Failed to change directory.");
        return;
    }else{

        priv_sock_send_cmd(sess->proto_fd, 6);//告诉nobody进程改变工作目录

        priv_sock_send_int(sess->proto_fd, strlen(sess->com));//发送com 的长度
        priv_sock_send_str(sess->proto_fd, sess->com, strlen(sess->com)); //发送com文件地址

        priv_sock_send_int(sess->proto_fd, strlen(sess->args));//发送args 的长度
        priv_sock_send_str(sess->proto_fd, sess->args, strlen(sess->args)); //发送args 文件地址

        if (priv_sock_recv_int(sess->proto_fd) == -1){
            reply_ftp(sess, 550, "Failed to change directory.");
            return;
        }
        
        //250 Directory successfully changed.
        char text[1124] = {0};
        snprintf(text, sizeof text, "Successfully changed to %s", sess->args);
        reply_ftp(sess, 250, text);
    }
}

void run_pwd(Session_t *sess)
{
    char tmp[1024] = {0};
    if(getcwd(tmp, sizeof tmp) == NULL)
    {
        //return值为-1/0，函数进入系统内核，
        //返回值判断用perror
        //返回值为NULL,不用perror，fprintf(stderr, "a");
        fprintf(stderr, "get cwd error\n");
        reply_ftp(sess, 504, "error");
        return;
    }
    char text[1024] = {0};
    snprintf(text, sizeof text, "Your dictionary: \"%s\"", tmp);
    reply_ftp(sess, 257, text);
}

void run_rmd(Session_t *sess)
{
    if(rmdir(sess->args) == -1){
        //550 Remove directory operation failed.
        reply_ftp(sess, 550, "Remove directory operation failed.");
        return;
    }else{
        //250 Remove directory operation successful.
        reply_ftp(sess, 250, "Remove directory operation successful.");
    }
}

void run_rnfr(Session_t *sess)
{
    if(sess->rnfr_name) //防止内存泄露
    {
        free(sess->rnfr_name);
        sess->rnfr_name = NULL;
    }
    sess->rnfr_name = (char*)malloc(strlen(sess->args)+1);
    strcpy(sess->rnfr_name, sess->args);
    //350 Ready for RNTO.
    reply_ftp(sess, 350, "Ready for RNTO.");
}

void run_rnto(Session_t *sess)
{
    if(sess->rnfr_name == NULL){
        //503 RNFR required first.
        reply_ftp(sess, 503, "RNFR required first.");
        return;
    }

    if(rename(sess->rnfr_name, sess->args) == -1){
        reply_ftp(sess, 550, "Rename failed.");
        return;
    }
    
    char text[1224] = {0};
    snprintf(text, sizeof text, "Change from %s to %s", sess->rnfr_name, sess->args);
    
    if(sess->rnfr_name != NULL) {
        free(sess->rnfr_name);
        sess->rnfr_name = NULL;
    }

    //250 Rename successful.
    reply_ftp(sess, 250, text);
}

void run_list(Session_t *sess){
    //进入数据传输阶段
    sess->is_translating_data = 1;

    //获取data_fd 现在主要发args
    if(get_trans_data_fd(sess) == 0) 
    {
        reply_ftp(sess, 550, "Failed to get_trans_data_fd.");
        return;
    }

    int readMark = priv_sock_recv_int(sess->proto_fd);
    if (readMark == 1){
        reply_ftp(sess, 150, "OK before the transfer");
    }else{
        reply_ftp(sess, 550, "Failed to open file");
        return;
    }
    

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

    sess->is_translating_data = 0;    
}

void run_rest(Session_t *sess){
    sess->restart_pos = atoi(sess->args);
    printf("sess->restart_pos=%d\n", sess->restart_pos);
    reply_ftp(sess, 350, "Waiting for the next command.");
}

void run_dele(Session_t *sess){
    struct stat buf; 
    if(stat(sess->args, &buf) == -1) //获取文件状态
    {
        printf("Failed to get the stat of file.\n");
        reply_ftp(sess, 500, "Failed to get the stat of file.");
        return;
    }

    if(__S_IFDIR & buf.st_mode){
        reply_ftp(sess, 500, "This is a folder, please use rmd");
        return;
    }

    if(remove(sess->args)==0){
        reply_ftp(sess, 250, "Successful delete file");
    }else{
        reply_ftp(sess, 500, "Unsuccessful delete file");
    }
}