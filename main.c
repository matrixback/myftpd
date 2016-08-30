#include "def.h"
#include "tool.h"
#include "stdio.h"
#include "session.h"

int main()
{
	if(getuid() != 0)
	{
		fprintf(stderr, "the ftp server must be run as root.\n");
		exit(1);
	}

	//load_conf("./myftp.conf");  //加载配置文件

	signal(SIGCHLD, SIG_IGN);

	int listen_fd = tcp_server(21);
	printf("listenfd: %d\n", listen_fd);
	int conn;
	pid_t pid;
	//can not create object use func;

	while(1)
	{
		conn = accept(listen_fd, NULL, 0);
		if(conn == -1)
			err_exit("accept");
			
		pid = fork();
		if(pid < 0)
			err_exit("fork");
		if(pid == 0)
		{
			session_t sess;
		 	init_session(&sess);
 
			sess.ctrl_fd = conn;
			close(listen_fd);
			run_session(&sess);
		}
		else
		{
			close(conn);
		}
	}

	return 0;
}
