include ../../../htx.mk

TARGET= \
	cdrom.mm dvdrom.p1 cdrom.vios hxecd.readme

.PHONY: all clean

all: ${TARGET}
	${MKDIR} ${REGRULES}/hxecd/
	${CP} ${TARGET} ${REGRULES}/hxecd/ 

clean:
	${RM} -rf ${REGRULES}/hxecd/* 
