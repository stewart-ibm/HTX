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

/* @(#)46	1.4  src/htx/usr/lpp/htx/bin/stxclient/init_screen.c, eserv_gui, htxubuntu 5/24/04 17:06:48 */

#include "hxssup.h"

void	init_screen(void)
{
#ifndef	__HTX_LINUX__
	do_colors = TRUE;	/* reset colors on endwin()          */
#endif
	initscr();	/* standard CURSES init function     */
	crmode();	/* turn off canonical processing     */
	noecho();	/* turn off auto echoing             */
	nonl();		/* turn off NEWLINE mapping          */

#ifdef	__HTX_LINUX__
	keypad((WINDOW *)stdscr, TRUE);
#else
	keypad(TRUE);	/* turn on key mapping               */
#endif
	clear();		/* clear screen                      */
	refresh();	/* display screen                    */

	/*
	 * This is a serious hack. Since Linux uses the POSIX compliant 
	 * curses library, it does not provide the 'wcolorout()' function; 
	 * it also does not provide explicit foreground and background colors
	 * as LINUX does. So we have to set up each possible combination of
	 * foreground and background colors to facilitate the use through
	 * the wcolorout() function, which is actually a macros that expands
	 * the 'wattrset()' in the Linux port.
	 *
	 * Remember, all these combinations may not work, they have just been
	 * defined. It is up to the programmer to select proper foreground
	 * and background combinations.
	 * 
	 * Ashutosh S. Rajekar.
	 */
#ifdef	__HTX_LINUX__
	if(has_colors())  {
		start_color();
		init_pair(1, COLOR_BLACK, COLOR_BLACK);
		init_pair(2, COLOR_BLACK, COLOR_RED);
		init_pair(3, COLOR_BLACK, COLOR_GREEN);
		init_pair(4, COLOR_BLACK, COLOR_YELLOW);
		init_pair(5, COLOR_BLACK, COLOR_BLUE);
		init_pair(6, COLOR_BLACK, COLOR_MAGENTA);
		init_pair(7, COLOR_BLACK, COLOR_CYAN);
		init_pair(8, COLOR_BLACK, COLOR_WHITE);

		init_pair(9, COLOR_RED, COLOR_BLACK);
		init_pair(10, COLOR_RED, COLOR_RED);
		init_pair(11, COLOR_RED, COLOR_GREEN);
		init_pair(12, COLOR_RED, COLOR_YELLOW);
		init_pair(13, COLOR_RED, COLOR_BLUE);
		init_pair(14, COLOR_RED, COLOR_MAGENTA);
		init_pair(15, COLOR_RED, COLOR_CYAN);
		init_pair(16, COLOR_RED, COLOR_WHITE);

		init_pair(17, COLOR_GREEN, COLOR_BLACK);
		init_pair(18, COLOR_GREEN, COLOR_RED);
		init_pair(19, COLOR_GREEN, COLOR_GREEN);
		init_pair(20, COLOR_GREEN, COLOR_YELLOW);
		init_pair(21, COLOR_GREEN, COLOR_BLUE);
		init_pair(22, COLOR_GREEN, COLOR_MAGENTA);
		init_pair(23, COLOR_GREEN, COLOR_CYAN);
		init_pair(24, COLOR_GREEN, COLOR_WHITE);

		init_pair(25, COLOR_YELLOW, COLOR_BLACK);
		init_pair(26, COLOR_YELLOW, COLOR_RED);
		init_pair(27, COLOR_YELLOW, COLOR_GREEN);
		init_pair(28, COLOR_YELLOW, COLOR_YELLOW);
		init_pair(29, COLOR_YELLOW, COLOR_BLUE);
		init_pair(30, COLOR_YELLOW, COLOR_MAGENTA);
		init_pair(31, COLOR_YELLOW, COLOR_CYAN);
		init_pair(32, COLOR_YELLOW, COLOR_WHITE);

		init_pair(33, COLOR_BLUE, COLOR_BLACK);
		init_pair(34, COLOR_BLUE, COLOR_RED);
		init_pair(35, COLOR_BLUE, COLOR_GREEN);
		init_pair(36, COLOR_BLUE, COLOR_YELLOW);
		init_pair(37, COLOR_BLUE, COLOR_BLUE);
		init_pair(38, COLOR_BLUE, COLOR_MAGENTA);
		init_pair(39, COLOR_BLUE, COLOR_CYAN);
		init_pair(40, COLOR_BLUE, COLOR_WHITE);

		init_pair(41, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(42, COLOR_MAGENTA, COLOR_RED);
		init_pair(43, COLOR_MAGENTA, COLOR_GREEN);
		init_pair(44, COLOR_MAGENTA, COLOR_YELLOW);
		init_pair(45, COLOR_MAGENTA, COLOR_BLUE);
		init_pair(46, COLOR_MAGENTA, COLOR_MAGENTA);
		init_pair(47, COLOR_MAGENTA, COLOR_CYAN);
		init_pair(48, COLOR_MAGENTA, COLOR_WHITE);

		init_pair(49, COLOR_CYAN, COLOR_BLACK);
		init_pair(50, COLOR_CYAN, COLOR_RED);
		init_pair(51, COLOR_CYAN, COLOR_GREEN);
		init_pair(52, COLOR_CYAN, COLOR_YELLOW);
		init_pair(53, COLOR_CYAN, COLOR_BLUE);
		init_pair(54, COLOR_CYAN, COLOR_MAGENTA);
		init_pair(55, COLOR_CYAN, COLOR_CYAN);
		init_pair(56, COLOR_CYAN, COLOR_WHITE);

		init_pair(57, COLOR_WHITE, COLOR_BLACK);
		init_pair(58, COLOR_WHITE, COLOR_RED);
		init_pair(59, COLOR_WHITE, COLOR_GREEN);
		init_pair(60, COLOR_WHITE, COLOR_YELLOW);
		init_pair(61, COLOR_WHITE, COLOR_BLUE);
		init_pair(62, COLOR_WHITE, COLOR_MAGENTA);
		init_pair(63, COLOR_WHITE, COLOR_CYAN);
		init_pair(64, COLOR_WHITE, COLOR_WHITE);
	}
#endif
	
	return;
}				/* init_screen() */
