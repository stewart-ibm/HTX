include ../htx.mk

TARGET= mem.setup equaliser.setup hxecom.setup

.PHONY: all clean

all: ${TARGET}
	@echo "making dir - "${SETUP}
	${MKDIR} ${SETUP}
	${CP} ${TARGET} ${SETUP}

clean:
	${RM} -rf ${SETUP}/*
