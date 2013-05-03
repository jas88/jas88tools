#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

int listenport=443,listenip=INADDR_ANY,bufsize=4096,serverip=0,serverport=443;
char *servername=NULL;

static int lsock(int port) {
    int one=1,flags;
	struct sockaddr_in addr={0};
	int fd=socket(PF_INET,SOCK_STREAM,0);
	if (fd==-1) {
		perror("socket");
		return -1;
	}
    if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&one,sizeof(one))<0) {
        perror("setsockopt");
        close(fd);
        return -1;
    }
	addr.sin_family=AF_INET;
    addr.sin_port=htons(listenport);
	addr.sin_addr.s_addr=listenip;
	if (bind(fd,(struct sockaddr*)&addr,sizeof(addr))==-1) {
		perror("bind");
		close(fd);
		return -1;
	}
	if (listen(fd,8)==-1) {
		perror("listen");
		close(fd);
		return -1;
	}
    if ((flags=fcntl(fd,F_GETFL,0))==-1 || fcntl(fd,F_SETFL,flags|O_NONBLOCK)==-1) {
        perror("fcntl(F_GETFL)");
        close(fd);
        return -1;
    }
	return fd;
}

static int ssock(int port,uint32_t addr) {
    struct sockaddr_in sa={0};
	int flags,fd=socket(PF_INET,SOCK_STREAM,0);
	if (fd==-1) {
		perror("socket");
		return -1;
	}
	sa.sin_family=AF_INET;
    sa.sin_port=htons(serverport);
    sa.sin_addr.s_addr=serverip;
    if (connect(fd,(struct sockaddr*)&sa,sizeof(sa))==-1) {
        perror("connect");
        return -1;
    }
    if ((flags=fcntl(fd,F_GETFL,0))==-1 || fcntl(fd,F_SETFL,flags|O_NONBLOCK)==-1) {
        perror("fcntl(F_GETFL)");
        close(fd);
        return -1;
    }
    return fd;
}

static signed int getnum(char *numstr,int max) {
	int num;
	if (sscanf(numstr,"%d",&num)!=1 || num>max) return -1;
	return num;
}

/* Hostname to IPv4 address; 0 on error
 */
static int getip(char *name) {
    int ip=0;
    struct addrinfo hints={0},*res,*p;
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_STREAM;
    if (!getaddrinfo(name,NULL,&hints,&res)) {
        for (p=res;p!=NULL;p=p->ai_next) {
            if (p->ai_family==AF_INET) {
                struct sockaddr_in *sa=(struct sockaddr_in*)&p->ai_addr[0];
                ip=sa->sin_addr.s_addr;
            }
        }
        freeaddrinfo(res);
    } else {
        perror("getaddrinfo");
    }
    return ip;
}

static int options(int argc,char **argv) {
	int c;
	while((c=getopt(argc,argv,"a:b:p:r:s:"))!=-1) {
		switch(c) {
            case 'a':
                if ((listenip=getip(optarg))==0) {
                    fprintf(stderr,"Invalid bind address '%s'\n",optarg);
                    return -1;
                }
                break;

            case 'b':
			if ((bufsize=getnum(optarg,1<<24))<0) {
				fprintf(stderr,"Invalid buffer size '%s'\n",optarg);
				return -1;
			}
			break;

			case 'p':
			if ((listenport=getnum(optarg,65535))<0) {
				fprintf(stderr,"Invalid listen port '%s'\n",optarg);
				return -1;
			}
			break;
                
			case 'r':
			if ((serverport=getnum(optarg,65535))<0) {
				fprintf(stderr,"Invalid server port '%s'\n",optarg);
				return -1;
			}
			break;
                
            case 's':
                if ((serverip=getip(servername=optarg))==0) {
                    fprintf(stderr,"Invalid remote server '%s'\n",optarg);
                    return -1;
                }
            break;
			
			default:
			fprintf(stderr,"Unknown option %c\n",c);
			return -1;
		}
	}
	return 0;
}

static int w(struct pollfd *in,struct pollfd *out,char *buffb,int *qb) {
	int a;
	a=write(out->fd,buffb,*qb);
	if (a<0)
		return (errno==EAGAIN)?0:1;	// Write error
	*qb-=a;
	if (*qb==0) {
		// Buffer drained, revert to waiting for input
		in->events |= POLLIN;
		out->events&= ~POLLOUT;
	} else {
        // Some data remaining: *qb bytes, starting at a
        memmove(buffb,buffb+a,*qb-a);
    }
	return 0;
}

static int rw(struct pollfd *in,struct pollfd *out,char *buffb,int *qb) {
	int a;
    *qb=read(in->fd,buffb,bufsize);
    if (*qb<0 && errno!=EAGAIN)
        return 1;   // Error on client side
    if (*qb==0)
        return 1;   // EOF, output buffer is empty already
    a=write(out->fd,buffb,*qb);
    if (a<0 && errno!=EAGAIN)
        return 1;   // Error on server side
    *qb-=a;
    if (*qb>0) {
        // Still have *qb bytes of data in output buffer awaiting writing:
        memmove(buffb,buffb+a,*qb);
        in->events &= ~POLLIN; // Stop reading input until buffer drained
        out->events |= POLLOUT; // Wait to write output
    }
	return 0;
}

static int innerloop(struct pollfd *polls,char *buffa,char *buffb,int *qa,int *qb) {
	if ((polls[0].revents & (POLLERR|POLLHUP|POLLNVAL)) ||
	(polls[1].revents & (POLLERR|POLLHUP|POLLNVAL))) {
		return 1;
	}
    
	if (polls[0].revents & POLLIN) {
		if (rw(&polls[0],&polls[1],buffb,qb))
			return 1;
	}
    
	if (polls[1].revents & POLLIN) {
		if (rw(&polls[1],&polls[0],buffa,qa))
			return 1;
	}
    
	if (polls[0].revents & POLLOUT) {
		if (w(&polls[0],&polls[1],buffb,qb))
			return 1;
	}
    
	if (polls[1].revents & POLLOUT) {
		if (w(&polls[1],&polls[0],buffa,qa))
			return 1;
	}
	return 0;
}

static int relay(int fda,int fdb) {
	struct pollfd polls[2]={{fda,POLLIN,0},{fdb,POLLIN,0}};
	char buffa[bufsize],buffb[bufsize];
    int qa=0,qb=0,quit=0;
	while(!quit && poll(polls,2,-1)) {
		quit=innerloop(polls,buffa,buffb,&qa,&qb);
	}
    close(fda);
    close(fdb);
	return 0;
}

int main(int argc,char **argv) {
	struct pollfd polls[1]={{0,POLLIN,0}};

	if (options(argc,argv)) return -1;
    if (!serverip) {
        fprintf(stderr,"No remote server configured\n");
        return -1;
    }
	if ((polls[0].fd=lsock(listenport))<0) {
		return -1;
	}
	while(poll(polls,1,-1)!=-1) {
		// TODO
		struct sockaddr csock={0};
		unsigned int csocksize=sizeof(csock);
		int csockfd,ssockfd=ssock(serverport,serverip);
		if (ssockfd==-1) {
			sleep(1);	// Defer WITHOUT accepting connection
		} else {
			csockfd=accept(polls[0].fd,&csock,&csocksize);
			if (csockfd==-1) {
				perror("accept");
			} else {
				switch(fork()) {
					case 0: // Child
					close(polls[0].fd);
					relay(csockfd,ssockfd);
					return 0;
					break;
				
					case -1: // Fork failed
					perror("fork");
					close(csockfd);
					close(ssockfd);
					break;
				
					default:
					close(csockfd);
					close(ssockfd);
					break;
				}
			}
		}
	}
	return 0;
}