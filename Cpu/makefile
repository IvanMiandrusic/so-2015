#**************************************************************#
#			MAKEFILE Cpu                  #
#**************************************************************#

# C COMPILER
CC = gcc

# C FLAGS
CFLAGS = -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP

# RM COMAND
RM = rm -rf

#SOURCE & OBJECT FILES
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
HEADERS = $(wildcard src/*.h)

#LIBS
LIBS := -lc-sockets-library -lcommons -lpthread

# Each subdirectory must supply rules for building sources it contributes
src/%.o: src/%.c
	@echo 'Compilando archivo: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"~/tp-2015-2c-elclan/lib/" $(CFLAGS) -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Compilacion terminada: $<'
	@echo ' '


# All Target
all: Cpu

dependents:
	-cd ~/tp-2015-2c-elclan/lib/ && $(MAKE) all

# Tool invocations
Cpu: $(OBJS) ~/tp-2015-2c-elclan/lib/libc-sockets-library.so
	@echo 'Compilando: $@'
	@echo 'Invoking: GCC C Linker'
	$(CC) -L"/home/utnso/tp-2015-2c-elclan/lib/" -o "Cpu" $(OBJS) $(LIBS)
	@echo 'Compilacion terminada: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(DEPS) Cpu
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
~/tp-2015-2c-elclan/lib/libc-sockets-library.so: