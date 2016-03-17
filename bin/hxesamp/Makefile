include ../../htx.mk

TARGET=hxesamp

LDFLAGS +=

OBJ_SUFF=.o

OBJECTS = hxesamp${OBJ_SUFF} 

CFLAGS += 
 
LIBS =-lpthread -lhtx64

.PHONY: all clean

all: ${OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o ${TARGET}
	${CP} ${TARGET} ${SHIPBIN}/

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}
