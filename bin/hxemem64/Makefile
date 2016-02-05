include ../../htx.mk

TARGET= hxemem64 

LDFLAGS +=

OBJ_SUFF=.o
OBJECTS = hxemem${OBJ_SUFF} \
        mem_operation${OBJ_SUFF} \
        rand_operation${OBJ_SUFF} \
        pat_operation${OBJ_SUFF} \
        test_L4${OBJ_SUFF} \
        gen_tlbie${OBJ_SUFF}
 
SRCS = $(patsubst %.o, %.c, $(OBJECTS))

CFLAGS += -D__64_LINUX__ -D_GNU_SOURCE -DKERNEL_2_6 -D_REENTRANT -g -O5 -D__HTX_LINUX_ -D__USE_GNU -DTRUE=1 -DFALSE=0 
 
LIBS = -lhtx64 -lpthread 

.PHONY: all clean

all: ${OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o ${TARGET}
	${CP} ${TARGET} ${SHIPBIN}
	-${CP} mem.runsetup ${RUNSETUP}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -f ${RUNSETUP}/mem.runsetup 
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}
