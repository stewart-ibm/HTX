include ../../htx.mk

TARGET= hxsmsg

LDFLAGS +=

OBJ_SUFF=.o
OBJECTS = write_log${OBJ_SUFF} upd_ckpt${OBJ_SUFF} sum${OBJ_SUFF} start_hrt${OBJ_SUFF} start_display${OBJ_SUFF} set_sig${OBJ_SUFF} send_sig${OBJ_SUFF} send_msg${OBJ_SUFF} rm_file${OBJ_SUFF} read_msg${OBJ_SUFF} open_logs${OBJ_SUFF} open_ipc${OBJ_SUFF} open_file${OBJ_SUFF} max_filesz${OBJ_SUFF} hxsmsg${OBJ_SUFF} do_argv${OBJ_SUFF} copy_msg${OBJ_SUFF} cls_file${OBJ_SUFF} clean_up${OBJ_SUFF} ck_values${OBJ_SUFF} check_wrap${OBJ_SUFF} check_logs${OBJ_SUFF} SIGTERM_h${OBJ_SUFF} SIGCHLD_h${OBJ_SUFF} 
 
SRCS = $(patsubst %.o, %.c, $(OBJECTS))

CFLAGS += 
LIBS = -lhtx64

.PHONY: all clean

all: ${OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o ${TARGET}
	${CP} ${TARGET} ${SHIPBIN}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}
