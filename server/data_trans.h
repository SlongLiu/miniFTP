#ifndef DATA_TRANS_H
#define DATA_TRANS_H

#include "session_ftp.h"

int get_trans_data_fd(Session_t *sess); //返回值表示成功与否
void get_port_data_fd(Session_t *sess);//port下获取文件fd
void get_pasv_data_fd(Session_t *sess);//pasv下传输文件

#endif