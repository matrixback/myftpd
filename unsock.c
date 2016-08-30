#include "unsock.h"
#include "def.h"
#include "tool.h"
void unsock_send_cmd(int fd, char cmd)
{
	int ret = writen(fd, &cmd, sizeof(cmd));
	if(ret != sizeof(cmd))
	{
		fprintf(stderr, "unsock_send_cmd error\n");
		exit(1);
	}
}
char unsock_get_cmd(int fd)
{
	char res;
	int ret;
	ret = readn(fd, &res, sizeof(res));
	if(ret != sizeof(res))
	{
		fprintf(stderr, "unsock_get_cmd error\n");
		exit(EXIT_FAILURE);
	}

	return res;
}
void unsock_send_result(int fd, char res)
{
	int ret;
	ret = writen(fd, &res, sizeof(res));
	if(ret != sizeof(res))
	{
		fprintf(stderr, "unsock_send_result error\n");
		exit(EXIT_FAILURE);
	}
}
char unsock_get_result(int fd)
{
	char res;
	int ret;
	ret = readn(fd, &res, sizeof(res));
	if(ret != sizeof(res))
	if (ret != sizeof(res))
	{
		fprintf(stderr, "unsock_get_result error\n");
		exit(EXIT_FAILURE);
	}

	return res;
}

void unsock_send_int(int fd, int the_int)
{
	int ret;
	ret = writen(fd, &the_int, sizeof(the_int));
	if(ret != sizeof(the_int))
	{
		fprintf(stderr, "unsock_send_int error\n");
		exit(EXIT_FAILURE);
	}
}
int unsock_get_int(int fd)
{
	int res;
	int ret;
	ret = readn(fd, &res, sizeof(res));
	if(ret != sizeof(res))
	{
		fprintf(stderr, "unsock_get_int error\n");
		exit(1);
	}
	return res;
}
void unsock_send_buf(int fd, const char *buf, unsigned int len)
{
	unsock_send_int(fd, (int)len);
	int ret;
	ret = writen(fd, buf, len);
	if(ret != len)
	{
		fprintf(stderr, "unsock_send_buf error\n");
		exit(1);
	}
}
void unsock_recv_buf(int fd, char *buf, unsigned int len)
{
	unsigned int recv_len = (unsigned int)unsock_get_int(fd);
	if(recv_len > len)
	{
		fprintf(stderr, "unsock_recv_buf error\n");
		exit(1);
	}
	
	int ret = readn(fd, buf, recv_len);
	if(ret != (int)recv_len)
	{
		fprintf(stderr, "unsock_recv_buf error\n");
		exit(EXIT_FAILURE);
	}
}
void unsock_send_fd(int sock_fd, int fd)
{
	send_fd(sock_fd, fd);
}

int unsock_recv_fd(int sock_fd)
{
	return recv_fd(sock_fd);
}

