include ../../htx.mk
TARGET= hxeasy

OBJ_SUFF=.o
LIBS +=  -lhtx64 -lmisc64 -pthread
CFLAGS	+=  -Wall -O2

OBJECTS = rules${OBJ_SUFF} classlib${OBJ_SUFF} Cevent${OBJ_SUFF} Cargdata${OBJ_SUFF} Ccom${OBJ_SUFF} \
		  Cgetspace${OBJ_SUFF} Cmsg${OBJ_SUFF} Cmsgbase${OBJ_SUFF} Cthread${OBJ_SUFF} hxeasy+${OBJ_SUFF} 

LDFLAGS +=

CFLAGS += -O1 -g
 
LIBS =-lpthread -lhtx64 

.PHONY: all clean

all: ${OBJECTS}
	${CXX} ${LDFLAGS} ${LIBPATH} ${OBJECTS} ${LIBS} -o ${TARGET}
	${CP} ${TARGET} ${SHIPBIN}

%.o: %.C
	${CXX} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}
