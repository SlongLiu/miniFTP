#ifndef HANDLE_FTP_H
#define HANDLE_FTP_H

#include "session_ftp.h"
#include "headers.h"

void handle_proto(Session_t* sess);
void handle_nobody(Session_t* sess);
void set_nobody();
void set_bind_capabilities();
int capset(cap_user_header_t hdrp, const cap_user_data_t datap);


#endif