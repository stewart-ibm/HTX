include ../htx.mk

TARGET= mdt.readme 

.PHONY: all clean

all: ${TARGET}
	${MKDIR} ${MDT}
	${CP} ${TARGET} ${MDT}

clean:
	${RM} -rf ${MDT}/${TARGET}
