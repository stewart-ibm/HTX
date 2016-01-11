include ../../htx.mk

TARGET= show_syscfg 

LDFLAGS +=

OBJ_SUFF=.o
OBJECTS = show_syscfg${OBJ_SUFF} 
SRCS = $(patsubst %.o, %.c, $(OBJECTS))

CFLAGS += -D__64_LINUX__ -D_GNU_SOURCE -DKERNEL_2_6 -D_REENTRANT -D__HTX_LINUX__ -D__USE_GNU -O2 -DGLIBC_GR 

CFLAGS += -DLOG_TO_PHY_NEEDED

LIBS = -Wl,--start-group -lhtx64 -lhtxsyscfg64 -lmisc64 -lpthread -Wl,--end-group 

.PHONY: all clean

all: ${OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o ${TARGET}
	${CP} ${TARGET} ${SHIPBIN}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}
