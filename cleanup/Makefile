include ../htx.mk

TARGET= hxecom.cleanup
 

.PHONY: all clean

all: ${TARGET}
	${MKDIR} ${CLEANUP}
	${CP} ${TARGET} ${CLEANUP}

clean:
	${RM} -rf ${CLEANUP}/*.cleanup
