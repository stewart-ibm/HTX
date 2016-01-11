include ../../../htx.mk

TARGET= \
	default.dvd default0 hxehd.doc typical default default.old hdsound maximum hd.cache default.perf rules.seq1090 rules.seq4060 rules.seq5050 default.hdbuster rules.app_server rules.web_server rules.business_int_db_opk rules.business_int_db_pk rules.comm_db_opk rules.comm_db_pk rules.hpc_comp_node rules.hpc_io_node certify easytier

.PHONY: all clean

all: ${TARGET}
	${MKDIR} ${REGRULES}/hxehd/
	${CP} ${TARGET} ${REGRULES}/hxehd/ 

clean:
	${RM} -rf ${REGRULES}/hxehd/*
