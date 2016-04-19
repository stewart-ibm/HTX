include ../../htx.mk

TARGET= create_eq_cfg 

LDFLAGS +=

OBJ_SUFF=.o
OBJECTS = create_eq_cfg${OBJ_SUFF}
SRCS = $(patsubst %.o, %.c, $(OBJECTS))

CFLAGS += -g -D__64_LINUX__ -D_GNU_SOURCE -DKERNEL_2_6

LIBS = -Wl,--start-group -lhtx64 -lpthread -Wl,--end-group

.PHONY: all clean

all: ${OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o ${TARGET}
	${CP} ${TARGET} ${SHIPBIN}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}

