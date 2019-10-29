#include "headers.h"
#include "session_ftp.h"
#include "data_trans.h"
#include "priv_sock.h"


//返回值表示成功与否
int get_trans_data_fd(Session_t *sess){
    int is_port = 0;
    int is_pasv = 0;

    if (sess->p_addr != NULL ) is_port = 1;
    priv_sock_send_cmd(sess->proto_fd, 2);//通过给nobody进程发消息判断是否激活
    if (priv_sock_recv_int(sess->proto_fd) == 1) is_pasv = 1;

    if (is_port == 0 && is_pasv == 0){
        reply_ftp(sess, 425, "Use PASV or PORT first.");
        return 0;
    }

    if (is_port && is_pasv){
        fprintf(stderr, "both of PORT and PASV are active\n");
        exit(EXIT_FAILURE);
    }

    if (is_port){
        get_port_data_fd(sess);
    }

    if(is_pasv){
        get_pasv_data_fd(sess);
        // printf("Something wrong for is_pasv==1\n");
    }

    return 1;
}

void get_port_data_fd(Session_t *sess){
   //发送cmd
    priv_sock_send_cmd(sess->proto_fd, PRIV_SOCK_GET_DATA_SOCK);
    //发送ip port
    char *ip = inet_ntoa(sess->p_addr->sin_addr);
    uint16_t port = ntohs(sess->p_addr->sin_port);
    priv_sock_send_str(sess->proto_fd, ip, strlen(ip));
    priv_sock_send_int(sess->proto_fd, port);

    priv_sock_send_int(sess->proto_fd, strlen(sess->com));//发送com 的长度
    priv_sock_send_str(sess->proto_fd, sess->com, strlen(sess->com)); //发送com文件地址

    priv_sock_send_int(sess->proto_fd, strlen(sess->args));//发送args 的长度
    priv_sock_send_str(sess->proto_fd, sess->args, strlen(sess->args)); //发送args 文件地址

    printf("sess->restart_pos=%d in proto\n", sess->restart_pos);
    priv_sock_send_int(sess->proto_fd, sess->restart_pos);//发送断点长度
    //补充相关代码

    // //接收应答
    // char result = priv_sock_recv_result(sess->proto_fd);
    // if(result == PRIV_SOCK_RESULT_BAD)
    // {
    //     reply_ftp(sess, 500, "get pasv data_fd error");
    //     fprintf(stderr, "get data fd error\n");
    //     exit(EXIT_FAILURE);
    // }
    // //接收fd
    // // sess->data_fd = priv_sock_recv_int(sess->proto_fd);
    // printf("Received data_fd: %d\n", sess->data_fd);
    // printf("Origin connfd: %d\n", sess->connfd);

    //释放port模式
    free(sess->p_addr);
    sess->p_addr = NULL;    
}

void get_pasv_data_fd(Session_t *sess){
    //先给nobody发命令
    priv_sock_send_cmd(sess->proto_fd, PRIV_SOCK_PASV_ACCEPT);

    priv_sock_send_int(sess->proto_fd, strlen(sess->com));//发送com 的长度
    priv_sock_send_str(sess->proto_fd, sess->com, strlen(sess->com)); //发送com文件地址

    priv_sock_send_int(sess->proto_fd, strlen(sess->args));//发送args 的长度
    priv_sock_send_str(sess->proto_fd, sess->args, strlen(sess->args)); //发送args 文件地址

    printf("sess->restart_pos=%d in proto\n", sess->restart_pos);
    priv_sock_send_int(sess->proto_fd, sess->restart_pos);//发送断点长度


    // //接收结果
    // char res = priv_sock_recv_result(sess->proto_fd);
    // if(res == PRIV_SOCK_RESULT_BAD)
    // {
    //     ftp_reply(sess, 500, "get pasv data_fd error");
    //     fprintf(stderr, "get data fd error\n");
    //     exit(EXIT_FAILURE);
    // }

    // //接收fd
    // sess->data_fd = priv_sock_recv_fd(sess->proto_fd);
}