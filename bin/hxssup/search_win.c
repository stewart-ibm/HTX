static char sccsid[] = "@(#)69	1.1  src/htx/usr/lpp/htx/bin/hxssup/search_win.c, htx_sup, htxubuntu 12/30/09 07:00:42";

/*****************************************************************************/
/*                                                                           */
/* MODULE NAME    =    search_win.c                                          */
/* COMPONENT NAME =    hxssup (supervisor)                                   */
/* LPP NAME       =    HTX                                                   */
/*                                                                           */
/* DESCRIPTIVE NAME =  Screen to search a device.                            */
/*                                                                           */
/* COPYRIGHT =         (C) COPYRIGHT IBM CORPORATION 1988                    */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM           */
/*                                                                           */
/* STATUS =            Release 1 Version 0                                   */
/*                                                                           */
/* FUNCTION =          Displays search screen.                               */
/*                                                                           */
/*****************************************************************************/

#include "hxssup.h"

/*****************************************************************************/
/*****  h e l p ( )  *********************************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     search_win()                                          */
/*                                                                           */
/* DESCRIPTIVE NAME =  Screen to search a device.                            */
/*                                                                           */
/* FUNCTION =          Displays search screen.                               */
/*                                                                           */
/* INPUT =             keyboard input                                        */
/*                                                                           */
/* OUTPUT =            Formatted help screens                                */
/*                                                                           */
/* NORMAL RETURN =     Length of string read in normal case.                 */
/*                     0  - When Esc is pressed.                             */
/*                     -1 -  When nothing is entered/error condition.        */
/*                                                                           */
/* ERROR RETURNS =     None (I'm an optimist)                                */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = display_scn() - displays screen data arrays           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

int search_win(int nw_numlines, int nw_numcols, int nw_firstline, int nw_firstcol,
	 char *scn_file, int max_page, char * exer_name)
{
	/*
	 ***  variable definitions  *************************************************
	 */

	char input[2];			/* keyboard input				*/

	int  key_input;			/* input key					*/
	int  page = 1;			/* main menu search page number	*/

	static int col = 0;		/* column value 				*/
	static int row = 0;		/* row value 					*/

	WINDOW * srchscr;		/* search screen window 		*/

	/*
	 ***  beginning of executable code  *****************************************
	 */
	/*
	 ***  define a CURSES window for the help screens  **************************
	 */
	srchscr = newwin(nw_numlines, nw_numcols, nw_firstline, nw_firstcol);

	/*
	 ***  display search screen  ********************************************
	 */
	(void) display_scn(srchscr, nw_firstline, nw_firstcol, nw_numlines,
					nw_numcols, scn_file, page, &row, &col, nw_numlines,
					nw_numcols + 1, 'n');

	(void) wmove(srchscr, 13, 28);

	/*
	 ***  process keyboard input  ***********************************************
	 */


	strncpy(input, "", DIM(input));	 /* clear input */

#if 0
	(void) get_string(stdscr, nw_numlines, nw_numcols, input,
		(size_t) DIM(input), NULL, (tbool) TRUE);
#endif

	key_input = 0;

	(void) wrefresh(srchscr);          /* refresh window */
	while ( (input[0]=getchar()) != ENTER_KEY && input[0] != ESC_KEY && key_input < 15) {
		*(exer_name+key_input) = input[0];
		*(exer_name+key_input+1) = '\0';

		(void) mvwaddstr(srchscr, 13, 28, exer_name);
		(void) wmove(srchscr, 13, (28 + key_input + 1));
		(void) wrefresh(srchscr);          /* refresh window */
		key_input++;
	}

	if( key_input == 0 && input[0] == ENTER_KEY ) {
		(void) delwin(srchscr);
		return -1;
	}

	if ( input[0] == ESC_KEY ) {
		(void) delwin(srchscr);
		return 0;
	}

	*(exer_name+key_input) = '\0';

	/* PRTMSG(MSGLINE, 0, ("%s", exer_name)); */
	(void) delwin(srchscr);

	return(key_input);

} /* search_win() */


