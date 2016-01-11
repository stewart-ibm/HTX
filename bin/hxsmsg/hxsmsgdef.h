/*@(#)16  1.6.4.1  src/htx/usr/lpp/htx/bin/hxsmsg/hxsmsgdef.h, htx_msg, htxubuntu 8/3/12 07:59:29*/
/* *   ORIGINS: 27 */


/*
 *  hxsmsgdef.h -- HTX Message Handler (hxsmsg) Function Declarations
 *          
 *    This include file is the respository for all HTX Message Handler
 *    (hxsmsg) function declarations.
 *
 */                                                          

#ifndef HXSMSGDEF_H
#define HXSMSGDEF_H

#include <hxihtx.h>

#include <sys/signal.h>

struct sigcontext;	/*400*/


  /*
   *  function to open a file
   */
  int open_file PARMS((char *, int, int));



  /*
   *  function to read ipc messages 
   */
  mtyp_t read_message PARMS((struct htx_msg_xbuf *));



  /*
   *  function to check the specified maximum log threshold value.
   */
  off_t ck_threshold PARMS((char *, char *, off_t));

  /*
   *
   */
  off_t get_max_filesize PARMS((void));



  /*
   *  function to start Stress Lab "heartbeat"
   */
  pid_t start_heart PARMS((char *, char *));




  /*
   *  function to calculate ckecksum
   */
  long sum PARMS((register char *, off_t));



  /*
   *  function to check the pre-existing logs in an auto-start environment
   */
  short check_ckpt PARMS((off_t, off_t, off_t, off_t, int));

  /*
   *  function to check the pre-existing logs in an auto-start environment
   */
  short check_logs PARMS((off_t, off_t, off_t, off_t));

  /*
   *  function to check the pre-existing logs in an auto-start environment
   */
  short check_time PARMS((char *, time_t *, off_t *));

  /*
   *  function to check to see if a log file's wrap/max size threshold has
   *  been met
   */
  short check_wrap PARMS((int *, char *, int, off_t, tbool, tbool, off_t,
			  off_t *, tbool));

  /*
   *  function to check the specified Stress Lab "heartbeat" cycle value.
   */
  short ck_heartbeat_cycle PARMS((char *));

  /*
   *  function to check the specified yes/no flag to make sure that a "y/Y" or
   *  "n/N" was really specified.
   */
  short ck_yn_flag PARMS((char *, char *, tbool *));

  /*
   *  function to release resources before exit
   */
  short clean_up PARMS((void));

  /*
   *  function to close a file and check the return code
   */
  short close_file PARMS((int *, char *));

  /*
   *  function to copy ipc messages to logs/stress-pipe.
   */
  short copy_message PARMS((struct htx_msg_xbuf *, off_t, off_t, off_t, off_t,
			    tbool, tbool, tbool));

  /*
   *  function to check and process argv[] parameters
   */
  short do_argv PARMS((char **, int, off_t *, off_t *, off_t *, off_t *,
		       tbool *, tbool *,tbool *, tbool *, char **, char *, size_t));

  /*
   *  function to open IPC message queue
   */
  short open_ipc PARMS((void));

  /*
   *  function to open log files  
   */
  short open_logs PARMS((off_t, off_t, off_t, off_t, tbool));

  /*
   *  function to remove a file
   */
  short rm_file PARMS((char *));

  /*
   *  function to update the checkpoint file
   */
  short send_message PARMS((char *, int, int, mtyp_t));

  /*
   *  function to update the checkpoint file
   */
  short send_signal PARMS((pid_t, int));

  /*
   *  function to set log file offsets from checkpoint file data
   */
  short set_file_offsets PARMS((void));

  /*
   *  function to update the checkpoint file
   */
  short set_signal_hdl PARMS((int, void (*)(int, int, struct sigcontext *)));

  /*
   *  function to update the checkpoint file
   */
  short update_ckpt PARMS((int, char *, off_t *, off_t, time_t *));

  /*
   *  function to write a message out to a log file
   */
  short write_log PARMS((int *, char *, char *, size_t, off_t, tbool, tbool,
			 off_t *, time_t *, tbool));

  /*
   *  function to write a message out to a log file
   */
  short write_pipe PARMS((struct htx_msg_xbuf *));



  /*
   *  function to handle SIGCHLD (death of a child process) signal
   */
  void SIGCHLD_hdl PARMS((int, int, struct sigcontext *));

  /*
   *  function to handle SIGTERM (terminate) signal
   */
  void SIGTERM_hdl PARMS((int, int, struct sigcontext *));

#ifdef __HTX_LINUX__
  int start_display_device PARMS((void));
#endif


#endif  /* HXSMSGDEF_H */
