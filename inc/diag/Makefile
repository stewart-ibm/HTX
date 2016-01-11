include ../../htx.mk

TARGET= diag_err.h diag_intr.h diag_proto.h 
TARGET_CLEAN=$(patsubst %,%.clean,$(TARGET)) 

.PHONY: all ${TARGET} clean ${TARGET_CLEAN}

all: ${TARGET}
	${MKDIR} ${EXPINC}/diag/
	${CP} ${TARGET} ${EXPINC}/diag/

${SUBDIRS}:
	make -C $@

clean: ${TARGET_CLEAN}

${TARGET_CLEAN}:
	${RM} -rf ${EXPINC}/diag/$<$(@:.clean=)

%.clean: %
	${RM} -rf ${EXPINC}/diag/$<
