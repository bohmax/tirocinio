CC		=  gcc
AR              =  ar
CFLAGS	        += -Wall -pedantic -g
ARFLAGS         =  rvs
OPTFLAGS	= #-O3 
LIBS        = -pthread -lpcap -lavformat -lavcodec -lswscale -lavutil
CHMOD       = chmod +x

#inserire lo script che fa partire il client
SCRIPT      = start.sh

# aggiungere qui altri targets se necessario
TARGETS		= client


# aggiungere qui i file oggetto da compilare
OBJECTS		=	struct_fun.o \
				utility_fun.o \
				receiver.o \
				icl_hash.o  \
				h264toPng.o \
				lista.o \
				threads.o 

# aggiungere qui gli altri include 
INCLUDE_FILES   = 	utility.h

ifeq ($(shell uname),Darwin)
	HEADER = libclient.a
endif
ifeq ($(shell uname),Linux)
	HEADER = libclient.a $(INCLUDE_FILES)
endif

.PHONY: all client
.SUFFIXES: .c .h

%: %.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c -o $@ $<

all		: $(TARGETS)
		  $(CHMOD) $(SCRIPT)


client: main.o $(HEADER)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

libclient.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^