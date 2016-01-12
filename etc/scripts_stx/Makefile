include ../../htx.mk

TARGET= eMerge.pl \
        runstx \
        stopstx \
        stx.post_i \
        addlog.pl \
        addstxround.pl \
        runsyslogd 
 

.PHONY: all clean

all: ${TARGET}
	@echo "making dir - "${SCRIPTS_STX}
	${MKDIR} ${SCRIPTS_STX}
	${CP} ${TARGET} ${SCRIPTS_STX}

clean:
	${RM} -f ${SCRIPTS_STX}/*
