#include "ftpcmd.h"
#include "def.h"
#include "session.h"
#include "tool.h"
#include "unsock.h"

void reply_cli(session_t* sess, int status, const char* info);
void lreply_cli(session_t *sess, int status, const char *info);
static void exec_user(session_t *sess);
static void exec_pass(session_t *sess);
static void exec_type(session_t *sess);
static void exec_pwd(session_t *sess);
static void exec_mkd(session_t *sess);
static void exec_rmd(session_t *sess);
static void exec_dele(session_t *sess);
static void exec_syst(session_t *sess);
static void exec_feat(session_t *sess);
static void exec_size(session_t *sess);
static void exec_port(session_t *sess);
static void exec_pasv(session_t *sess);
static void exec_type(session_t *sess);
static void exec_cwd(session_t *sess);
static void exec_cdup(session_t *sess);
static void exec_quit(session_t *sess);
static void exec_stru(session_t *sess);
static void exec_mode(session_t *sess);
static void exec_retr(session_t *sess);
static void exec_stor(session_t *sess);
static void exec_appe(session_t *sess);
static void exec_list(session_t *sess);
static void exec_nlst(session_t *sess);
static void exec_rest(session_t *sess);
static void exec_abor(session_t *sess);
static void exec_rnfr(session_t *sess);
static void exec_rnto(session_t *sess);
static void exec_site(session_t *sess);
static void exec_stat(session_t *sess);
typedef struct ftpcmd
{
	const char* cmd;		
	void (*exec_cmd)(session_t* sess);		// 执行命令函数指针

}ftpcmd_t;

// 命令数组，用于“根据命令字符串查找对应的函数”。
static ftpcmd_t ftp_cmds[] = {		
	// 访问控制命令 
	
	{"USER",	exec_user	},
	{"PASS",	exec_pass	},

	{"CWD",		exec_cwd	},
	{"XCWD",	exec_cwd	},
	{"CDUP",	exec_cdup	},
	{"XCUP",	exec_cdup	},
	{"QUIT",	exec_quit	},
	{"ACCT",	NULL	},
	{"SMNT",	NULL	},
	{"REIN",	NULL	},

// 传输参数命令 
	{"PORT",	exec_port	},
	{"PASV",	exec_pasv	},
	{"TYPE",	exec_type	},
	{"STRU",	exec_stru	},
	{"MODE",	exec_mode	},

	// 服务命令
	{"RETR",	exec_retr	},
	{"STOR",	exec_stor	},
	{"APPE",	exec_appe	},
	{"LIST",	exec_list	},
	{"NLST",	exec_nlst	},
	{"REST",	exec_rest	},
	{"ABOR",	exec_abor	},
	{"\377\364\377\362ABOR", exec_abor},
	{"PWD",		exec_pwd	},
	{"XPWD",	exec_pwd	},
	{"MKD",		exec_mkd	},
	{"XMKD",	exec_mkd	},
	{"RMD",		exec_rmd	},
	{"XRMD",	exec_rmd	},
	{"DELE",	exec_dele	},
	{"RNFR",	exec_rnfr	},
	{"RNTO",	exec_rnto	},
	{"SYST",	exec_syst	},
	{"FEAT",	exec_feat },
	{"SIZE",	exec_size	},
	{"STOU",	NULL	},
	{"ALLO",	NULL	}
	
};

// 解析命令。
//  命令有效返回 1， 否则返回 0.
int is_valid_cmdline(char* cmdline);
void trim_crlf_cmdline(char* cmdline);
void get_cmd(char* cmd, char* cmdline);
void get_arg(char* arg, char* cmdline);
void upper_cmd(char* cmd);

int is_valid_cmdline(char* cmdline)
{
	int size = strlen(cmdline);
	if(cmdline[size - 1] != '\n' && cmdline[size -2] != '\r')
		return 1;
	else
		return 0;
}

void trim_crlf_cmdline(char* cmdline)
{
	int length = strlen(cmdline);
	char* p = cmdline + length - 1;
	while(*p == '\r' || *p == '\n')
		(*p--) = '\0';
}

void get_cmd(char* cmd, char* cmdline)
{
	char* space = strchr(cmdline, ' ');
	if(space == NULL)
		strcpy(cmd, cmdline);	// 会将'\0'复制到 cmd 中
	else
	{
		int size = space - cmdline;
		strncpy(cmd, cmdline, size);	// 不会自动添加 '\0'，手动添加
		cmd[size] = '\0';			
	}
}
void get_arg(char* arg, char* cmdline)
{
	char* space = strchr(cmdline, ' ');
	if(space != NULL)
	{
		strcpy(arg, space + 1);	// 会自动添加 '\0'
	}
}
void upper_cmd(char* cmd)
{
	while(*cmd != '\0')
	{
		*cmd = toupper(*cmd);
		cmd++;
	}
}
//命令解析结束


void run_user(session_t *sess)
{
	reply_cli(sess, FTP_GREET, "(matrixftpd 1.0)");
	int ret;
	while(1)
	{
		bzero(sess->cmdline, sizeof(sess->cmdline));
		bzero(sess->cmd, sizeof(sess->cmd));
		bzero(sess->arg, sizeof(sess->arg));
		
		ret = readline(sess->ctrl_fd, sess->cmdline, MAX_CMD_LINE);
		if(ret == -1)
			err_exit("readline");
		else if(ret == 0)
			exit(0);       // 客户端关闭，正常退出进程。
		// 解析读到的命令
		trim_crlf_cmdline(sess->cmdline);
		printf("cmdline=[%s]\n", sess->cmdline);

		get_cmd(sess->cmd, sess->cmdline);
		get_arg(sess->arg, sess->cmdline);
		printf("cmd=[%s] arg=[%s]\n", sess->cmd, sess->arg);

		upper_cmd(sess->cmd);
		
		// 根据命令查找对应的函数。
		int cmds_size = sizeof(ftp_cmds) / sizeof(ftpcmd_t);
		int i;
		for(i = 0; i < cmds_size; i++)
		{
			if(strcmp(ftp_cmds[i].cmd, sess->cmd) == 0)
			{
				if(ftp_cmds[i].exec_cmd != NULL)
				{
					ftp_cmds[i].exec_cmd(sess);		// 找到命令后，直接执行。
				}
				else
				{
					reply_cli(sess, FTP_COMMANDNOTIMPL, "Unimplement command.");
				}
				break;
			}
		}

		if (i == cmds_size)
		{
			reply_cli(sess, FTP_BADCMD, "Unknown command.");
			printf("not find cmd: %s\n", sess->cmd);
		}
	}		
}

void reply_cli(session_t* sess, int status, const char* info)
{
	char buf[1024] = {0};
	sprintf(buf, "%d %s\r\n", status, info);   // 用 sprintf 格式化字符串，一起发送
	writen(sess->ctrl_fd, buf, strlen(buf));
}

void lreply_cli(session_t *sess, int status, const char *info)
{
	char buf[1024] = {0};
	sprintf(buf, "%d-%s\r\n", status, info);  // notice: difference from the reply_cli, statuc linked with info by '-', indicate there is a few lines.
	writen(sess->ctrl_fd, buf, strlen(buf));
}

static void exec_user(session_t *sess)
{
	struct passwd* pw = getpwnam(sess->arg);
	if(pw == NULL)
	{
		reply_cli(sess, FTP_LOGINERR, "Login incorrect.");
		return;			// 不能直接 exit，万一客户端重新给个用户名
	}
	sess->uid = pw->pw_uid;
	reply_cli(sess, FTP_GIVEPWORD, "Please specify the password.");
}
static void exec_pass(session_t *sess)
{
	struct passwd* pw = getpwuid(sess->uid);
	if(pw == NULL)
	{
		reply_cli(sess, FTP_LOGINERR, "Login incorrect.");
		return;
	}
	printf("name=[%s]\n", pw->pw_name);

	struct spwd* sp = getspnam(pw->pw_name);
	if(sp == NULL)
	{
		reply_cli(sess, FTP_LOGINERR, "Login incorrect.");
		return;
	}
	char* pass = crypt(sess->arg, sp->sp_pwdp);			// crypt 函数，第一个参数是 key， 第二个参数是 salt，会智能地直接从密文中获取 salt。
	if(strcmp(pass, sp->sp_pwdp) != 0)
	{
		reply_cli(sess, FTP_LOGINERR, "Login incorrect.");
		return;
	}

	setegid(pw->pw_gid);
	seteuid(pw->pw_uid);
	chdir(pw->pw_dir);
	reply_cli(sess, FTP_LOGINOK, "Login successful.");
}

// 服务器的系统
static void exec_syst(session_t* sess)
{
	reply_cli(sess, FTP_SYSTOK, "UNIX Type: L8");
}

// ftp 服务器有哪些特色
static void exec_feat(session_t* sess)
{
	lreply_cli(sess, FTP_FEAT, "Features:");	// a few lines, use lreply
	writen(sess->ctrl_fd, " EPRT\r\n", strlen(" EPRT\r\n"));
	writen(sess->ctrl_fd, " EPSV\r\n", strlen(" EPSV\r\n"));
	writen(sess->ctrl_fd, " MDTM\r\n", strlen(" MDTM\r\n"));
	writen(sess->ctrl_fd, " PASV\r\n", strlen(" PASV\r\n"));
	writen(sess->ctrl_fd, " REST STREAM\r\n", strlen(" REST STREAM\r\n"));
	writen(sess->ctrl_fd, " SIZE\r\n", strlen(" SIZE\r\n"));
	writen(sess->ctrl_fd, " TVFS\r\n", strlen(" TVFS\r\n"));
	writen(sess->ctrl_fd, " UTF8\r\n", strlen(" UTF8\r\n"));
	reply_cli(sess, FTP_FEAT, "End");
}

// 文件的大小
static void exec_size(session_t* sess)
{
	struct stat buf;
	if(stat(sess->arg, &buf) < 0)
	{
		reply_cli(sess, FTP_FILEFAIL, "SIZE operation failed.");
		return;
	}
	if(!S_ISREG(buf.st_mode))
	{
		reply_cli(sess, FTP_FILEFAIL, "Could not get file size.");
		return;
	}

	char text[1024] = {0};
	sprintf(text, "%lld", (long long)buf.st_size);
	reply_cli(sess, FTP_SIZEOK, text);
}

// 删除一个文件
static void exec_dele(session_t* sess)
{
	if(unlink(sess->arg) < 0)
	{
		reply_cli(sess, FTP_FILEFAIL, "Delete operation failed");
		return;
	}
	reply_cli(sess, FTP_DELEOK, "Delete operation successfully");
}
// 删除一个目录
static void exec_rmd(session_t* sess)
{
	if(rmdir(sess->arg) < 0)
	{
		reply_cli(sess, FTP_FILEFAIL, "Remoce dir operation failed");
		return;
	}
	reply_cli(sess, FTP_RMDIROK, "Romove dir operation successfully");
}

// 创建一个目录
static void exec_mkd(session_t* sess)
{
	if(mkdir(sess->arg, 0777) < 0)
	{
		reply_cli(sess, FTP_FILEFAIL, "Create dir operation failed");
		return;
	}
	// mkdir 函数会根据 pathname 提供的绝对或相对路径建立目录
	char path[1024] = {0};
	if(sess->arg[0] == '/')  // 绝对路径直接写入,相对路径先获取当前路径名，再拼接字符串
	{
		sprintf(path, "%s created", sess->arg);
	}
	else
	{
		char cur_dir[1024];
		getcwd(cur_dir, 1024);	
		// getcwd 获得的路径末尾没有带 '/'
		sprintf(path, "%s/%s created", cur_dir, sess->arg);
	}
	reply_cli(sess, FTP_MKDIROK, path);
}

static void exec_type(session_t* sess)
{
	if(strcmp(sess->arg, "A") == 0)
	{
		sess->is_ascii = 1;
		reply_cli(sess, FTP_TYPEOK, "Switching to ASCII mode.");
	}
	else if(strcmp(sess->arg, "I") == 0)
	{
		sess->is_ascii = 0;
		reply_cli(sess, FTP_TYPEOK, "Switching to Binary mode.");
	}
	else
	{
		reply_cli(sess, FTP_BADCMD, "Unrecognised TYPE command.");
	}
}


static void exec_cwd(session_t *sess)
{
	if(chdir(sess->arg) < 0)
 	{
		reply_cli(sess, FTP_FILEFAIL, "Failed to change dir\n");
	}

	reply_cli(sess, FTP_CWDOK, "Directory succesfully changed\n");
}

static void exec_cdup(session_t *sess)
{
	if (chdir("..") < 0)
	{
		reply_cli(sess, FTP_FILEFAIL, "Failed to change directory.");
		return;
	}

	reply_cli(sess, FTP_CWDOK, "Directory successfully changed.");
}

// 
int port_get_data_fd(session_t *sess)	// 取得 port 模式下的 data_fd。
{
	unsock_send_cmd(sess->nobody_fd, UNSOCK_PORT_GET_DATA_FD);
	// 先发送 port, 然后发送 ip。
	unsigned short port = ntohs(sess->port_addr->sin_port);
	char *ip = inet_ntoa(sess->port_addr->sin_addr);
	unsock_send_int(sess->nobody_fd, (int)port);
	unsock_send_buf(sess->nobody_fd, ip, strlen(ip));

	char res = unsock_get_result(sess->nobody_fd);
	printf("port get data fd result is %d\n", res);
	if(res == UNSOCK_RESULT_BAD)
		return 0;
	else if(res == UNSOCK_RESULT_OK)
		sess->data_fd = unsock_recv_fd(sess->nobody_fd);	
	return 1;
}

static void exec_port(session_t *sess)
{
	struct sockaddr_in* addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;

	// 将发送过来的 ip, port 字符串先格式化为数字 
	unsigned int v[6];
	sscanf(sess->arg, "%u,%u,%u,%u,%u,%u", &v[2], &v[3], &v[4], &v[5], &v[0], &v[1]);

	unsigned char *p = (unsigned char *)&addr->sin_port;
	p[0] = v[0];
	p[1] = v[1];

	p = (unsigned char *)&addr->sin_addr;
	p[0] = v[2];
	p[1] = v[3];
	p[2] = v[4];
	p[3] = v[5];

	sess->port_addr = addr;

	// 此处先不建立连接，等到需要 data_fd 的时候，再调用 get_port_fd 要求 nobody 进程建立连接。
	reply_cli(sess, FTP_PORTOK, "PORT command successful. Consider using PASV.");
}

int pasv_get_data_fd(session_t *sess)
{
	unsock_send_cmd(sess->nobody_fd, UNSOCK_PASV_GET_ACCEPT_FD);
	char res = unsock_get_result(sess->nobody_fd);
	if(res == UNSOCK_RESULT_BAD)
		return 0;
	else if(res == UNSOCK_RESULT_OK)
		sess->data_fd = unsock_recv_fd(sess->nobody_fd);
	return 1;
}

static void exec_pasv(session_t *sess)
{
	unsock_send_cmd(sess->nobody_fd, UNSOCK_PASV_LISTEN);		// 让 nobody 去建立一个 20 端口的服务器，返回结果。
	char res = unsock_get_result(sess->nobody_fd);
	if(res == UNSOCK_RESULT_BAD)
	{
		reply_cli(sess, FTP_EPSVBAD, "Server error, Can not build a connection");
		return ;
	}	
	else if(res == UNSOCK_RESULT_OK)
	{
		// 设置 sess 的 listenfd 为 1，这个只是为 is_pasv_active 函数用。真正的连接的listenfd 在 nobody 进程中
		sess->pasv_listen_fd = 1;

		// 将 ip，20端口发给 客户端
		char ip[16] = "192.168.64.32";
	    unsigned int v[4];
	    sscanf(ip, "%u.%u.%u.%u", &v[0], &v[1], &v[2], &v[3]);

	    unsigned short port = (int)20;

	    char info[1024] = {0};
	    sprintf(info, "Entering Passive Mode (%u,%u,%u,%u,%u,%u).", v[0], v[1], v[2], v[3], port>>8, port&0xFF);
	     
	    reply_cli(sess, FTP_PASVOK, info);
	}
}

int is_port_active(session_t* sess)
{
	return sess->port_addr != NULL;
}

int is_pasv_active(session_t* sess)
{
	return sess->pasv_listen_fd == 1;
}

int get_data_fd(session_t* sess)		// get data fd from nobody process depend on difference mode
{
	if(is_port_active(sess) && is_pasv_active(sess))
	{
		fprintf(stderr, "Both port and pasv are active");
		exit(1);
	}
	else if(is_port_active(sess))
	{
		printf("port active, geting data fd...\n");
		return port_get_data_fd(sess);
	}
	else if(is_pasv_active(sess))
	{
		printf("pasv active, geting data fd...\n");
		return pasv_get_data_fd(sess);
	}

	return 0;
}

static void exec_quit(session_t *sess)
{
	close(ctrl_fd);
	exit(0);
}
static void exec_stru(session_t *sess)
{

}
static void exec_mode(session_t *sess)
{

}
static void exec_retr(session_t *sess)// retransfer a fle, download 
{
	if(get_data_fd(sess) == 0)
	{
		printf("get data fd error\n");
		return;
	}
	
	//get offset, the features can support 断点传送
	long long offset = sess->restart_pos;
	sess->restart_pos = 0;

	int fd = open(sess->arg, O_RDONLY);
	if(fd == -1)
	{
		reply_cli(sess, FTP_FILEFAIL, "Failed to open file.");
		return;
	}
	
	int ret;
	// add a read lock
	ret = lock_file_read(fd);
	if(ret == -1)
	{
		reply_cli(sess, FTP_FILEFAIL, "Failed to open file.");
		return;
	}
	
	// is a regular file?
	struct stat sbuf;
	ret = fstat(fd, &sbuf);
	if(!S_ISREG(sbuf.st_mode))
	{
		reply_cli(sess, FTP_FILEFAIL, "Failed to open file.");
		return;
	}
	
	if(offset != 0)
	{
		ret = lseek(fd, offset, SEEK_SET);
		if(ret == -1)
		{
			reply_cli(sess, FTP_FILEFAIL, "Failed to open file.");
			return;
		}
	}
	
	// return a reply, tell them start to transfer, as to is_ascii, no useful.
	// either ascii or binary, the transfer way is binary. 
	char info[1024] = {0};
	if(sess->is_ascii)
	{
		sprintf(info, "Opening ASCII mode data connection for %s (%lld bytes).",
			sess->arg, (long long)sbuf.st_size);
	}
	else
	{
		sprintf(info, "Opening BINARY mode data connection for %s (%lld bytes).",
			sess->arg, (long long)sbuf.st_size);
	}

	reply_cli(sess, FTP_DATACONN, info);
	
	// start to transfer
	char buf[4096];
	int nread;
	while(1)
	{
		nread = read(fd, buf, sizeof(buf));
		if(nread == 0)
			break;
		if(writen(sess->data_fd, buf, nread) != nread)
		{
			printf("writen error\n");
			return;
		}
	}
	close(fd);
	reply_cli(sess, FTP_TRANSFEROK, "Transfer complete.");
}

// uppload a file, support 断点上传, 不支持 append。
static void exec_stor(session_t *sess)
{
	if(get_data_fd(sess) == 0)
	{
		fprintf(stderr, "get_data_fd error\n");
		return;
	}
	

	int fd = open(sess->arg, O_CREAT | O_WRONLY, 0666);
	if(fd == -1)
	{
		reply_cli(sess,  FTP_UPLOADFAIL, "Could not create file.");
		return;
	}
	
	int ret;
	ret = lock_file_write(fd);
	if(ret == -1)
	{
		reply_cli(sess, FTP_UPLOADFAIL, "Could not create file.");
		return;
	}
	
	long long offset = sess->restart_pos;
	sess->restart_pos = 0;
	if(lseek(fd, offset, SEEK_SET) < 0)
	{
		reply_cli(sess, FTP_UPLOADFAIL, "Could not create file.");
			return;
	}
	
	struct stat sbuf;
	ret = fstat(fd, &sbuf);
	if(ret == -1)
	{
		fprintf(stderr, "fstat error\n");
		return;	
	}
	
	char text[1024] = {0};
	if(sess->is_ascii)
	{
		sprintf(text, "Opening ASCII mode data connection for %s (%lld bytes).",
			sess->arg, (long long)sbuf.st_size);
	}
	else
	{
		sprintf(text, "Opening BINARY mode data connection for %s (%lld bytes).",
			sess->arg, (long long)sbuf.st_size);
	}

	reply_cli(sess, FTP_DATACONN, text);
	
	// upload start	
	int flag = 0;
	char buf[1024];
	while(1)
	{
		ret = read(sess->ctrl_fd, buf, sizeof(buf));
		if(ret == -1)
		{
			if(errno == EINTR)
				continue;
			else
			{
				flag = 2;
				break;
			}
		}
		else if(ret == 0)
		{
			flag == 0;
			break;
		}
		
		if(writen(fd, buf, ret) != ret)
		{
			flag = 1;
			break;
		}
	}
	
	close(sess->data_fd);
	sess->data_fd = -1;
	close(fd);
	
	if(flag == 0)
	{
		reply_cli(sess, FTP_TRANSFEROK, "Transfer complete.");
	}
	else if (flag == 1)
	{
		reply_cli(sess, FTP_BADSENDFILE, "Failure writting to local file.");
	}
	else if (flag == 2)
	{
		reply_cli(sess, FTP_BADSENDNET, "Failure reading from network stream.");
	}
}
static void exec_appe(session_t *sess)
{

}

static int list_dir(session_t* sess)
{
	DIR *dir = opendir(".");
	if (dir == NULL)
	{
		return 0;
	}

	struct dirent *dt;
	struct stat sbuf;
	while ((dt = readdir(dir)) != NULL)
	{
		if (lstat(dt->d_name, &sbuf) < 0)
		{
			continue;
		}
		if (dt->d_name[0] == '.')
			continue;

		char perms[] = "----------";
		perms[0] = '?';

		mode_t mode = sbuf.st_mode;
		switch (mode & S_IFMT)
		{
		case S_IFREG:
			perms[0] = '-';
			break;
		case S_IFDIR:
			perms[0] = 'd';
			break;
		case S_IFLNK:
			perms[0] = 'l';
			break;
		case S_IFIFO:
			perms[0] = 'p';
			break;
		case S_IFSOCK:
			perms[0] = 's';
			break;
		case S_IFCHR:
			perms[0] = 'c';
			break;
		case S_IFBLK:
			perms[0] = 'b';
			break;
		}

		if (mode & S_IRUSR)
		{
			perms[1] = 'r';
		}
		if (mode & S_IWUSR)
		{
			perms[2] = 'w';
		}
		if (mode & S_IXUSR)
		{
			perms[3] = 'x';
		}
		if (mode & S_IRGRP)
		{
			perms[4] = 'r';
		}
		if (mode & S_IWGRP)
		{
			perms[5] = 'w';
		}
		if (mode & S_IXGRP)
		{
			perms[6] = 'x';
		}
		if (mode & S_IROTH)
		{
			perms[7] = 'r';
		}
		if (mode & S_IWOTH)
		{
			perms[8] = 'w';
		}
		if (mode & S_IXOTH)
		{
			perms[9] = 'x';
		}
		if (mode & S_ISUID)
		{
			perms[3] = (perms[3] == 'x') ? 's' : 'S';
		}
		if (mode & S_ISGID)
		{
			perms[6] = (perms[6] == 'x') ? 's' : 'S';
		}
		if (mode & S_ISVTX)
		{
			perms[9] = (perms[9] == 'x') ? 't' : 'T';
		}

		char buf[1024] = {0};
		int off = 0;
		off += sprintf(buf, "%s ", perms);
		off += sprintf(buf + off, " %3d %-8d %-8d ", (int)sbuf.st_nlink, (int)sbuf.st_uid, (int)sbuf.st_gid);
		off += sprintf(buf + off, "%8lu ", (unsigned long)sbuf.st_size);

		const char *p_date_format = "%b %e %H:%M";
		struct timeval tv;
		gettimeofday(&tv, NULL);
		time_t local_time = tv.tv_sec;
		if (sbuf.st_mtime > local_time || (local_time - sbuf.st_mtime) > 60*60*24*182)
		{
			p_date_format = "%b %e  %Y";
		}

		char datebuf[64] = {0};
		struct tm* p_tm = localtime(&local_time);
		strftime(datebuf, sizeof(datebuf), p_date_format, p_tm);
		off += sprintf(buf + off, "%s ", datebuf);
		if (S_ISLNK(sbuf.st_mode))
		{
			char tmp[1024] = {0};
			readlink(dt->d_name, tmp, sizeof(tmp));
			off += sprintf(buf + off, "%s -> %s\r\n", dt->d_name, tmp);
		}
		else
		{
			off += sprintf(buf + off, "%s\r\n", dt->d_name);
		}


		//printf("%s", buf);
		writen(sess->data_fd, buf, strlen(buf));
	}

	closedir(dir);

	return 1;


}
static void exec_list(session_t *sess)
{
	if(get_data_fd(sess) == 0)
	{
		printf("get data fd fail, exit...\n");
		return;
	}
	reply_cli(sess, FTP_DATACONN, "Here comes the directory listing.");
	list_dir(sess);
	close(sess->data_fd);  // 发送完后立即完毕。防止一直连接，浪费资源。
	sess->data_fd = -1;
	reply_cli(sess, FTP_TRANSFEROK, "Directory send OK.");
}
static void exec_nlst(session_t *sess)
{

}
static void exec_rest(session_t *sess)
{

}
static void exec_abor(session_t *sess)
{

}
static void exec_rnfr(session_t *sess)
{

}
static void exec_rnto(session_t *sess)
{

}
static void exec_site(session_t *sess)
{

}
static void exec_stat(session_t *sess)
{
	
}
static void exec_pwd(session_t *sess)
{
	char text[1024] = {0};
	char dir[1024+1] = {0};
	getcwd(dir, 1024);
	sprintf(text, "\"%s\"", dir);

	reply_cli(sess, FTP_PWDOK, text);
}

