include ../../htx.mk

TARGET= libiomm64.a

LDFLAGS +=

OBJ_SUFF=.o

OBJECTS = lib_mm64${OBJ_SUFF} 

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
