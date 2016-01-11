
/* @(#)33	1.3.4.2  src/htx/usr/lpp/htx/bin/hxssup/AH_device.c, htx_sup, htxubuntu 12/16/14 03:57:31 */

/*****************************************************************************/
/*                                                                           */
/* MODULE NAME    =    AH_device.c                                           */
/* COMPONENT NAME =    hxssup (supervisor)                                   */
/* LPP NAME       =    HTX                                                   */
/*                                                                           */
/* DESCRIPTIVE NAME =  Activate/Halt devices                                 */
/*                                                                           */
/* COPYRIGHT =         (C) COPYRIGHT IBM CORPORATION 1988                    */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM           */
/*                                                                           */
/* STATUS =            Release 1 Version 0                                   */
/*                                                                           */
/* FUNCTION =          Activates/Halts one or more devices.                  */
/*                                                                           */
/* COMPILER OPTIONS =  -I/src/master/htx/common -g -Nn3000 -Nd4000 -Np1200   */
/*                     -Nt2000                                               */
/*                                                                           */
/*****************************************************************************/

#include "hxssup.h"

#define NUM_COLS	(23)
#define NUM_ROWS	(81)

#define AH_IN_ROW	(21)
#define AH_IN_COL	(18)

/*
 * Define this so that the rest of the code fails.
 */
#ifdef	__HTX_LINUX__
char	*__loc1;
int tmpp;

#ifndef	BUFSIZ
#define	BUFSIZ	(1024)
#endif

extern union semun semctl_arg;


char *regcmp(char *arg1, char *arg2)
{
	if(arg1 == NULL)  {
		return NULL;
	}

	if(arg2 == NULL)  {
		return arg1;
	}

	strcat(arg1, arg2);
	return arg1;
}

char *regex(char *arg1, char *arg2)
{
	if(arg1 == NULL || arg2 == NULL)  {
		return NULL;
	}
	else  {
		while(*arg1++ != *arg2++)
			;

		if(arg1 != NULL && arg2 != NULL)  {
			__loc1 = (--arg1);
		}
		else  {
			__loc1 = NULL;
			return NULL;
		}
		while(*arg1++ == *arg2++)
			;

		return (++arg1);
	}

	return NULL;
}
#endif



/*****************************************************************************/
/*****  A H _ d e v i c e ( )  ***********************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     AH_device.c                                           */
/*                                                                           */
/* DESCRIPTIVE NAME =  Activate/Halt devices                                 */
/*                                                                           */
/* FUNCTION =          Activates/Halts one or more devices.                  */
/*                                                                           */
/* INPUT =             p_loc_thres: pointer to the hft data structure which  */
/*                         contains the current locator (mouse) threshold    */
/*                         values                                            */
/*                                                                           */
/*                     Operator keyboard and/or mouse input                  */
/*                                                                           */
/* OUTPUT =            Device Active/Halted Status screen(s)                 */
/*                                                                           */
/*                     Updated system semaphore structure to activate/halt   */
/*                     individual hardware exerciser programs                */
/*                                                                           */
/* NORMAL RETURN =     0 to indicate successful completion                   */
/*                                                                           */
/* ERROR RETURNS =     -1 to indicate no hardware exerciser programs         */
/*                         currently in memory.                              */
/*                     -2 to indicate an unsuccessful attempt to allocate    */
/*                         memory for the activate/halt device table.        */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = display_scn -  displays screen image                  */
/*                     init_ahd_tbl - initializes and sorts the list of      */
/*                                    devices                                */
/*                     send_message - outputs a message to the screen and    */
/*                                    the message log file                   */
/*                                                                           */
/*    DATA AREAS =     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                     SEMHEKEY semaphores (identified by the semhe_id       */
/*                        variable)                                          */
/*                                                                           */
/*    MACROS =         CLRLIN - Clear line CURSES macro                      */
/*                              (defined in common/hxiconv.h)                */
/*                     PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                                                                           */
/*****************************************************************************/

int AH_device(void)
{
	boolean_t run_device = FALSE;	/* TRUE = activate, FALSE = halt    */
	char	*cmp_regexp = NULL;	/* ptr to compiled reg expression    */
	char	input[32];		/* input string                      */
	char	ret[3];			/* return array for regex()    */
	char	*work = NULL;		/* work int */
	char	workstr[128];		/* work string                       */
	char	page_str[15];

	extern char *__loc1;		/* beginning of reg exp match */
	extern int semhe_id;		/* semaphore id                      */

	extern int COLS;		/* number of cols in CURSES screen   */
	extern int LINES;		/* number of lines in CURSES screen  */
	extern WINDOW *stdscr;		/* standard screen window            */

	extern union shm_pointers shm_addr;	/* shared memory union pointers */

	int	col = 0;		/* first column displayed on screen  */
	int	err_semval = 0;		/* error semaphore value */
	int	i = 0;			/* loop counter                      */
	int k;               /* loop counter                      */
	int rc;              /* return value.                     */
	int dev_len;         /* loop counter                      */
	int dev_no=-1;       /* loop counter                      */
	int	max_strt_ent = 0;	/* max starting entry                */
	int	num_disp = 0;		/* number of ahd entries to show     */
	int	num_entries = 0;	/* local number of shm HE entries    */
	int	row = 0;		/* first row displayed on screen     */
	int	semval = 0;		/* semaphore values                  */
	int	strt_ent = 1;		/* first entry on screen             */
	int	update_screen = 0;	/* update screen flag         */
	char                choice;
	struct ahd_entry *p_ahd_table = NULL;	/* points to begin ahd seq tbl */
	struct ahd_entry *p_into_table = NULL;	/* points into ahd sequence table */
	struct htxshm_HE *p_htxshm_HE = NULL;	/* pointer to a htxshm_HE struct */
	struct semid_ds sembuffer;	/* semaphore buffer */
	union shm_pointers shm_addr_wk;	/* work ptr into shm */

	shm_addr_wk.hdr_addr = shm_addr.hdr_addr;	/* copy addr to work space */
	(shm_addr_wk.hdr_addr)++;	/* skip over header */

	/******************************************************************/
	/**  display screen outline  **************************************/
	/******************************************************************/
	clear();
	refresh();
	display_scn(stdscr, 0, 0, LINES, COLS, "AHD_scn", 1, &row, &col, NUM_COLS, NUM_ROWS, 'n');

	/********************************************************************/
	/**  loop until operator says to quit  ******************************/
	/********************************************************************/

	for (;;) {
		if ((shm_addr.hdr_addr)->max_entries == 0) {	/* no HE programs? */
			PRTMSG(MSGLINE, 0, ("There are no Hardware Exerciser programs currently defined."));
			getch();
			fflush(stdin);
			CLRLIN(MSGLINE, 0);

			if (p_ahd_table != NULL)  {
				/*
				 * release memory for ahd order tbl
				 */
				free((char *) p_ahd_table);
				return (-1);
			}
		}

		/*
		 * some HE's defined.
		 */
			else {
				/*
				 * # HE change?
				 */
				if ((shm_addr.hdr_addr)->max_entries != num_entries) {
					num_entries = init_ahd_tbl(&p_ahd_table);
					if (num_entries <= 0)  {
						return (num_entries);
					}


					/*
					 * problem in init fcn - bye!
					 */
					/*
					 * init fcn ran ok.
					 */

					else  {

						/*
						 * 15 or fewer entries?
						 */
						if (num_entries <= 15) {
							max_strt_ent = 1;

						}

						/*
						 * more than 15 entries
						 */

						else {
							max_strt_ent = num_entries;
						}	/* endif */
					}	/* endif */
				}	/* endif */
			}		/* endif */


			/*
			 * build screen data for the current strt_ent
			 */

			p_into_table = p_ahd_table + strt_ent - 1;
			num_disp = num_entries - strt_ent + 1;

			if (num_disp > 15)  {
				num_disp = 15;
			}

			for (i = 1; i <= 15; i++) {
				if (i <= num_disp) {	/* something there? */
					p_htxshm_HE = shm_addr_wk.HE_addr + p_into_table->shm_pos;

					/*
					 * build output characters & attributes
					 * for entry
					 * screen entry number
					 */

#ifdef	__HTX_LINUX__
					wcolorout(stdscr, WHITE_BLUE | NORMAL);
#else
					wcolorout(stdscr, NORMAL | F_WHITE | B_BLUE);
#endif
					wmove(stdscr, (i + 4), 3);
					wprintw(stdscr, "%2d", i);


					/*
					 * Regular Active/Halted Status
					 */

					/*
					 * get sem value of entry
					 */

					semval = semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6), GETVAL, &sembuffer);

					if (semval == 0) {	/* set to run? */
#ifdef	__HTX_LINUX__
						wcolorout(stdscr, RED_BLACK | NORMAL | BOLD);
#else
						wcolorout(stdscr, NORMAL | F_RED | B_BLACK | BOLD);
#endif
						mvwaddstr(stdscr, (i + 4), 6, " ACTIVE ");
					}

					else {		/* halted. */
#ifdef	__HTX_LINUX__
						wcolorout(stdscr, RED_BLACK | STANDOUT | BOLD);
#else
						wcolorout(stdscr, STANDOUT | F_RED | B_BLACK | BOLD);
#endif
						mvwaddstr(stdscr, (i + 4), 6, " HALTED ");
					}	/* endif */

					/*
					 * Error/Halted Status
					 */

					/*
					 * do not cont on error ?
					 */

					if(p_htxshm_HE->cont_on_err == 0) {
						/*
						 * get sem value
						 */
						semval = semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6 + 1), GETVAL, &sembuffer);

						/*
						 * halted on error ?
						 */
						if (semval != 0) {
#ifdef	__HTX_LINUX__
							wcolorout(stdscr, RED_BLACK | STANDOUT | BOLD);
#else
							wcolorout(stdscr, STANDOUT | F_RED | B_BLACK | BOLD);
#endif

							mvwaddstr(stdscr, (i + 4), 6, " ERROR  ");
						}	/* endif */
					}		/* endif */

					/*
					 * Slot, Port, /dev/id, Adapt Desc,
					 * and Device Desc
					 */
					wmove(stdscr, i + 4, 15);
#ifdef	__HTX_LINUX__
					wcolorout(stdscr, WHITE_BLUE | NORMAL);
#else
					wcolorout(stdscr, NORMAL | F_WHITE | B_BLUE);
#endif
					sprintf(workstr, " %.4d ", p_htxshm_HE->slot);
					mvwaddstr(stdscr, (i + 4), 15, workstr);
					sprintf(workstr, " %.4d ", p_htxshm_HE->port);
					mvwaddstr(stdscr, (i + 4), 22, workstr);
					if ( (dev_no+1) == i ) {
						wcolorout(stdscr, RED_BLACK | STANDOUT | BOLD);
					}
					else {
						wcolorout(stdscr, WHITE_BLUE | NORMAL);
					}
					sprintf(workstr, " %-7s ", p_htxshm_HE->sdev_id);
					mvwaddstr(stdscr, (i + 4), 29, workstr);
					wcolorout(stdscr, WHITE_BLUE | NORMAL);
					sprintf(workstr, " %-11s ", p_htxshm_HE->adapt_desc);
					mvwaddstr(stdscr, (i + 4), 39, workstr);
					sprintf(workstr, " %-18s ", p_htxshm_HE->device_desc);
					mvwaddstr(stdscr, (i + 4), 53, workstr);
				}

				/*
				 * no HE entry for this area of screen
				 */
				else  {
#ifdef	__HTX_LINUX__
					wcolorout(stdscr, WHITE_BLUE | NORMAL);
#else
					wcolorout(stdscr, NORMAL | F_WHITE | B_BLUE);
#endif
					/*
					 * scn entry num
					 */
					mvwaddstr(stdscr, (i + 4), 3, "  ");
					mvwaddstr(stdscr, (i + 4), 6, "        ");

					/*
					 * COE Flag slot fld
					 */
					mvwaddstr(stdscr, (i + 4), 15, "      ");
					/*
					 * port fld
					 */
					mvwaddstr(stdscr, (i + 4), 22, "      ");
					/*
					 * sdev_id
					 */
					mvwaddstr(stdscr, (i + 4), 29, "         ");
					/*
					 * adapt_desc field
					 */
					mvwaddstr(stdscr, (i + 4), 39, "             ");
					/*
					 * device_desc field
					 */
					mvwaddstr(stdscr, (i + 4), 53, "                    ");
				}	/* endif */

				p_into_table++;

			}		/* endfor */

#ifdef	__HTX_LINUX__
			wcolorout(stdscr, NORMAL);
#else
			wcolorout(stdscr, NORMAL);
#endif
			for (update_screen = FALSE; (update_screen == FALSE); ) {

				/*
				 * Read input from keyboard...
				 */
				htx_strncpy(input, "", DIM(input));	/* clear input */
				get_string(stdscr, AH_IN_ROW, AH_IN_COL, input, (size_t) DIM(input), (char *) NULL, (tbool) TRUE);

				CLRLIN(MSGLINE, 0);
				switch (input[0]) {
					/*
					 * activate all devices
					 */
					case 'a':                /* Activate all the devices in the current screen */
						dev_no = -1;
						update_screen = TRUE;
						for (i = 1; i <= num_disp; i++)  {
							run_device = FALSE;
							/*
							 * compute pointers for this device
							 */

							p_into_table = p_ahd_table + (strt_ent - 1) + (i - 1); 
							p_htxshm_HE = shm_addr_wk.HE_addr + p_into_table->shm_pos;
							/*
							 * get semaphore value of entry
							 */

							semval = semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6), GETVAL, &sembuffer);

							/*
							 * get semaphore value of entry
							 */

							err_semval = semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6 + 1), GETVAL, &sembuffer);

							/*
							 * not COE and halted on error now
							 */

							if (p_htxshm_HE->cont_on_err == 0 && err_semval == 1) {
								run_device = TRUE;
								sprintf(workstr, "Request to BYPASS HALT ON ERROR CONDITION for \"%s\" device issued by operator.", p_htxshm_HE->sdev_id);
							}

							/*
							 * HOE & not halted on error OR COE
							 */

							if (semval == 1) {
								/*
								 * HOE & halted on error OR COE
								 */

								if ((p_htxshm_HE->cont_on_err == 0 && err_semval == 0) || p_htxshm_HE->cont_on_err == 1) {
									run_device = TRUE;
									sprintf(workstr, "Request to ACTIVATE \"%s\" device issued by operator.", p_htxshm_HE->sdev_id);
								} /* endif */
							}

							if (run_device) {
								/*
								 * semaphores
								 * to run
								 */
								run_device = TRUE;
								semctl_arg.val =0 ;
								semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6), SETVAL, semctl_arg);
								semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6 + 1), SETVAL, semctl_arg);

								send_message(workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
							}	/* if  */
						}	/* for */
						break;
        					case 'A':       	/* Activate all the devices */
								dev_no = -1;
	                                                update_screen = TRUE;
	                                                for (i = 1; i <= num_entries; i++)  {
                                                        run_device = FALSE;
                                                        p_into_table = p_ahd_table + (i - 1);
							p_htxshm_HE = shm_addr_wk.HE_addr + p_into_table->shm_pos;
                                                        semval = semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6), GETVAL, &sembuffer);

                                                        err_semval = semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6 + 1), GETVAL, &sembuffer);

                                                        if (p_htxshm_HE->cont_on_err == 0 && err_semval == 1) {
                                                                run_device = TRUE;
                                                                sprintf(workstr, "Request to BYPASS HALT ON ERROR CONDITION for \"%s\" device issued by operator.", p_htxshm_HE->sdev_id);
                                                        }

                                                        if (semval == 1) {

                                                                if ((p_htxshm_HE->cont_on_err == 0 && err_semval == 0) || p_htxshm_HE->cont_on_err == 1) {
                                                                        run_device = TRUE;
                                                                        sprintf(workstr, "Request to ACTIVATE \"%s\" device issued by operator.", p_htxshm_HE->sdev_id);
                                                                } /* endif */
                                                        }

                                                        if (run_device) {
                                                                /*
                                                                 * semaphores
                                                                 * to run
                                                                 */
                                                                run_device = TRUE;
                                                                semctl_arg.val =0 ;
                                                                semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6), SETVAL, semctl_arg);
                                                                semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6 + 1), SETVAL, semctl_arg);

                                                                send_message(workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
                                                        }       /* if  */
                                                }       /* for */
                                                break;

					case 'f':
					case 'F':
						if (strt_ent < max_strt_ent) {
							strt_ent += 15;

							if (strt_ent > max_strt_ent) {
								strt_ent = max_strt_ent;
							}
							update_screen = TRUE;
						}

						else {
							beep();
						}	/* endif */
						dev_no = -1;
						break;

					case 'b':
					case 'B':
						if (strt_ent > 1) {
							strt_ent -= 15;
							if (strt_ent < 1)  {
								strt_ent = 1;
							}

							update_screen = TRUE;

						}

						else {
							beep();
						}	/* endif */
						dev_no = -1;
						break;

					case 'h':	/* halt all devices in current screen*/
						dev_no = -1;
						update_screen = TRUE;
						for (i = 1; i <= num_disp; i++)  {
							/* compute pointers for this device */

							p_into_table = p_ahd_table + (strt_ent - 1) + (i - 1);
							p_htxshm_HE = shm_addr_wk.HE_addr + p_into_table->shm_pos;

							/* get semaphore value of entry  */
							semval = semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6), GETVAL, &sembuffer);
							/* get semaphore value of entry */
							err_semval = semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6 + 1), GETVAL, &sembuffer);

							/* running and not stopped on error */
							if (semval == 0 && err_semval == 0)  {
								/* semaphore to halt */
								semctl_arg.val=1;
								semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6), SETVAL, semctl_arg);
								sprintf(workstr, "Request to HALT \"%s\" device issued by operator.", p_htxshm_HE->sdev_id);
								send_message(workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
							}	/* if  */
						}	/* for */
						break;

				        case 'H':         /* Halt all the devices */
							dev_no = -1;
						 update_screen = TRUE;
                                                for (i = 1; i <= num_entries; i++)  {
                                                        /* compute pointers for this device */

                                                        p_into_table = p_ahd_table + (i - 1);
                                                        p_htxshm_HE = shm_addr_wk.HE_addr + p_into_table->shm_pos;

                                                        /* get semaphore value of entry  */
                                                        semval = semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6), GETVAL, &sembuffer);
                                                        /* get semaphore value of entry */
                                                        err_semval = semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6 + 1), GETVAL, &sembuffer);

                                                        /* running and not stopped on error */
                                                        if (semval == 0 && err_semval == 0)  {
                                                                /* semaphore to halt */
                                                                semctl_arg.val=1;
                                                                semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6), SETVAL, semctl_arg);
									sprintf(workstr, "Request to HALT \"%s\" device issued by operator.", 
											p_htxshm_HE->sdev_id);
                                                                send_message(workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
                                                        }       /* if  */
                                                }       /* for */
                                                break;
					case 'q':
					case 'Q':
						if (p_ahd_table != NULL)  {
							free((char *) p_ahd_table);	/* rel mem for ahd table */
						}
						return (0);

					case 'd':
					case 'D':
						clear();
						refresh();
						display_scn(stdscr, 0, 0, LINES,COLS, "AHD_scn", 1, &row, &col, NUM_COLS, NUM_ROWS, 'n');
						update_screen = TRUE;
						dev_no = -1;
						break;

					case '/':
						rc = search_win(16, 50, 2, 2, "ddsrch_scn", 1, page_str);
						dev_len = strlen(page_str);
			
						if ( rc == -1 ) {
							(void) clear();
							wrefresh(stdscr);
							/* (void) refresh(); */
							(void) display_scn(stdscr, 0, 0, LINES, COLS, "AHD_scn",
								1, &row, &col, NUM_COLS, NUM_ROWS, 'n');
							PRTMSG(MSGLINE, 0, ("No String entered."));
							update_screen = TRUE;
							dev_no = -1;
							break;
						}

						if ( rc == 0 ) {
							(void) clear();
							wrefresh(stdscr);
							(void) display_scn(stdscr, 0, 0, LINES, COLS, "AHD_scn",
								1, &row, &col, NUM_COLS, NUM_ROWS, 'n');
							update_screen = TRUE;
							dev_no = -1;
							break;
						}

						for ( k=0; k<num_entries; k++) {
							struct ahd_entry    *p_into_table_temp;
							struct htxshm_HE    *p_htxshm_HE_temp;
			
							p_into_table_temp = p_ahd_table + k;
							p_htxshm_HE_temp = shm_addr_wk.HE_addr + p_into_table_temp->shm_pos;
			
							if ( strncmp(p_htxshm_HE_temp->sdev_id, page_str, dev_len) == 0 ) {
								break;
							}
						}
			
						if ( k >= num_entries ) {
							(void) clear();
							wrefresh(stdscr);
							(void) refresh();
							(void) display_scn(stdscr, 0, 0, LINES, COLS, "AHD_scn",
								1, &row, &col, NUM_COLS, NUM_ROWS, 'n');
							PRTMSG(MSGLINE, 0, ("Device not found."));
							update_screen = TRUE;
							dev_no = -1;
							break;
						}

						dev_no = k%15;
			
						/* Points to the 1st entry of 15-entry-page 
						 * in which this device will be displayed.
						 */ 
						strt_ent = (((k/15) * 15) + 1); 
						if (strt_ent > max_strt_ent) strt_ent = max_strt_ent;
						if (strt_ent <= 0) strt_ent = 1;

						(void) clear();
						(void) refresh();
						(void) display_scn(stdscr, 0, 0, LINES, COLS, "AHD_scn",
								1, &row, &col, NUM_COLS, NUM_ROWS, 'n');
						update_screen = TRUE;
						break;
					default:
						dev_no = -1;	
						cmp_regexp = regcmp(input, (char *) 0);	/* compile exp */
						if (cmp_regexp != NULL) {	/* regular expression OK? */
							for (i = num_disp; i >= 1; i--)  {
								sprintf(workstr, "%d", i);
								work = regex(cmp_regexp, workstr);
								if (work != NULL)  {
							               htx_strncpy(ret, "", (size_t) sizeof(ret));	/* clear return (ret) array */
								       htx_strncpy(ret, __loc1, (size_t)(work - __loc1));

									/* matched reg exp? */
									if (htx_strcmp(workstr, ret) == 0)  {
										update_screen = TRUE;
										p_into_table =  p_ahd_table + (strt_ent	- 1) + (i - 1);
										p_htxshm_HE = shm_addr_wk.HE_addr + p_into_table->shm_pos;
										semval = semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6), GETVAL, &sembuffer);
										/* get semaphore value of entry */
										err_semval = semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6 + 1), GETVAL, &sembuffer);
										/* get semaphore value of entry */
										if((p_htxshm_HE->cont_on_err == 1) || (err_semval ==  0)) {
											if(semval == 0) {	/* set to run?                 */
												semctl_arg.val=1;
												semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6), SETVAL, semctl_arg);
												/* set semaphore for halt    */
												sprintf(workstr, "Request to HALT \"%s\" device issued by operator.", p_htxshm_HE->sdev_id);
											}
											else {	/* halted. */
												semctl_arg.val=0;
												semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6), SETVAL, semctl_arg); /* semaphore to active state */
												sprintf(workstr, "Request to ACTIVATE \"%s\" device issued by operator.", p_htxshm_HE->sdev_id);
											}	/* endif */
										}
										else {	/* release error semaphore wait */
											semctl_arg.val=0;
											semctl(semhe_id, ((p_into_table->shm_pos * SEM_PER_EXER) + 6 + 1), SETVAL, semctl_arg);	/* clear error semaphore */
											sprintf(workstr, "Request to BYPASS HALT ON ERROR CONDITION for \"%s\" device issued by operator.", p_htxshm_HE->sdev_id);
										}	/* endif */
										send_message(workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);

										/*
										 * NOTE: The method has been reversed to count
										 * downwards to eliminate duplicate sem
										 * operations for Linux, so this 'break'
										 * required.
										 *
										 * Ashutosh S. Rajekar.
										 */
										break;
									}	/* endif */
								}	/* endif */
							}	/* endfor */
							if (update_screen != TRUE) {	/* no match? */
								beep();
							}	/* endif */
						}
						else {	/* invalid entry_number */
							beep();
						}	/* endif */
						break;
				}	/* endswitch */
			}		/* endfor */
		}		/* endfor */
}				/* AH_device() */


/*****************************************************************************/
/*****  i n i t _ a h d _ t b l ( )  *****************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME  =    init_ahd_tbl()                                        */
/*                                                                           */
/* DESCRIPTIVE NAME =  Initialize Activate/Halt Device Table                 */
/*                                                                           */
/* FUNCTION =          Initializes and sorts the activate/halt device table  */
/*                                                                           */
/* INPUT =             p_ahd_tbl_ptr: pointer to a pointer which will (after */
/*                         the function completes) point to the activate/    */
/*                         halt device table                                 */
/*                                                                           */
/* OUTPUT =            The sorted activate/halt device status table          */
/*                                                                           */
/* NORMAL RETURN =     The number of entries in the activate/halt device     */
/*                     table                                                 */
/*                                                                           */
/* ERROR RETURN =      -1 to indicate no hardware exercisers currently in    */
/*                         memory.                                           */
/*                     -2 to indicate an unsuccessful attempt to allocate    */
/*                         memory for the activate/halt device table.        */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = AHD_compar() - compares hardware exerciser entries    */
/*                                    for sequence sort                      */
/*                                                                           */
/*    DATA AREAS =     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                                                                           */
/*    MACROS =         PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

int	init_ahd_tbl(struct ahd_entry **p_ahd_tbl_ptr)
{
	char	workstr[128];		/* work string */
	extern union shm_pointers shm_addr;  /* shared memory union pointers */
	int	i;			/* loop counter */
	int	num_entries;		/* local number of entries */
	union shm_pointers shm_addr_wk;	/* work ptr into shm */

	/*
	 * allocate space for ahd order table
	 */
	if(*p_ahd_tbl_ptr != NULL)  {
		free((char *) *p_ahd_tbl_ptr);
	}

	if ((num_entries = (shm_addr.hdr_addr)->max_entries) == 0) {
		PRTMSG(MSGLINE, 0, ("There are no Hardware Exerciser programs currently executing."));
		return (-1);
	}			/* endif */

	*p_ahd_tbl_ptr = (struct ahd_entry *) calloc((unsigned) num_entries, sizeof(struct ahd_entry));
	if (*p_ahd_tbl_ptr == NULL) {
		sprintf(workstr, "Unable to allocate memory for ahd sort.  errno = %d.", errno);
		PRTMSG(MSGLINE, 0, ("%s", workstr));
		send_message(workstr, errno, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
		return (-1);
	} /* endif */


	/*
	 * build ahd display order table
	 */
	shm_addr_wk.hdr_addr = shm_addr.hdr_addr; /* copy addr to work space */
	(shm_addr_wk.hdr_addr)++;	/* skip over header */

	for (i = 0; i < num_entries; i++) {
		(*p_ahd_tbl_ptr + i)->shm_pos = i;
		(*p_ahd_tbl_ptr + i)->slot = (shm_addr_wk.HE_addr + i)->slot;
		(*p_ahd_tbl_ptr + i)->port = (shm_addr_wk.HE_addr + i)->port;
		strcpy(((*p_ahd_tbl_ptr + i)->sdev_id), ((shm_addr_wk.HE_addr + i)->sdev_id));
	}			/* endfor */

#ifdef	__HTX_LINUX__
#include <stdlib.h>
	qsort((void *) *p_ahd_tbl_ptr, (size_t) num_entries, (size_t) sizeof(struct ahd_entry), (int (*)(const void *, const void *)) AHD_compar);
#else
	qsort(*p_ahd_tbl_ptr, (size_t) num_entries, (size_t) sizeof(struct ahd_entry), AHD_compar);
#endif

	return (num_entries);
}				/* init_ahd_tbl() */

