include ../../../htx.mk

TARGET= \
	rules.default

.PHONY: all clean

all: ${TARGET}
	${MKDIR} ${REGRULES}/hxerng/
	${CP} ${TARGET} ${REGRULES}/hxerng/ 

clean:
	${RM} -rf ${REGRULES}/hxerng/*
