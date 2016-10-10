#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

static void usage(char *name) {
	fprintf(stderr,"Usage: %s <n>\n",name);
	exit(EXIT_FAILURE);
}

int main(int argc,char **argv) {
	char buff[4096];
	int n,total,sofar=0,prev=0;
	
	if (argc!=2) usage(*argv);
	total=atoi(argv[1]);
	while((n=read(fileno(stdin),buff,sizeof(buff)))>0) {
		for (int i=0;i<n;i++) {
			if (buff[i]=='\n') {
				sofar++;
			}
		}
		if (prev!=sofar) {
			printf("%d\n",sofar*100/total);
			fflush(stdout);
			prev=sofar;
		}
	}
	return 0;
}