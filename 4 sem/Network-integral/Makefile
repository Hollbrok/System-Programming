# flags

VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all

CFLAGS = -g -MD -c 
ADDIT_FLAGS = -pthread -lm

# pathes

MAIN_PC_PATH 	= MainPC
MAIN_PC_SOURCES = $(MAIN_PC_PATH)/main.c
MAIN_PC_OBJS	= $(MAIN_PC_SOURCES:.c=.o)
MAIN_PC_EXEC	= mainPc_exec


CLIENT_PATH  	= Client
CLIENT_SOURCES 	= $(CLIENT_PATH)/main.c $(CLIENT_PATH)/threads_int_func.c
CLIENT_OBJS		= $(CLIENT_SOURCES:.c=.o)
CLIENT_EXEC		= client_exec



COMMON_CODE_PATH = .

SOURCES = $(COMMON_CODE_PATH)/network_int_func.c

OBJECTS = $(SOURCES:.c=.o)

# names of output files

all: client mainPC # run_tests

### compile stuff

%.o: %.c
		$(CC) $(CFLAGS) -o $@ $<

# client

client: $(CLIENT_SOURCES) $(CLIENT_EXEC)

$(CLIENT_EXEC): $(CLIENT_OBJS)
		$(CC) $(CLIENT_OBJS) $(ADDIT_FLAGS) -o $@

# mainPC

mainPC: $(MAIN_PC_SOURCES) $(MAIN_PC_EXEC)

$(MAIN_PC_EXEC): $(MAIN_PC_OBJS)
		$(CC) $(MAIN_PC_OBJS) $(ADDIT_FLAGS) -o $@

## 

-include *.d

# for testing speed 

#maxThreads = 16

#run_tests:
#		sh ./test_speed.sh $(maxThreads)

# clean stuff

.PHONY: clean

clean:
	cd $(MAIN_PC_PATH) && rm -rf *.o *.d
	cd $(CLIENT_PATH) && rm -rf *.o *.d

