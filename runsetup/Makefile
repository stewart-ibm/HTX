include ../htx.mk

TARGET= ablink.runsetup xyzlink.runsetup cpu.runsetup fpu.runsetup

.PHONY: all clean

all: ${TARGET}
	@echo "making dir - "${RUNSETUP}
	${MKDIR} ${RUNSETUP}
	${CP} ${TARGET} ${RUNSETUP}

clean:
	${RM} -rf ${RUNSETUP}/*.runsetup
