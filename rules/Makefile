include ../htx.mk

SUBDIRS=reg
SUBDIRS_CLEAN = $(patsubst %,%.clean,$(SUBDIRS))

.PHONY: all ${SUBDIRS} clean ${SUBDIRS_CLEAN}

all: ${SUBDIRS}
	${MKDIR} ${REGRULES} 

${SUBDIRS}:
	make -C $@

clean: ${SUBDIRS_CLEAN}

${SUBDIRS_CLEAN}:
	@make -C $(@:.clean=) clean

%.clean: %
	@make -C $< clean
