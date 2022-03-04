#ifndef LIBS_H_INC
#define LIBS_H_INC

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <sysexits.h>
#include <err.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/prctl.h>


/////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <stddef.h>
#include <stdlib.h>

#include "debug.h"

#ifndef likely
#   define likely(x)       __builtin_expect(!!(x),1)
#endif

#ifndef unlikely
#   define unlikely(x)     __builtin_expect(!!(x),0)
#endif



#endif
