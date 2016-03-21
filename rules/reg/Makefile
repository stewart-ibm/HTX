include ../../htx.mk

SUBDIRS= hxecd hxetape hxeasy hxetlbie hxecorsa \
        hxemem64 bootme hxecom  hxerng hxecpu hxefpu64 hxesctu \
        hxecache hxefabricbus hxesamp hxestorage 
SUBDIRS_CLEAN = $(patsubst %,%.clean,$(SUBDIRS))

.PHONY: all ${SUBDIRS} clean ${SUBDIRS_CLEAN}

all: ${SUBDIRS}
	@echo "making dir - ${REGRULES}"
	${MKDIR} ${REGRULES}

clean: ${SUBDIRS_CLEAN}

${SUBDIRS}:
	make -C $@

${SUBDIRS_CLEAN}:
	@make -C $(@:.clean=) clean

%.clean: %
	@make -C $< clean
