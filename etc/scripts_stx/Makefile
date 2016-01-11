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
	${MKDIR} ${SHIPDIR}/etc/scripts_stx/
	${CP} ${TARGET} ${SHIPDIR}/etc/scripts_stx/

clean:
	${RM} -rf ${SHIPDIR}/etc/scripts_stx/*
