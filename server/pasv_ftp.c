#include "pasv_ftp.h"
#include "headers.h"
#include "priv_sock.h"
#include <unistd.h>

/*
创建监听fd， 监听并返回20端口
*/
void privop_pasv_listen(Session_t *sess){
    printf("There is the func: privop_pasv_listen\n");
    // 创建socket
    int listenfd;
    struct sockaddr_in addr;

    //创建socket - 创建监听fd
	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return;
	}else{
        printf("Secceed listenfd: %d in PASV\n", listenfd);
    }

    //设置本机的ip和port
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(20);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);	//监听"0.0.0.0"

    //设置端口复用
    int on = 1;
    if((setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on))) < 0)
        ERR_EXIT("setsockopt");

    //将本机的ip和port与socket绑定
	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return;
	}else{
        printf("Secceed bind in PASV\n");
    }

    //开始监听socket
	if (listen(listenfd, 10) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return;
	} else{
        printf("Begin listen in PASV\n");
    }

    sess->listen_fd = listenfd;
     //发送应答
    priv_sock_send_result(sess->nobody_fd, PRIV_SOCK_RESULT_OK);
    //发送port
    uint16_t net_endian_port = ntohs(addr.sin_port);
    priv_sock_send_int(sess->nobody_fd, net_endian_port);

}

// 返回pasv模式是否激活
void privop_pasv_active(Session_t *sess){
    priv_sock_send_int(sess->nobody_fd, ( sess->listen_fd != -1));
}

//创建文件传输链接 port模式
void privop_pasv_get_data_sock(Session_t *sess){
    char ip[16] = {0};
    priv_sock_recv_str(sess->nobody_fd, ip, sizeof ip);
    uint16_t port = priv_sock_recv_int(sess->nobody_fd);

    //创建fd
    printf("=====Begin to create data_fd, port =%d=====\n", port);
    int data_fd;
    
    //创建socket
	if ((data_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return;
	}

    //设置本机ip和port
    struct sockaddr_in selfAddr;
	memset(&selfAddr, 0, sizeof(selfAddr));
	selfAddr.sin_family = AF_INET;
	selfAddr.sin_port = htons(20);
	if (inet_pton(AF_INET, "127.0.0.1", &selfAddr.sin_addr) <= 0) {			//转换ip地址:点分十进制-->二进制
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		return;
	}

    //设置端口复用
    int on = 1;
    if((setsockopt(data_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on))) < 0)
        ERR_EXIT("setsockopt");

     //将本机的ip和port与socket绑定
     if(bind(data_fd, (struct sockaddr *)&selfAddr, sizeof selfAddr) == -1){
            // printf("")
            ERR_EXIT("bind");
     }
         
    printf("Finished bind self ip and port\n");

	//设置目标主机的ip和port
    struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {			//转换ip地址:点分十进制-->二进制
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		return;
	}
    
    // printf("Before the connect()\n");
    // int ss  = 0;
    // scanf("%d", &ss);
	
    //连接上目标主机（将socket和目标主机连接）-- 阻塞函数
	if (connect(data_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		return;
	}else{
		printf("Connected !\n");
	}

    // scanf("%d", &ss);

    sess->data_fd = data_fd;
    printf("=====Finished to create data_fd: %d=====\n", data_fd);
    
    int comLen= priv_sock_recv_int(sess->nobody_fd); //接受com 的长度
    priv_sock_recv_str(sess->nobody_fd, sess->com, comLen); //接受com 文件地址

    int argsLen = priv_sock_recv_int(sess->nobody_fd); //接受args 的长度
    priv_sock_recv_str(sess->nobody_fd, sess->args, argsLen); //接受args 文件地址
    

    printf("sess->com: %s\n", sess->com);
    printf("sess->args: %s\n", sess->args);
    
    int transret=2;
    if (strcmp(sess->com, "RETR") == 0){
        transret = transferFIleNobody(sess);
    }else if (strcmp(sess->com, "STOR") == 0){
        transret = uploadFIleNobody(sess, 1);
    }else{
        printf("Error command: %s", sess->com);
    }

    printf("transret = %d\n", transret);
    priv_sock_send_result(sess->nobody_fd, transret);

    // priv_sock_send_int(sess->nobody_fd, data_fd);
    // close(data_fd);
}

// pasv模式下传输文件
void privop_pasv_accept(Session_t *sess){

    int comLen= priv_sock_recv_int(sess->nobody_fd); //接受com 的长度
    priv_sock_recv_str(sess->nobody_fd, sess->com, comLen); //接受com 文件地址

    int argsLen = priv_sock_recv_int(sess->nobody_fd); //接受args 的长度
    priv_sock_recv_str(sess->nobody_fd, sess->args, argsLen); //接受args 文件地址
    
    printf("sess->com: %s\n", sess->com);
    printf("sess->args: %s\n", sess->args);

    int data_fd;
    if ((data_fd = accept(sess->listen_fd, NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
		}else
        {
            printf("Successful accpepted!\n");
        }
        
    sess->data_fd = data_fd;
    printf("=====Finished to create data_fd: %d=====\n", data_fd);

    int transret=2;
    if (strcmp(sess->com, "RETR") == 0){
        transret = transferFIleNobody(sess);
    }else if (strcmp(sess->com, "STOR") == 0){
        transret = uploadFIleNobody(sess, 1);
    }else{
        printf("Error command: %s", sess->com);
    }
    
    printf("transret = %d\n", transret);
    priv_sock_send_result(sess->nobody_fd, transret);
    close(sess->listen_fd);
    sess->listen_fd = -1;

    // priv_sock_send_int(sess->nobody_fd, data_fd);
    // close(data_fd);

}

// 返回传输结果 1 正常 2 失败
int transferFIleNobody(Session_t *sess){

    //open 文件
    int fd = open(sess->args, O_RDONLY);
    if(fd == -1)
    {
        printf("Failed to open file.\n");
        priv_sock_send_int(sess->nobody_fd, 2);
        return -1;
    }else
    {
        priv_sock_send_int(sess->nobody_fd, 1);
    }
    

    struct stat sbuf;
    if(fstat(fd, &sbuf) == -1) //获取文件状态
    {
        printf("Failed to get the stat of file.\n");
        ERR_EXIT("fstat");
    }

            

    //判断断点续传
    unsigned long filesize = sbuf.st_size;//剩余的文件字节
    int offset = sess->restart_pos;
    if(offset != 0)
    {
        filesize -= offset;
    }

    if(lseek(fd, offset, SEEK_SET) == -1) //lseek用于重新定位文件读写的位移
    {
        printf("Error lseek\n");
        ERR_EXIT("lseek");
    }
        
    //仅有二进制模式
    printf("Binary mode\n");

    // //记录时间
    // sess->start_time_sec = get_curr_time_sec();
    // sess->start_time_usec = get_curr_time_usec();
    // int qq;
    // scanf("%d", qq);
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

        printf("block_size = %d, nwrite = %d\n", block_size, nwrite);

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
            printf("Something wrong in transfer.\n");
            flag = 1; //错误
            break;
        }
        nleft -= nwrite;
    }
    if(nleft == 0)
        flag = 0; //正确退出

    close(fd); //关闭文件描述符
    close(sess->data_fd); //关闭tcp连接
    sess->data_fd = -1;

    if (flag == 1) return -1;

    printf("Finished transfer in transferFIleNobody.\n");
    return 1;
}

// 返回传输结果 1 正常 2 失败
// is_appe 表示是否覆盖源文件
int uploadFIleNobody(Session_t *sess, int is_appe){
    //open 文件
    int fd = open(sess->args, O_WRONLY | O_CREAT, 0666);
    // int fd = open(sess->args, O_WRONLY);
    if(fd == -1){
        // printf("Failed to open file.\n");
        printf("Error open(): %s(%d)\n", strerror(errno), errno);
        priv_sock_send_int(sess->nobody_fd, 2);
        close(sess->data_fd);
        return 2;
    }else{
        priv_sock_send_int(sess->nobody_fd, 1);
    }

    //判断是否是普通文件
    struct stat sbuf;
    if(fstat(fd, &sbuf) == -1)
        ERR_EXIT("fstat");

    //区分模式
    long long offset = sess->restart_pos;
    if(!is_appe && offset == 0) //STOR
    {
        //创建新的文件
        ftruncate(fd, 0); //如果源文件存在则直接覆盖
    }  else if(!is_appe && offset != 0) // REST + STOR
    {
        //lseek进行偏移
        ftruncate(fd, offset); //截断后面的内容
        if(lseek(fd, offset, SEEK_SET) == -1)
            ERR_EXIT("lseek");
    } else //APPE
    {
        //对文件进行扩展 偏移到末尾进行追加
        if(lseek(fd, 0, SEEK_END) == -1)
            ERR_EXIT("lseek");
    }

    // //150 ascii
    // ftp_reply(sess, FTP_DATACONN, "OK to send.");

    // //更新当前时间
    // sess->start_time_sec = get_curr_time_sec();
    // sess->start_time_usec = get_curr_time_usec();

    //上传
    char buf[65535] = {0};
    int flag = 0;
    while(1)
    {
        int nread = read(sess->data_fd, buf, sizeof buf);

        // if(sess->is_receive_abor == 1)
        // {
        //     flag = 3; //ABOR
        //     //426
        //     ftp_reply(sess, FTP_BADSENDNET, "Interupt uploading file.");
        //     sess->is_receive_abor = 0;
        //     break;
        // }

        if(nread == -1)
        {
            if(errno == EINTR)
                continue;
            flag = 1;
            break;
        }
        else if(nread == 0)
        {
            flag = 0;
            break;
        }

        if(writen(fd, buf, nread) != nread)
        {
            flag = 2;
            break;
        }
    }

    //清理 关闭fd 文件解锁
    close(fd);
    close(sess->data_fd);
    sess->data_fd = -1;

    if (flag == 0){
        return 1;
    }else{
        printf("Something wrong in uploadFIleNobody\n");
        return 2;
    }
    
}