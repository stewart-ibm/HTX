HTX_RELEASE="htxubuntu"
MKDIR=/bin/mkdir -p
CC=/usr/bin/gcc -m64
CPP=/usr/bin/g++ -m64
RM=/bin/rm
CP=/bin/cp
AR=/usr/bin/ar
LD=/usr/bin/ld
ifeq ($(HTX_RELEASE), $(filter ${HTX_RELEASE},"htxubuntu","htxsles12","htxrhel7LE"))
	CFLAGS=-D__HTX_LINUX__ -DTRUE=1 -DFALSE=0 -D__HTX_LE__ -D__64BIT__
else
	CFLAGS=-D__HTX_LINUX__ -DTRUE=1 -DFALSE=0 -D__64BIT__
endif
LDFLAGS=

#Define TOPDIR as source top
TOPDIR=
INCLUDES=-I./ -I/usr/include/ -I${EXPINC}
SHIPDIR=${TOPDIR}/inst.images/ppc64_linux_2/
SCRIPTS=${SHIPDIR}/usr/lpp/htx/etc/scripts/
RUNCLEANUP=${SHIPDIR}/usr/lpp/htx/runcleanup/
RUNSETUP=${SHIPDIR}/usr/lpp/htx/runsetup/
EXPORT=${TOPDIR}/export/ppc64_linux_2/
EXPINC=${EXPORT}/include/
EXPLIB=${EXPORT}/lib/
LIBPATH=-L${EXPLIB}/
SHIPBIN=${SHIPDIR}/usr/lpp/htx/bin/
CLEANUP=${SHIPDIR}/usr/lpp/htx/cleanup/
PATTERN=${SHIPDIR}/usr/lpp/htx/pattern/
REGRULES=${SHIPDIR}/usr/lpp/htx/rules/reg/
SETUP=${SHIPDIR}/usr/lpp/htx/setup/
MDT=${SHIPDIR}/usr/lpp/htx/mdt/
SCREENS=${SHIPDIR}/usr/lpp/htx/screens/
DOCUMENTATION=${SHIPDIR}/usr/lpp/htx/Documentation/
