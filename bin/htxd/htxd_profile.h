/* @(#)64	1.2  src/htx/usr/lpp/htx/bin/htxd/htxd_profile.h, htxd, htxubuntu 8/23/15 23:34:43 */



#ifndef HTXD__PROFILE__HEADER
#define HTXD__PROFILE__HEADER


/* store .htx_profile config details */
typedef struct
{
	int	emc_run_mode;
	char	max_htxerr_size[16];
	char	htxerr_wrap[8];
	char	max_htxerr_save_size[16];
	char	max_htxmsg_size[16];
	char	htxmsg_wrap[8];
	char	htxmsg_archive[8];
	char	max_htxmsg_save_size[16];
	int	hang_monitor_period;
	int	slow_shutdown_wait;
	int	max_added_devices;
	char	stress_device[64];
	char	stress_cycle[64];
	int	max_exerciser_entries;
	int	max_ecg_entries;
	int	auto_start;
	int	dr_alarm_value;
	char	dr_restart[8];
	int	hotplug_signal_delay;
	int	hotplug_restart_delay;
} htxd_profile;


extern int	htxd_init_profile(htxd_profile **);
extern void	htxd_display_profile(htxd_profile *);
extern int	htxd_get_max_add_device(void);
extern int	htxd_get_emc_mode(void);
extern int	htxd_get_slow_shutdown_wait(void);
extern int	htxd_get_max_exer_entries(void);
extern int	htxd_reload_htx_profile(htxd_profile **);
extern int	htxd_get_profile_max_exer_entries(void);
extern int	htxd_get_hang_monitor_period(void);
extern int	htxd_get_dr_alarm_value(void);
extern char *	htxd_get_dr_restart_flag(void);
extern int	htxd_get_hotplug_signal_delay(void);
extern int	htxd_get_hotplug_restart_delay(void);


#endif
