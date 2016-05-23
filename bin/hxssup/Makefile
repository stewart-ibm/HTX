include ../../htx.mk

TARGET= hxssup

LDFLAGS +=

OBJ_SUFF=.o
OBJECTS = nf${OBJ_SUFF} \
        start_msg${OBJ_SUFF} \
        shell${OBJ_SUFF} \
        screens${OBJ_SUFF} \
        mmenu${OBJ_SUFF} \
        misc${OBJ_SUFF} \
        hxssup${OBJ_SUFF} \
        help${OBJ_SUFF} \
        search_win${OBJ_SUFF} \
        hang_monitor${OBJ_SUFF} \
        get_dispid${OBJ_SUFF} \
        erpt${OBJ_SUFF} \
        edit${OBJ_SUFF} \
        disp_dst${OBJ_SUFF} \
        build_ipc${OBJ_SUFF} \
        T_device${OBJ_SUFF} \
        R_device${OBJ_SUFF} \
        IT_device${OBJ_SUFF} \
        COE_devic${OBJ_SUFF} \
        A_device${OBJ_SUFF} \
        ART_device${OBJ_SUFF} \
        AH_system${OBJ_SUFF} \
        AH_device${OBJ_SUFF} \
        random_ahd${OBJ_SUFF} \
        equaliser${OBJ_SUFF} \
	hotplug${OBJ_SUFF}
 
SRCS = $(patsubst %.o, %.c, $(OBJECTS))

CFLAGS += -D_GNU_SOURCE

LIBS += -Wl,--start-group
LIBS += -lrt -lhtx64 -lpthread -lc -lcfgc64 -lncurses -ltinfo -lc
LIBS += -Wl,--end-group

.PHONY: all clean

all: ${OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o ${TARGET}
	${CP} ${TARGET} ${SHIPBIN}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}
