include ../htx.mk

TARGET= equaliser.readme

.PHONY: all clean

all:
	@echo "making dir - "${SHIPDOCDIR}
	${MKDIR} ${SHIPDOCDIR}
	${CP} ${TARGET} ${SHIPDOCDIR}

clean:
	@echo "Removing dir - "${SHIPDOCDIR}
	${RM} -rf ${SHIPDOCDIR}
