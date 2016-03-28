include ../htx.mk

TARGET= ALTPADS   \
        DATA00FF  \
        HEXAA     \
        HXPC5    \
        HXPPATH   \
        PROPAT    \
        tmdef01  \
        ASCII     \
        DIAG1     \
        HEXFF     \
        HXPDB6   \
        IBM5084   \
        SAL       \
        tmdef02  \
        CDM102    \
        EMCPRINT  \
        HEXZEROS  \
        HXPFFB   \
        KEYBOARD  \
        SCROLL_H  \
        tmdef03  \
        CDM2F102  \
        HEX255    \
        HXPA5     \
        HXPPAT1  \
        PBEFILE   \
        tmdef04  \
        CDM2F202  \
        HEX55     \
        HXPB5     \
        HXPPAT2  \
        POPTIONS  \
        DCUTEST  \
        CDDVD001 \
        CDDVD002 \
        CDDVD101 \
        CDDVD102 \
        CDDVDS5  \
        CDDVDS9
 

.PHONY: all clean

all: ${TARGET}
	${MKDIR} ${PATTERN}
	${CP} ${TARGET} ${PATTERN}

clean:
	${RM} -rf ${PATTERN}/*
