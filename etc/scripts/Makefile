include ../../htx.mk

.PHONY: all clean ${TARGET}

TARGET = \
	create_shift_sctu_stanzas.awk \
	create_shift_mem_L4test.awk \
	htxlogout  \
	runsup  \
	devconfig    \
	htx.cleanup  \
	heconf   \
	htxinfo.pl \
	htxselect.pl \
	stx.cleanup \
	heload	\
	net.mdt	\
	exer_cleanup  \
	bootme        \
	bootmeStuff   \
	boot_pre   \
	stx_bootme    \
	stx_bootmeStuff  \
	gen_vpd   \
	exer_setup \
	create.trial.log.linux	\
	ver \
	devlist.awk \
	rem_usb_floppy.awk \
	update_kernmod  \
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
	create_mdt_powermixer.sh \
	create_ab_mem_mdt \
	get_node_details.sh  \
	create_mdt_ab \
	create_mdt_xyz \
	get_mem_node_details.sh \
	create_my_sctu_stanzas.awk \
	parse_equaliser_configuration_file.awk \
	create_cfg_maxpower.pl \
	create_mdt_maxpower.pl \
	create_mdt_tpsm \
	create_mdt_tpsm_new \
	patch_rules_file_tpsm.pl \
	create_all_mdt_with_tpsm \
	add_mdts \
	check_disk \
	start_shift \
	start_stx_lpar \
	stop_shift \
	collect_shift_logs \
	stx_shift_framework.readme \
	create_all_mdt_with_powermixer

ifeq ($(HTX_RELEASE), $(filter ${HTX_RELEASE}, "htxfedoraBE" "htxltsbml"))
	TARGET += create_mdt_mpp
endif
 
ifeq (${HTX_RELEASE},"htxubuntu")
 	TARGET += create_surelock_mdt 
endif	
 
ifeq (${HTX_RELEASE}, "htxltsbml")
	create_mdt_fastipi \
	create_th_trans_mix_cfg.pl \
	create_th_trans_switch_cfg.pl 
endif

all: ${TARGET}
	@echo "making dir - "${SCRIPTS}
	${MKDIR} ${SCRIPTS}
	${CP} ${TARGET} ${SCRIPTS}
