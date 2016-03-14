include htx.mk

SUBDIRS= inc lib bin rules etc mdt cleanup pattern setup runsetup Documentation
SUBDIRS_CLEAN = $(patsubst %,%.clean,$(SUBDIRS))

TARGET= .htx_profile \
        .htxrc \
        .bash_profile \
        .bashrc \
        .exrc \
        htx_eq.cfg \
	run_htx_cmdline.sh \
        hxsscripts

.PHONY: all ${SUBDIRS} deb

${SUBDIRS}:
	make -C $@

all: ${SUBDIRS}
	@echo "making dir - "${SHIPTOPDIR}
	${MKDIR} ${SHIPTOPDIR}
	${CP} ${TARGET} ${SHIPTOPDIR}

.PHONY: clean ${SUBDIRS_CLEAN} clean_local

clean: ${SUBDIRS_CLEAN} clean_local
	@echo "Removing dir - "${SHIPDIR}
	${RM} -rf ${SHIPDIR}

${SUBDIRS_CLEAN}:
	@make -C $(@:.clean=) clean

%.clean: %
	@make -C $< clean

deb:
	@echo "Making Debian package..."
	cp -r $(PACKAGINGDIR)/ubuntu/* $(SHIPDIR)/
	dpkg-deb -b $(SHIPDIR)  $(TOPDIR)/htxubuntu.deb
