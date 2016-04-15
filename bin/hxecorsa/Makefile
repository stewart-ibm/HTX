include ../../htx.mk

TARGET=hxecorsa

LDFLAGS += -L/usr/lib64/genwqe/ -Xlinker -rpath=/usr/lib64/genwqe/

OBJ_SUFF=.o

vpath= %.% ../hxemem64/

OBJECTS = \
	hxemem${OBJ_SUFF} \
	mem_operation${OBJ_SUFF} \
	rand_operation${OBJ_SUFF} \
	pat_operation${OBJ_SUFF} \
	test_L4${OBJ_SUFF} \
	gen_tlbie${OBJ_SUFF} \
	corsa${OBJ_SUFF}

SRCS = $(patsubst %.o, %.c, $(OBJECTS))

CFLAGS += -DCORSA_ENABLE -D__64_LINUX__ -D_GNU_SOURCE -DKERNEL_2_6 -D_REENTRANT -g -D__HTX_LINUX__ -D__USE_GNU -I./../hxemem64 -I./../../inc
 
LIBS += -lpthread -lhtx64 -lz

.PHONY: all clean

%.o: ../hxemem64/%.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

%.o: ../hxemem64/%.s
	${AS} -c $< -o $@

all: ${OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o ${TARGET}
	${CP} ${TARGET} ${SHIPBIN}


#%.o: %.s
#	${AS} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -rf *.o ${TARGET} ${SHIPBIN}/${TARGET}

