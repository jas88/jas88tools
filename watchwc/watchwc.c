#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>


static int opt=0,lines=0,bytes=0,chars=0,words=0;

enum {OPT_C=1,OPT_L=2,OPT_M=4,OPT_W=8};

static void output()
{
	putchar('\r');
	if (opt&OPT_L) printf("% 8d",lines);
	if (opt&OPT_W) printf("% 8d",words);
	if (opt&OPT_M) printf("% 8d",chars);
	if (opt&OPT_C) printf("% 8d",bytes);
}

int main(int argc,char **argv)
{
	char buff[1024];
	int sp=0,n,i,bsize;
	
	/* Process switches: */
	while((++argv)[0] && argv[0][0]=='-') {
		n=1;
		while(argv[0][n]) {
			switch(argv[0][n++]) {
				case 'c': opt|=OPT_C; break;
				case 'l': opt|=OPT_L; break;
				case 'm': opt|=OPT_M; break;
				case 'w': opt|=OPT_W; break;
			}
		}
	}
	
	if (opt==0) opt=OPT_C|OPT_L|OPT_W;
	
	while((bsize=read(0,buff,sizeof(buff)))>0) {
		bytes+=bsize;
		for(i=0;i<bsize;i++) {
			if ((buff[i]&0xc0)!=0x80) chars++;	// Count all but UTF-8 continuation octets
			if (isspace(buff[i])) sp=1;
			else if (sp) {
				++words;
				sp=0;
			}
			if (buff[i]=='\n') {
				lines++;
				output();
			}
		}
	}
	output();
	putchar('\n');
	return 0;
}