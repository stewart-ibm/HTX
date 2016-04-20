include ../../htx.mk

SUBDIRS=  

TARGET= misclib.h 
 
.PHONY: all ${SUBDIRS}

all: ${SUBDIRS}
	${MKDIR} ${EXPINC}/sys/
	${CP} ${TARGET} ${EXPINC}/sys/

${SUBDIRS}:
	$(MAKE) -C $@

clean:
	${RM} -rf ${EXPINC}/sys/*.h 
