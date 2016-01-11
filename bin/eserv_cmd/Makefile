include ../../htx.mk

TARGET= esrv 

LDFLAGS +=

OBJ_SUFF=.o
OBJECTS = eserv_cmd${OBJ_SUFF} parse_cmd${OBJ_SUFF} 
 
SRCS = $(patsubst %.o, %.c, $(OBJECTS))

CFLAGS += -DKERNEL64 -DFALSE=0 -DTRUE=1
 
LIBS = 

.PHONY: all clean

all: ${OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o ${TARGET}
	${CP} ${TARGET} ${SHIPBIN}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}
