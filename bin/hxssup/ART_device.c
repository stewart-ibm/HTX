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

/* @(#)35	1.2.4.1  src/htx/usr/lpp/htx/bin/hxssup/ART_device.c, htx_sup, htxubuntu 3/20/13 03:54:53 */

/*****************************************************************************/
/*                                                                           */
/* MODULE NAME    =    ART_device.c                                          */
/* COMPONENT NAME =    hxssup (supervisor)                                   */
/* LPP NAME       =    HTX                                                   */
/*                                                                           */
/* DESCRIPTIVE NAME =  Display Screen Functions                              */
/*                                                                           */
/* COPYRIGHT =         (C) COPYRIGHT IBM CORPORATION 1999                    */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM           */
/*                                                                           */
/* STATUS =            Release 1 Version 0                                   */
/*                                                                           */
/* FUNCTION =          Presents Add/Restart/Terminate Exerciser menu and     */
/*                     handles top level processing for all operations.      */
/*                                                                           */
/* CHANGE ACTIVITY =                                                         */
/*    DATE    :LEVEL:PROGRAMMER:DESCRIPTION                                  */
/*    MMMMMMMMNMMMMMNMMMMMMMMMMNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM */
/*    11/29/99:1.0  :R GEBHARDT:INITIAL RELEASE                              */
/*            :     :          :                                             */
/*                                                                           */
/*****************************************************************************/

#include "hxssup.h"

#define NUM_COLS	(81)
#define NUM_ROWS	(23)

#define ARTD_IN_ROW	(19)
#define ARTD_IN_COL	(52)

/*
 * restrict characters in the name to eliminate conflicts with
 */
/*
 * shell special characters.
 * */

#define AD_NAME_GOOD_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@%^,.0123456789+=-_"
#define AD_METHOD_ROW	(5)
#define AD_METHOD_COL	(37)
#define AD_NAME_ROW		(6)
#define AD_NAME_COL		(51)
#define AD_STARTNOW_ROW	(7)
#define AD_STARTNOW_COL	(51)
#define AD_FORCEREDEF_ROW	(8)
#define AD_FORCEREDEF_COL	(55)
#define AD_NAME_SIZE 	(26)

extern int COLS;			/* number of cols in CURSES screen   */
extern int LINES;			/* number of lines in CURSES screen  */
extern WINDOW *stdscr;			/* standard screen window       */

extern union shm_pointers shm_addr;	/* shared memory union pointers      */

extern	void	A_device(void);	/* Add New Device menu */
extern	int	R_device(void);	/* Restart */
extern	int	T_device(void);


/*****************************************************************************/
/*****  A D _ s c n   ********************************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     AD_scn                                                */
/*                                                                           */
/* DESCRIPTIVE NAME =  Add Device Screen.                                    */
/*                                                                           */
/* FUNCTION =          Displays Add New Device Exercisers screen and gets    */
/*                     user input.                                           */
/*                                                                           */
/* INPUT =             Device name or name of MDT attribute file to use      */
/*                     for device definition.  Also flags to override        */
/*                     start_halted and to force redefinition of an existing */
/*                     device.                                               */
/*                                                                           */
/* OUTPUT =            Updated semaphores and shared memory.                 */
/*                                                                           */
/* NORMAL RETURN =     0 - OK                                                */
/*                     1 - operator quit menu.                               */
/*                                                                           */
/* ERROR RETURNS =     None.                                                 */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*                                                                           */
/*    OTHER ROUTINES = various curses functions.                             */
/*                     get_string - To get the name of the device or MDT     */
/*                                  attribute file from the menu.            */
/*                     disp_dst() - display device status                    */
/*                     edit() - start vi editor to edit mdt file.            */
/*    DATA AREAS =                                                           */
/*                     LINES, COLS - from curses lib.                        */
/*                     stdscr - from curses lib.                             */
/*                                                                           */
/*    MACROS =         PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                                                                           */
/*****************************************************************************/

int	AD_scn(char menu_add_name[],	/* I/O  device or file name             */
		enum t_add_method *addMethod,	/* I/O  method to generate MDT  */
		enum t_start_now *startNow,	/* I/O for start_halted attribute  */
		boolean_t * forceRedef,	/* I/O  Overwrite existing dev          */
		char op_info_msg[])	/* I    operator message                    */
{
		boolean_t	refresh_screen = TRUE;
		char	in_opt[AD_NAME_SIZE];
		static char *menu_options[5] = { "No", "Yes", "File", "Name", "Default" };
		char	workstr[128];	/* work string area.        */
		static	int col = 0;	/* column number                     */
		static	int row = 0;	/* row number                        */
		int		rc;

		/*
		 * init menu items and handle defaults
		 */
		if (*addMethod == AD_Method_Default)  {
				*addMethod = AD_Method_Name;
			}

		for (;;) {
				if (refresh_screen) {
						clear();
						display_scn(stdscr, 0, 0, LINES, COLS, "AD_scn", 1, &row, &col, NUM_ROWS, NUM_COLS, 'n');
						/*
						 * fill in defaults
						 */

#ifdef	__HTX_LINUX__
						wcolorout(stdscr, CYAN_BLACK | NORMAL | BOLD | UNDERSCORE);
#else
						wcolorout(stdscr, NORMAL | F_CYAN | B_BLACK | BOLD | UNDERSCORE);
#endif
						wmove(stdscr, AD_METHOD_ROW, AD_METHOD_COL);
						mvwaddstr(stdscr, AD_METHOD_ROW, AD_METHOD_COL,
						menu_options[*addMethod]);

						wmove(stdscr, AD_STARTNOW_ROW, AD_STARTNOW_COL);
						mvwaddstr(stdscr, AD_STARTNOW_ROW, AD_STARTNOW_COL,
						menu_options[*startNow]);

						wmove(stdscr, AD_FORCEREDEF_ROW, AD_FORCEREDEF_COL);
						mvwaddstr(stdscr, AD_FORCEREDEF_ROW, AD_FORCEREDEF_COL,
						menu_options[*forceRedef]);

						wmove(stdscr, AD_NAME_ROW, AD_NAME_COL);
						mvwaddstr(stdscr, AD_NAME_ROW, AD_NAME_COL, "                         ");
						mvwaddstr(stdscr, AD_NAME_ROW, AD_NAME_COL, menu_add_name);

						/*
						 * Restore to normal text
						 */
#ifdef	__HTX_LINUX__
						wcolorout(stdscr, BLACK_WHITE | NORMAL);
#else
						wcolorout(stdscr, NORMAL | B_BLACK | F_WHITE);
#endif

						if (op_info_msg == NULL)  {
								CLRLIN(MSGLINE, 0);
						}
						
						else {
								PRTMSG(MSGLINE, 0, (op_info_msg));
								op_info_msg = NULL;	/* only put it up once */
						}

						refresh_screen = FALSE;
				}

				/* if */
				htx_strncpy(in_opt, "", DIM(in_opt));	/* clear input */
				get_string(stdscr, ARTD_IN_ROW, ARTD_IN_COL, in_opt, (size_t) (2), "123456789dDqQS", (tbool) TRUE);

#ifdef	__HTX_LINUX__
				wcolorout(stdscr, CYAN_BLACK | NORMAL | BOLD | UNDERSCORE);
#else
				wcolorout(stdscr, NORMAL | F_CYAN | B_BLACK | BOLD | UNDERSCORE);
#endif
				switch (in_opt[0]) {
						case '1':	/* Specify New Device by XXXX */
								*addMethod =  *addMethod == AD_Method_Name ? AD_Method_File : AD_Method_Name;
								mvwaddstr(stdscr, AD_METHOD_ROW, AD_METHOD_COL, menu_options[*addMethod]);
								break;

						case '2':		/*AIX Device or MDT Stanza File Name */
								rc = get_string(stdscr, AD_NAME_ROW, AD_NAME_COL, menu_add_name, (size_t) AD_NAME_SIZE, AD_NAME_GOOD_CHARS, (tbool) TRUE);
							mvwaddstr(stdscr, AD_NAME_ROW, AD_NAME_COL, menu_add_name);
							break;

						case '3':	/* Start Device Exerciser Immediately */
#ifdef	__HTX_LINUX__
							wcolorout(stdscr, BLUE_WHITE | NORMAL);
#else
							wcolorout(stdscr, NORMAL | B_BLUE | F_WHITE);
#endif
							mvwaddstr(stdscr, AD_STARTNOW_ROW, AD_STARTNOW_COL, "       ");
							if (*startNow == AD_Start_Active)  {
									*startNow = AD_Start_Halted;
							}

							else if (*startNow == AD_Start_Halted)  {
									*startNow = AD_Start_Default;
							}

							else  {
									*startNow = AD_Start_Active;
							}

#ifdef	__HTX_LINUX__
							wcolorout(stdscr, CYAN_BLACK | NORMAL | BOLD | UNDERSCORE);
#else
							wcolorout(stdscr, NORMAL | F_CYAN | B_BLACK | BOLD | UNDERSCORE);
#endif
							mvwaddstr(stdscr, AD_STARTNOW_ROW, AD_STARTNOW_COL, menu_options[*startNow]);
							break;

						case '4':	/* Force Redefinition of Device Exerciser */
#ifdef	__HTX_LINUX__
							wcolorout(stdscr, BLUE_WHITE | NORMAL);
#else
							wcolorout(stdscr, NORMAL | B_BLUE | F_WHITE);
#endif
							mvwaddstr(stdscr, AD_FORCEREDEF_ROW, AD_FORCEREDEF_COL, "   ");
							*forceRedef = (*forceRedef == TRUE) ? FALSE : TRUE;

#ifdef	__HTX_LINUX__
							wcolorout(stdscr, CYAN_BLACK | NORMAL | BOLD | UNDERSCORE);
#else
							wcolorout(stdscr, NORMAL | F_CYAN | B_BLACK | BOLD | UNDERSCORE);
#endif
							mvwaddstr(stdscr, AD_FORCEREDEF_ROW, AD_FORCEREDEF_COL, menu_options[*forceRedef]);
							break;

						case '5':
							PRTMSG1(MSGLINE, 69, ("working..."));
							return (0);
							refresh_screen = TRUE;
							break;	/* add device   */

						case '6':	/* Display Device Status Table */
							(void) disp_dst();
							refresh_screen = TRUE;
							break;

						case '7':
						case '8':
						case '9':		/**** set standard files NOT to close on exec */
							fcntl(fileno(stdin), F_SETFD, 0);
							fcntl(fileno(stdout), F_SETFD, 0);
							fcntl(fileno(stderr), F_SETFD, 0);

							switch (in_opt[0]) {
									case '7':	/* Edit HTX Error Log */
											edit("/tmp/htxerr");
											break;

									case '8':	/* Edit HTX Message Log */
											edit("/tmp/htxmsg");
											break;

									case '9':	/* Edit MDT file */

											if (htx_strlen(menu_add_name))  {
												htx_strcpy(workstr, "../mdt/");
												htx_strcat(workstr, menu_add_name);
												edit(workstr);
											} /* if */
											else {
													op_info_msg = htx_strcpy(workstr, "Use option 2 to enter the MDT stana file name.");
											}	/* else */
											break;
							}	/* switch */

							/****** set standard files to close on exec ******/
							fcntl(fileno(stdin), F_SETFD, 1);
							fcntl(fileno(stdout), F_SETFD, 1);
							fcntl(fileno(stderr), F_SETFD, 1);
							refresh_screen = TRUE;
							break;

						case 'q':
						case 'Q':
							return (1);
							break;

						case 'd':
						case 'D':
							refresh_screen = TRUE;
							break;

						default:
							break;
				}		/* switch */
		}			/* for ever */
}				/* AD_scn */


/*****************************************************************************/
/*****  A R T _ d e v i c e  *************************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     ART_device()                                          */
/*                                                                           */
/* DESCRIPTIVE NAME =  Add/Restart/Terminate device.                         */
/*                                                                           */
/* FUNCTION =          This function displays the Add/Restart/Terminate      */
/*                     device menu and processes the option chosen from      */
/*                     that screen.                                          */
/*                                                                           */
/* INPUT =             None.                                                 */
/*                                                                           */
/* OUTPUT =            Displays the ART screen and calls the appropriate     */
/*                     function based on the option chosen.                  */
/*                                                                           */
/* NORMAL RETURN =     None - void function.                                 */
/*                                                                           */
/* ERROR RETURNS =     None - void function.                                 */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = display_scn - the display screen function.            */
/*                     A_device - Add new device exercisers.                 */
/*                     R_device - Restart device exercisers.                 */
/*                     T_device - Terminate device exercisers.               */
/*                                                                           */
/*    DATA AREAS =     LINES - number of lines on CURSES screen.             */
/*                     COLS  - number of columns on CURSES screen.           */
/*                                                                           */
/*                     shm_addr - shared memory address.                     */
/*                     stdscr - standard CURSES screen.                      */
/*                                                                           */
/*****************************************************************************/


void	ART_device(void)
{
		char	input[32];		/* input string                      */
		char	msg_text[MAX_TEXT_MSG];
		int		rc;
		static int col = 0;	/* column number                     */
		static int row = 0;	/* row number                        */

		rc = 0;
		for (;;) {
				/*
				 * display fresh screen
				 */
				clear();
				refresh();
				display_scn(stdscr, 0, 0, LINES, COLS, "ARTD_scn", 1, &row, &col, NUM_ROWS, NUM_COLS, 'n');

				/*
				 * Check for bad return code from previous operation.
				 */

				if (rc != 0) {
						PRTMSG(MSGLINE, 0, ("%s\n", msg_text));
						send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
						rc = 0;
				}


				/*
				 * Read input from keyboard...
				 */

				htx_strncpy(input, "", DIM(input));	/* clear input */
				get_string(stdscr, ARTD_IN_ROW, ARTD_IN_COL, input, (size_t) (2), "123dDqQ", (tbool) TRUE);

				switch (input[0]) {
						case 'd':	/* refresh */
						case 'D':
								break;

						case 'q':	/* quit menu */
						case 'Q':
								return;

						case '1':
								A_device();	/* Add New Device menu */
								break;

						case '2':
								rc = R_device();	/* Restart */
								if (rc != 0)  {
										sprintf(msg_text, "INTERNAL HTX ERROR!: R_device() returned %d.", rc);
								}
								break;

						case '3':
								rc = T_device();	/* Terminate */
								if (rc != 0)  {
										sprintf(msg_text, "INTERNAL HTX ERROR!: T_device() returned %d.", rc);
								}
								break;

						default:
								beep();
								break;
				}		/* switch */
		}			/* for forever */

		return;
}				/* ART_device */


