include ../htx.mk

SUBDIRS=cfgc64 gr64 htx64 iomm64 libsimstub
SUBDIRS_CLEAN = $(patsubst %,%.clean,$(SUBDIRS))

.PHONY: all ${SUBDIRS} clean ${SUBDIRS_CLEAN}

all: ${SUBDIRS}
	${MKDIR} ${EXPLIB}

${SUBDIRS}:
	@echo "making dir - "${EXPLIB}
	${MKDIR} ${EXPLIB}
	$(MAKE) -C $@

clean: ${SUBDIRS_CLEAN}

${SUBDIRS_CLEAN}:
	@$(MAKE) -C $(@:.clean=) clean

%.clean: %
	@$(MAKE) -C $< clean

