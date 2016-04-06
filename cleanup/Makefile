include ../htx.mk

TARGET= hxecom.cleanup
 

.PHONY: all clean

all: ${TARGET}
	@echo "making dir - "${CLEANUP}
	${MKDIR} ${CLEANUP}
	${CP} ${TARGET} ${CLEANUP}

clean:
	${RM} -rf ${CLEANUP}/*.cleanup
