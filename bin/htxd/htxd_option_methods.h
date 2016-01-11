/* @(#)61	1.3  src/htx/usr/lpp/htx/bin/htxd/htxd_option_methods.h, htxd, htxubuntu 9/15/15 20:28:35 */



#ifndef HTXD__OPTION__METHODS__HEADER
#define HTXD__OPTION__METHODS__HEADER
#include <netinet/in.h>
#include <errno.h>

#include "htxd.h"


typedef struct
{
	htxd *htxd_instance;
	char command_ecg_name[MAX_ECG_NAME_LENGTH];
	char command_option_list[MAX_OPTION_LIST_LENGTH];
	char error_string[512];
	int return_code;
} htxd_option_method_object;


#define OPTION_METHOD_ARGS (char **)

//typedef int(*ecg_process_function)(char*);

extern void init_option_list(void);
extern int htxd_validate_command_requirements(htxd *, char *);
extern void htxd_init_option_method(htxd_option_method_object *);
extern int htxd_process_command(char **);
//int htxd_process_all_active_ecg(ecg_process_function, char *);

extern int htxd_option_method_create_mdt OPTION_METHOD_ARGS;
extern int htxd_option_method_list_mdt OPTION_METHOD_ARGS;
extern int htxd_option_method_run_mdt OPTION_METHOD_ARGS;
extern int htxd_option_method_getactecg OPTION_METHOD_ARGS;
extern int htxd_option_method_shutdown_mdt OPTION_METHOD_ARGS;
extern int htxd_option_method_refresh OPTION_METHOD_ARGS;
extern int htxd_option_method_activate OPTION_METHOD_ARGS;
extern int htxd_option_method_suspend OPTION_METHOD_ARGS;
extern int htxd_option_method_terminate OPTION_METHOD_ARGS;
extern int htxd_option_method_restart OPTION_METHOD_ARGS;
extern int htxd_option_method_coe OPTION_METHOD_ARGS;
extern int htxd_option_method_soe OPTION_METHOD_ARGS;
extern int htxd_option_method_status OPTION_METHOD_ARGS;
extern int htxd_option_method_getstats OPTION_METHOD_ARGS;
extern int htxd_option_method_geterrlog OPTION_METHOD_ARGS;
extern int htxd_option_method_clrerrlog OPTION_METHOD_ARGS;
extern int htxd_option_method_cmd OPTION_METHOD_ARGS;
extern int htxd_option_method_set_eeh OPTION_METHOD_ARGS;
extern int htxd_option_method_set_kdblevel OPTION_METHOD_ARGS;
extern int htxd_option_method_set_hxecom OPTION_METHOD_ARGS;
extern int htxd_option_method_getvpd OPTION_METHOD_ARGS;
extern int htxd_option_method_getecgsum OPTION_METHOD_ARGS;
extern int htxd_option_method_getecglist OPTION_METHOD_ARGS;
extern int htxd_option_method_select_mdt OPTION_METHOD_ARGS;
extern int htxd_option_method_exersetupinfo OPTION_METHOD_ARGS;
extern int htxd_option_method_bootme OPTION_METHOD_ARGS;
extern int htxd_option_method_query OPTION_METHOD_ARGS;

#endif
