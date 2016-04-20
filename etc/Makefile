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
	$(MAKE) -C $@

clean: ${SUBDIRS_CLEAN}

${SUBDIRS_CLEAN}:
	@$(MAKE) -C $(@:.clean=) clean

%.clean: %
	@$(MAKE) -C $< clean
