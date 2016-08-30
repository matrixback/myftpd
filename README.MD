#myftpd

简单的 FTP 服务器，支持 port/pasv 模式，断点续传与下载。

##服务器工作流程：

1. root先建立一个服务器，等待连接。连接进来时，创建一个子进程进行处理。

2. 在子进程中再创建一个子进程，将两个进程的有效用户名分别设为 user（需要客户端传入），nobody。 

   user 进程用于主要的数据传输和控制连接，nobody 进程用于打开 21 端口。两个进程通过上面创建的 UNIX 域套接字通信。

##文件说明

def.h                         一些常用的头文件
tool.h      tool.c 				网络编程常用的一些工具函数，比如tcp_server, readn等。
session.h   session.c 			会话
ftpcmd.h    ftpcmd.c          处理user进程，实现了常用的tp命令
nobody.h    nobody.c          处理nobody 进程
unsock.h    unsock.c			   处理nser 和nobody 进程的会话，实现了一个简单的会话协议
