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
	${MKDIR} ${SHIPDIR}/usr/lpp/htx/
	${CP} ${TARGET} ${SHIPDIR}/usr/lpp/htx/

.PHONY: clean ${SUBDIRS_CLEAN} clean_local

clean: ${SUBDIRS_CLEAN} clean_local

${SUBDIRS_CLEAN}:
	@make -C $(@:.clean=) clean

%.clean: %
	@make -C $< clean

#clean_local:
#	${RM} -rf ${SHIPDIR}/usr/lpp/htx/${TARGET}
