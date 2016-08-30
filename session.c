#include "session.h"
#include "def.h"
#include "nobody.h"
#include "ftpcmd.h"

void init_session(session_t* sess)
{
	session_t tmp = 
	{
		0, 				//开始时以 root 用户运行。
		-1, -1, -1, -1,
		"", "", "",
		NULL, -1,
		0, 0, NULL,
		0, 0, 0, 0
	};
	*sess = tmp;
}

void run_session(session_t *sess)
{
	int sockfds[2];		// 建立UNIX 域套接字，进程结束的时候会自动关闭
	if(socketpair(PF_UNIX, SOCK_STREAM, 0, sockfds) < 0)
		err_exit("socketpair");

	pid_t pid = fork();   //建立两个进程，user 进程用于主要的数据传输和控制连接，nobody 进程用于打开端口。
						 //两个进程通过上面创建的 UNIX 域套接字通信。通信协议在 unsock.h 中定义。
	if(pid < 0)
		err_exit("fork");
	if(pid == 0)  // user process，主要工作在 ftpcmd.c 中实现
	{
		close(sockfds[0]);
		sess->nobody_fd = sockfds[1];
		run_user(sess);   //ftpcmd.c 中定义
	}
	else		// nobody precess, 主要工作在 nobody.c 中实现
	{
		close(sockfds[1]);
		sess->user_fd = sockfds[0];
		run_nobody(sess);  //nobody.c 中定义
	}
}
