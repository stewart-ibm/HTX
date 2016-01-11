include ../../htx.mk

TARGET= libhtxsyscfg64.a

LDFLAGS +=

OBJ_SUFF=.o

OBJECTS = htxsyscfg_new${OBJ_SUFF} 

SRCS = $(patsubst %.o, %.c, $(OBJECTS))

CFLAGS += -D__64_LINUX__ -D_GNU_SOURCE -DKERNEL_2_6 -D_REENTRANT -D__HTX_LINUX__ -D__USE_GNU -O2 -DGLIBC_GR -DLOG_TO_PHY_NEEDED -DMFPVR_ALLOWED  -DGLIBC_GR

.PHONY: all clean

all: ${OBJECTS}
	${AR} ${ARFLAGS} ${TARGET} ${OBJECTS}
	${CP} ${TARGET} ${EXPLIB}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -rf *.o ${TARGET} ${EXPLIB}/${TARGET}
