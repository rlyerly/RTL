# Executable name
BINARY=cyclictest

# Common compiler flags
CC=gcc
CFLAGS=-O3 -Wall -D_GNU_SOURCE -o $(BINARY)

# Common include directories
INCLUDE=-I../include/

# LD search directories and libraries
LDFLAGS=-L/usr/local/realtime/ -L../lib/
LIBS=-lrt -lm -lpthread

# Source files
SRC=$(shell ls *.c) #$(shell ls ../src/*.c)

# Get the kernel name
KERNEL=$(shell uname --kernel-release | sed 's/[2-3]\.[0-9]*\.[0-9]*-\([a-zA-Z0-9-]*\).*/\1/')

###############################################################################
#                                  Vanilla                                    #
###############################################################################
VANILLA_LIBS=-lrealtime $(LIBS)

###############################################################################
#                                  PREEMPT                                    #
###############################################################################
PREEMPT_FLAGS=$(CFLAGS) -DKERN_PREEMPT
PREEMPT_LIBS=-lpreempt_realtime $(LIBS)

###############################################################################
#                                    EDF                                      #
###############################################################################
EDF_FLAGS=$(CFLAGS) -DKERN_SCHED_DEAD
EDF_LIBS=-lsched_dead_realtime $(LIBS)

###############################################################################
#                                   IRMOS                                     #
###############################################################################
IRMOS_FLAGS=$(CFLAGS) -DKERN_IRMOS
IRMOS_LIBS=-lirmos_realtime $(LIBS)

###############################################################################
#                                  LitmusRT                                   #
###############################################################################
LITMUS_FLAGS=$(CFLAGS) -DKERN_LITMUS
LITMUS_INCLUDE=$(INCLUDE) -I../include/litmus/ -I../include/litmus/x86/
LITMUS_LIBS=-llitmus_realtime -llitmus $(LIBS)

###############################################################################
#                                  ChronOS                                    #
###############################################################################
CHRONOS_FLAGS=$(CFLAGS) -DKERN_CHRONOS -Wl,-rpath ../lib/
CHRONOS_INCLUDE=$(INCLUDE) -I../include/chronos/
CHRONOS_LIBS=-lchronos -lchronos_realtime $(LIBS)

###############################################################################
#                                  Xenomai                                    #
###############################################################################
XENOMAI_FLAGS=$(CFLAGS) -DKERN_XENOMAI -Wl,-rpath /usr/xenomai/lib/
XENOMAI_INCLUDE=$(INCLUDE) -I/usr/xenomai/include/
XENOMAI_LDFLAGS=$(LDFLAGS) -L/usr/xenomai/lib/
XENOMAI_LIBS=-lxeno_realtime -lnative -lxenomai -lpthread_rt $(LIBS)

###############################################################################
#                                    RTAI                                     #
###############################################################################
# TODO


all:
	if [ "$(KERNEL)" = "rt16" ]; then \
		$(CC) $(PREEMPT_FLAGS) $(INCLUDE) $(LDFLAGS) $(SRC) $(PREEMPT_LIBS); \
	elif [ "$(KERNEL)" = "edf-rc6" ]; then \
		$(CC) $(EDF_FLAGS) $(INCLUDE) $(LDFLAGS) $(SRC) $(EDF_LIBS); \
	elif [ "$(KERNEL)" = "irmos" ]; then \
		$(CC) $(IRMOS_FLAGS) $(INCLUDE) $(LDFLAGS) $(SRC) $(IRMOS_LIBS); \
	elif [ "$(KERNEL)" = "xenomai" ]; then \
		$(CC) $(XENOMAI_FLAGS) $(XENOMAI_INCLUDE) $(XENOMAI_LDFLAGS) $(SRC) $(XENOMAI_LIBS); \
	elif [ "$(KERNEL)" = "rtai" ]; then \
		echo "RTAI not supported in the makefile yet"; \
	elif [ "$(KERNEL)" = "litmus" ]; then \
		$(CC) $(LITMUS_FLAGS) $(LITMUS_INCLUDE) $(LDFLAGS) $(SRC) $(LITMUS_LIBS); \
	elif [ "$(KERNEL)" = "chronos" ]; then \
		$(CC) $(CHRONOS_FLAGS) $(CHRONOS_INCLUDE) $(LDFLAGS) $(SRC) $(CHRONOS_LIBS); \
	else \
		$(CC) $(CFLAGS) $(INCLUDE) $(LDFLAGS) $(SRC) $(VANILLA_LIBS); \
	fi

clean:
	rm $(BINARY)
