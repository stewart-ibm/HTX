include ../../htx.mk

.PHONY: all clean ${TARGET}

TARGET = \
	create_shift_mem_L4test.awk \
	htxlogout  \
	runsup  \
	devconfig    \
	htx.cleanup  \
	heconf   \
	htxinfo.pl \
	htxselect.pl \
	heload	\
	net.mdt	\
	exer_cleanup  \
	bootme        \
	bootmeStuff   \
	boot_pre   \
	gen_sysdata \
	gen_vpd   \
	exer_setup \
	create.trial.log.linux	\
	ver \
	devlist.awk \
	rem_usb_floppy.awk \
	create_mdt_with_devices.awk \
	create_mdt_without_devices.awk \
	part.pl	\
	ioctl.ph \
	create_my_mdt \
	htxd_run \
	htxd_shutdown \
	htxd_cleanup \
	htxd_bootme \
	htx.d \
	htxconf.awk		\
	create_ab_mem_mdt \
	get_node_details.sh  \
	create_mdt_ab \
	create_mdt_xyz \
	get_mem_node_details.sh \
	create_my_sctu_stanzas.awk \
	check_disk \
	get_mem_pool_details.sh \
	htx_setup.sh 

ifeq ($(HTX_RELEASE), $(filter ${HTX_RELEASE}, "htxfedoraBE" "htxltsbml"))
	TARGET += create_mdt_mpp
endif
 
 

all: ${TARGET}
	@echo "making dir - "${SCRIPTS}
	${MKDIR} ${SCRIPTS}
	${CP} ${TARGET} ${SCRIPTS}
