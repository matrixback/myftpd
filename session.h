/*将每一个用户连接当做一个会话，创建一个结构体的会话对象。
 *此对象是整个程序的核心，是最重要的数据结构。
 *因为每个用户连接都要开一个进程来处理作业，所以每个进程共享
 *这个会话，将其作为全局对象，在函数中用指针引用。
 *为方便使用及程序的简洁性，定义为 session_t 数据类型。
 */

#ifndef _SESSION_H_
#define _SESSION_H_

#include "def.h"

typedef struct session
{

	uid_t uid;  //用户信息

	int ctrl_fd;	//控制连接和数据连接套接字
	int data_fd;

	int user_fd;	//UNIX 域套接字
	int nobody_fd;

	char cmdline[MAX_CMD_LINE]; //会话的命令
	char cmd[MAX_CMD];
	char arg[MAX_ARG];

	struct sockaddr_in *port_addr;	//port命令时用
	int pasv_listen_fd;				//pasv模式时用

	int is_ascii;	//ftp 的一些状态有关
	long long restart_pos;	//端点续传
	char *rnfr_name;	//重命名

	unsigned int upload_rate_max; 	//上传及下载速度
	unsigned int download_rate_max;
	long transfer_start_sec;
	long transfer_start_usec;
}session_t;

// 初始化会话函数只是简单地将会话对象的各个值设置一下，后面在需要时会详细设置的。
void init_session(session_t* sess);

/* 将会话跑起来，会建立两个进程，euid 分别是登录用户，nobody（分别在ftp.h nobody.h 中定义其行为）。
 * 其中登录用户进程处理 ftp 的核心工作，即控制连接和数据连接。
 * nobody 进程用于辅助完成任务，准确来说就是帮助数据连接绑定 20 端口，
 * 因为不能给予连接的用户太多的权限，或者说普通用户不能绑定 20 端口，给予nobody进程这个权限让其辅助完成。
 * 两个进程间通过 UNIX 域套接字通信，在 unsock.h (即user-nobody sock)中声明了通信的函数。
 */
void run_session(session_t *sess);
 
#endif
