include ../htx.mk

TARGET= htx.setup mem.setup equaliser.setup hxecom.setup

ifeq (${HTX_RELEASE},"htxltsbml")
TARGET+=ioad.setup 
endif

.PHONY: all clean

all: ${TARGET}
	${MKDIR} ${SETUP}
	${CP} ${TARGET} ${SETUP}

clean:
	${RM} -rf ${SETUP}/*
