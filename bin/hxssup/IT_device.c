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

/* @(#)38	1.2  src/htx/usr/lpp/htx/bin/hxssup/IT_device.c, htx_sup, htxubuntu 5/24/04 13:41:39 */

/*****************************************************************************/
/*                                                                           */
/* MODULE NAME    =    IT_device.c                                           */
/* COMPONENT NAME =    hxssup (supervisor)                                   */
/* LPP NAME       =    HTX                                                   */
/*                                                                           */
/* DESCRIPTIVE NAME =  Show/Set Idle Time(s) for Device(s)                   */
/*                                                                           */
/* COPYRIGHT =         (C) COPYRIGHT IBM CORPORATION 1988                    */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM           */
/*                                                                           */
/* STATUS =            Release 1 Version 0                                   */
/*                                                                           */
/* FUNCTION =          Displays all the current idle times for the defined   */
/*                     devices and allows the operator to modify their       */
/*                     values.                                               */
/*                                                                           */
/* COMPILER OPTIONS =  -I/src/master/htx/common -g -Nn3000 -Nd4000 -Np1200   */
/*                     -Nt2000                                               */
/*                                                                           */
/* CHANGE ACTIVITY =                                                         */


#ifdef	__HTX_LINUX__
#include <hxiconv.h>
#include <hxiipc.h>
#else
#include "hxiconv.h"
#include "hxiipc.h"
#endif
    
#include <stdlib.h>

#ifdef	__HTX_LINUX__
#define	SCN_ROWS	(24)
#define	SCN_COLS	(80)
#endif

extern	int display_scn(void);	/* display screen function           */
extern	char	*htx_strcpy(char *, const char*);
/*
 * Define this static -- re-defined in COE_devic.c - Ashutosh S. Rajekar
 */
static	int	init_coe_tbl(struct ahd_entry **p_coe_tbl_ptr);
extern short send_message(char *msg_text, int errno_val, int severity,
		   mtyp_t msg_type);

/*****************************************************************************/
/*****  I T _ d e v i c e ( )  ***********************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     IT_device.c                                           */
/*                                                                           */
/* DESCRIPTIVE NAME =  Show/Set Idle Time(s) for Device(s)                   */
/*                                                                           */
/* FUNCTION =          Displays all the devices' current idle time values    */
/*                     and allows the operator to modify the values.         */
/*                                                                           */
/* INPUT =             p_loc_thres: pointer to the hft data structure which  */
/*                         contains the current locator (mouse) threshold    */
/*                         values                                            */
/*                                                                           */
/*                     Operator keyboard and/or mouse input                  */
/*                                                                           */
/* OUTPUT =            Device "Idle Time" Value(s)                           */
/*                                                                           */
/*                     Updated "Idle Time" values in shared memory           */
/*                                                                           */
/* NORMAL RETURN =     0 to indicate successful completion                   */
/*                                                                           */
/* ERROR RETURNS =     -1 to indicate no hardware exerciser programs         */
/*                         currently in memory.                              */
/*                     -2 to indicate an unsuccessful attempt to allocate    */
/*                         memory for the IT device table.                   */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = display_scn -  displays screen image                  */
/*                     init_coe_tbl - initializes and sorts the list of      */
/*                                    devices                                */
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

/*
 * struct hfloth *p_loc_thres ==> pointer to hft loc thres struct
 */

#ifndef	__HTX_LINUX__
int	IT_device(struct hfloth	*p_loc_thres)
{
	char	*p_in_sa_char = NULL;	/* ptr to char in scn array          */
	char	*p_in_sa_attr = NULL;	/* ptr to attributes in scn array    */
	char	workstr[128];	/* work string                       */


	extern int COLS;	/* number of cols in CURSES screen   */
	extern int LINES;	/* number of lines in CURSES screen  */
	extern WINDOW *stdscr;	/* standard screen window            */

	extern union shm_pointers shm_addr; /* shared memory union pointers */

	int	clr_msg_flg = 0;	/* clear MSGLINE if this flag is set */
	int	col = 0;		/* first column displayed on screen  */
	int	cur_col = 6;	/* cursor column position            */
	int	cur_row = 6;	/* cursor row position               */
	int	cursor_only = 0;	/* cursor movement only flag         */
	int	i = 0;			/* loop counter                      */
	int	key_input = 0;		/* input from keyboard               */
	int	max_strt_ent = 0;	/* max starting entry                */
	int	mode_array[10];	/* holds field attribute data        */
	int	num_disp = 0;		/* number of coe entries to show     */
	int	num_entries = 0;	/* local number of shm HE entries    */
	int	row = 0;		/* first row displayed on screen     */
	int	strt_ent = 1;	/* first entry on screen             */
	int	workint = 0;		/* work space                        */

#define NUM_COLS	(81)
#define NUM_ROWS	(24)

	static char scn_array[2][SCN_ROWS][SCN_COLS] = {
		{
		 "00000000000000000000000000000000000000000000000000000000000000000000000000000000",
		 "01111111111111111111112222222222222222222222222221111111111111111111111111111110",
		 "01111111111111777777777777777777777777777777777777777777777711111111111111111110",
		 "01111111111111111111111111111111111111111111111111111111111111111111111111111110",
		 "01111133333333433333343333334333333333433333333333334333333333333333333311111110",
		 "01111144444444444444444444444444444444444444444444444444444444444444444441111110",
		 "01111155555555411111141111114111111111411111111111114111111111111111111111111110",
		 "01111155555555411111141111114111111111411111111111114111111111111111111111111110",
		 "01111155555555411111141111114111111111411111111111114111111111111111111111111110",
		 "01111155555555411111141111114111111111411111111111114111111111111111111111111110",
		 "01111155555555411111141111114111111111411111111111114111111111111111111111111110",
		 "01111155555555411111141111114111111111411111111111114111111111111111111111111110",
		 "01111155555555411111141111114111111111411111111111114111111111111111111111111110",
		 "01111155555555411111141111114111111111411111111111114111111111111111111111111110",
		 "01111155555555411111141111114111111111411111111111114111111111111111111111111110",
		 "01111155555555411111141111114111111111411111111111114111111111111111111111111110",
		 "01111155555555411111141111114111111111411111111111114111111111111111111111111110",
		 "01111155555555411111141111114111111111411111111111114111111111111111111111111110",
		 "01111155555555411111141111114111111111411111111111114111111111111111111111111110",
		 "01111155555555411111141111114111111111411111111111114111111111111111111111111110",
		 "01111155555555411111141111114111111111411111111111114111111111111111111111111110",
		 "01111111111111111111111111111111111111111111111111111111111111111111111111111110",
		 "01111111111111111111111111111111111111111111111111111111115555555511111111111110",
		 "00000000000000000000000000000000000000000000000000000000000000000000000000000000"},
		{
		 "ÛßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßÛ",
		 "Û                     Set/Clear Continue on Error                              Û",
		 "Û             (COE = Continue on Error, HOE = Halt on Error)                   Û",
		 "Û                                                                              Û",
		 "Û      Status ³ Slot ³ Port ³ /dev/id ³ Adapt Desc  ³ Device Desc              Û",
		 "Û     ÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÅÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ      Û",
		 "Û       COE   ³ 1234 ³ 1234 ³ tty4567 ³ 12345678901 ³ 123456789012345678       Û",
		 "Û       HOE   ³      ³      ³         ³             ³                          Û",
		 "Û             ³      ³      ³         ³             ³                          Û",
		 "Û             ³      ³      ³         ³             ³                          Û",
		 "Û             ³      ³      ³         ³             ³                          Û",
		 "Û             ³      ³      ³         ³             ³                          Û",
		 "Û             ³      ³      ³         ³             ³                          Û",
		 "Û             ³      ³      ³         ³             ³                          Û",
		 "Û             ³      ³      ³         ³             ³                          Û",
		 "Û             ³      ³      ³         ³             ³                          Û",
		 "Û             ³      ³      ³         ³             ³                          Û",
		 "Û             ³      ³      ³         ³             ³                          Û",
		 "Û             ³      ³      ³         ³             ³                          Û",
		 "Û             ³      ³      ³         ³             ³                          Û",
		 "Û             ³      ³      ³         ³             ³                          Û",
		 "Û                                                                              Û",
		 "Û    Place the cursor by the desired device and press the \"Action\"             Û",
		 "ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß"}
	};

	struct ahd_entry *p_coe_table = NULL;	/* points to begin coe seq tbl  */
	struct ahd_entry *p_into_table;	/* points into coe sequence table    */

	struct hflocator *mouse_rpt;	/* pointer to mouse report           */

	struct htxshm_HE *p_htxshm_HE;	/* pointer to a htxshm_HE struct     */

	union {
		char hf_char_array[sizeof(short)];
		short hf_short_val;
	} hf_work2, hf_work3;	/* work unions for hf short vars     */

	union shm_pointers shm_addr_wk;	/* work ptr into shm                 */

	/*
	 * initialize mode_array
	 */
	mode_array[0] = NORMAL | F_WHITE | B_BLUE;
	mode_array[1] = NORMAL | F_WHITE | B_BLUE;
	mode_array[2] = STANDOUT | BOLD | F_BLACK | B_RED | UNDERSCORE;
	mode_array[3] = NORMAL | F_BLACK | B_BLUE | UNDERSCORE | BOLD;
	mode_array[4] = NORMAL | F_BLACK | B_BLUE;
	mode_array[5] = STANDOUT | BOLD | F_BLACK | B_RED;
	mode_array[6] = NORMAL | BOLD | F_BLACK | B_RED;
	mode_array[7] = NORMAL | F_WHITE | B_BLUE;
	mode_array[8] = NORMAL | F_WHITE | B_BLUE;
	mode_array[9] = NORMAL | F_WHITE | B_BLUE;

	shm_addr_wk.hdr_addr = shm_addr.hdr_addr; /* copy addr to work space */
	(shm_addr_wk.hdr_addr)++;	/* skip over header                  */

	mouse_rpt = (struct hflocator *) ESCSTR; /* mouse report address */

	/*
	 * loop until operator says to quit
	 */
	for (;;) {
		if ((shm_addr.hdr_addr)->num_entries == 0) {	/* no HE programs? */
			PRTMSG(MSGLINE, 0, ("There are no Hardware Exerciser programs currently defined."));
			if (p_coe_table != NULL)  {
				free((char *) p_coe_table);	/* release memory for coe order tbl */
			}

			return (-1);

		}
		
		else {	/* some HE's defined.                */
			if ((shm_addr.hdr_addr)->num_entries != num_entries) {	/* # HE change? */
				num_entries = init_coe_tbl(&p_coe_table);

				if (num_entries <= 0)  {
					return (num_entries);	/* problem in init fcn - bye! */
				}

				else {	/* init fcn ran ok.                */
					if (num_entries <= 15) {	/* 15 or fewer entries? */
						max_strt_ent = 1;
					}
					
					else {	/* more than 15 entries */
						max_strt_ent = num_entries - 14;
					}	/* endif */
				}	/* endif */
			}	/* endif */
		}		/* endif */

		/*
		 * build screen data for the current strt_ent
		 */
		p_into_table = p_coe_table + strt_ent - 1;
		num_disp = num_entries - strt_ent + 1;
		if (num_disp > 15)  {
			num_disp = 15;
		}

		for (i = 1; i <= 15; i++) {
			p_in_sa_char = &(coe_scn_array[1][i + 5][6]);
			p_in_sa_attr = &(coe_scn_array[0][i + 5][6]);

			if (i <= num_disp) {	/* something there? */
				p_htxshm_HE = shm_addr_wk.HE_addr + p_into_table->shm_pos;

				/*
				 * build output characters & attributes for
				 * entry
				 */
				/*
				 * Status
				 */
				if (p_htxshm_HE->cont_on_err == 1) {	/* continue on error flag set?   */
					htx_strncpy(p_in_sa_char, "  COE    ", 8);
					htx_strncpy(p_in_sa_attr, "555555555", 8);
				}
				
				else {	/* halted. */
					htx_strncpy(p_in_sa_char, "  HOE    ", 8);
					htx_strncpy(p_in_sa_attr, "666666666", 8);
				}	/* endif */

				/*
				 * Slot, Port, /dev/id, Adapt Desc, and Device
				 * Desc
				 */
				sprintf(workstr, " %.4d ³ %.4d ³ %-7s ³ %-11s ³ %-18s  \n", p_htxshm_HE->slot, p_htxshm_HE->port, p_htxshm_HE->sdev_id, p_htxshm_HE->adapt_desc, p_htxshm_HE->device_desc);

				htx_strncpy((p_in_sa_char + 9), workstr, (htx_strlen(workstr) - 1));
			}

			else {	/* no HE entry for this area of screen */
				htx_strncpy(p_in_sa_char, "        ³      ³      ³         ³             ³                     ", 67);
				htx_strncpy(p_in_sa_attr, "111111111", 8);
			}	/* endif */

			p_into_table++;
		}		/* endfor */

		/*
		 * display screen
		 */
		display_scn(stdscr, 0, 0, LINES, COLS, coe_scn_array, 1, &row, &col, COE_SCN_ROWS, COE_SCN_COLS, mode_array);
		for (cursor_only = TRUE; (cursor_only == TRUE);) {	/* cur move only   */
			/*
			 * position cursor
			 */
			move(cur_row, cur_col);
			refresh();
			cursor_only = FALSE; /* reset to FALSE - When I know */

			/*
			 * that is really only a cursor movement, I reset the
			 * flag to TRUE.
			 */
			/*
			 * read input from keyboard
			 */

			for (; (((key_input = getch()) == -1) && (errno == EINTR));)
				;

			if (clr_msg_flg != 0) {
				CLRLIN(MSGLINE, 0);
				clr_msg_flg = 0;
			}	/* endif */

			switch (key_input) {
				case KEY_MOUSE:
					for (i = 0; i < sizeof(short); i++) {
						hf_work2.hf_char_array[i] = mouse_rpt->hf_deltay[i];
						hf_work3.hf_char_array[i] = p_loc_thres->hf_vthresh[i];
					}	/* end_for */

					workint = hf_work2.hf_short_val / hf_work3.hf_short_val;
					if (workint < 0) { /* cursor down? */
						for (i = 0; i < abs(workint); i++) {
							if ((cur_row < 20) && ((cur_row - 5) < num_disp)) {
								cur_row++;
								cursor_only = TRUE;
							}
							
							else {
								if (strt_ent < max_strt_ent) {
									strt_ent++;
									cursor_only = FALSE;
								}
								
								else  {
									beep();
								}
							}	/* endif */
						}	/* endfor */
					}
					
					else {
						if (workint > 0) { /* cursor up? */
							for (i = 0; i < workint; i++) {
								if (cur_row > 6) {
									cur_row--;
									cursor_only = TRUE;
								}
								
								else {
									if(strt_ent > 1) {	/* code folded from here */
										strt_ent--;
										cursor_only = FALSE;	/* unfolding */
									}
									
									else  {
										beep();
									}	/* endif */
								}	/* endfor */
							}	/* endif */
						}	/* endif */

						if (mouse_rpt->hf_buttons != 0) {
							key_input = getch();	/* discard button - off character  */
							cursor_only = FALSE;

							p_into_table = p_coe_table + (strt_ent - 1) + (cur_row - 6);
							p_htxshm_HE = shm_addr_wk.HE_addr + p_into_table->shm_pos;

							if (p_htxshm_HE->cont_on_err == 1) {	/* COE flag set?               */
								p_htxshm_HE->cont_on_err = 0;	/* clear COE flag */
								sprintf(workstr, "Request to CLEAR \"%s\" continue on error flag issued by operator.", p_htxshm_HE->sdev_id);
							}
							
							else {	/* COE flag clear */
								p_htxshm_HE->cont_on_err = 1;	/* clear COE flag */
								sprintf(workstr, "Request to SET \"%s\" continue on error flag issued by operator.", p_htxshm_HE->sdev_id);
							}	/* endif */
							send_message(workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
						}	/* endif */
					}

					break;
						

				case KEY_DOWN:
					if ((cur_row < 20) && ((cur_row - 5) < num_disp)) {
						cur_row++;
						cursor_only = TRUE;
					}
						
					else {
						if (strt_ent < max_strt_ent) {
							strt_ent++;
							cursor_only = FALSE;
						}
							
						else {
							beep();
							cursor_only = TRUE;
						}	/* endif */
					}	/* endif */
					break;


				case KEY_UP:
					if (cur_row > 6) {
						cur_row--;
						cursor_only = TRUE;
					}

					else {
						if (strt_ent > 1)  {
							strt_ent--;
							cursor_only = FALSE;
						}
							
						else {
							beep();
							cursor_only = TRUE;
						}	/* endif */
					}	/* endif */
					break;


				case KEY_NPAGE:
					if (strt_ent < max_strt_ent) {
						strt_ent += 14;
						if (strt_ent > max_strt_ent)  {
							strt_ent = max_strt_ent;
						}
					}
						
					else  {
						beep();
					}
					break;

				case KEY_PPAGE:
					if (strt_ent > 1) {
						strt_ent -= 14;

						if (strt_ent < 1)  {
							strt_ent = 1;
						}
					}
						
					else  {
						beep();
					}
					break;

				case KEY_ACTION:
					p_into_table = p_coe_table + (strt_ent - 1) + (cur_row - 6);
					p_htxshm_HE = shm_addr_wk.HE_addr + p_into_table->shm_pos;

					if (p_htxshm_HE->cont_on_err == 1) {	/* COE flag set?               */
						p_htxshm_HE->cont_on_err = 0;	/* clear COE flag */
						sprintf(workstr, "Request to CLEAR \"%s\" continue on error flag issued by operator.", p_htxshm_HE->sdev_id);
					}
						
					else {	/* COE flag clear */
						p_htxshm_HE->cont_on_err = 1;	/* clear COE flag */
						sprintf(workstr, "Request to SET \"%s\" continue on error flag issued by operator.", p_htxshm_HE->sdev_id);
					}	/* endif */

					send_message(workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
					break;

				case KEY_F2:
					touchwin(stdscr);
					break;

				case KEY_F3:
					if (p_coe_table != NULL)  {
						free((char *) p_coe_table);	/* release mem for coe order table */
					}
					return (0);
	
				case -1:
					sprintf(workstr, "Error on getch() in AH_device.  errno = %d.", errno);
					PRTMSG(MSGLINE, 0, ("%s", workstr));
					send_message(workstr, errno, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
					clr_msg_flg = 1;

	
				default:	/* unsupported character            */
					PRTMSG(MSGLINE, 0, ("Sorry, that key is not valid with this screen."));
					clr_msg_flg = 1;
			}	/* endswitch */
		}		/* endfor */
	}			/* endfor */
}				/* IT_device() */

#endif

/*****************************************************************************/
/*****  i n i t _ a h d _ t b l ( )  *****************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME  =    init_coe_tbl()                                        */
/*                                                                           */
/* DESCRIPTIVE NAME =  Initialize Activate/Halt Device Table                 */
/*                                                                           */
/* FUNCTION =          Initializes and sorts the activate/halt device table  */
/*                                                                           */
/* INPUT =             p_coe_tbl_ptr: pointer to a pointer which will (after */
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

/*
 * struct ahd_entry **p_coe_tbl_ptr; ==> ptr to coe table pointer
 */

/*
 * This function is also found in COE_devic.c, and does almost the same stuff.
 * - Ashutosh S. Rajekar (6th Sept. 2000).
 */

static	int	init_coe_tbl(struct ahd_entry **p_coe_tbl_ptr)
{
	char workstr[128];	/* work string                       */
	extern int AHD_compar(void);	/* function to compare coe_entry's   */
	extern union shm_pointers shm_addr; /* shared memory union pointers */

	int	i;			/* loop counter                      */
	int	j;			/* loop counter                      */
	int	num_entries;	/* local number of entries           */

	union shm_pointers shm_addr_wk;	/* work ptr into shm                 */

#ifndef	__HTX_LINUX__
	void	qsort();		/* system sort function */
#endif

	/*
	 * allocate space for coe order table
	 */
	if (*p_coe_tbl_ptr != NULL)  {
		free((char *) *p_coe_tbl_ptr);
	}

	if ((num_entries = (shm_addr.hdr_addr)->num_entries) == 0) {
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
	(shm_addr_wk.hdr_addr)++;		/* skip over header */

	i = 0;
	for (j = 0; j < num_entries; j++) {
		if ((shm_addr_wk.HE_addr + i)->sdev_id[0] == '\0')  {	/* blank entry?    */
			j--;
		}

		else {
			(*p_coe_tbl_ptr + j)->shm_pos = i;
			(*p_coe_tbl_ptr + j)->slot = (shm_addr_wk.HE_addr + i)->slot;
			(*p_coe_tbl_ptr + j)->port = (shm_addr_wk.HE_addr + i)->port;
			htx_strcpy(((*p_coe_tbl_ptr + j)->sdev_id), ((shm_addr_wk.HE_addr + i)->sdev_id));
		}		/* endif */

		i++;
	}			/* endfor */

#ifdef	__HTX_LINUX__
#include <stdlib.h>
	qsort((void *) *p_coe_tbl_ptr, (size_t) num_entries, (size_t) sizeof(struct ahd_entry), (int (*)(const void *, const void *)) AHD_compar);
#else
	qsort((char *) *p_coe_tbl_ptr, (unsigned int) num_entries, sizeof(struct ahd_entry), AHD_compar);
#endif
	
	return (num_entries);
}				/* init_coe_tbl() */

