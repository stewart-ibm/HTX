include ../../htx.mk

TARGET= libhtx64.a

LDFLAGS +=

OBJ_SUFF=.o

OBJECTS =hxfupdate${OBJ_SUFF} \
        hxfpat${OBJ_SUFF} \
        hxfmsg${OBJ_SUFF} \
        hxfcbuf${OBJ_SUFF} \
        linux${OBJ_SUFF} \
        keycheck${OBJ_SUFF} \
        hxfSigSafe${OBJ_SUFF}

SRCS = $(patsubst %.o, %.c, $(OBJECTS))

CFLAGS +=  

.PHONY: all clean

all: ${OBJECTS}
	${AR} ${ARFLAGS} ${TARGET} ${OBJECTS}
	${CP} ${TARGET} ${EXPLIB}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -rf *.o ${TARGET} ${EXPLIB}/${TARGET}
