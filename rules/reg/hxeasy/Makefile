include ../../../htx.mk

TARGET= default \
	lion \
        termrule \
        128port \
        wraprule 
 

.PHONY: all clean

all: ${TARGET}
	${MKDIR} ${REGRULES}/hxeasy/
	${CP} ${TARGET} ${REGRULES}/hxeasy/ 

clean:
	${RM} -rf ${REGRULES}/hxeasy/* 
