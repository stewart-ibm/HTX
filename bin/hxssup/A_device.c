
/* @(#)36	1.6.4.2  src/htx/usr/lpp/htx/bin/hxssup/A_device.c, htx_sup, htxubuntu 12/16/14 03:58:07 */

/*****************************************************************************/
/*                                                                           */
/* MODULE NAME    =    A_device.c                                            */
/* COMPONENT NAME =    hxssup (supervisor)                                   */
/* LPP NAME       =    HTX                                                   */
/*                                                                           */
/* DESCRIPTIVE NAME =  Add Device Exerciser                                  */
/*                                                                           */
/* STATUS =            Release 1 Version 0                                   */
/*                                                                           */
/* FUNCTION =          Adds a device and starts the exerciser for it.        */
/*                                                                           */
/* COMPILER OPTIONS =                                                        */
/*                                                                           */
/* CHANGE ACTIVITY =                                                         */
/*    DATE    :LEVEL:PROGRAMMER:DESCRIPTION                                  */
/*    MMMMMMMMNMMMMMNMMMMMMMMMMNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM */
/*    11/18/99:1.0  :R GEBHARDT:INITIAL RELEASE                              */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*****************************************************************************/

#include "hxssup.h"

#ifndef	__HTX_LINUX__
#include <string.h>
#endif

#ifdef	__HTX_LINUX__
#include "cfgclibdef.h"
#else
#include "cfgclibdef.h"
#endif
#include <scr_info.h>

extern tecg_struct *ecg_info;
extern int cur_ecg_pos;

extern union semun semctl_arg;
extern	int	AD_scn(char menu_add_name[],		/* I/O device or file name */
		enum t_add_method *addMethod,	/* I/O method to generate MDT */
		enum t_start_now *startNow,	/* I/O for start_halted attribute */
		boolean_t * forceRedef,		/* I/O Overwrite existing dev */
		char op_info_msg[]);		/* I/O operator message */
extern int run_cmd(char *cmd_string, char *cmd_output, int *ct_output, char *cmdout_filename);
extern int process_mdt_to_shm(char *stanza, int stanza_length,  union shm_pointers shm_point, char **default_stanza, CFG__SFT * mdt_fd);
extern int load_exerciser(struct htxshm_HE *p_HE, struct sigaction *sigvec);
extern CFG__SFT *cfgcopsf(char *);
extern int cfgcclsf(CFG__SFT *);


extern char *default_mdt_snz;	/* ptr to default mdt stanza    */

extern char HTXPATH[];
extern unsigned int max_wait_tm;	/* maximum semop wait time      */
extern int MAX_ADDED_DEVICES;	/* shm_HE entries for new devices */
extern int semhe_id;		/* semaphore id                      */
extern union shm_pointers shm_addr;	/* shared memory union pointers      */
extern volatile int system_call;

/*****************************************************************************/
/*****  A _ d e v i c e  *****************************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     A_device                                              */
/*                                                                           */
/* DESCRIPTIVE NAME =  Add device and exerciser to test environment.         */
/*                                                                           */
/* FUNCTION =          Top level Add Device function, called from            */
/*                     ART_device().  Puts up the screen and gets user       */
/*                     input then calls subordinate functions to             */
/*                     add the device to the share memory structures,        */
/*                     initialized semaphores, and start the exerciser       */
/*                     process.                                              */
/*                                                                           */
/* INPUT =             Gets operator input from AD_scn().  This could        */
/*                     include an MDT attribute file.                        */
/*                                                                           */
/* OUTPUT =            Updated semaphores and shared memory.                 */
/*                                                                           */
/* NORMAL RETURN =     N/A - void function                                   */
/*                                                                           */
/* ERROR RETURNS =     N/A - void function                                   */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*                                                                           */
/*    OTHER ROUTINES =                                                       */
/*                     send_message() - outputs a message to the screen and  */
/*                                     the message log file                  */
/*                     AD_scn() - Gets operator input.                       */
/*                     cfgcclsf() - close MDT file.                          */
/*                     cfgcopsf() - open MDT file.                           */
/*                     process_mdt_to_shm()                                  */
/*                     exec_HE_script()                                      */
/*                                                                           */
/*    DATA AREAS =                                                           */
/*                     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                                                                           */
/*    MACROS =         PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                                                                           */
/*****************************************************************************/
void	A_device(void)
{

	boolean_t confirm_errmsg;
	CFG__SFT *mdt_fd = NULL;	/* pointer to MDT attribute file */
	char	AIX_logical_device_name[32];
	char	HTX_logical_device_name[32];
	char	*MDT_filename = NULL;
	char	cmd_string[PATH_MAX];	/* command to pass to shell */
	char	*htx_mkmdt_str;	/* command to pass to shell from environment */
	char	msg[81];		/* message line message text buffer */
	char	msg_text[MSG_TEXT_SIZE];
	char	*p_default_snz = NULL;
	char	*p_msg = NULL;
	char	*p_temp_filename = NULL;	/* pointer to temp_filename if file is open */
	char	reg_expr[256];
	char	stanza[4096];	/* attribute file stanza        */
	char	tag[40];
	char	temp_filename[L_tmpnam];	/* name of temporary file to hold awk output */
    char cmd_str[1024], buf[1024];

    int	AD_scn_rc = 0;
	int	cfg_rc = 0;		/* return from process_mdt_to_shm */
	int	ct_HE_started = 0;	/* number of HE's actually spawned     */
	int	ct_stanza = 0;		/* number of stanzas found in MDT file */
	int no_of_inst,i = 0;
	int	prev_max_wait_tm = 0;
	int	rc = 0;
	int	semval = 0;
	int	workint = 0;
	size_t	AIX_ldn_length = 0;	/* length of AIX logical device name */
    FILE *fp;

        union shm_pointers shm_point;
	struct htxshm_HE *p_shm_HE_base, tmp_HE_entry, *p_added_shm_HE;
	struct sigaction sigvector;	/* structure for signal specs */

	unsigned int max_entries, prev_max_entries;	/* local copy */

	/*
	 * Menu Items
	 */
	static char add_name[26];	/* device to add or MDT file name */
	static enum t_add_method addMethod = AD_Method_Default;
	static enum t_start_now startNow = AD_Start_Default;
	static boolean_t forceRedef = FALSE;

	/*
	 * Do things that only get done once
	 */

        shm_point.hdr_addr = &tmp_HE_entry;

	p_shm_HE_base = ((struct htxshm_HE *) ((struct htxshm_hdr *) shm_addr.hdr_addr + 1));	/* skip over header */

    /*
	 * Set signals to SIG_DFL for the newly restarted HE's
	 */
	sigvector.sa_handler = SIG_DFL;		/* set default action */
	sigemptyset(&(sigvector.sa_mask));	/* do not block other signals */
	sigvector.sa_flags = 0;				/* No special flags */

	/*
	 * set file descriptor to close on exec
	 */
	fcntl(fileno(stdin), F_SETFD, 1);	/* stdin to close on exec  */
	fcntl(fileno(stdout), F_SETFD, 1);	/* stdout to close on exec */
	fcntl(fileno(stderr), F_SETFD, 1);	/* stderr to close on exec */

	/*
	 * init menu stuff
	 */
	htx_strncpy(add_name, "", DIM(add_name));	/* empty name field */
	p_msg = NULL;							/* no message */
	confirm_errmsg = FALSE;

	for (;;)  {

			/*
			 * Do the cleanup from the previous loop.  The conditions are
			 * always false the first time through.
			 */

			if (confirm_errmsg) {
					PRTMSG(MSGLINE - 2, 0, ("Press any key to continue..."));
					getch();
					fflush(stdin);
			}

			CLRLIN(MSGLINE, 0);
			confirm_errmsg = FALSE;

			if (mdt_fd != (CFG__SFT *) NULL) {	/* close MDT cfg file */
					if (cfgcclsf(mdt_fd) != CFG_SUCC) {
							sprintf(msg_text, "ERROR: Unable to close device MDT file.");
							send_message(msg_text, 0, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
							PRTMSG(MSGLINE, 0, (msg_text));
							break;	/* fatal programming error, bail out */
					} /* endif */

					else  {
							mdt_fd = NULL;
					}
			}		/* end if */

			if (MDT_filename != NULL) {
					free(MDT_filename);
					MDT_filename = NULL;
			}

			/* if */
			if (p_temp_filename != NULL) {	/* remove temporary file */
					remove(temp_filename);	/* cleanup temp file */
					p_temp_filename = NULL;
			}



			/*
			 * Display menu and get operator request
			 *           NORMAL EXIT FROM LOOP HERE
			 *           exit loop if previous return was non-zero
			 */
			if (AD_scn_rc != 0)  {
					break;
			}

			/*
			 *              Display menu
			 */
			AD_scn_rc = AD_scn(add_name, &addMethod, &startNow, &forceRedef, p_msg);

			/*
			 * skip the rest of the loop if the user wants to quit.
			 */
			if (AD_scn_rc == 1)  {
					continue;	/* operator quit the menu */
			}

			if (htx_strlen(add_name) == 0) {
				p_msg = htx_strcpy(msg, "Specify device or MDT file name (option 2).");
					continue;
			}		/* if */

			p_msg = NULL;	/* init message pointer */
			AIX_logical_device_name[0] = ' ';
			prev_max_entries = max_entries = shm_addr.hdr_addr->max_entries;	/* init to current value */

			/*
			 *      Generate the MDT stanza if add method is by device name.
			 */

			if (addMethod == AD_Method_Name) {
					htx_strcpy(AIX_logical_device_name, add_name);

					/*
					 * Get the temp file name.  The pointer to it is used to
					 * clean up later.  Save the name in malloc'd memory.
					 */
					p_temp_filename = tmpnam(temp_filename);
					MDT_filename = malloc(htx_strlen(temp_filename) + 1);
					htx_strcpy(MDT_filename, temp_filename);

					/*
					 * Special case device name for Tigershark PCI adapter name.
					 * Transforms ddriciopNN to ddricioNN
					 */

					AIX_ldn_length = htx_strlen(AIX_logical_device_name);
					htx_strcpy(HTX_logical_device_name, AIX_logical_device_name);

					if (AIX_ldn_length > 8)  {
							if (0 == htx_strncmp(AIX_logical_device_name, "ddriciop", 8))  {
									htx_strcpy(&HTX_logical_device_name[7], strpbrk(&AIX_logical_device_name[8], "0123456789"));
							}
					}

					/*
					 * Format the command to generate the MDT file. If
					 * HTX_MKMDT is set, use its value to generate the
					 * MDT stanza.
					 */

					htx_mkmdt_str = getenv("HTX_MKMDT");

					if (htx_mkmdt_str != NULL)  {
							sprintf(cmd_string, "%s %s", htx_mkmdt_str, AIX_logical_device_name);
							sprintf(cmd_string, "%s > %s 2>&1", cmd_string, MDT_filename);
					}

					else  {	/* use standard command */
					(void) sprintf(cmd_string,
                                        	"lsdev -C -Sa -l %s -F 'status name type location class subclass parent' | /usr/lpp/htx/etc/scripts/htxconf.awk 1>/tmp/htxconf.op 2>/dev/null; /usr/lpp/htx/etc/scripts/htxselect.pl %s > %s 2>&1" ,                                             AIX_logical_device_name, HTX_logical_device_name,MDT_filename);
					}


					/*
					 * Run the command, dont care about getting output.
					 */

					rc = run_cmd(cmd_string, (char *) NULL, &workint, MDT_filename);
					/*
					 * if grep returns 1, assume that it did not find the
					 * device stanza
					 */

					if (rc == 1) {
							sprintf(msg, "Device %s not found.", AIX_logical_device_name);
							p_msg = msg;
							continue;
					}		/* endif */

					if (rc != 0) {
							sprintf(msg, "Problem generating the MDT for this device.  Not added.");
							p_msg = msg;
							continue;
					}	/* endif */
			} /* endif addMethod == AD_Method_Name */

			else {		/* use the MDT file specified by the user */

				MDT_filename = malloc(htx_strlen(add_name) + htx_strlen(HTXPATH) + htx_strlen("mdt/"));
					htx_strcpy(MDT_filename, HTXPATH);
					htx_strcat(MDT_filename, "/mdt/");
					htx_strcat(MDT_filename, add_name);
			}		/* else */

			/*
			 *      Open the MDT attribute file.
			 */
			mdt_fd = cfgcopsf(MDT_filename);
			if (mdt_fd == (CFG__SFT *) NULL) {
					sprintf(msg, "Unable to open MDT file %s", MDT_filename);
					p_msg = msg;
					continue;
			}		/* endif */

			/*
			 *           Fill in the temporary HE entry
			 */
			ct_stanza = ct_HE_started = 0;

			/*
			 * setup for first call to process_mdt_to_shm
			 */
			p_default_snz = (char *) &default_mdt_snz;

			/*
			 * EOF is normal exit when processing user specified MDT and at
			 * least one stanza was found in it.
			 */

			/* (addMethod == AD_Method_File && ct_stanza > 0 && cfg_rc == CFG_OK) */
			do {
					bzero(&tmp_HE_entry, sizeof(tmp_HE_entry));
					cfg_rc = process_mdt_to_shm(stanza, sizeof(stanza), (union shm_pointers)shm_point, &p_default_snz, mdt_fd);

					if (cfg_rc != CFG_SUCC) {	/* not OK ?   */
							if (addMethod == AD_Method_File && cfg_rc == CFG_EOF && ct_stanza > 0) {
									if (ct_HE_started > 0)  {
											htx_strncpy(add_name, "", DIM(add_name));	/* empty name field */
									}

									continue;	/* NORMAL EXIT when addMethod is file */
				}			/* if */

							else {

									sprintf(msg_text, "ERROR: MDT file read failure CFG_");
									switch (cfg_rc) {
											case CFG_EOF:	/* empty file */
													sprintf(tag, "EOF");
													break;

											case CFG_SZNF:	/* req stanza not found   */
													sprintf(tag, "SZNF");
													break;
											case CFG_SZBF:	/* stanza too long */
													sprintf(tag, "SZBF");
													break;

											case CFG_UNIO:	/*  I/O error */
													sprintf(tag, "UNIO");
													break;

											default:	/* unexpected error */
													sprintf(tag, "????");
													break;
									}	/* endswitch */

									htx_strcat(msg_text, tag);
									sprintf(tag, ", rc = %d.", cfg_rc);
									p_msg = htx_strcat(msg_text, tag);
									send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
									continue;
							}	/* else */
					} /* endif */

					else
							ct_stanza++;

					/*
					 * Look for matching device name. Allocate a new entry in
					 * the HE shm if no match. Setup pointer to HE shm entry.
					 * Copy temporary HE entry to shared memory.
					 */

					for (i = 0; i < max_entries; i++) {
							if (!(htx_strcmp(tmp_HE_entry.sdev_id, (p_shm_HE_base + i)->sdev_id)))  {
									break;
							}
					}	/* for */

					p_added_shm_HE = (p_shm_HE_base + i);	/* point to entry for new dev */

					if (i >= max_entries) {	/* Device name not found? */
							if (max_entries < shm_addr.hdr_addr->pseudo_entry_0)  {		/* room left? */
									strcpy(EXER_NAME(max_entries),tmp_HE_entry.sdev_id);
									ECGEXER_SHMKEY(max_entries) = HTXSHMKEY;
									ECGEXER_SEMKEY(max_entries) = SEMHEKEY;
									max_entries++;	/* allocate new entry */
									NUM_EXERS = max_entries;
							}

							else {	/* Can't add any more new ones */
									sprintf(msg_text, "All %d entries in shared memory allocated for new devices are used.\nIncrease \"max_added_devices\" in /usr/lpp/htx/.htx_profile and restart HTX", MAX_ADDED_DEVICES);

									send_message(msg_text, 0, HTX_SYS_INFO,	HTX_SYS_MSG);
									sprintf(msg_text, "No room for new devices, see /tmp/htxmsg");
									send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
									p_msg = htx_strcpy(msg, msg_text);
									htx_strncpy(add_name, "", DIM(add_name));	/* empty name field */
									continue;
							}	/* else */
					} /* if i >= max_entries */

					else {	/* over writing existing device */
							if (forceRedef) {
									if (p_added_shm_HE->PID != 0) {	/* old HE must be deceased */
											sprintf(msg_text, "Device %s lives!  Terminate the exerciser before redefining it.", tmp_HE_entry.sdev_id);
											p_msg = msg_text;
											continue;
									}	/* if */
							} /* if */

							else {	/* dont force redefinition */
									sprintf(msg_text, "Device %s already defined, toggle option 4 to override.", tmp_HE_entry.sdev_id);
									p_msg = msg_text;
									continue;
							}	/* else */
					}	/* else */

					/*
					 * Ok, the entry is good, copy it to shared memory and
					 * initialize  the semaphores.
					 */
					bcopy(&tmp_HE_entry, (char *) p_added_shm_HE, sizeof(tmp_HE_entry));

					/*
					 * Initialize the new HE semaphores.
					 */
					switch (startNow) {
							case AD_Start_Active:
									semval = 0;
									break;

							case AD_Start_Halted:
									semval = 1;
									break;

							case AD_Start_Default:
									semval = p_added_shm_HE->start_halted;
									break;
					}	/* switch */

					semctl_arg.val = semval;
					semctl(semhe_id, ((i * SEM_PER_EXER) + 6), SETVAL, semctl_arg);

					/*
					 * Setup to run setup/runsetup scripts and spawn exercisers.
					 */

					sprintf(msg_text, "Adding %s by operator request.", tmp_HE_entry.sdev_id);
					send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);

					/*
					 * Run the scripts
					 */
            		if (strncmp(p_added_shm_HE->HE_name, "hxesct", 6) == 0)
            	       sprintf(cmd_str, "ps -ef | grep hxesct | grep -v grep | wc -l");
            		else
            		   sprintf(cmd_str, "ps -ef | grep %s | grep -v grep | wc -l", p_added_shm_HE->HE_name);
           			if ((fp = popen(cmd_str, "r")) == NULL)  {
           			  (void) sprintf(msg_text,"popen error in A_device");
		   			  (void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
           			}
           			if (fgets(buf,1024,fp) == NULL) {
           			  (void) sprintf(msg_text,"fgets error in A_device");
		   			  (void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
           			}
           			sscanf(buf, "%d", &no_of_inst);
				system_call = TRUE;
           			pclose(fp);
				system_call = FALSE;
           			if(no_of_inst == 0) {
				   		htx_strcpy(reg_expr, "^");
						htx_strcat(reg_expr, p_added_shm_HE->HE_name);
						htx_strcat(reg_expr, ".*setup[\t ]*$");
						rc = exec_HE_script(reg_expr, p_added_shm_HE->sdev_id, &confirm_errmsg);
					    if (rc < 0) {	/* could not find out what to run or could not run it */
							sprintf(msg_text, "WARNING: Failed running setup script(s) for %s", tmp_HE_entry.sdev_id);
							PRTMSG((MSGLINE - 1), 0, (msg_text));
							p_msg = htx_strcpy(msg, msg_text);
							send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
					    }		/* endif */
				     }

					/*
					 * Set counters to reflect presence of the added device.
					 * This commits the new shared memory entry.
					 * check/reset maximum time allowed on a semop system wait
					 */
					prev_max_wait_tm = max_wait_tm;
					workint = (p_added_shm_HE->max_run_tm * 2) + p_added_shm_HE->idle_time;
					if (workint > max_wait_tm) {	/* new maximum wait time? */
							max_wait_tm = workint;
					}		/* endif */

					/*
					 * increment counters if device add was sucessfull
					 */
					shm_addr.hdr_addr->num_entries++;
					shm_addr.hdr_addr->max_entries = max_entries;
					p_added_shm_HE->tm_last_upd = -1;

					/*
					 * Spawn the exerciser.
					 */
					rc = load_exerciser(p_added_shm_HE, &sigvector);	/* fork & exec */
					if (rc != 0) {		/* restore counters */
							max_wait_tm = prev_max_wait_tm;
							shm_addr.hdr_addr->num_entries--;
							shm_addr.hdr_addr->max_entries = max_entries = prev_max_entries;
							sprintf(msg_text, "ERROR: Failed to start %s. ", p_added_shm_HE->HE_name);
							/*
							 * setup lingering message for user
							 */
							p_msg = htx_strcpy(msg, msg_text);
							PRTMSG(MSGLINE, 0, (msg_text));
							send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
							continue;
					}		/* endif */

					ct_HE_started++;
					/*
					 * Set successful completion message and log in htxmsg.
					 */
					p_msg = msg;
					sprintf(msg, "Started /dev/%s", p_added_shm_HE->sdev_id);
					sprintf(msg_text, "Addition or replacement of exerciser for %s completed sucessfully.", tmp_HE_entry.sdev_id);
					send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);

					/*
					 * clear device/file name for next entry
					 */
					htx_strncpy(add_name, "", DIM(add_name)); /* empty name field */
			} while (addMethod == AD_Method_File && ct_stanza > 0 && cfg_rc == CFG_SUCC);

					/*
					 * Tell the operator how many were started if there was
					 * more than one device in the MDT file
					 */
					if (addMethod == AD_Method_File && ct_stanza > 1) {
							sprintf(msg, "%d devices found, %d exercisers started.", ct_stanza, ct_HE_started);
							p_msg = msg;
					}	/* if */
	}		/* for ever */


	/*
	 * Cleanup
	 */
	fcntl(fileno(stdin), F_SETFD, 0);	/* stdin NOT to close on exec */
	fcntl(fileno(stdout), F_SETFD, 0);	/*stdout NOT to clse on exec */
	fcntl(fileno(stderr), F_SETFD, 0);	/*stderr NOT to clse on exec */

}				/* A_device */



