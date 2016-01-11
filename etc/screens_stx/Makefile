include ../../htx.mk

TARGET= AD_scn \
        AD_scn.a \
        AHD_scn \
        AHD_scn.a \
        ARTD_scn \
        ARTD_scn.a \
        COE_scn \
        COE_scn.a \
        RD_scn \
        RD_scn.a \
        TD_scn \
        TD_scn.a \
        ddhelp_scn \
        ddhelp_scn.a \
        dst_scn \
        dst_scn.a \
        ecg_scn  \
        ecg_scn.a \
        func_scn \
        func_scn.a \
        htx_scn \
        htx_scn.a \
        mmenu_scn \
        mmenu_scn.a \
        mmhelp_scn \
        mmhelp_scn.a \
        s0help_scn \
        s0help_scn.a \
        s2help_scn \
        s2help_scn.a \
        s3help_scn \
        s3help_scn.a \
        s9help_scn \
        s9help_scn.a \
        sRhelp_scn  \
        sRhelp_scn.a \
        sThelp_scn \
        sThelp_scn.a \
        sahelp_scn \
        sahelp_scn.a
 
 

.PHONY: all clean ${TARGET}

all: ${TARGET}
	${MKDIR} ${SHIPDIR}/etc/screens_stx/
	${CP} ${TARGET} ${SHIPDIR}/etc/screens_stx/

clean:
	${RM} -rf ${SHIPDIR}/etc/screens_stx/*
