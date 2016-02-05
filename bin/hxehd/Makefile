include ../../htx.mk

TARGET= hxehd

LDFLAGS +=

OBJ_SUFF=.o
OBJECTS = prt_msg_common${OBJ_SUFF} hd_linux${OBJ_SUFF} io_oper_common${OBJ_SUFF} get_rule_common${OBJ_SUFF} hxehd_common${OBJ_SUFF} dskcomm_common${OBJ_SUFF}
SRCS = $(patsubst %.o, %.c, $(OBJECTS))

CFLAGS += -D__GR2600__ -D_GNU_SOURCE -O2 -D__HTX_LINUX__ -D_target_ppc -DHTXTHREADED -D_FILE_OFFSET_BITS=64
LIBS = -lhtxmp64 -lhtx64 -lpthread -lc

.PHONY: all clean

all: ${OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o ${TARGET}
	${CP} ${TARGET} ${SHIPBIN}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}
