#********************************************************#
#			MAKEFILE LIBSOCKET SHARED LIBRARY		     #
#********************************************************#

# C COMPILER
CC = gcc
# C FLAGS
CFLAGS = -fPIC -Wall -Wextra -O2 -g
# LINKING FLAGS
LDFLAGS = -shared
# RM COMAND
RM = rm -rf
#TARGET LIB NAME
TARGET_LIB = libc-sockets-library.so

#SOURCE & OBJECT FILES
SRCS = libsocket.c
OBJS = $(SRCS:.c=.o)

#COMPILATION RULES
.PHONY: all
all: ${TARGET_LIB} clean

$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CFLAGS) -MM $< >$@

include $(SRCS:.c=.d)

.PHONY: clean
clean:
	-${RM} ${OBJS} $(SRCS:.c=.d)
