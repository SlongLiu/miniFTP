#ifndef PASV_FTP_H
#define PASV_FTP_H

#include "session_ftp.h"
#include "headers.h"

void privop_pasv_get_data_sock(Session_t *sess);
void privop_pasv_listen(Session_t *sess);
void privop_pasv_active(Session_t *sess);
void privop_pasv_accept(Session_t *sess);
// void privop_pasv_accept(Session_t *sess);
int transferFIleNobody(Session_t *sess);
int uploadFIleNobody(Session_t *sess, int is_appe);
int transFileList(Session_t *sess);

#endif