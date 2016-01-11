include ../../htx.mk

TARGET= libgr64.a

LDFLAGS +=

OBJ_SUFF=.o

OBJECTS = sys_err${OBJ_SUFF} \
        strlencpy${OBJ_SUFF} \
        rule_file${OBJ_SUFF} \
        pro_rdt${OBJ_SUFF} \
        htx_err${OBJ_SUFF} \
        getrule${OBJ_SUFF} 

SRCS = $(patsubst %.o, %.c, $(OBJECTS))

CFLAGS += -DLIBC_23 

.PHONY: all clean

all: ${OBJECTS}
	${AR} ${ARFLAGS} ${TARGET} ${OBJECTS}
	${CP} ${TARGET} ${EXPLIB}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -rf *.o ${TARGET} ${EXPLIB}/${TARGET}
