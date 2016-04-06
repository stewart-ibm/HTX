include ../htx.mk

SUBDIRS=cfgc64 gr64 htx64 htxmp64 iomm64 libsimstub
SUBDIRS_CLEAN = $(patsubst %,%.clean,$(SUBDIRS))

.PHONY: all ${SUBDIRS} clean ${SUBDIRS_CLEAN}

all: ${SUBDIRS}
	${MKDIR} ${EXPLIB}

${SUBDIRS}:
	@echo "making dir - "${EXPLIB}
	${MKDIR} ${EXPLIB}
	make -C $@

clean: ${SUBDIRS_CLEAN}

${SUBDIRS_CLEAN}:
	@make -C $(@:.clean=) clean

%.clean: %
	@make -C $< clean

