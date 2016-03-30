include ../htx.mk

TARGET= equaliser.readme building_htx.txt HTX_user_manual.txt \
	HTX_developers_manual.txt htxd_user_manual.pdf network_setup_procedure.txt

.PHONY: all clean

all:
	@echo "making dir - "${SHIPDOCDIR}
	${MKDIR} ${SHIPDOCDIR}
	${CP} ${TARGET} ${SHIPDOCDIR}

clean:
	@echo "Removing dir - "${SHIPDOCDIR}
	${RM} -rf ${SHIPDOCDIR}
