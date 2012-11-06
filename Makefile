CC = cc
CCFLAGS = -I./3rd -O2 -Wall -ggdb -fPIC -fpic

JMM=src/jmm

OPT_3RD_OBJ=3rd/ciniconfig.o 3rd/clog.o

MY_OBJ=src/myserver.o

SRV_OBJ=src/jmm.o src/jmm_conf.o src/jmm_event.o src/jmm_proc.o \
		src/jmm_shm.o src/jmm_util.o src/jmm_log.o \

JMM_OBJ=$(OPT_3RD_OBJ) $(SRV_OBJ) $(MY_OBJ)

EXT_CCFLAGS = -lm -L/usr/local/lib -levent_core

all:$(JMM)

$(JMM):$(JMM_OBJ)
	$(CC) $(EXT_CCFLAGS) -o $@ $(JMM_OBJ)

install:all
	cp -f $(JMM) ./bin

clean:
	rm -rf $(JMM_OBJ) 
	rm -rf $(JMM)

.PRECIOUS:%.c
.SUFFIXES:
.SUFFIXES:.c .o
.c.o:
	$(CC) $(CCFLAGS) -c -o $*.o $<

