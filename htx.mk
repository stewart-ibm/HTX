HTX_RELEASE="htxubuntu"
ifeq ($(HTX_RELEASE), $(filter ${HTX_RELEASE},"htxubuntu" "htxsles12" "htxrhel7LE"))
ARCH=ppc64le
else
ARCH=ppc64
endif
MKDIR=/bin/mkdir -p
CC=/usr/bin/gcc -m64
CPP=/usr/bin/g++ -m64
RM=/bin/rm
CP=/bin/cp
AR=/usr/bin/ar
LD=/usr/bin/ld
LDFLAGS=
CFLAGS=-D__HTX_LINUX__ -DTRUE=1 -DFALSE=0 -D__64BIT__
ifeq ($(HTX_RELEASE), $(filter ${HTX_RELEASE},"htxubuntu" "htxsles12" "htxrhel7LE"))
	CFLAGS+= -D__HTX_LE__
endif
#Set the TOPDIR 
TOPDIR=/home/mehpatel/test_tree/HTX/
SHIPDIR=${TOPDIR}/install/${ARCH}/
SHIPTOPDIR=${SHIPDIR}/usr/lpp/htx/
INCLUDES=-I./ -I/usr/include/ -I${EXPINC}
EXPORT=${TOPDIR}/export/${ARCH}/
EXPINC=${EXPORT}/include/
EXPLIB=${EXPORT}/lib/
LIBPATH=-L${EXPLIB}/
SHIPBIN=${SHIPTOPDIR}/bin/
RUNCLEANUP=${SHIPTOPDIR}/runcleanup/
RUNSETUP=${SHIPTOPDIR}/runsetup/
CLEANUP=${SHIPTOPDIR}/cleanup
PATTERN=${SHIPTOPDIR}/pattern
REGRULES=${SHIPTOPDIR}/rules/reg/
SETUP=${SHIPTOPDIR}/setup/
MDT=${SHIPTOPDIR}/mdt/
ETC=${SHIPTOPDIR}/etc/
SCRIPTS=${ETC}/scripts/
SCRIPTS_STX=${ETC}/scripts_stx/
SCREENS=${ETC}/screens/
SCREENS_STX=${ETC}/screens_stx/

