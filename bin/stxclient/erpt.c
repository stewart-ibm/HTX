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

/* @(#)58	1.4  src/htx/usr/lpp/htx/bin/stxclient/erpt.c, eserv_gui, htxubuntu 5/24/04 17:05:23 */

/*****************************************************************************/
/*                                                                           */
/* MODULE NAME    =    erpt.c                                                */
/* COMPONENT NAME =    hxssup (supervisor)                                   */
/* LPP NAME       =    HTX                                                   */
/*                                                                           */
/* DESCRIPTIVE NAME =  error report generator                                */
/*                                                                           */
/* COPYRIGHT =         (C) COPYRIGHT IBM CORPORATION 1992                    */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM           */
/*                                                                           */
/* STATUS =            Release 1 Version 0                                   */
/*                                                                           */
/* FUNCTION =          Invokes LINUX system error report command.            */
/*                                                                           */
/* COMPILER OPTIONS =  -I/src/master/htx/common -g -Nn3000 -Nd4000 -Np1200   */
/*                     -Nt2000                                               */
/*                                                                           */
/* CHANGE ACTIVITY =                                                         */
/*    DATE    :LEVEL:PROGRAMMER:DESCRIPTION                                  */
/*    MMMMMMMMNMMMMMNMMMMMMMMMMNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM */
/*    08/19/92:1.0  :J BURGESS :INITIAL RELEASE                              */
/*    11/18/99:1.4  :R GEBHARDT:Fix for feature 290676, added system_call=F  */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*                                                                           */
/*****************************************************************************/

#include "hxssup.h"


/*****************************************************************************/
/*****  e r p t ( )  *********************************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     erpt()                                                */
/*                                                                           */
/* DESCRIPTIVE NAME =  generates an LINUX error report.                        */
/*                                                                           */
/* FUNCTION =          Invokes LINUX errpt command.                            */
/*                                                                           */
/* INPUT =                                                                   */
/*                     HTX_ERPT environment variable -- specifies errpt      */
/*                         options and pager, e.g., "errpt -a | pg -s"       */
/*                                                                           */
/* OUTPUT =            Error report screen output.                           */
/*                                                                           */
/* NORMAL RETURN =     none - void function.                                 */
/*                                                                           */
/* ERROR RETURNS =     none - void function.                                 */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES =                                                       */
/*                                                                           */
/*    DATA AREAS =                                                           */
/*                                                                           */
/*    MACROS =         CLRLIN - Clear line CURSES macro                      */
/*                              (defined in common/hxiconv.h)                */
/*                     PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                                                                           */
/*****************************************************************************/

void	erpt(void)
{
	char htx_erpt[128];	/* error report string      */
	char workstr[256];	/* work string              */
	extern int system_call;	/* set to TRUE before system() */
	int system_rc;		/* system() call return code. */

#ifdef	__HTX_LINUX__
	saveterm();
#else
	csavetty(FALSE);	/* save current CURSES tty state. */
	resetty(TRUE);	/* reset original tty state and clear screen. */
#endif


	 /*
	  * load error report program
	  */

	 /*
	  * get $HTX_ERPT environment variable
	  */

	if (strlen(strcpy(htx_erpt, getenv("HTX_ERPT"))) == 0)  {
		strcpy(htx_erpt, "/bin/errpt -a | pg -s");
	}

	 system_call = TRUE;
	 if ((system_rc = system(htx_erpt)) == -1) {
		 sprintf(workstr, "Error on system(%s) call.  errno = %d.", htx_erpt, errno);
		 //send_message(workstr, errno, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
	 }			/* endif */

	 system_call = FALSE;

#ifdef	__HTX_LINUX__
	fixterm();
#else
	 resetty(TRUE);		/* reset orig tty state & clr scn */
	 cresetty(FALSE);	/* reset orig tty state & clr scr */
#endif
	 crmode();		/* turn off canonical processing  */
	 noecho();		/* turn off auto echoing */
	 nonl();		/* turn off NEWLINE mapping */

#ifdef	__HTX_LINUX__
	 keypad((WINDOW *) stdscr, TRUE);		/* turn on key mapping */
#else
	 keypad(TRUE);		/* turn on key mapping */
#endif
	 clear();		/* clear screen      */
	 refresh();		/* display screen              */

	return;
}				/* erpt() */


