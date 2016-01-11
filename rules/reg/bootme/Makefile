include ../../../htx.mk

TARGET= default 
 

.PHONY: all clean

all: ${TARGET}
	${MKDIR} ${REGRULES}/bootme/
	${CP} ${TARGET} ${REGRULES}/bootme/ 

clean:
	${RM} -rf ${REGRULES}/bootme/*
