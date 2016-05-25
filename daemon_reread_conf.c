/**
 * gcc daemonize.c lockfile.c daemon_reread_conf.c -o daemon_reread_conf -pthread
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <syslog.h>

sigset_t mask;

extern int already_running(void);
extern void daemonize();

void reread(void) {
	syslog(LOG_ERR,"reread");
}

void *thr_fn(void *arg) {
	int err, signo;
	for(;;) {
		err = sigwait(&mask, &signo);	
		if(err != 0) {
			syslog(LOG_ERR, "sigwait failed");
			exit(1);
		}
		switch (signo) {
		case SIGHUP:
			syslog(LOG_INFO,"Re-reading configuration file");
			reread();
			break;
		case SIGTERM:
			syslog(LOG_INFO,"got SIGTERM; exiting");
			exit(0);
		default:
			syslog(LOG_INFO, "unexpected signal %d\n",signo);
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int err;
	pthread_t tid;
	char *cmd;
	struct sigaction sa;
	
	if((cmd = strrchr(argv[0],'/')) == NULL)
		cmd = argv[0];
	else	
		cmd++;

	printf("test1\n");
	daemonize(cmd);
	printf("test2\n");

	if(already_running()) {
		syslog(LOG_ERR,"daemon already running");
		exit(1);
	}
	syslog(LOG_CRIT,"13-7");

	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if(sigaction(SIGHUP, &sa, NULL) < 0)
		syslog(LOG_ERR,"can't restore SIGHUP default");
	sigfillset(&mask);
	if((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0)
		syslog(LOG_ERR,"SIG_BLOCK error");
	
	err = pthread_create(&tid, NULL, thr_fn, 0);
	if(err != 0)
		syslog(LOG_ERR, "can't create thread");

	syslog(LOG_CRIT,"13-7 over");
	sleep(60);
	exit(0);
}
