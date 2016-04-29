include ../../htx.mk

TARGET= hxefabricbus 

LDFLAGS +=

OBJ_SUFF=.o
OBJECTS = main${OBJ_SUFF} \
                setup${OBJ_SUFF} \
                rf_parser${OBJ_SUFF}

SRCS = $(patsubst %.o, %.c, $(OBJECTS))

CFLAGS += -D__64_LINUX__ -D_GNU_SOURCE  -D_REENTRANT -D__HTX_LINUX__ -D_target_ppc -D__GR2600__  -O2 -DFALSE=0 -DTRUE=1
 
LIBS = -lhtx64 -lpthread 
 

.PHONY: all clean

all: ${OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o ${TARGET}
	${CP} ${TARGET} ${SHIPBIN}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}
