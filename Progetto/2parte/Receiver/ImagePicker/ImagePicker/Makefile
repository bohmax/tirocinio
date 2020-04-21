CC		=  gcc
AR              =  ar
CFLAGS	        += -Wall -pedantic -g
ARFLAGS         =  rvs
INCLUDES	= -I/usr/local/include
LDFLAGS 	= -L/usr/local/lib
OPTFLAGS	= #-O3 
LIBS        = -pthread -lpcap -lavformat -lavcodec -lswscale -lavutil
CHMOD       = chmod +x

#inserire lo script che fa partire il client
SCRIPT      = start.sh

# aggiungere qui altri targets se necessario
TARGETS		= client


# aggiungere qui i file oggetto da compilare
OBJECTS		=	receiver.o \
				icl_hash.o  \
				h264toPng.o \
				lista.o \
				struct_fun.o \
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
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS) 

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<

all		: $(TARGETS)
		  $(CHMOD) $(SCRIPT)


client: main.o $(HEADER)
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ $^ $(LIBS)

libclient.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^