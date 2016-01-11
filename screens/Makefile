include /htx.mk

TARGET= AD_scn     \
        ARTD_scn    \
        RD_scn    \
        ddhelp_scn    \
        htx_scn    \
        mmenu_scn.S \
        AD_scn.a   \
        ARTD_scn.a  \
        RD_scn.a  \
        ddhelp_scn.a  \
        htx_scn.a  \
        mmenu_scn.a \
        AHD_scn    \
        COE_scn     \
        TD_scn    \
        dst_scn   \
        mmhelp_scn \
        AHD_scn.a  \
        COE_scn.a   \
        TD_scn.a  \
        dst_scn.a  \
        mmenu_scn  \
        mmhelp_scn.a 
 

.PHONY: all clean

all: ${TARGET}
	${MKDIR} ${SCREENS}
	${CP} ${TARGET} ${SCREENS}

clean:
	${RM} -rf ${SCREENS}/*
