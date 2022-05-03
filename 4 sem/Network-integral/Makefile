# flags

VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all

CFLAGS = -g -MD -c 
ADDIT_FLAGS = -pthread -lm

# pathes

LIB_PATH		= Common/
LIB_SOURCES 	= $(LIB_PATH)/error.c $(LIB_PATH)/wrapped.c
LIB_OBJS		= $(LIB_SOURCES:.c=.o)

SERVER_PATH 	= Server
SERVER_SOURCES 	= $(SERVER_PATH)/main.c
SERVER_OBJS		= $(SERVER_SOURCES:.c=.o)
SERVER_EXEC		= server


CLIENT_PATH  	= Client
CLIENT_SOURCES 	= $(CLIENT_PATH)/main.c $(CLIENT_PATH)/threads_int_func.c
CLIENT_OBJS		= $(CLIENT_SOURCES:.c=.o)
CLIENT_EXEC		= client



COMMON_CODE_PATH = .

SOURCES = $(COMMON_CODE_PATH)/network_int_func.c

OBJECTS = $(SOURCES:.c=.o)

# names of output files

all: Bclient Bserver # run_tests

### compile stuff

%.o: %.c
		$(CC) $(CFLAGS) -o $@ $<

# client

Bclient: $(CLIENT_SOURCES) $(CLIENT_EXEC)

$(CLIENT_EXEC): $(CLIENT_OBJS) $(LIB_OBJS)
		$(CC) $(CLIENT_OBJS) $(LIB_OBJS) $(ADDIT_FLAGS) -o $@

# Server

Bserver: $(SERVER_SOURCES) $(SERVER_EXEC)

$(SERVER_EXEC): $(SERVER_OBJS) $(LIB_OBJS)
		$(CC) $(SERVER_OBJS) $(LIB_OBJS) $(ADDIT_FLAGS) -o $@

## 

-include *.d

# for testing speed 

#maxThreads = 16

#run_tests:
#		sh ./test_speed.sh $(maxThreads)

# clean stuff

.PHONY: clean

clean:
	cd $(SERVER_PATH) && rm -rf *.o *.d
	cd $(CLIENT_PATH) && rm -rf *.o *.d
	cd $(LIB_PATH) && rm -rf *.o *.d

