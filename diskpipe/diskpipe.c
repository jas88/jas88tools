#define _GNU_SOURCE         /* See feature_test_macros(7) */
#define __USE_LARGEFILE64
#define __USE_FILE_OFFSET64
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int setnonblock(int fd) {
	int flags;
	if ((flags=fcntl(fd,F_GETFL,0))==-1) return 1;
	if (fcntl(fd,F_SETFL,flags|O_NONBLOCK)==-1) return 1;
	return 0;
}

static void usage(const char *name) {
	fprintf(stderr,"Usage:\t%s\n",name);
}

static void about(const char *name) {
	fprintf(stderr,"diskpipe v 0.01, © James A Sutherland 2016\nBuilt " __DATE__ "\n");
	usage(name);
}

int main(int argc,char **argv) {
	struct pollfd poller[2] = {{STDIN_FILENO,POLLIN,0},{STDOUT_FILENO,POLLOUT,0}};
	loff_t winstart=0,winend=0;
	ssize_t result;
	int tmpfd,state=1;
	char tempname[]="/tmp/diskpipe.XXXXXXXX";	
	
	if (argc>1 && strcmp(argv[1],"-v")==0) {
		about(*argv);
		exit(EXIT_SUCCESS);
	}
	
	if (setnonblock(STDIN_FILENO) || setnonblock(STDOUT_FILENO)) {
		perror("set nonblock");
		return -1;
	}
	tmpfd=mkstemp(tempname);
	if (tmpfd==-1 || unlink(tempname)==-1) {
		perror(tempname);
		return -1;
	}
	// Now loop, reading, writing and truncating as appropriate:
	while(state) {
		result=splice(STDIN_FILENO,0,tmpfd,&winend,1<<20,SPLICE_F_NONBLOCK);
		if (result==0) {
			state=0;
			poller[0].fd=-1;
			close(STDIN_FILENO);
		}	 // Exit read loop, proceed to drain buffer
		else if (result<0 && errno!=EAGAIN) { perror("read"); exit(EXIT_FAILURE); }
		if (winend>winstart) {
			result=splice(tmpfd,&winstart,STDOUT_FILENO,0,winend-winstart,SPLICE_F_NONBLOCK);
			if (result>=0) {
				if (fallocate(tmpfd,FALLOC_FL_PUNCH_HOLE|FALLOC_FL_KEEP_SIZE,0,winstart)) {
					perror("trim temp file");
				}
			}
			else if (errno!=EAGAIN) { perror("write"); exit(EXIT_FAILURE); }
		}
		
		poll(poller,2,-1);	// Wait until something's ready to process
	}
	
	// Now drain the buffer:
	while(winstart<winend) {
		result=splice(tmpfd,&winstart,STDOUT_FILENO,0,winend-winstart,SPLICE_F_NONBLOCK);
		if (result>=0) {
			if (fallocate(tmpfd,FALLOC_FL_PUNCH_HOLE|FALLOC_FL_KEEP_SIZE,0,winstart)) {
				perror("trim temp file");
			}
		 }
		else if (errno!=EAGAIN) { perror("write"); exit(EXIT_FAILURE); }
	}
	return 0;
}