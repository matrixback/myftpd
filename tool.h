/* 此工具文件封装了一些网络套接字接口，字符串函数，
 *
 */

#ifndef _TOOL_H_
#define _TOOL_H_

//套接字函数
#include "def.h"
 
int tcp_server(int port);
ssize_t recv_peek(int sockfd, void* buf, size_t len);
ssize_t readline(int sockfd, void *buf, size_t max_line);
ssize_t readn(int fd, void *buf, size_t count);
ssize_t writen(int fd, const void *buf, size_t count);

void send_fd(int sock_fd, int fd);
int recv_fd(const int sock_fd);
int lock_file_read(int fd);
int lock_file_write(int fd);
int unlock_file(int fd);

#endif
