
/* @(#)37	1.2.4.2  src/htx/usr/lpp/htx/bin/hxssup/COE_devic.c, htx_sup, htxubuntu 12/16/14 03:57:37 */

/*****************************************************************************/
/*                                                                           */
/* MODULE NAME    =    COE_device.c                                          */
/* COMPONENT NAME =    hxssup (supervisor)                                   */
/* LPP NAME       =    HTX                                                   */
/*                                                                           */
/* DESCRIPTIVE NAME =  Show/Set/Clear "Continue on Error" Flag(s) for        */
/*                     Device(s)                                             */
/*                                                                           */
/* COPYRIGHT =         (C) COPYRIGHT IBM CORPORATION 1988                    */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM           */
/*                                                                           */
/* STATUS =            Release 1 Version 0                                   */
/*                                                                           */
/* FUNCTION =          Displays all the devices' current COE (Continue On    */
/*                     Error) flag states and allows the operator to change  */
/*                     the COE flags.                                        */
/*                                                                           */
/* COMPILER OPTIONS =  -I/src/master/htx/common -g -Nn3000 -Nd4000 -Np1200   */
/*                     -Nt2000                                               */
/*                                                                           */
/* CHANGE ACTIVITY =                                                         */
/*    DATE    :LEVEL:PROGRAMMER:DESCRIPTION                                  */
/*    MMMMMMMMNMMMMMNMMMMMMMMMMNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM */
/*    07/15/88:1.0  :J BURGESS :INITIAL RELEASE                              */
/*    12/20/99:1.6  :R GEBHARDT:Added "c" and "h" operations.                */
/*            :     :          :                                             */
/*                                                                           */
/*****************************************************************************/

#include "hxssup.h"

#define NUM_COLS	(23)
#define NUM_ROWS	(81)

#define CE_IN_ROW	(21)
#define CE_IN_COL	(18)

static	int	init_coe_tbl(struct ahd_entry **p_coe_tbl_ptr);
extern	char	*regcmp(char *, char *);	/* compile exp */
extern	char	*regex(char *, char *);

extern union semun semctl_arg;


/*****************************************************************************/

/*****  C O E _ d e v i c e ( )  *********************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     COE_device.c                                          */
/*                                                                           */
/* DESCRIPTIVE NAME =  Show/Set/Clear "Continue on Error" Flag(s) for        */
/*                     Device(s)                                             */
/*                                                                           */
/* FUNCTION =          Displays all the devices' current COE (Continue On    */
/*                     Error) flag states and allows the operator to change  */
/*                     the COE flags.                                        */
/*                                                                           */
/* INPUT =             Operator keyboard input.                              */
/*                                                                           */
/* OUTPUT =            Device "Continue on Error" Flag Screen(s)             */
/*                                                                           */
/*                     Updated COE flags in shared memory.                   */
/*                                                                           */
/* NORMAL RETURN =     0 to indicate successful completion                   */
/*                                                                           */
/* ERROR RETURNS =     -1 to indicate no hardware exerciser programs         */
/*                         currently in memory.                              */
/*                     -2 to indicate an unsuccessful attempt to allocate    */
/*                         memory for the COE device table.                  */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = display_scn -  displays screen image                  */
/*                     init_coe_tbl - initializes and sorts the list of      */
/*                                    devices                                */
/*                     send_message - outputs a message to the screen and    */
/*                                    the message log file                   */
/*                                                                           */
/*    DATA AREAS =     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                                                                           */
/*    MACROS =         CLRLIN - Clear line CURSES macro                      */
/*                              (defined in common/hxiconv.h)                */
/*                     PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                                                                           */
/*****************************************************************************/

int	COE_device(void)
{
	char	*cmp_regexp = NULL;	/* ptr to compiled reg expression    */
	char	input[32];		/* input string                      */
	char	ret[3];		/* return array for regex()    */
	char	*work = NULL;		/* work int */
	char	workstr[128];	/* work string                       */
	char	page_str[15];

	extern char *__loc1;	/* beginning of reg exp match */

	extern int COLS;	/* number of cols in CURSES screen   */
	extern int LINES;	/* number of lines in CURSES screen  */
	extern int semhe_id;	/* HE semaphore id                   */
	extern WINDOW *stdscr;	/* standard screen window            */

	extern union shm_pointers shm_addr; /* shared memory union pointers */

	int	col = 0;		/* first column displayed on screen  */
	int	i = 0;			/* loop counter                      */
	int k;               /* loop counter                      */
	int rc;              /* return value.                     */
	int dev_len;         /* loop counter                      */
	int dev_no=-1;       /* loop counter                      */

	int	max_strt_ent = 0;	/* max starting entry                */
	int	num_disp = 0;		/* number of coe entries to show     */
	int	num_entries = 0;	/* local number of shm HE entries    */
	int	row = 0;		/* first row displayed on screen     */
	int	strt_ent = 1;	/* first entry on screen             */
	int	update_screen = 0;	/* update screen flag         */

	struct ahd_entry *p_coe_table = NULL; /* points to begin coe seq tbl */
	struct ahd_entry *p_into_table = NULL;	/* points into coe sequence table    */
	struct htxshm_HE *p_htxshm_HE = NULL;	/* pointer to a htxshm_HE struct     */
	union shm_pointers shm_addr_wk;	/* work ptr into shm                 */
	unsigned short err_semnum;	/* the HE error semaphore number.    */

	shm_addr_wk.hdr_addr = shm_addr.hdr_addr; /* copy addr to work space */
	(shm_addr_wk.hdr_addr)++;	/* skip over header                  */

	/*
	 * display screen outline
	 */
	clear();
	refresh();
	display_scn(stdscr, 0, 0, LINES, COLS, "COE_scn", 1, &row, &col, NUM_COLS, NUM_ROWS, 'n');

	/*
	 * loop until operator says to quit
	 */
	for (;;) {
		if ((shm_addr.hdr_addr)->max_entries == 0) {	/* no HE programs? */
			PRTMSG(MSGLINE, 0, ("There are no Hardware Exerciser programs currently defined."));
			getch();
			fflush(stdin);
			CLRLIN(MSGLINE, 0);
			if (p_coe_table != NULL)  {
				free((char *) p_coe_table);	/* release memory for coe order tbl */
			}

			return (-1);
		}
		
		else {	/* some HE's defined.                */
			if ((shm_addr.hdr_addr)->max_entries != num_entries) {	/* # HE change? */
				num_entries = init_coe_tbl(&p_coe_table);
				if (num_entries <= 0)  {
					return (num_entries);
				}

				/* problem in init fcn - bye!      */
				else {	/* init fcn ran ok.                */
					if (num_entries <= 14) {	/* 14 or fewer entries? */
						max_strt_ent = 1;
					}
					
					else {	/* more than 14 entries */
						max_strt_ent = num_entries;
					}	/* endif */
				}	/* endif */
			}	/* endif */
		}		/* endif */



		/*
		 * build screen data for the current strt_ent
		 */

		p_into_table = p_coe_table + strt_ent - 1;
		num_disp = num_entries - strt_ent + 1;
		if (num_disp > 14)  {
			num_disp = 14;
		}

		for (i = 1; i <= 14; i++) {
			if (i <= num_disp) {	/* something there? */
				p_htxshm_HE = shm_addr_wk.HE_addr + p_into_table->shm_pos;

				/*
				 * build output characters & attributes for entry
				 */
				/*
				 * screen entry number
				 */
#ifdef	__HTX_LINUX__
				wcolorout(stdscr, WHITE_BLUE | NORMAL);
#else
				wcolorout(stdscr, NORMAL | F_WHITE | B_BLUE);
#endif
				wmove(stdscr, (i + 5), 3);
				wprintw(stdscr, "%2d", i);

				/*
				 * Status
				 */
				if (p_htxshm_HE->cont_on_err == 1) {	/* continue on error flag set?   */
#ifdef	__HTX_LINUX__
					wcolorout(stdscr, BLACK_RED | BOLD | STANDOUT);
#else
					wcolorout(stdscr, STANDOUT | F_BLACK | B_RED | BOLD);
#endif
					mvwaddstr(stdscr, (i + 5), 6, "  COE   ");
				}
				
				else {	/* halted. */
#ifdef	__HTX_LINUX__
					wcolorout(stdscr, BLACK_RED | BOLD | NORMAL);
#else
					wcolorout(stdscr, NORMAL | F_BLACK | B_RED | BOLD);
#endif
					mvwaddstr(stdscr, (i + 5), 6, "  HOE   ");
				}	/* endif */

				/*
				 * Slot, Port, /dev/id, Adapt Desc, and Device
				 * Desc
				 */
				wmove(stdscr, i + 5, 15);
#ifdef	__HTX_LINUX__
				wcolorout(stdscr, WHITE_BLUE | NORMAL);
#else
				wcolorout(stdscr, NORMAL | F_WHITE | B_BLUE);
#endif
				sprintf(workstr, " %.4d ", p_htxshm_HE->slot);
				mvwaddstr(stdscr, (i + 5), 15, workstr);
				sprintf(workstr, " %.4d ", p_htxshm_HE->port);
				mvwaddstr(stdscr, (i + 5), 22, workstr);
				if ( (dev_no+1) == i ) {
					wcolorout(stdscr, RED_BLACK | STANDOUT | BOLD);
				}
				else {
					wcolorout(stdscr, WHITE_BLUE | NORMAL);
				}
				sprintf(workstr, " %-7s ", p_htxshm_HE->sdev_id);
				mvwaddstr(stdscr, (i + 5), 29, workstr);
				wcolorout(stdscr, WHITE_BLUE | NORMAL);
				sprintf(workstr, " %-11s ", p_htxshm_HE->adapt_desc);
				mvwaddstr(stdscr, (i + 5), 39, workstr);
				sprintf(workstr, " %-18s ", p_htxshm_HE->device_desc);
				mvwaddstr(stdscr, (i + 5), 53, workstr);
			}
			
			else {	/* no HE entry for this area of screen */
#ifdef	__HTX_LINUX__
				wcolorout(stdscr, WHITE_BLUE | NORMAL);
#else
				wcolorout(stdscr, NORMAL | F_WHITE | B_BLUE);
#endif
				mvwaddstr(stdscr, (i + 5), 3, "  ");	/* scn entry num */
				mvwaddstr(stdscr, (i + 5), 6, "        "); /* COE Flag  */
				mvwaddstr(stdscr, (i + 5), 15, "      "); /* slot field */
				mvwaddstr(stdscr, (i + 5), 22, "      "); /* port field */
				mvwaddstr(stdscr, (i + 5), 29, "         "); /* sdev_id */
				mvwaddstr(stdscr, (i + 5), 39, "             ");

				/* adapt_desc field */
				mvwaddstr(stdscr, (i + 5), 53, "                    ");
				/* device_desc field */
			}	/* endif */

			p_into_table++;
		}		/* endfor */

#ifdef	__HTX_LINUX__
		wcolorout(stdscr, NORMAL);
#else
		wcolorout(stdscr, NORMAL);
#endif

		for (update_screen = FALSE; (update_screen == FALSE);) {

			/*
			 * Read input from keyboard...
			 */
			htx_strncpy(input, "", DIM(input));	/* clear input */
			get_string(stdscr, CE_IN_ROW, CE_IN_COL, input, (size_t) DIM(input), (char *) NULL, (tbool) TRUE);

			switch (input[0]) {

				case 'c':
				case 'C':	/* mark all to COE */
					dev_no = -1;
					for (i = 1; i <= num_disp; i++) {
						p_into_table = p_coe_table + (strt_ent - 1) + (i - 1);
						p_htxshm_HE = shm_addr_wk.HE_addr + p_into_table->shm_pos;
						if (p_htxshm_HE->cont_on_err == 0) {	/* COE flag clear */
							update_screen = TRUE;
							p_htxshm_HE->cont_on_err = 1;
							/* set COE flag */
							sprintf(workstr, "Request to SET \"%s\" continue on error flag issued by operator.", p_htxshm_HE->sdev_id);
							err_semnum = (unsigned short) ((p_into_table->shm_pos * SEM_PER_EXER) + 6 + 1);
							semctl_arg.val = 0;
							semctl(semhe_id,
								      (int) err_semnum, SETVAL, semctl_arg);
							send_message(workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
						}	/* endif */
					}	/* for */
					break;

				case 'h':
				case 'H':	/* mark all to HOE */
					dev_no = -1;
					for (i = 1; i <= num_disp; i++) {
						p_into_table = p_coe_table + (strt_ent - 1) + (i - 1);
						p_htxshm_HE = shm_addr_wk.HE_addr + p_into_table->shm_pos;
						if (p_htxshm_HE->cont_on_err == 1) {	/* COE flag set? */
							update_screen = TRUE;
							p_htxshm_HE->cont_on_err = 0;	/* clear COE flag */
							sprintf(workstr, "Request to CLEAR \"%s\" continue on error flag issued by operator.", p_htxshm_HE->sdev_id);
							send_message(workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
						}	/* if */
					}	/* for */
					break;

				case 'f':
				case 'F':
					dev_no = -1;
					if (strt_ent < max_strt_ent) {
						strt_ent += 14;

						if (strt_ent > max_strt_ent)  {
							strt_ent = max_strt_ent;
						}

						update_screen = TRUE;
					}
					
					else {
						beep();
					}	/* endif */
					break;

				case 'b':
				case 'B':
					dev_no = -1;
					if (strt_ent > 1) {
						strt_ent -= 14;
						if (strt_ent < 1)  {
							strt_ent = 1;
						}

						update_screen = TRUE;
					}
					
					else {
						beep();
					}	/* endif */
					break;

				case 'q':
				case 'Q':
					if (p_coe_table != NULL)  {
						free((char *) p_coe_table);	/* rel mem for ahd tbl */
					}
					return (0);

				case 'd':
				case 'D':
					dev_no = -1;
					clear();
					refresh();
					display_scn(stdscr, 0, 0, LINES, COLS, "COE_scn", 1, &row, &col, NUM_COLS, NUM_ROWS, 'n');
					update_screen = TRUE;
					break;

				case '/':
					rc = search_win(16, 50, 2, 2, "ddsrch_scn", 1, page_str);
					dev_len = strlen(page_str);

					if ( rc == -1 ) {
						(void) clear();
						wrefresh(stdscr);
						/* (void) refresh(); */
						(void) display_scn(stdscr, 0, 0, LINES, COLS, "COE_scn",
							1, &row, &col, NUM_COLS, NUM_ROWS, 'n');
						PRTMSG(MSGLINE, 0, ("No String entered."));
						update_screen = TRUE;
						dev_no = -1;
						break;
					}

					if ( rc == 0 ) {
						(void) clear();
						wrefresh(stdscr);
						(void) display_scn(stdscr, 0, 0, LINES, COLS, "COE_scn",
							1, &row, &col, NUM_COLS, NUM_ROWS, 'n');
						update_screen = TRUE;
						dev_no = -1;
						break;
					}

					for ( k=0; k<num_entries; k++) {
						struct ahd_entry    *p_into_table_temp;
						struct htxshm_HE    *p_htxshm_HE_temp;
		
						p_into_table_temp = p_coe_table + k;
						p_htxshm_HE_temp = shm_addr_wk.HE_addr + p_into_table_temp->shm_pos;
		
						if ( strncmp(p_htxshm_HE_temp->sdev_id, page_str, dev_len) == 0 ) {
							break;
						}
					}

					if ( k >= num_entries ) {
						(void) clear();
						wrefresh(stdscr);
						(void) refresh();
						(void) display_scn(stdscr, 0, 0, LINES, COLS, "COE_scn",
							1, &row, &col, NUM_COLS, NUM_ROWS, 'n');
						PRTMSG(MSGLINE, 0, ("Device not found."));
						update_screen = TRUE;
						dev_no = -1;
						break;
					}

					dev_no = k%14;

					/* Points to the 1st entry of 14-entry-page
					 * in which this device will be displayed.
					 */
					strt_ent = (((k/14) * 14) + 1);
					if (strt_ent > max_strt_ent) strt_ent = max_strt_ent;
					if (strt_ent <= 0) strt_ent = 1;

					(void) clear();
					(void) refresh();
					(void) display_scn(stdscr, 0, 0, LINES, COLS, "COE_scn",
							1, &row, &col, NUM_COLS, NUM_ROWS, 'n');
					update_screen = TRUE;
					break;

				default:
					dev_no = -1;
					cmp_regexp = regcmp(input, (char *) 0);	/* compile exp */
					if (cmp_regexp != NULL) {	/* regular expression OK? */
						for (i = 1; i <= num_disp; i++) {
							sprintf(workstr, "%-d", i);
							work = regex(cmp_regexp, workstr);
							if (work != NULL) {
								htx_strncpy(ret, " ", (size_t) sizeof(ret));	/* clear return (ret) array */
								htx_strncpy(ret, __loc1, (size_t) (work - __loc1));

								if(htx_strcmp(workstr, ret) == 0)	/* matched reg exp? */  {
									update_screen = TRUE;
									p_into_table = p_coe_table + (strt_ent - 1) + (i - 1);
									p_htxshm_HE = shm_addr_wk.HE_addr + p_into_table->shm_pos;

									if(p_htxshm_HE->cont_on_err == 1) {	/* COE flag set?               */
										p_htxshm_HE->cont_on_err =  0; /* clear COE flag */
										sprintf(workstr, "Request to CLEAR \"%s\" continue on error flag issued by operator.", p_htxshm_HE->sdev_id);
									}
									
									else {	/* COE flag clear */
										p_htxshm_HE->cont_on_err = 1;	/* set COE flag */
										sprintf(workstr, "Request to SET \"%s\" continue on error flag issued by operator.", p_htxshm_HE->sdev_id);
										err_semnum = (unsigned short) ((p_into_table->shm_pos * SEM_PER_EXER) + 6 + 1);
										semctl_arg.val = 0 ; 
										semctl(semhe_id, (int) err_semnum, SETVAL, semctl_arg);
									}	/* endif */
									send_message(workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
								} /* endif */
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
	}			/* endfor */
}				/* COE_device() */


/*****************************************************************************/
/*****  i n i t _ c o e _ t b l ( )  *****************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME  =    init_coe_tbl()                                        */
/*                                                                           */
/* DESCRIPTIVE NAME =  Initialize Continue on Error Flag Table.              */
/*                                                                           */
/* FUNCTION =          Initializes and sorts the COE flag device table       */
/*                                                                           */
/* INPUT =             p_coe_tbl_ptr: pointer to a pointer which will (after */
/*                         the function completes) point to the COE flag     */
/*                         device table.                                     */
/*                                                                           */
/* OUTPUT =            The sorted COE flag device status table               */
/*                                                                           */
/* NORMAL RETURN =     The number of entries in the COE flag device table.   */
/*                                                                           */
/* ERROR RETURN =      -1 to indicate no hardware exercisers currently in    */
/*                         memory.                                           */
/*                     -2 to indicate an unsuccessful attempt to allocate    */
/*                         memory for the COE flag device table.             */
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

/*
 * This function is also found in IT_device.c, and does almost the same thing
 * - Ashutosh S. Rajekar
 */

static	int	init_coe_tbl(struct ahd_entry **p_coe_tbl_ptr)
{
	char	workstr[128];			/* work string */
	extern union shm_pointers shm_addr;  /* shared memory union pointers */

	int	i;			/* loop counter                      */
	int	num_entries;		/* local number of entries           */

	union shm_pointers shm_addr_wk;	/* work ptr into shm                 */

	/*
	 * allocate space for coe order table
	 */
	if(*p_coe_tbl_ptr != NULL)  {
		free((char *) *p_coe_tbl_ptr);
	}

	if ((num_entries = (shm_addr.hdr_addr)->max_entries) == 0) {
		PRTMSG(MSGLINE, 0, ("There are no Hardware Exerciser programs currently executing."));
		return (-1);
	}			/* endif */

	*p_coe_tbl_ptr = (struct ahd_entry *) calloc((unsigned) num_entries, sizeof(struct ahd_entry));

	if (*p_coe_tbl_ptr == NULL) {
		sprintf(workstr, "Unable to allocate memory for coe sort.  errno = %d.", errno);
		PRTMSG(MSGLINE, 0, ("%s", workstr));
		send_message(workstr, errno, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
		return (-1);
	}	/* endif */

	/*
	 * build coe display order table
	 */
	shm_addr_wk.hdr_addr = shm_addr.hdr_addr; /* copy addr to work space */
	(shm_addr_wk.hdr_addr)++;	/* skip over header  */

	for (i = 0; i < num_entries; i++) {
		(*p_coe_tbl_ptr + i)->shm_pos = i;
		(*p_coe_tbl_ptr + i)->slot = (shm_addr_wk.HE_addr + i)->slot;
		(*p_coe_tbl_ptr + i)->port = (shm_addr_wk.HE_addr + i)->port;
		htx_strcpy(((*p_coe_tbl_ptr + i)->sdev_id), ((shm_addr_wk.HE_addr + i)->sdev_id));
	}			/* endfor */

#ifdef AIXVER2
	qsort((char *) *p_coe_tbl_ptr, (unsigned int) num_entries,
	      sizeof(struct ahd_entry), AHD_compar);
#else
#ifdef	__HTX_LINUX__
#include <stdlib.h>
	qsort((void *) *p_coe_tbl_ptr, (size_t) num_entries,
	      (size_t) sizeof(struct ahd_entry), (int (*)(const void *, const void *)) AHD_compar);
#else
	qsort(*p_coe_tbl_ptr, (size_t) num_entries,
	      (size_t) sizeof(struct ahd_entry), AHD_compar);
#endif
#endif

	return (num_entries);
}				/* init_coe_tbl() */


