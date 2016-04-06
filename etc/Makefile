include ../htx.mk

SUBDIRS= screens scripts
SUBDIRS_CLEAN = $(patsubst %,%.clean,$(SUBDIRS))
TARGET= 
LOCAL_CLEAN=$(patsubst %,%.cleanl,${TARGET})

.PHONY: all ${SUBDIRS} clean ${SUBDIRS_CLEAN} clean_local ${LOCAL_CLEAN}

all: ${SUBDIRS}

${SUBDIRS}:
	@echo "making dir - "${ETC}
	${MKDIR} ${ETC}
	make -C $@

clean: ${SUBDIRS_CLEAN}

${SUBDIRS_CLEAN}:
	@make -C $(@:.clean=) clean

%.clean: %
	@make -C $< clean
