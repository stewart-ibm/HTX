include ../../htx.mk

TARGET=bufdisp

LDFLAGS +=

OBJ_SUFF=.o

OBJECTS = bufdisp${OBJ_SUFF} 

CFLAGS += -O1 -g

.PHONY: all clean

all: ${OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o ${TARGET}
	${CP} ${TARGET} ${SHIPBIN}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}
