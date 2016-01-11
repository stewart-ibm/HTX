/* @(#)66	1.1  src/htx/usr/lpp/htx/bin/htxd/htxd_signal.h, htxd, htxubuntu 7/17/13 09:10:11 */



#ifndef HTXD__SIGNAL__HEADER
#define HTXD__SIGNAL__HEADER


extern void register_signal_handlers(void);
extern int htxd_send_SIGTERM(pid_t);
extern int htxd_send_SIGKILL(pid_t);
extern int htxd_send_SIGUSR1(pid_t);


#endif
