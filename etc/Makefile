include ../htx.mk

SUBDIRS= screens scripts screens_stx scripts_stx
SUBDIRS_CLEAN = $(patsubst %,%.clean,$(SUBDIRS))
TARGET= version
LOCAL_CLEAN=$(patsubst %,%.cleanl,${TARGET})

.PHONY: all ${SUBDIRS} clean ${SUBDIRS_CLEAN} clean_local ${LOCAL_CLEAN}

all: ${SUBDIRS}
	${CP} ${TARGET} ${ETC}

${SUBDIRS}:
	@echo "making dir - "${ETC}
	${MKDIR} ${ETC}
	make -C $@

clean: ${SUBDIRS_CLEAN}
	${RM} -f ${ETC}/version

${SUBDIRS_CLEAN}:
	@make -C $(@:.clean=) clean

%.clean: %
	@make -C $< clean
