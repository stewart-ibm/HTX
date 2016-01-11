include ../../htx.mk

TARGET= libcfgc64.a

LDFLAGS +=

OBJ_SUFF=.o

OBJECTS = linux${OBJ_SUFF} cfgcskwd${OBJ_SUFF} cfgcread${OBJ_SUFF} cfgcrdsz${OBJ_SUFF} cfgcopsf${OBJ_SUFF} cfgcdflt${OBJ_SUFF} cfgcclsf${OBJ_SUFF}

SRCS = $(patsubst %.o, %.c, $(OBJECTS))

CFLAGS +=  

LIBS = -lhtxmp64 -lhtx64 -lpthread -lmisc64 -lc

.PHONY: all clean

all: ${OBJECTS}
	${AR} ${ARFLAGS} ${TARGET} ${OBJECTS}
	${CP} ${TARGET} ${EXPLIB}

%.o: ../cfgc/%.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -rf *.o ${TARGET} ${EXPLIB}/${TARGET}
