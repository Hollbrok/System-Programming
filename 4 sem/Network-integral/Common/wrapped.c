/* include writen */
#include	"info.h"

long getNumber(char *numString, int *errorState)
{
    if (*numString == '\0')
    {
        fprintf(stderr, "empty number argument\n");
        *errorState = -1;
        return 0; //exit(EXIT_FAILURE);
    }

    long gNumber;
    char* endOfEnter;

    const int baseOfNumber = 10;
    errno = 0;

    gNumber = strtol(numString, &endOfEnter, baseOfNumber);

    if(*endOfEnter != '\0')
    {
        fprintf(stderr, "strtol error ( != \'0\')\n");
        *errorState = -2;
        return 0; //exit(EXIT_FAILURE);
    }
    if (errno != 0)
    {
        fprintf(stderr, "strtol error (errno != 0)\n");
        *errorState = -3;
        return 0; //exit(EXIT_FAILURE);
    }
    
    return gNumber;
}

/* Write "n" bytes to a descriptor. */
ssize_t writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr   = vptr;
	nleft = n;
	while (nleft > 0) 
    {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) 
        {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return -1;			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return n;
}
/* end writen */

void Writen(int fd, void *ptr, size_t nbytes)
{
	if (writen(fd, ptr, nbytes) != nbytes)
		err_sys("writen error");
}


int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	int n;

	if ( (n = accept(fd, sa, salenptr)) < 0) 
    {
		if (errno == EAGAIN)
            return -2;
		else
			err_sys("accept error");
	}
	return n;
}

void Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
	if (bind(fd, sa, salen) < 0)
		err_sys("bind error");
}

void Connect(int fd, const struct sockaddr *sa, socklen_t salen)
{
	if (connect(fd, sa, salen) < 0)
		err_sys("connect error");
}

void Getpeername(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	if (getpeername(fd, sa, salenptr) < 0)
		err_sys("getpeername error");
}

void Getsockname(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	if (getsockname(fd, sa, salenptr) < 0)
		err_sys("getsockname error");
}

void Getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlenptr)
{
	if (getsockopt(fd, level, optname, optval, optlenptr) < 0)
		err_sys("getsockopt error");
}

void Listen(int fd, int backlog)
{
	char	*ptr;

		/*4can override 2nd argument with environment variable */
	if ( (ptr = getenv("LISTENQ")) != NULL)
		backlog = atoi(ptr);

	if (listen(fd, backlog) < 0)
		err_sys("listen error");
}

int Socket(int family, int type, int protocol)
{
	int n;

	if ( (n = socket(family, type, protocol)) < 0)
		err_sys("socket error");
	return n;
}

const char* Inet_ntop(int family, const void *addrptr, char *strptr, size_t len)
{
	const char	*ptr;

	if (strptr == NULL)		/* check for old code */
		err_quit("NULL 3rd argument to inet_ntop");
	if ( (ptr = inet_ntop(family, addrptr, strptr, len)) == NULL)
		err_sys("inet_ntop error");		/* sets errno */
	return ptr;
}

int inet_aton(const char *cp, struct in_addr *ap)
{
    int dots = 0;
    register unsigned long acc = 0, addr = 0;

    do 
    {
	    register char cc = *cp;

	    switch (cc) 
        {
	        case '0':
	        case '1':
	        case '2':
	        case '3':
	        case '4':
	        case '5':
	        case '6':
	        case '7':
	        case '8':
	        case '9':
	            acc = acc * 10 + (cc - '0');
	            break;

	        case '.':
	            if (++dots > 3)
		            return 0;

	        /* Fall through */

	        case '\0':
	            if (acc > 255) 
		            return 0;
	            addr = addr << 8 | acc;
	            acc = 0;
	            break;

	        default:
	            return 0;
	    }

    } while (*cp++);

    /* Normalize the address */
    if (dots < 3)
	    addr <<= 8 * (3 - dots) ;

    /* Store it if requested */
    if (ap)
	    ap->s_addr = htonl(addr);

    return 1;    
}

/* include inet_pton */
int inet_pton(int family, const char *strptr, void *addrptr)
{
    if (family == AF_INET) {
    	struct in_addr  in_val;
        if (inet_aton(strptr, &in_val)) {
            memcpy(addrptr, &in_val, sizeof(struct in_addr));
            return (1);
        }
		return(0);
    }
	errno = EAFNOSUPPORT;
    return (-1);
}
/* end inet_pton */

void Inet_pton(int family, const char *strptr, void *addrptr)
{
	int n;

	if ( (n = inet_pton(family, strptr, addrptr)) < 0)
		err_sys("inet_pton error for %s", strptr);	/* errno set */
	else if (n == 0)
		err_quit("inet_pton error for %s", strptr);	/* errno not set */

	/* nothing to return */
}


static int	read_cnt;
static char	*read_ptr;
static char	read_buf[MAXLINE];

static ssize_t my_read(int fd, char *ptr)
{

	if (read_cnt <= 0) 
    {
again:
		if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) 
        {
			if (errno == EINTR)
				goto again;
			return -1;
		} 
        else if (read_cnt == 0)
			return 0;
		read_ptr = read_buf;
	}

	read_cnt--;
	*ptr = *read_ptr++;
	return 1;
}

ssize_t readline(int fd, void *vptr, size_t maxlen)
{
	ssize_t	n, rc;
	char	c, *ptr;

	ptr = vptr;
	for (n = 1; n < maxlen; n++) 
    {
		if ( (rc = my_read(fd, &c)) == 1) 
        {
			*ptr++ = c;
			if (c == '\n')
				break;	    /* newline is stored, like fgets() */
		} 
        else if (rc == 0) 
        {
			*ptr = 0;
			return(n - 1);	/* EOF, n - 1 bytes were read */
		} 
        else
			return(-1);		/* error, errno set by read() */
	}

	*ptr = 0;	/* null terminate like fgets() */
	return n;
}

ssize_t readlinebuf(void **vptrptr)
{
	if (read_cnt)
		*vptrptr = read_ptr;
	return read_cnt;
}

ssize_t Readline(int fd, void *ptr, size_t maxlen)
{
	ssize_t n;

	if ( (n = readline(fd, ptr, maxlen)) < 0)
		err_sys("readline error");
	return n;
}


/* Read "n" bytes from a descriptor. */
ssize_t readn(int fd, void *vptr, size_t n)
{
	size_t	nleft;
	ssize_t	nread;
	char	*ptr;

	ptr   = vptr;
	nleft = n;
	while (nleft > 0) 
    {
		if ( (nread = read(fd, ptr, nleft)) < 0) 
        {
			if (errno == EINTR)
				nread = 0;		/* and call read() again */
			else
				return -1;
		} 
        else if (nread == 0)
			break;				/* EOF */

		nleft -= nread;
		ptr   += nread;
	}
	return n - nleft;		/* return >= 0 */
}


ssize_t Readn(int fd, void *ptr, size_t nbytes)
{
	ssize_t n;

	if ( (n = readn(fd, ptr, nbytes)) < 0)
		err_sys("readn error");
	return n;
}

ssize_t Sendto(int fd, const void *ptr, size_t nbytes, int flags,
	   const struct sockaddr *sa, socklen_t salen)
{
	if (sendto(fd, ptr, nbytes, flags, sa, salen) != (ssize_t)nbytes)
		err_sys("sendto error");
    else 
        return (ssize_t)nbytes; 
}

ssize_t Recv(int fd, void *ptr, size_t nbytes, int flags)
{
	ssize_t n;

	if ( (n = recv(fd, ptr, nbytes, flags)) < 0)
		err_sys("recv error");
	return(n);
}

void Send(int fd, const void *ptr, size_t nbytes, 
    int flags)
{
	if (send(fd, ptr, nbytes, flags) != (ssize_t)nbytes)
		err_sys("send error");
}

void Setsockopt(int socket, int level, int option_name,
    const void *option_value, socklen_t option_len)
{
    if (setsockopt (socket, level, option_name, option_value, option_len) != 0) 
        err_sys("setsockopt");
}


