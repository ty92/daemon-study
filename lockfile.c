/**
 * if want to exec the program(only it), need to add main function,
 * and use superuser to exec it
 * 进程退出，则该进程加的锁自动失效
 * 关闭该文件描述符fd，加的锁失效
 * 锁的状态不会被子进程继承
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <sys/stat.h>

#define LOCKFILE "/var/run/daemon.pid" 
#define LOCKMODE (S_IRUSR |S_IWUSR | S_IRGRP | S_IROTH)

extern int lockfile(int);

int already_running(void)
{
	int fd;
	char buf[16];
	struct flock lock;
	
	fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);
	if(fd < 0) {
		printf("open\n");
		syslog(LOG_ERR, "can't open %s:%s",LOCKFILE,strerror(errno));
		exit(1);
	}

	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;

	if(fcntl(fd,F_SETLK,&lock) < 0) {   //判断文件锁能否获取成功
		printf("fcntl\n");
		if(errno == EACCES || errno == EAGAIN) {
			close(fd);
			return(1);
		}
		syslog(LOG_ERR,"can't lock %s:%s",LOCKFILE,strerror(errno));
		exit(1);
	}
	syslog(LOG_ERR,"over");
	ftruncate(fd,0);   //将文件长度截断为0
	sprintf(buf,"%ld",(long)getpid());
	write(fd,buf,strlen(buf)+1);
	return(0);
}

/*
main()
{	
	int i;
	i = already_running();
	printf("i = %d\n",i);
	while(1);  //第一个进程持有文件锁，不退出，第二个进程运行该程序获取文件锁会失败
}
*/
