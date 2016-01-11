/* @(#)26	1.4.5.2  src/htx/usr/lpp/htx/bin/eservd/defines.h, eserv_daemon, htxubuntu 3/13/12 04:17:29 */
#ifndef DEFINES_H
#define DEFINES_H

/*
 *  defines.h -- eserv Daemon (eservd) Function Declarations
 *
 *    This include file is the respository for all HTX Supervisor
 *    (eservd) function declarations.
 *
 */

#ifdef  __HTX_LINUX__
typedef int boolean_t;
#endif


/*
 *  libPW.a function to compile regular expressions...
 */
#ifndef	__HTX_LINUX__
char *regcmp PARMS((char *arg, ...));

/*
 *  libPW.a function to evaluate compiled regular expressions...
 */

char *regex PARMS((char *, char *, ...));
void DR_handler PARMS((int, int, struct sigcontext *));
#endif

/*
 *  Function to compare Act/Halt table entries...
 */
int AHD_compar PARMS((struct ahd_entry *, struct ahd_entry *));

     /*
      *  Function to Act/Halt Devices...
      */
int AH_device PARMS((char *, char **, int, int *));

     /*
      *  Function to compare Act/Halt the system...
      */
int AH_system PARMS((int,char *));

     /*
      *  Function to Add/Restart/Terminate Devices...
      */
void ART_device PARMS((void));

     /*
      *  Function to handle the SIGALRM signal...
      */
void alarm_signal PARMS((int, int, struct sigcontext *));

     /*
      *  Function to handle the SIGCHLD signal...
      */
int build_ipc PARMS((char *));

     /*
      *  Function to handle the SIGCHLD signal...
      */
void child_death PARMS((int, int, struct sigcontext *));

     /*
      *  Function to cleanup configuration odm session...
      */
int cleanup_odm PARMS((void));

     /*
      *  Function to set/show/clear Continue On Error flags...
      */
int COE_device PARMS((char *, char **, int, int *));

     /*
      *  Function to display Device Status Table...
      */
int disp_dst PARMS((char *,char **,int, int *));

     /*
      *  Function to display HTX screen files...
      */

/*
 *  Function to free odm allocated resources... only for AIX
 */

#ifndef	__HTX_LINUX__
int do_odm_free_list PARMS((void *, struct listinfo *));
#endif


/*
 *  Function to perform HTX main menu option...
 */
void do_option PARMS((int, int));

     /*
      *  Function to compare Device Run Table entries...
      */
int DRT_compar PARMS((struct htxshm_HE *, struct htxshm_HE *));

     /*
      *  Function to compare Device Status Table entries...
      */
int DST_compar PARMS((struct dst_entry *, struct dst_entry *));

     /*
      *  Function to edit a file...
      */
void edit PARMS((char *));

     /*
      *  Function to handle shutdown child processes and the supervisor...
      */
void end_it PARMS((int));
void end_ecg PARMS((void));

     /*
      *  Function to display AIX error report...
      */
void erpt PARMS((void));

     /*
      *  Function to run individual setup/cleanup scripts
      */
int exec_HE_script PARMS((char *, char *, boolean_t *));

#ifndef	__HTX_LINUX__
     /*
      *  Function to get the specified attribute list from CuAt objects...
      */
int get_CuAt PARMS((char *, char *));

     /*
      *  Function to get the set of customized devices and linked predefined
      *  for the named object...
      */
int get_CuDv PARMS((char *, char *));
#endif

     /*
      *  Function to get the display_id attribute of a display device...
      */
int get_dispid PARMS((char *));

     /*
      *  Function to get the display_id attribute of a display device...
      */
char *get_dst PARMS((struct htxshm_HE *, int, enum t_dev_status *));

#ifndef	__HTX_LINUX__
     /*
      *  Function to get odm session error message...
      */
int get_odm_error_msg PARMS((long, char **));

     /*
      *  Function to get the specified attribute list from PdAt objects...
      */
int get_PdAt PARMS((char *, char *));

#endif
     /*
      *  Function to get a string from keyboard input...
      */
int get_string PARMS((WINDOW *, int, int, char *, size_t, char *, tbool));

     /*
      *  Function to display help information screens...
      */
int help PARMS((int, int, int, int, char *, int));

     /*
      *  Function to parse the .htx_profile file and set system variables
      *  accordingly...
      */
void htx_profile PARMS((int *));

     /*
      *  Function to check for hung HE's...
      */
void hang_monitor PARMS((void));

     /*
      *  Function to handle signal termination for hang_monitor process...
      */
void hang_sig_end PARMS((int, int, struct sigcontext *));

     /*
      *  Function to handle signal termination for equaliser process...
      */
void equaliser_sig_end PARMS((int, int, struct sigcontext *));

     /*
      *  Function to initialize display Act/Halt Device Table...
      */
int init_ahd_tbl PARMS((struct ahd_entry **));

     /*
      *  Function to initialize display Continue On Error Table...
      *
int init_coe_tbl PARMS((struct ahd_entry **));
*/

     /*
      *  Function to initialize display Device Status Table...
      */
int init_dst_tbl PARMS((struct dst_entry **));

     /*
      *  Function to initialize configuration odm session...
      */
int initialize_cfgodm PARMS((void));


     /*
      *  Function to compare load (HE) entries...
      */
int load_compar PARMS((struct load_tbl *, struct load_tbl *));

     /*
      *  Function to work with HTX main menu...
      */
int mmenu PARMS((int));

/*
 *  Function to set values in shared memory from MDT stanza
 */
/*
 * int process_mdt_to_shm PARMS(( char *, int, union shm_pointers, char **, CFG__SFT *));
*/


/*
 * Function to send a message the HTX message handler...
 */
short send_message PARMS((char *, int, int, mtyp_t));


/*
 *  Function to start a shell...
 */
void shell PARMS((void));

     /*
      *  Function to handle signals that should cause an HTX Shutdown...
      */
void sig_end PARMS((int, int, struct sigcontext *));

     /*
      *  Function to handle signal to look for HE termination candidate
      */
void sig_restart_HE PARMS((int, int, struct sigcontext *));

     /*
      *  Function to start the hxsmsg Message Handler Program...
      */
short start_msg_hdl PARMS((int));

     /*
      *  Function to terminate an HE
      */
int terminate_exerciser PARMS((struct htxshm_HE *, int, int, char *));

     /*
      *  Function to strip the quotes off a string...
      */
char *unquote PARMS((char *));

     /*
      *  Function to get currently running ecg/mdt name...
      */
char *get_running_ecg_name (char *);

     /*
      *  Function to attach currently running ecg/mdt name at the front 
      */
char *attach_ecg_name_front (char *);

int  A_device PARMS((char *));
int  R_device PARMS((char *, char **, int , int *));
int  T_device PARMS((char *, char **, int , int *));
void get_htx_level PARMS((void));
void setup_server PARMS((void));
int  test_summary PARMS((char *));

pid_t fork PARMS((void));
int execl PARMS(( const char *, const char *, ...));
int setpgid PARMS((pid_t , pid_t ));
int setpgrp PARMS((void));
unsigned int alarm PARMS((unsigned int ));
int nice PARMS ((int inc));
pid_t setsid PARMS((void));
unsigned int sleep PARMS((unsigned int seconds));
int pause PARMS((void));
int access PARMS((const char *pathname, int mode));
ssize_t read PARMS((int fd, void *buf, size_t count));
int close PARMS((int fd));
pid_t getpid PARMS((void));
int chdir PARMS((const char *path));
int set_sig_handler PARMS((void));


#endif				/* HXSSUPDEF_H */
