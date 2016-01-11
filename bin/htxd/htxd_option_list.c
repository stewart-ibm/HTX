/* @(#)53	1.3  src/htx/usr/lpp/htx/bin/htxd/htxd_option_list.c, htxd, htxubuntu 9/15/15 20:28:19 */



#include "htxd_option_methods.h"
#include "htxd_option.h"

extern option option_list[];

void init_option_list(void)
{
	int index = 0;

	option_list[index++].option_method = htxd_option_method_create_mdt;
	option_list[index++].option_method = htxd_option_method_list_mdt;
	option_list[index++].option_method = htxd_option_method_run_mdt;
	option_list[index++].option_method = htxd_option_method_getactecg;
	option_list[index++].option_method = htxd_option_method_shutdown_mdt;
	option_list[index++].option_method = htxd_option_method_refresh;
	option_list[index++].option_method = htxd_option_method_activate;
	option_list[index++].option_method = htxd_option_method_suspend;
	option_list[index++].option_method = htxd_option_method_terminate;
	option_list[index++].option_method = htxd_option_method_restart;
	option_list[index++].option_method = htxd_option_method_coe;
	option_list[index++].option_method = htxd_option_method_soe;
	option_list[index++].option_method = htxd_option_method_status;
	option_list[index++].option_method = htxd_option_method_getstats;
	option_list[index++].option_method = htxd_option_method_geterrlog;
	option_list[index++].option_method = htxd_option_method_clrerrlog;
	option_list[index++].option_method = htxd_option_method_cmd;
	option_list[index++].option_method = htxd_option_method_set_eeh;
	option_list[index++].option_method = htxd_option_method_set_kdblevel;
	option_list[index++].option_method = htxd_option_method_set_hxecom;
	option_list[index++].option_method = htxd_option_method_getvpd;
	option_list[index++].option_method = htxd_option_method_getecgsum;
	option_list[index++].option_method = htxd_option_method_getecglist;
	option_list[index++].option_method = htxd_option_method_select_mdt;
	option_list[index++].option_method = htxd_option_method_exersetupinfo;
	option_list[index++].option_method = htxd_option_method_bootme;
	option_list[index++].option_method = htxd_option_method_query;
}
