include ../../htx.mk

TARGET= libmisc64.a

LDFLAGS +=

OBJ_SUFF=.o

OBJECTS = misc${OBJ_SUFF} xscom${OBJ_SUFF}

SRCS = $(patsubst %.o, %.c, $(OBJECTS))

CFLAGS += -D_GNU_SOURCE 

.PHONY: all clean

all: ${OBJECTS}
	${AR} ${ARFLAGS} ${TARGET} ${OBJECTS}
	${CP} ${TARGET} ${EXPLIB}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -rf *.o ${TARGET} ${EXPLIB}/${TARGET}
