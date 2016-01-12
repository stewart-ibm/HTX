include htx.mk

SUBDIRS= inc lib bin rules etc mdt cleanup pattern setup runsetup
SUBDIRS_CLEAN = $(patsubst %,%.clean,$(SUBDIRS))

TARGET= .htx_profile \
        .htxrc \
        .bash_profile \
        .bashrc \
        .exrc \
        htx_eq.cfg \
        equaliser.readme \
	run_htx_cmdline.sh \
        hxsscripts

.PHONY: all ${SUBDIRS}

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

#clean_local:
#	${RM} -rf ${SHIPDIR}/usr/lpp/htx/${TARGET}
