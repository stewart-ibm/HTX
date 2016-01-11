include ../../htx.mk

SUBDIRS= utils
TARGET= hxecd 
all: ${TARGET} ${SUBDIRS}

LDFLAGS +=

OBJ_SUFF=.o
OBJECTS = hxecd${OBJ_SUFF} \
                        get_rule${OBJ_SUFF} \
                        proc_rule${OBJ_SUFF} \
                        buf_oper${OBJ_SUFF} \
                        io_oper${OBJ_SUFF} \
                        prt_msg${OBJ_SUFF} \
                        rnd_oper${OBJ_SUFF} \
                        seq_oper${OBJ_SUFF}
 
SRCS = $(patsubst %.o, %.c, $(OBJECTS))
SUBDIRS_CLEAN = $(patsubst %,%.clean,$(SUBDIRS))

CFLAGS += -D__DEFINE_PAGE_MACROS__ -D_FILE_OFFSET_BITS=64 -DFALSE=0 -DTRUE=1
 
LIBS = -lpthread -lc -lhtx64 -lmisc64

.PHONY: all clean ${SUBDIRS} ${SUBDIRS_CLEAN}

${TARGET}: ${OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o ${TARGET} 
	${CP} ${TARGET} ${SHIPBIN}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

${SUBDIRS}:
	make -C $@

clean: ${SUBDIRS_CLEAN}
	${RM} -f *.o  ${SHIPBIN}/${TARGET}

${SUBDIRS_CLEAN}:
	@make -C $(@:.clean=) clean

%.clean: %
	@make -C $< clean

