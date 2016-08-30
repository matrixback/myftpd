#ifndef _UN_SOCK_H_
#define _UN_SOCK_H_

#include "session.h"

#define UNSOCK_PORT_GET_DATA_FD 	1
#define UNSOCK_PASV_LISTEN	2
#define UNSOCK_PASV_GET_ACCEPT_FD	3

#define UNSOCK_RESULT_OK			1
#define UNSOCK_RESULT_BAD			2

void unsock_send_cmd(int fd, char cmd);
char unsock_get_cmd(int fd);
void unsock_send_result(int fd, char res);
char unsock_get_result(int fd);

void unsock_send_int(int fd, int the_int);
int unsock_get_int(int fd);
void unsock_send_buf(int fd, const char *buf, unsigned int len);
void unsock_recv_buf(int fd, char *buf, unsigned int len);
void unsock_send_fd(int sock_fd, int fd);
int unsock_recv_fd(int sock_fd);

#endif
