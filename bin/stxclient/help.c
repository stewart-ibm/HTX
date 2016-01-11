
/* @(#)57	1.4  src/htx/usr/lpp/htx/bin/stxclient/help.c, eserv_gui, htxubuntu 5/24/04 17:06:11 */

/*****************************************************************************/
/*                                                                           */
/* MODULE NAME    =    help.c                                                */
/* COMPONENT NAME =    hxssup (supervisor)                                   */
/* LPP NAME       =    HTX                                                   */
/*                                                                           */
/* DESCRIPTIVE NAME =  Help Screen Function                                  */
/*                                                                           */
/* COPYRIGHT =         (C) COPYRIGHT IBM CORPORATION 1988                    */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM           */
/*                                                                           */
/* STATUS =            Release 1 Version 0                                   */
/*                                                                           */
/* FUNCTION =          Displays help screens.                                */
/*                                                                           */
/* COMPILER OPTIONS =  -I/src/master/htx/common -g -Nn3000 -Nd4000 -Np1200   */
/*                     -Nt2000                                               */
/*                                                                           */
/* CHANGE ACTIVITY =                                                         */
/*    DATE    :LEVEL:PROGRAMMER:DESCRIPTION                                  */
/*    MMMMMMMMNMMMMMNMMMMMMMMMMNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM */
/*    06/27/88:1.0  :J BURGESS :INITIAL RELEASE                              */
/*    12/15/99:1.5  :R GEBHARDT:Cleanup for hotplug support.                 */
/*            :     :          :                                             */
/*                                                                           */
/*****************************************************************************/

#include "hxssup.h"

/*****************************************************************************/
/*****  h e l p ( )  *********************************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     help()                                                */
/*                                                                           */
/* DESCRIPTIVE NAME =  Help Screen Function                                  */
/*                                                                           */
/* FUNCTION =          Displays help screens.                                */
/*                                                                           */
/* INPUT =             keyboard input                                        */
/*                                                                           */
/* OUTPUT =            Formatted help screens                                */
/*                                                                           */
/* NORMAL RETURN =     0 to indicate succussful completion                   */
/*                                                                           */
/* ERROR RETURNS =     None (I'm an optimist)                                */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = display_scn() - displays screen data arrays           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

int	help(int nw_numlines, int nw_numcols, int nw_firstline, int nw_firstcol, char *scn_file, int max_page)
{
	char input[2];		/* keyboard input                   */
	int page = 1;		/* main menu help page number        */

	static int col = 0;	/* column value */
	static int row = 0;	/* row value */

	WINDOW *hlpscr;		/* help screen window */

	/*
	 * define a CURSES window for the help screens
	 */
	hlpscr = newwin(nw_numlines, nw_numcols, nw_firstline, nw_firstcol);

	/*
	 * display first help screen
	 */
	display_scn(hlpscr, nw_firstline, nw_firstcol, nw_numlines, nw_numcols, scn_file, page, &row, &col, nw_numlines, nw_numcols + 1, 'n');

	/*
	 * process keyboard input
	 */

	for (;;) {
		wrefresh(hlpscr);	/* refresh window */
		strncpy(input, "", DIM(input));	/* clear input */
		get_string(stdscr, nw_numlines, nw_numcols, input, (size_t) DIM(input), "bBfFrRqQ", (tbool) FALSE);

		switch (input[0]) {
			case 'b':
			case 'B':
				if (page > 1) {
					page--;
					display_scn(hlpscr, nw_firstline, nw_firstcol, nw_numlines, nw_numcols, scn_file, page, &row, &col, nw_numlines, nw_numcols + 1, 'n');
				}
				else  {
					beep();
				}
				break;

			case 'f':
			case 'F':
				if (page < max_page) {
					page++;
					display_scn(hlpscr, nw_firstline, nw_firstcol, nw_numlines, nw_numcols, scn_file, page, &row, &col, nw_numlines, nw_numcols + 1, 'n');
				}
				else  {
					beep();
				}
				break;

			case 'r':
			case 'R':
				wclear(hlpscr);
				wrefresh(hlpscr);
				display_scn(hlpscr, nw_firstline, nw_firstcol, nw_numlines, nw_numcols, scn_file, page, &row, &col, nw_numlines, nw_numcols + 1, 'n');
				break;

			case 'q':
			case 'Q':
				delwin(hlpscr);
				return (0);

			default:
				beep();
				break;
		}		/* endswitch */
	}			/* endfor */
}				/* ddhelp() */


