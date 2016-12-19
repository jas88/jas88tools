#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/syscall.h>

/*
static ssize_t js_writev(int fd,const struct iovec *iov,int ion) {
	return (ssize_t)syscall(SYS_writev,fd,iov,ion);
}

static ssize_t js_read(int fd,void *buff,size_t len) {
	return (ssize_t)syscall(SYS_read,fd,buff,len);
}
*/

static int js_atoi(const char *nptr) {
	if (*nptr>='0' && *nptr<='9') {
		return (*nptr-'0')*10+js_atoi(nptr+1);
	}
	return 0;
}

static int js_strlen(const char *s) {
	int n=0;
	while(*s++) {
		n++;
	}
	return n;
}

static void usage(char *name) {
	char pre[]="Usage: ",post[]=" <n>\n\tn - number of lines expected\n";
	struct iovec bits[]={
		{pre,sizeof(pre)-1},
		{0},
		{post,sizeof(post)-1}
	};
	bits[1].iov_base=name;
	bits[1].iov_len=js_strlen(name);
	writev(STDOUT_FILENO,bits,3);
	//fprintf(stderr,"Usage: %s",name);
	exit(EXIT_FAILURE);
}

static int itoa(int n,char *out,unsigned int len) {
	int shifter=n;
	
	if (len<1) {
		return -1; // Error, can't do anything with empty string!
	}
	if (n<0) {
		*out='-';
		return itoa(-n,out+1,len-1);
	}
	do {
		out++;
		shifter/=10;
	} while(shifter);
	*out='\0';
	do {
		*--out='0'+n%10;
		n/=10;
	} while(n);
	return 0;
}

int main(int argc,char **argv) {
	char buff[4096];
	int n,total,sofar=0,prev=0;
	struct iovec bits[]={{buff,0},{"\r",1}};
	
	if (argc!=2) usage(*argv);
	total=js_atoi(argv[1]);
	if (total==0) usage(*argv);
	while((n=read(STDIN_FILENO,buff,sizeof(buff)))>0) {
		for (int i=0;i<n;i++) {
			if (buff[i]=='\n') {
				sofar++;
			}
		}
		if (prev!=sofar) {
			itoa(sofar*100/total,buff,sizeof(buff));
			bits[0].iov_len=js_strlen(buff);
			writev(STDOUT_FILENO,bits,2);
			//printf("%d%%\r",sofar*100/total);
			//fflush(stdout);
			prev=sofar;
		}
	}
	return 0;
}