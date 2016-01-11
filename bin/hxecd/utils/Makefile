include ../../../htx.mk

TARGET= getDVD
all: ${TARGET}

LDFLAGS +=

OBJ_SUFF=.o
OBJECTS = getDVD${OBJ_SUFF}

SRCS = $(patsubst %.o, %.c, $(getDVD_OBJECTS))

CFLAGS += -D_target_ppc -DHTXTHREADED -D_FILE_OFFSET_BITS=64 

LIBS = -lpthread -lc -lhtx64 -lmisc64

.PHONY: all clean

${TARGET}: ${OBJECTS} 
	${CC} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o getDVD 
	${CP} getDVD ${SHIPBIN}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}
