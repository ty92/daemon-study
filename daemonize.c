/**
 * only daemonize.c want to run, need to add main function
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>

#define oops(m) {perror(m); exit(1);}

void daemonize(const char * cmd)
{
	int i, fd0, fd1, fd2;
	pid_t pid;
	struct rlimit r1;
	struct sigaction sa;
	
//clear file creation mask
	umask(0);
	
//Become a session leader to close controlling TTY
	if((pid = fork()) < 0)
		syslog(LOG_ERR,"canot fork");
	else if(pid != 0)
		exit(0);
	setsid();

//Ensure future opens won't allocate controlling TTYs
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if(sigaction(SIGHUP, &sa, NULL))
		syslog(LOG_ERR,"cannot ignore SIGHUP");
	if ((pid = fork()) < 0)
		syslog(LOG_ERR,"cannot fork");
	else if(pid != 0)
		exit(0);

//Change the current working directory to the root so
//wo don't prevent file systems from being unmounted.
	if(chdir("/") < 0)
		syslog(LOG_ERR,"can't change directory to '/'");
	
//Get maximum number of file descriptors
	if(getrlimit(RLIMIT_NOFILE, &r1) < 0)
		syslog(LOG_ERR,"can't get file limit");
//Close all open file descriptors
	if(r1.rlim_max == RLIM_INFINITY)
		r1.rlim_max = 1024;
	for(i = 0; i < r1.rlim_max; i++)
		close(i);

//attach file descriptors 0,1 and 2 to /dev/null
	fd0 = open("/dev/null",O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

//Initialize the log file
	openlog(cmd, LOG_CONS, LOG_DAEMON);
	if(fd0 != 0 || fd1 != 1 || fd2 != 2) {
		syslog(LOG_ERR, "unexpected file descriptors %d %d %d", fd0, fd1, fd2);
		exit(1);
	}
}
/*
main()
{
	daemonize("ls");
	printf("daemonize\n");   //here it is a daemon program, not has tty, not output
	sleep(60);  //need main function into sleep,otherwise not see a daemon
}
*/
