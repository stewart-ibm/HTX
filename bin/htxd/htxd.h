/* @(#)35	1.5  src/htx/usr/lpp/htx/bin/htxd/htxd.h, htxd, htxubuntu 11/24/15 23:59:18 */



#ifndef HTXD__HEADER
#define HTXD__HEADER


#include <unistd.h>

#include "htxd_profile.h"
#include "htxd_ecg.h"
#include "htxd_common_define.h"
#include "htxd_thread.h"


#define		HTXD_LIGHTWEIGHT		10
#define		HTXD_SHUTDOWN			20


#define		HTXD_TRACE_NO			0
#define		HTXD_TRACE_LOW			1
#define		HTXD_TRACE_MEDIUM		2
#define		HTXD_TRACE_HIGH			3


#define		HTXD_DAEMON_UNVALIDATED		0
#define		HTXD_DAEMON_IDLE		1
#define		HTXD_DAEMON_SELECTED		2
#define		HTXD_DAEMON_RUNNING		3
#define		HTXD_DAEMON_BUSY		4


typedef struct
{
	int	command_index;
	char	ecg_name[MAX_ECG_NAME_LENGTH];
	char	option_list[MAX_OPTION_LIST_LENGTH];
} htxd_command;



typedef struct
{
	htxd_ecg_manager *	p_ecg_manager;
	htxd_thread *		p_hang_monitor_thread;
	htxd_thread *		p_hotplug_monitor_thread;
	char			program_name[80];
	char			htx_path[80];
	int			shutdown_flag;
	pid_t			daemon_pid;
	pid_t			htx_msg_pid;
	pid_t			htx_stats_pid;
	pid_t			dr_child_pid;
	pid_t			equaliser_pid;
	int			run_level;
	int			run_state;
	int			trace_level;
	int			port_number;
	pid_t *			p_child_pid_list;
	htxd_profile *		p_profile;
	htxd_command *		p_command;
	int			dr_sem_key;
	int			dr_sem_id;
	int			dr_reconfig_restart;
	int			dr_is_done; /* check usage */
	int			equaliser_debug_flag;
	union shm_pointers	equaliser_shm_addr;
	int			equaliser_semhe_id;
	char *			equaliser_conf_file;
	int			is_auto_started;
	int			init_syscfg_flag;
} htxd;

extern htxd *htxd_global_instance;

extern int htxd_start_daemon(htxd*);

#endif
