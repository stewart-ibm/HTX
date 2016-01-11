/* @(#)73	1.5  src/htx/usr/lpp/htx/bin/htxd/htxd_util.h, htxd, htxubuntu 9/15/15 20:28:45 */



#ifndef HTXD__UTIL__HEADER
#define HTXD__UTIL__HEADER

#include "hxiipc.h"

extern char *	htxd_unquote(char *);
extern int	htxd_is_file_exist(char *);
extern int	htxd_get_time_details(time_t, char *, char *, char *);
extern void	htxd_set_FD_close_on_exec_flag(void);
extern void	htxd_reset_FD_close_on_exec_flag(void);
extern void	htxd_set_value_FD_close_on_exec_flag(int);
extern pid_t	htxd_create_child_process(void);
extern int	htxd_read_file(char *, char **);
extern int	htxd_is_all_device_found(char *);
extern void	htxd_correct_device_list_for_all_devices(char *);
extern int	htxd_get_regular_file_count(char *);
extern int	htxd_wrtie_mdt_list(char *, char *, int, char *, char *);
extern short	htxd_send_message(char *, int, int, mtyp_t);
extern short send_message(char *, int, int, mtyp_t);
extern int 	htxd_execute_shell_profile(void);
extern int	htxd_truncate_error_file(void);

#endif
