include ../../htx.mk

SUBDIRS=  

TARGET= lib_mm64.h 
 
.PHONY: all ${SUBDIRS}

all: ${SUBDIRS}
	${MKDIR} ${EXPINC}/iomm/
	${CP} ${TARGET} ${EXPINC}/iomm/

${SUBDIRS}:
	make -C $@

clean:
	${RM} -rf ${EXPINC}/iomm/*.h
