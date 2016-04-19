include ../../htx.mk

TARGET= hxecache

LDFLAGS +=

OBJ_SUFF=.o
OBJECTS = hxecache${OBJ_SUFF} \
        prefetch_irritator${OBJ_SUFF} \
        prefetch${OBJ_SUFF} \
        prefetch_dcbtt${OBJ_SUFF} \
        prefetch_dcbtp${OBJ_SUFF} \
        prefetch_n_stride${OBJ_SUFF} \
        prefetch_dcbtna${OBJ_SUFF}
 
SRCS = $(patsubst %.o, %.c, $(OBJECTS))

CFLAGS += -D__64_LINUX__ -D_GNU_SOURCE -DKERNEL_2_6 -D_REENTRANT -D__HTX_LINUX__ -O2

LIBS = -lhtx64 -lpthread -lm

.PHONY: all clean

all: ${OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o ${TARGET}
	${CP} ${TARGET} ${SHIPBIN}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}
