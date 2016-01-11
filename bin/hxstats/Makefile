include ../../htx.mk

TARGET= hxstats

LDFLAGS +=

OBJ_SUFF=.o
OBJECTS = set_sig${OBJ_SUFF} send_msg${OBJ_SUFF} hxstats${OBJ_SUFF}
 
SRCS = $(patsubst %.o, %.c, $(OBJECTS))

CFLAGS += 
LIBS = -lhtx64 -lcfgc64

.PHONY: all clean

all: ${OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o ${TARGET}
	${CP} ${TARGET} ${SHIPBIN}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}
