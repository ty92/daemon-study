CC=gcc
CFLAGS=-Wall -g
SRCS=daemonize.c lockfile.c daemon_reread_conf.c
OBJS=daemonize.o lockfile.o daemon_reread_conf.o

all:daemon_reread_conf
daemon_reread_conf:$(OBJS)
	$(CC) $(CFLAGS) $(SRCS) -o daemon_reread_conf -pthread
clean:
	rm -fr $(OBJS) daemon_reread_conf
