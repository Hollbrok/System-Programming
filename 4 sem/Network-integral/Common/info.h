#ifndef	__info_h__
#define	__info_h__


#include	<sys/types.h>	/* basic system data types */
#include	<sys/socket.h>	/* basic socket definitions */

#include	<time.h>		/* old system? */

#include	<netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include	<arpa/inet.h>	/* inet(3) functions */
#include	<errno.h>
#include	<fcntl.h>		/* for nonblocking */
#include	<netdb.h>
#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/stat.h>	/* for S_xxx file mode constants */
#include	<sys/uio.h>		/* for iovec{} and readv/writev */
#include	<unistd.h>
#include	<sys/wait.h>
#include	<sys/un.h>		/* for Unix domain sockets */

#include    <sys/time.h>
#include    <math.h>




/* Define bzero() as a macro if it's not in standard C library. */
#ifndef	HAVE_BZERO
#define	bzero(ptr,n)		memset(ptr, 0, n)
#endif


/* POSIX renames "Unix domain" as "local IPC."
   Not all systems define AF_LOCAL and PF_LOCAL (yet). */
#ifndef	AF_LOCAL
#define AF_LOCAL	AF_UNIX
#endif
#ifndef	PF_LOCAL
#define PF_LOCAL	PF_UNIX
#endif


#define MAX_PC_DIGITS 4     /*maximum number of digits in NO PCs number ===> max NO PCs 9999 */

/* Following could be derived from SOMAXCONN in <sys/socket.h>, but many
   kernels still #define it as 5, while actually supporting many more */
#define	LISTENQ		1024	/* 2nd argument to listen() */

/* Miscellaneous constants */
#define	MAXLINE		4096	/* max text line length */
#define	BUFFSIZE	8192	/* buffer size for reads and writes */

/* timeout constants */
#define ACCEPT_TIMEOUT_SEC  1       /* in sec  waiting time in accept */
#define ACCEPT_TIMEOUT_USEC 0  /* in usec -//-                                                      */

#define CALC_TIMEOUT_SEC  120    /* in sec  allotted time for calculating the integral for the client    */
#define CALC_TIMEOUT_USEC 0     /* in usec -//-                                                         */

/* Define some port number that can we use*/
#define	SERV_PORT		 9878			/* server port */
#define	SERV_PORT_STR	"9878"			/* string version of server port */

#define CL_PORT          9888           /* client port  */
#define CL_PORT_STR     "9888"          /* string version of client port */

#define min(a,b)                    \
   ({   __typeof__ (a) _a = (a);    \
        __typeof__ (b) _b = (b);    \
        _a < _b ? _a : _b;        })

#define max(a,b)                    \
   ({   __typeof__ (a) _a = (a);    \
        __typeof__ (b) _b = (b);    \
        _a > _b ? _a : _b;        })


/* structures */

#define GENERAL_START_INT 0
#define GENERAL_FINISH_INT 5

struct CalcInfo
{
    double a;
    double b;
};

struct CliInfo
{
    size_t noThreads;       /* NO client threads*/
};

struct IntResult
{
    double result;          /* client integral result */
};


/* cmd line number parser ( returns <long> number according to numString with error handling) */
long    getNumber(char *numString, int *errorState);


ssize_t readline(int fd, void *vptr, size_t maxlen);
ssize_t	writen(int fd, const void *vptr, size_t n);


int		inet_pton(int family, const char *strptr, void *addrptr);
void    Inet_pton(int family, const char *strptr, void *addrptr);

#define	SA struct sockaddr

			/* prototypes for our socket wrapper functions: see {Sec errors} */
int		 Accept(int fd, struct sockaddr *sa, socklen_t *salenptr);
void	 Bind(int fd, const struct sockaddr *sa, socklen_t salen);
void	 Connect(int fd, const struct sockaddr *sa, socklen_t salen);
void	 Getpeername(int fd, struct sockaddr *sa, socklen_t *salenptr);

void	 Getsockname(int fd, struct sockaddr *sa, socklen_t *salenptr);
void	 Getsockopt(int fd, int level, int optname, 
            void *optval, socklen_t *optlenptr);

void	 Listen(int fd, int backlog);

ssize_t	 Readline(int fd, void *ptr, size_t maxlen);
ssize_t	 Readn(int fd, void *ptr, size_t nbytes);
ssize_t	 Recv(int fd, void *ptr, size_t nbytes, int flags);

//ssize_t	 Recvfrom(int, void *, size_t, int, SA *, socklen_t *);
//ssize_t	 Recvmsg(int, struct msghdr *, int);
//int		 Select(int, fd_set *, fd_set *, fd_set *, struct timeval *);

void	 Send(int fd, const void *ptr, size_t nbytes, 
    int flags);

ssize_t	 Sendto(int fd, const void *ptr, size_t nbytes, int flags,
	   const struct sockaddr *sa, socklen_t salen);

void	 Sendmsg(int, const struct msghdr *, int);
void	 Setsockopt(int, int, int, const void *, socklen_t);

//void	 Shutdown(int, int);
//int    Sockatmark(int);

int		 Socket(int family, int type, int protocol);

//void	 Socketpair(int, int, int, int *);

void	 Writen(int fd, void *ptr, size_t nbytes);

void	 err_dump(const char *fmt, ...);
void	 err_msg(const char *fmt, ...);
void	 err_quit(const char *fmt, ...);
void	 err_ret(const char *fmt, ...);
void	 err_sys(const char *fmt, ...);

#endif /* __info_h__ */