#include "handle_ftp.h"
#include "headers.h"
#include "strtools.h"
#include "execute_func.h"
#include "priv_sock.h"
#include "pasv_ftp.h"

// 主进程处理用户输入的命令
void handle_proto(Session_t* sess){
    reply_ftp(sess, 220, "(FtpServer lsl is READY!)");

    // char text[8192];
    while(1){
        session_reset_command(sess);
        if(receive_ftp(sess->connfd, sess->command, MAX_COMMAND)==-1){
            printf("The client has been closed\n");
            break;
        }else{
            printf("sess.connfd:%d\n", sess->connfd);
        }
        // printf("sess->command:%s", sess->command);
        // scanf("%d", &kkk);

        str_trim_crlf(sess->command);
        str_split(sess->command, sess->com, sess->args, ' ');
        str_upper(sess->com);	

        printf("COMMD=[%s], ARGS=[%s]\n", sess->com, sess->args);
        if (strcmp(sess->com,"QUIT")==0){
            reply_ftp(sess, 221, "Good Bye!");
            printf("Receive QUIT, return.\n");
            return;
        }
        
        execute_map(sess);

    }
    close(sess->connfd);
}

//nobody时刻准备从子进程接受命令
void handle_nobody(Session_t* sess){
    //设置为nobody进程
    // set_nobody();
    //添加绑定20端口的特权
    // set_bind_capabilities();
    printf("FInished the setting\n");

    char cmd;
    while(1)
    {
        cmd = priv_sock_recv_cmd(sess->nobody_fd);
        printf("proto-->nobody: Receive cmd %d\n", cmd);
        switch (cmd)
        {
            case PRIV_SOCK_GET_DATA_SOCK:
                printf("1 PRIV_SOCK_GET_DATA_SOCK\n");
                privop_pasv_get_data_sock(sess); //port模式下 接收到命令
                break;
            case PRIV_SOCK_PASV_ACTIVE:
                printf("2 PRIV_SOCK_PASV_ACTIVE\n");
                privop_pasv_active(sess); //pasv是否激活
                break;
            case PRIV_SOCK_PASV_LISTEN:
                printf("3 PRIV_SOCK_PASV_LISTEN\n");
                privop_pasv_listen(sess); //接受到pasv命令打开监听端口进行监听
                break;
            case PRIV_SOCK_PASV_ACCEPT:
                printf("4 PRIV_SOCK_PASV_ACCEPT\n");
                privop_pasv_accept(sess); //pasv模式下 接收到命令 
                break;
            case 5:
                printf("5 CLOSE NOBODY\n");
                break;
            default:
                fprintf(stderr, "Unkown command\n");
                exit(EXIT_FAILURE);
        }
    }
}


void set_nobody()
{
    //基本思路
    //1. 首先获取nobody的uid、gid
    //2. 然后逐项进行设置
    struct passwd *pw;
    if((pw = getpwnam("nobody")) == NULL)
        ERR_EXIT("getpwnam");

    //先获取gid
    if(setegid(pw->pw_gid) == -1)
        ERR_EXIT("setegid");

    //euid---有效的用户ID
    if(seteuid(pw->pw_uid) == -1)
        ERR_EXIT("seteuid");
}


void set_bind_capabilities(){
    struct __user_cap_header_struct cap_user_header;
    cap_user_header.version = _LINUX_CAPABILITY_VERSION_1;
    cap_user_header.pid = getpid();

    struct __user_cap_data_struct cap_user_data;
    __u32 cap_mask = 0; //类似于权限的集合
    cap_mask |= (1 << CAP_NET_BIND_SERVICE); //0001000000
    cap_user_data.effective = cap_mask;
    cap_user_data.permitted = cap_mask;
    cap_user_data.inheritable = 0; //子进程不继承特权

    if(capset(&cap_user_header, &cap_user_data) == -1)
        ERR_EXIT("capset");
}

int capset(cap_user_header_t hdrp, const cap_user_data_t datap){
    return syscall(SYS_capset, hdrp, datap);
}