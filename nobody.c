#include "nobody.h"
#include "session.h"
#include "unsock.h"
#include "tool.h"
#include "def.h"

static void do_port_get_data_fd(session_t *sess);
static void do_pasv_listen(session_t* sess);
static void do_pasv_get_accept_fd(session_t* sess);
// 设置有效用户为 nobody， 并更改 Nobody 的权限，只允许其可拥有绑定网络接口的权限。
int capset(cap_user_header_t hdrp, const cap_user_data_t datap)
{
	return syscall(__NR_capset, hdrp, datap);
}

void limit_privilege(void)
{
	struct passwd *pw = getpwnam("nobody");
	if (pw == NULL)
		return;

	if (setegid(pw->pw_gid) < 0)
		err_exit("setegid");
	if (seteuid(pw->pw_uid) < 0)
		err_exit("seteuid");


	struct __user_cap_header_struct cap_header;
	struct __user_cap_data_struct cap_data;

	memset(&cap_header, 0, sizeof(cap_header));
	memset(&cap_data, 0, sizeof(cap_data));

	cap_header.version = _LINUX_CAPABILITY_VERSION_1;
	cap_header.pid = 0;

	__u32 cap_mask = 0;
	cap_mask |= (1 << CAP_NET_BIND_SERVICE);

	cap_data.effective = cap_data.permitted = cap_mask;
	cap_data.inheritable = 0;

	capset(&cap_header, &cap_data);
}


void run_nobody(session_t *sess)
{
	limit_privilege();
	tcp_server(50);
	while(1)
	{
		char cmd;
		while(1)
		{
			cmd = unsock_get_cmd(sess->user_fd);
			if(cmd ==  UNSOCK_PORT_GET_DATA_FD)
				do_port_get_data_fd(sess);
			else if(cmd == UNSOCK_PASV_LISTEN)
				do_pasv_listen(sess);
			else if(cmd == UNSOCK_PASV_GET_ACCEPT_FD)
				do_pasv_get_accept_fd(sess);
		}
	}
}

// port 模式，接收客户端 ip，然后建立服务器建立一个“客户端”，绑定 20 接口，连接客户，将连接结果发送给 user process.
static void do_port_get_data_fd(session_t *sess)
{
	unsigned short port = (unsigned short)unsock_get_int(sess->user_fd);
	char ip[16] = {0};
	unsock_recv_buf(sess->user_fd, ip, sizeof(ip));
	
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons(port);

	printf("in nobody, before sock bind 20 \n");
	int fd = sock_bind_20();
	printf("in nobody, do port, sock has bind, sock: %d\n", fd);
	if(fd == -1)
	{
		printf("fd: %d\n", fd);
		unsock_send_result(sess->user_fd, UNSOCK_RESULT_BAD);
		return;
	}
	if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{	
		printf("connect fail\n");
		close(fd);	
		unsock_send_result(sess->user_fd, UNSOCK_RESULT_BAD);
		return;
	}
	printf("in nobody, has connected client\n");
	sess->data_fd = fd;			//建立的同时设置 data_fd
	unsock_send_result(sess->user_fd, UNSOCK_RESULT_OK);
	printf("send result succ\n");
	unsock_send_fd(sess->user_fd, fd);
	printf("in nobody, do port get data fd has send fd\n");
	close(fd);
}

static void do_pasv_listen(session_t* sess)
{
	printf("in do pasv listen, befor bind 20\n");
	sess->pasv_listen_fd = tcp_server(20);
	printf("in do pasv listen, bind 20 succ\n");
    unsock_send_result(sess->user_fd, UNSOCK_RESULT_OK);	//  不发送 fd,只是发送建立服务器的结果。
}

static void do_pasv_get_accept_fd(session_t* sess)
{
	int fd = accept(sess->pasv_listen_fd, NULL, 0);
	//得到已连接后的套接字后，就将监听套接字关闭，防止其他用户连接。
	close(sess->pasv_listen_fd);
	sess->pasv_listen_fd = -1;

	if (fd == -1)
	{
		unsock_send_result(sess->user_fd, UNSOCK_RESULT_BAD);
		return;
	}

	sess->data_fd = fd;			//建立的同时设置 data_fd
	unsock_send_result(sess->user_fd, UNSOCK_RESULT_OK);
	unsock_send_fd(sess->user_fd, fd);
	close(fd);
}
