include ../../../htx.mk

TARGET= \
	sample_rule default hxecom.readme

.PHONY: all clean

all: ${TARGET}
	${MKDIR} ${REGRULES}/hxecom/
	${CP} ${TARGET} ${REGRULES}/hxecom/ 

clean:
	${RM} -rf ${REGRULES}/hxecom/* 
