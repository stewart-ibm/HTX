include ../../htx.mk

TARGET= hxetape

LDFLAGS +=

OBJ_SUFF=.o
OBJECTS = buf_oper${OBJ_SUFF} get_rule${OBJ_SUFF} hxetape${OBJ_SUFF} \
          io_oper${OBJ_SUFF} posix${OBJ_SUFF} proc_rule${OBJ_SUFF} \
          prt_msg${OBJ_SUFF} 
 
SRCS = $(patsubst %.o, %.c, $(OBJECTS))

CFLAGS += 
 
LIBS = -lhtx64 -lcfgc64 -lmisc64 -lpthread 

.PHONY: all clean

all: ${OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o ${TARGET}
	${CP} ${TARGET} ${SHIPBIN}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}
