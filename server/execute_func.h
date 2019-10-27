#ifndef EXECUTE_FUNC_H
#define EXECUTE_FUNC_H

#include "session_ftp.h"

void execute_map(Session_t *sess);
void run_user(Session_t *sess);
void run_pass(Session_t *sess);

void run_syst(Session_t *sess);
void run_type(Session_t *sess);
void run_quit(Session_t *sess);
// void run_abor(Session_t *sess);

void run_port(Session_t *sess);
void run_pasv(Session_t *sess);

void run_retr(Session_t *sess);
void run_stor(Session_t *sess);

void run_mkd(Session_t *sess);
void run_cwd(Session_t *sess);
void run_pwd(Session_t *sess);
void run_list(Session_t *sess);
void run_rmd(Session_t *sess);
void run_rnfr(Session_t *sess);
void run_rnto(Session_t *sess);



#endif