include ../../htx.mk

TARGET= htxd htxcmdline 
all = ${TARGET}

LDFLAGS +=

OBJ_SUFF=.o
htxd_OBJECTS = htxd.o \
		htxd_daemon${OBJ_SUFF} \
		htxd_autostart${OBJ_SUFF} \
		htxd_process_command${OBJ_SUFF} \
		htxd_option_method_create_mdt${OBJ_SUFF} \
		htxd_option_method_list_mdt${OBJ_SUFF} \
		htxd_option_method_run_mdt${OBJ_SUFF} \
		htxd_option_method_misc${OBJ_SUFF} \
		htxd_option_method_shutdown_mdt${OBJ_SUFF} \
		htxd_option_list${OBJ_SUFF} \
		htxd_profile${OBJ_SUFF} \
		htxd_instance${OBJ_SUFF} \
		htxd_util${OBJ_SUFF} \
		htxd_ecg${OBJ_SUFF} \
		htxd_ipc${OBJ_SUFF} \
		htxd_signal${OBJ_SUFF} \
		htxd_exer${OBJ_SUFF} \
		htxd_thread${OBJ_SUFF} \
		htxd_hang_monitor${OBJ_SUFF} \
		htxd_dr${OBJ_SUFF} \
		htxd_hotplug${OBJ_SUFF} \
		htxd_equaliser${OBJ_SUFF} \
		htxd_socket${OBJ_SUFF}	

htxcmdline_OBJECTS = htxcmd${OBJ_SUFF} \
		htxcmd_display_result${OBJ_SUFF} \
		htxcmd_parse_command${OBJ_SUFF} \
		htxcmd_socket${OBJ_SUFF}
 

htxd_SRCS = $(patsubst %.o, %.c, $(htxd_OBJECTS))
htxcmdline_SRCS = $(patsubst %.o, %.c, $(htxcmdline_OBJECTS))

all= htxd htxcmdline 

ifneq "${HTX_RELEASE}" "htxltsbml"
#        CFLAGS += -DLIC_ENABLE
endif

CFLAGS += -D__HTX_LINUX_ -D_GNU_SOURCE -DTRUE=1 -DFALSE=0

LIBS += -Wl,--start-group
LIBS += -lhtx64 -lmisc64 -lhtxsyscfg64 -lpthread -lcfgc64
LIBS += -Wl,--end-group 

.PHONY: all clean

htxd: ${htxd_OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${htxd_OBJECTS} ${LIBS} -o htxd 
	${CP} htxd ${SHIPBIN}

htxcmdline: ${htxcmdline_OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${htxcmdline_OBJECTS} ${LIBS} -o htxcmdline 
	${CP} htxcmdline ${SHIPBIN}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}