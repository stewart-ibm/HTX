include ../../../htx.mk

TARGET= \
	1200mb-c \
        150mb \
        4mm2gb \
        4mm2gb2 \
        4mm4gb \
        525mb \
        7332 \
        8mm \
        8mm5gb \
        9trk \
        hid4mm \
        hxetape.readme \
        ost \
        scsd \
        scsd.mfg

.PHONY: all clean

all: ${TARGET}
	${MKDIR} ${REGRULES}/hxetape/
	${CP} ${TARGET} ${REGRULES}/hxetape/ 

clean:
	${RM} -rf ${REGRULES}/hxetape/*
