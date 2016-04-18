include ../htx.mk

SUBDIRS=reg
SUBDIRS_CLEAN = $(patsubst %,%.clean,$(SUBDIRS))

.PHONY: all ${SUBDIRS} clean ${SUBDIRS_CLEAN}

all: ${SUBDIRS}
	@echo "making dir - "${REGRULES}
	${MKDIR} ${REGRULES} 

${SUBDIRS}:
	$(MAKE) -C $@

clean: ${SUBDIRS_CLEAN}

${SUBDIRS_CLEAN}:
	@$(MAKE) -C $(@:.clean=) clean

%.clean: %
	@$(MAKE) -C $< clean
