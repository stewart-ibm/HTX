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

/* @(#)47	1.2.4.1  src/htx/usr/lpp/htx/bin/hxssup/help.c, htx_sup, htxubuntu 3/20/13 03:53:16 */

/*
 *   FUNCTIONS: help
 */


/*****************************************************************************/
/*                                                                           */
/* MODULE NAME    =    help.c                                                */
/* COMPONENT NAME =    hxssup (supervisor)                                   */
/* LPP NAME       =    HTX                                                   */
/*                                                                           */
/* DESCRIPTIVE NAME =  Help Screen Function                                  */
/*                                                                           */
/* FUNCTION =          Displays help screens.                                */
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
		htx_strncpy(input, "", DIM(input));	/* clear input */
		get_string(stdscr, nw_numlines, nw_numcols, input, (size_t) DIM(input), "bBfFdDqQ", (tbool) FALSE);

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

			case 'd':
			case 'D':
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


