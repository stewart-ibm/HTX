include ../../htx.mk

TARGET= hxefpscr

LDFLAGS +=

OBJ_SUFF=.o
OBJECTS = hxefpscr${OBJ_SUFF} \
        fpscr_bust${OBJ_SUFF} \
        save_regs${OBJ_SUFF} \
        sigfpe_handler${OBJ_SUFF} \
        getfpscr${OBJ_SUFF} 
 
SRCS = $(patsubst %.o, %.c, $(OBJECTS))

CFLAGS += -DFALSE=0 -DTRUE=1
 
LIBS = -lhtx64 -lpthread -lncurses -lgr64 

.PHONY: all clean

all: ${OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o ${TARGET}
	${CP} ${TARGET} ${SHIPBIN}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}
