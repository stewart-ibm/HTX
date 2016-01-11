include ../../htx.mk

TARGET= libsimstub.a

LDFLAGS +=

OBJ_SUFF=.o

OBJECTS = simulate_stub${OBJ_SUFF}

SRCS = $(patsubst %.o, %.c, $(OBJECTS))

.PHONY: all clean

all: ${OBJECTS}
	${AR} ${ARFLAGS} ${TARGET} ${OBJECTS}
	${CP} ${TARGET} ${EXPLIB}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -rf *.o ${TARGET} ${EXPLIB}/${TARGET}
