include ../../htx.mk

TARGET=stxclient 

LDFLAGS +=

OBJ_SUFF=.o
OBJECTS = nf${OBJ_SUFF} \
shell${OBJ_SUFF}        \
screens${OBJ_SUFF}      \
mmenu${OBJ_SUFF}        \
misc${OBJ_SUFF}         \
htxclient${OBJ_SUFF}    \
help${OBJ_SUFF}         \
get_dispid${OBJ_SUFF}   \
erpt${OBJ_SUFF}         \
edit${OBJ_SUFF}         \
disp_dst${OBJ_SUFF}     \
T_device${OBJ_SUFF}     \
R_device${OBJ_SUFF}     \
COE_devic${OBJ_SUFF}    \
A_device${OBJ_SUFF}     \
ART_device${OBJ_SUFF}   \
AH_device${OBJ_SUFF}    \
init_screen${OBJ_SUFF} 	\
client${OBJ_SUFF} 	\
func_menu${OBJ_SUFF} 	\
parse_input${OBJ_SUFF} 	\
disp_ecg${OBJ_SUFF}
 
SRCS = $(patsubst %.o, %.c, $(OBJECTS))

LIBS += -lpthread -lcfgc64 -lncurses -lhtx64

.PHONY: all clean

all: ${OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o ${TARGET}
	${CP} ${TARGET} ${SHIPBIN}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}
