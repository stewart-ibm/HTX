include ../../htx.mk

TARGET=hxedapl

LDFLAGS +=

OBJ_SUFF=.o

vpath = %.% ../hxecom/

OBJECTS =	hxecom.o                \
                comread_rdma.o  \
                comwrite_rdma.o \
                comdapl.o               \
                comsock.o               \
                comrw.o                 \
                rule.o                  \
                com1_new.o              \
                com2_new.o              \
                comwrite.o              \
                comread.o               \

SRCS = $(patsubst %.o, %.c, $(OBJECTS))

CFLAGS += -D__RDMA_SUPPORTED__ -D__HTX_LINUX__ -D_64BIT_ 

LIBS += -ldat2 -lrt -lhtx64 -lpthread

.PHONY: all clean

%.o: ../hxecom/%.c 
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

all: ${OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o ${TARGET}
	${CP} ${TARGET} ${SHIPBIN}

clean:
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}
