/* IBM_PROLOG_BEGIN_TAG */
/* 
 * Copyright 2003,2016 IBM International Business Machines Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 		 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* IBM_PROLOG_END_TAG */

/* @(#)50	1.3.4.2  src/htx/usr/lpp/htx/bin/hxssup/hxssupdef.h, htx_sup, htxubuntu 8/4/14 08:46:10 */

#ifndef HXSSUPDEF_H
#define HXSSUPDEF_H

/*
 *  hxssupdef.h -- HTX Supervisor (hxssup) Function Declarations
 *
 *    This include file is the respository for all HTX Supervisor
 *    (hxssup) function declarations.
 *
 */

#ifdef	__HTX_LINUX__
typedef	int boolean_t;
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
#endif

/*
 *  Function to compare Act/Halt table entries...
 */
int AHD_compar PARMS((struct ahd_entry *, struct ahd_entry *));

     /*
      *  Function to Act/Halt Devices...
      */
int AH_device PARMS((void));

     /*
      *  Function to compare Act/Halt the system...
      */
void AH_system PARMS((int));

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
int build_ipc PARMS((void));

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
int COE_device PARMS((void));

     /*
      *  Function to display Device Status Table...
      */
int disp_dst PARMS((void));

     /*
      *  Function to display HTX screen files...
      */
int display_scn PARMS((WINDOW *, int, int, int, int, char *, int, int *,
		       int *, int, int, char));

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
      *  Function to search device screens...
      */
int search_win PARMS((int, int, int, int, char *, int, char *));


     /*
      *  Function to parse the .htx_profile file and set system variables
      *  accordingly...
      */
void htx_profile PARMS((int *));

     /*
      *  Function to display HTX screen...
      */
void htx_scn PARMS((void));

     /*
      *  Function to check for hung HE's...
      */
void hang_monitor PARMS((void));

     /*
      *  Function to handle signal termination for hang_monitor process...
      */
void hang_sig_end PARMS((int, int, struct sigcontext *));

      /*
       * Function to randomly activate and halt devices...
       */
void rand_ahd PARMS((void));

     /*
      * Function to handle signal termination of rand_ahd process...
      */
void rand_sig_end PARMS((int, int, struct sigcontext *));

     /*
      * Function to handle SIGALRM signal of rand_ahd process...
      */
void SIGALARM_handler PARMS((int, int, struct sigcontext *));

     /*
      *  Equaliser Functtion
      */
void equaliser PARMS((void));

	 /*
	  * Function to handle signal termination of equaliser process...
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
      *  Function to initialize curses screen parameters...
      */
void init_screen PARMS((void));

     /*
      *  Function to compare load (HE) entries...
      */
int load_compar PARMS((struct load_tbl *, struct load_tbl *));

     /*
      *  Function to work with HTX main menu...
      */
int mmenu PARMS((int));

     /*
      *  Function to display HTX main menu...
      */
void mmenu_scn PARMS((int *));

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
void terminate_exerciser PARMS((struct htxshm_HE *, int));

     /*
      *  Function to strip the quotes off a string...
      */
char *unquote PARMS((char *));


#endif				/* HXSSUPDEF_H */
