include ../../htx.mk

SUBDIRS=  

TARGET= misclib.h 
 
.PHONY: all ${SUBDIRS}

all: ${SUBDIRS}
	${MKDIR} ${EXPINC}/sys/
	${CP} ${TARGET} ${EXPINC}/sys/

${SUBDIRS}:
	make -C $@

clean:
	${RM} -rf ${EXPINC}/sys/*.h 
