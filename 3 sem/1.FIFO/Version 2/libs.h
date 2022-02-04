#ifndef LIBS_H_INC
#define LIBS_H_INC


#ifndef _GNU_SOURCE
 
#define _GNU_SOURCE

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <signal.h>

/* for timing synchronization */

#include <sys/select.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#endif