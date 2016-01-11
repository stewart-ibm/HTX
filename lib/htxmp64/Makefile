include ../../htx.mk

TARGET= libhtxmp64.a

LDFLAGS +=

OBJ_SUFF=.o

OBJECTS =htxmp_new${OBJ_SUFF} \
                     hxfupdatemp_new${OBJ_SUFF} \
                        hxfpatmp_new${OBJ_SUFF} \
                        hxfcbufmp_new${OBJ_SUFF} \
                        hxfsbufmp_new${OBJ_SUFF}

SRCS = $(patsubst %.o, %.c, $(OBJECTS))

CFLAGS += -DHTXTHREADED -D__HTX_LINUX__ 

.PHONY: all clean

all: ${OBJECTS}
	${AR} ${ARFLAGS} ${TARGET} ${OBJECTS}
	${CP} ${TARGET} ${EXPLIB}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -rf *.o ${TARGET} ${EXPLIB}/${TARGET}
