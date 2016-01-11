include ../../htx.mk

TARGET= eservd 

LDFLAGS +=

OBJ_SUFF=.o
OBJECTS = misc${OBJ_SUFF}         \
        eservd${OBJ_SUFF}       \
        hang_monitor${OBJ_SUFF} \
        build_ipc${OBJ_SUFF}    \
	start_msg${OBJ_SUFF} \
        T_device${OBJ_SUFF}     \
        R_device${OBJ_SUFF}     \
        COE_devic${OBJ_SUFF}    \
        A_device${OBJ_SUFF}     \
        AH_system${OBJ_SUFF}    \
        AH_device${OBJ_SUFF} \
	server${OBJ_SUFF} \
	disp_dst${OBJ_SUFF} \
	parse_cmd${OBJ_SUFF} \
	query${OBJ_SUFF} \
	ecg_get${OBJ_SUFF} \
	list${OBJ_SUFF} \
	ecg_summary${OBJ_SUFF} \
	test_summary${OBJ_SUFF} \
	global${OBJ_SUFF} \
	print_log${OBJ_SUFF} \
	H_device${OBJ_SUFF} \
	equaliser${OBJ_SUFF} 
 
SRCS = $(patsubst %.o, %.c, $(OBJECTS))

LIBS += -Wl,--start-group -lcfgc64 -lhtx64 -lmisc64 -lhtxsyscfg64 -lpthread -lncurses -Wl,--end-group

.PHONY: all clean

all: ${OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o ${TARGET}
	${CP} ${TARGET} ${SHIPBIN}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}
