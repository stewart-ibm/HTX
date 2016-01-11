include ../htx.mk

TARGET= ablink.runsetup xyzlink.runsetup cpu.runsetup fpu.runsetup

.PHONY: all clean

all: ${TARGET}
	${MKDIR} ${RUNSETUP}
	${CP} ${TARGET} ${RUNSETUP}

clean:
	${RM} -rf ${RUNSETUP}/*.runsetup
