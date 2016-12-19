#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

loff_t payload;
ssize_t outoff=0;

static int usage(char *name) {
	char pre[]="Usage: ",post[]=" <filename>\n";
	struct iovec iov[3]={{pre,sizeof(pre)-1},{0},{post,sizeof(post)-1}};
	iov[1].iov_base=name;
	iov[1].iov_len=strlen(name);
	writev(STDERR_FILENO,iov,3);
	exit(EXIT_FAILURE);
}

int main(int argc,char **argv) {
	struct pollfd fds[]={{STDIN_FILENO,POLLIN,0},{STDOUT_FILENO,0,0}};
	int fd,quit=0;
	ssize_t r;

	if (argc!=2) usage(*argv);
	if ((fd=open(argv[1],O_RDWR|O_CREAT|O_TRUNC,0664))==-1) {
		perror(argv[1]);
		exit(EXIT_FAILURE);
	}
	if (fcntl(STDIN_FILENO,F_SETFL,O_NONBLOCK)==-1 ||
		fcntl(STDOUT_FILENO,F_SETFL,O_NONBLOCK)==-1 ||
	fcntl(fd,F_SETFL,O_NONBLOCK)==-1) {
		perror("set non-blocking I/O");
		exit(EXIT_FAILURE);
	}
	while(!quit) {
		poll(fds,2,-1);
		if (fds[0].revents) {
			// Read more into file
			r=splice(STDIN_FILENO,NULL,fd,&payload,1<<20,SPLICE_F_NONBLOCK);
			fds[1].revents=POLLOUT;
			if (r==0) {
				quit=1;
				close(STDIN_FILENO);
			}
			if (r==-1) {
				perror("read");
				exit(EXIT_FAILURE);
			}
		}
		if (fds[1].revents & POLLOUT) {
			// Send more to output
			if (payload-outoff>0) {
				r=splice(fd,&outoff,STDOUT_FILENO,NULL,payload-outoff,SPLICE_F_NONBLOCK);
				if (r==-1 && errno!=EAGAIN) {
					perror("write");
					exit(EXIT_FAILURE);
				}
			} else {	// Output all caught up - wait for more
				fds[1].revents=fds[1].events=0;
			}
		}
	}
	if (fcntl(STDOUT_FILENO,F_SETFL,0)==-1) {
		perror("clear non-blocking output");
		exit(EXIT_FAILURE);
	}
	while (outoff<payload) {
		if (splice(fd,&outoff,STDOUT_FILENO,NULL,1<<20,0)==-1) {
			perror("write");
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}