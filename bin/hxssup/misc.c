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

/* @(#)51	1.4.4.8  src/htx/usr/lpp/htx/bin/hxssup/misc.c, htx_sup, htxubuntu 1/4/16 05:30:20 */

/*
 *   FUNCTIONS: AHD_compar
 *		DRT_compar
 *		DST_compar
 *		alarm_signal
 *		child_death
 *		get_string
 *		load_compar
 *		send_message
 *		unquote
 *
 *   ORIGINS: 27
 */


/*****************************************************************************/
/*                                                                           */
/* MODULE NAME    =    misc.c                                                */
/* COMPONENT NAME =    hxssup (supervisor)                                   */
/* LPP NAME       =    HTX                                                   */
/*                                                                           */
/* DESCRIPTIVE NAME =  Miscellaneous support functions                       */
/*                                                                           */
/* COPYRIGHT =         (C) COPYRIGHT IBM CORPORATION 1988                    */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM           */
/*                                                                           */
/* STATUS =            Release 1 Version 0                                   */
/*                                                                           */
/* FUNCTION =          Miscellaneous support functions                       */
/*            AHD_compar - compares Activate/Halt Table entries for system   */
/*                qsort function.                                            */
/*            alarm_signal - alarm signal handler.                           */
/*            child_death - handles the death of a child process.            */
/*            DRT_compar - compares Device Run Table entries for system      */
/*                qsort function.                                            */
/*            DST_compar - compares Device Status Table entries for system   */
/*                qsort function.                                            */
/*            get_string - gets a string from keyboard input.                */
/*            load_compare - compares load table entries for the system      */
/*                qsort function.                                            */
/*            send_message - sends message to the message handler program    */
/*            unquote - strips quote characters off of string variables      */
/*                                                                           */
/*                                                                           */
/* COMPILER OPTIONS =  -I/src/master/htx/common -g -Nn3000 -Nd4000 -Np1200   */
/*                     -Nt2000                                               */
/*                                                                           */
/* CHANGE ACTIVITY =                                                         */
/*    DATE    :LEVEL:PROGRAMMER:DESCRIPTION                                  */
/*    MMMMMMMMNMMMMMNMMMMMMMMMMNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM */
/*    06/14/88:1.0  :J BURGESS :INITIAL RELEASE                              */
/*    12/14/99:1.15 :R GEBHARDT:Feature 290676 Add/Terminate Exercisers      */
/*            :     :          :                                             */
/*                                                                           */
/*****************************************************************************/

#include "hxssup.h"


/*
 * Error code #define's for send_message()
 */
#define MSG_TOO_LONG 0x0001
#define BAD_GETTIMER 0x0002
#define BAD_MSGSND 0x0004
#define NO_MSG_QUEUE 0x0008


/*****************************************************************************/
/*****  A H D _ c o m p a r ( )  *********************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     AHD_compar()                                          */
/*                                                                           */
/* DESCRIPTIVE NAME =  Compare activate/halt device table entries            */
/*                                                                           */
/* FUNCTION =                                                                */
/*                                                                           */
/*    compare ahd_entry structures according to the following precedence:    */
/*                                                                           */
/*            (1) slot number;                                               */
/*            (2) port number;                                               */
/*            (3) /dev/???? id.                                              */
/*                                                                           */
/*    if (item1 < item2) return -1;                                          */
/*                                                                           */
/*    if (item1 == item2) return 0;                                          */
/*                                                                           */
/*    if (item1 > item2) return 1;                                           */
/*                                                                           */
/* INPUT =             item1 - pointer to the first table entry for the      */
/*                         compare                                           */
/*                     item2 - pointer to the second table entry for the     */
/*                         compare                                           */
/*                                                                           */
/* OUTPUT =            Nothing except the return codes listed below.         */
/*                                                                           */
/* NORMAL RETURN =     if (item1 < item2) return -1;                         */
/*                                                                           */
/*                     if (item1 == item2) return 0;                         */
/*                                                                           */
/*                     if (item1 > item2) return 1;                          */
/*                                                                           */
/*                                                                           */
/* ERROR RETURNS =     None                                                  */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

int	AHD_compar(struct ahd_entry *item1, struct ahd_entry *item2)
{
	char alpha1[DEV_ID_MAX_LENGTH];	/* alpha part of item1->sdev_id      */
	char alpha2[DEV_ID_MAX_LENGTH];	/* alpha part of item2->sdev_id      */
	char *p_num1;		/* pts to num part of item1->sdev_id */
	char *p_num2;		/* pts to num part of item2->sdev_id */

	int num1;		/* int value for item1 num part      */
	int num2;		/* int value for item2 num part      */
	int result;		/* for result of htx_strcmp()            */

	if (item1->slot < item2->slot)  {
		return (-1);
	}
	if (item1->slot > item2->slot)  {
		return (1);
	}

	/*
	 * If here, slot numbers must be equal.
	 */
	if (item1->port < item2->port)  {
		return (-1);
	}
	if (item1->port > item2->port)  {
		return (1);
	}

	/*
	 * If here, port numbers must be equal.
	 */
	/*
	 * put alpha portion of sdev_id's in work strings
	 */

	p_num1 = strpbrk(item1->sdev_id, "0123456789");
	p_num2 = strpbrk(item2->sdev_id, "0123456789");

	if (p_num1 != NULL) {	/* numeric part exists?              */
		htx_strncpy(alpha1, item1->sdev_id, (size_t) (p_num1 - item1->sdev_id));
		alpha1[p_num1 - item1->sdev_id] = '\0';	/* concatonate null char      */

	}

	else {		/* all alpha                         */
		htx_strcpy(alpha1, item1->sdev_id);
	}			/* endif */

	if (p_num2 != NULL) {	/* numeric part exists?              */
		htx_strncpy(alpha2, item2->sdev_id, (size_t) (p_num2 - item2->sdev_id));
		alpha2[p_num2 - item2->sdev_id] = '\0';	/* concatonate null char      */
	}

	else {		/* all alpha                         */
		 htx_strcpy(alpha2, item2->sdev_id);
	}			/* endif */

	if ((result = htx_strcmp(alpha1, alpha2)) != 0)  {	/* alpha parts not equal?   */
		return (result);
	}

	else {			/* alpha parts =, look at numeric    */
		if (p_num1 == NULL) {	/* numeric portion not exist?      */
			num1 = 0;	/* set num to zero            */
		}

		else {	/* numeric portion exists          */
			num1 = atoi(p_num1);	/* convert to integer value   */
		}		/* endif */

		if (p_num2 == NULL) {	/* numeric portion not exist?      */
			num2 = 0;	/* set num to zero            */
		}

		else {	/* numeric portion exists          */
			num2 = atoi(p_num2);	/* convert to integer value   */
		}		/* endif */

		if (num1 == num2) {
			return (0);
		}

		else {
			return ((num1 - num2) / (abs(num1 - num2)));
		}		/* endif */

	}			/* endif */
}				/* AHD_compar() */


/*****************************************************************************/
/*****  a l a r m _ s i g n a l ( )  *****************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     alarm_signal()                                        */
/*                                                                           */
/* DESCRIPTIVE NAME =  alarm signal handler                                  */
/*                                                                           */
/* FUNCTION =          Upon reciept of an alarm signal, this function sets   */
/*                     the external variable alarm_sig to TRUE.              */
/*                                                                           */
/* INPUT =             None.                                                 */
/*                                                                           */
/* OUTPUT =            The alarm_sig variable (set to TRUE).                 */
/*                                                                           */
/* NORMAL RETURN =     None - void function.                                 */
/*                                                                           */
/* ERROR RETURNS =     None - void function.                                 */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

void alarm_signal(int sig, int code, struct sigcontext *scp)
{
	extern int alarm_sig;	/* set to TRUE on receipt of SIGALRM */

	alarm_sig = TRUE;

	return;			/* exit signal handler               */

}				/* alarm_signal() */


/*****************************************************************************/
/*****  c h i l d _ d e a t h ( )  *******************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     child_death()                                         */
/*                                                                           */
/* DESCRIPTIVE NAME =  SIGCLD signal (death of a child process) handler      */
/*                                                                           */
/* FUNCTION =          Upon receipt of the SIGCLD signal, this function will */
/*                     obtain information on how the child process died, and */
/*                     send the appropriate messages to the message handler  */
/*                     program.                                              */
/*                                                                           */
/* INPUT =             SIGCLD signal                                         */
/*                                                                           */
/* OUTPUT =            Explanatory messages to the message handler           */
/*                                                                           */
/* NORMAL RETURN =     None - void function.                                 */
/*                                                                           */
/* ERROR RETURNS =     None - void function.                                 */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = send_message - outputs a message to the screen and    */
/*                                    the message log file                   */
/*                                                                           */
/*    DATA AREAS =     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                                                                           */
/*    MACROS =         PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                                                                           */
/*****************************************************************************/

#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

void	child_death(int sig, int code, struct sigcontext *scp)
{
	char workstr[81];	/* work string for messages          */
	char tag[81];		/* tag on to work string             */

	extern int editor_PID;	/* editor process id (set only if    */
	/* no virtual support)               */
	extern int hxsmsg_PID;	/* message handler process id        */
	extern int hxstats_PID;	/* statistics program process id     */
	extern int msgqid;	/* message handler message queue id  */
	extern int shell_PID;	/* shell process id                  */
	extern  pid_t equaliser_PID;
	extern pid_t hotplug_monitor_ID;
	extern volatile int system_call;	/* no wait3 info from system() flag  */
    extern int autostart;   /* autostart flag  */

	extern union shm_pointers shm_addr; /* shared memory address */

	int child_PID;		/* process id of terminated child    */
	int i;			/* loop counter                      */
	int status;		/* status from terminated child      */

	union shm_pointers shm_ptrs;	/* pointer into shared memory        */


	while( (child_PID  = waitpid(-1, &status, WNOHANG) ) ) {
		if ( child_PID == -1) {		/* error on wait3() system call */
			if (system_call == FALSE) {  /* skipping error for system() */
				sprintf(workstr, "SIGCHLD signal received by hxssup.  wait3() failed.  errno = %d.", errno);
				PRTMSG(MSGLINE, 0, ("%s", workstr));
				send_message(workstr, errno, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
			}
			break;
		}			/* endif */
		if ( WIFEXITED(status) )  {	/* terminated by exit() call? */
			sprintf(tag, "exit(%d) call.", WEXITSTATUS(status) );
		}

		else if ( WIFSIGNALED(status) )  {
			 sprintf(tag, "signal %d.", WTERMSIG(status) );
		}

		else if (child_PID == hxsmsg_PID) {
			hxsmsg_PID = 0;
			msgctl(msgqid, IPC_RMID, (struct msqid_ds *) NULL);
			msgqid = -1;
			if (editor_PID == 0) {	/* no editor running on this term? */
				PRTMSG(MSGLINE, 0, ("hxsmsg program terminated by %s", tag));
			}	/* endif */
		}

		if (child_PID == editor_PID)  {
			editor_PID = 0;
		}

		else if (child_PID == shell_PID)  {
			shell_PID = 0;
		}

		else if (child_PID == hxstats_PID)  {
			hxstats_PID = 0;
		}

		else if (child_PID == equaliser_PID)  {
			equaliser_PID = 0;
		}

		else if (child_PID == hotplug_monitor_ID)  {
			hotplug_monitor_ID = 0;
		}

		else {
			shm_ptrs.hdr_addr = shm_addr.hdr_addr + 1;	/* skip over header    */
			for (i = 0; ((i < (shm_addr.hdr_addr)->max_entries) && (((shm_ptrs.HE_addr + i)->PID) != child_PID)); i++)
				;

			if (i < (shm_addr.hdr_addr)->max_entries) {	/* found it?                       */
				sprintf(workstr, "%s HE program for %s terminated by %s", (shm_ptrs.HE_addr + i)->HE_name, (shm_ptrs.HE_addr + i)->sdev_id, tag);
				if (((shm_ptrs.HE_addr + i)->user_term) || ((shm_ptrs.HE_addr + i)->DR_term == 1) || (autostart == 1)) {	/* terminated by user request? */
					send_message(workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
				}

				else {
					send_message(workstr, 0, HTX_HE_HARD_ERROR, HTX_SYS_MSG);
				}	/* endif */

				((shm_addr.hdr_addr)->num_entries)--;
				(shm_ptrs.HE_addr + i)->PID = 0;	/* set PID to zero */

				if (editor_PID == 0) {
					PRTMSG(MSGLINE, 0, ("%s", workstr));
				}	/* endif */
			}	/* endif */
		}	/* endif */
	}	/* while */


	return;			/* exit signal handler               */
}				/* child_death() */


/*****************************************************************************/
/*****  D R T _ c o m p a r ( )  *********************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     DRT_compar()                                          */
/*                                                                           */
/* DESCRIPTIVE NAME =  Compare Device Run Table entries                      */
/*                                                                           */
/* FUNCTION =                                                                */
/*                                                                           */
/*    compare drt_entry structures according to the following precedence:    */
/*                                                                           */
/*            (1) slot number;                                               */
/*            (2) port number;                                               */
/*            (3) /dev/???? id.                                              */
/*                                                                           */
/*    if (item1 < item2) return -1;                                          */
/*                                                                           */
/*    if (item1 == item2) return 0;                                          */
/*                                                                           */
/*    if (item1 > item2) return 1;                                           */
/*                                                                           */
/* INPUT =             item1 - pointer to the first table entry for the      */
/*                         compare                                           */
/*                     item2 - pointer to the second table entry for the     */
/*                         compare                                           */
/*                                                                           */
/* OUTPUT =            Nothing except the return codes listed below.         */
/*                                                                           */
/* NORMAL RETURN =     if (item1 < item2) return -1;                         */
/*                                                                           */
/*                     if (item1 == item2) return 0;                         */
/*                                                                           */
/*                     if (item1 > item2) return 1;                          */
/*                                                                           */
/*                                                                           */
/* ERROR RETURNS =     None                                                  */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

int	DRT_compar(struct htxshm_HE *item1, struct htxshm_HE *item2)
{
	char alpha1[DEV_ID_MAX_LENGTH];	/* alpha part of item1->sdev_id      */
	char alpha2[DEV_ID_MAX_LENGTH];	/* alpha part of item2->sdev_id      */
	char *p_num1;		/* pts to num part of item1->sdev_id */
	char *p_num2;		/* pts to num part of item2->sdev_id */

	int num1;		/* int value for item1 num part      */
	int num2;		/* int value for item2 num part      */
	int result;		/* for result of htx_strcmp()            */

	if (item1->slot < item2->slot)  {
		return (-1);
	}
	if (item1->slot > item2->slot)  {
		return (1);
	}

	/*
	 * If here, slot numbers must be equal.
	 */
	if (item1->port < item2->port)  {
		return (-1);
	}
	if (item1->port > item2->port)  {
		return (1);
	}

	/*
	 * If here, port numbers must be equal.
	 */
	/*
	 * put alpha portion of sdev_id's in work strings
	 */
	p_num1 = strpbrk(item1->sdev_id, "0123456789");
	p_num2 = strpbrk(item2->sdev_id, "0123456789");

	if (p_num1 != NULL) {	/* numeric part exists?              */
		 htx_strncpy(alpha1, item1->sdev_id, (size_t) (p_num1 - item1->sdev_id));
		 alpha1[p_num1 - item1->sdev_id] = '\0';	/* concatonate null char      */
	}

	else {		/* all alpha                         */
		 htx_strcpy(alpha1, item1->sdev_id);
	}			/* endif */

	if (p_num2 != NULL) {	/* numeric part exists?              */
		htx_strncpy(alpha2, item2->sdev_id, (size_t) (p_num2 - item2->sdev_id));
		alpha2[p_num2 - item2->sdev_id] = '\0';	/* concatonate null char    */
	}

	else {		/* all alpha                         */
		 htx_strcpy(alpha2, item2->sdev_id);
	}			/* endif */

	if ((result = htx_strcmp(alpha1, alpha2)) != 0)  {	/* alpha parts not equal?   */
		return (result);
	}

	else {			/* alpha parts =, look at numeric    */
		if (p_num1 == NULL) {	/* numeric portion not exist?      */
			num1 = 0;	/* set num to zero            */
		}

		else {	/* numeric portion exists          */
			num1 = atoi(p_num1);	/* convert to integer value   */
		}		/* endif */

		if (p_num2 == NULL) {	/* numeric portion not exist?      */
			num2 = 0;	/* set num to zero            */
		}

		else {	/* numeric portion exists          */
			num2 = atoi(p_num2);	/* convert to integer value   */
		}		/* endif */

		if (num1 == num2) {
			return (0);
		}

		else {
			return ((num1 - num2) / (abs(num1 - num2)));
		}		/* endif */

	}			/* endif */
}				/* DRT_compar() */


/*****************************************************************************/
/*****  D S T _ c o m p a r ( )  *********************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     DST_compar()                                          */
/*                                                                           */
/* DESCRIPTIVE NAME =  Compare Device Status Table entries                   */
/*                                                                           */
/* FUNCTION =                                                                */
/*                                                                           */
/*    compare dst_entry structures according to the following precedence:    */
/*                                                                           */
/*            (1) /dev/???? id.                                              */
/*                                                                           */
/*    if (item1 < item2) return -1;                                          */
/*                                                                           */
/*    if (item1 == item2) return 0;                                          */
/*                                                                           */
/*    if (item1 > item2) return 1;                                           */
/*                                                                           */
/* INPUT =             item1 - pointer to the first table entry for the      */
/*                         compare                                           */
/*                     item2 - pointer to the second table entry for the     */
/*                         compare                                           */
/*                                                                           */
/* OUTPUT =            Nothing except the return codes listed below.         */
/*                                                                           */
/* NORMAL RETURN =     if (item1 < item2) return -1;                         */
/*                                                                           */
/*                     if (item1 == item2) return 0;                         */
/*                                                                           */
/*                     if (item1 > item2) return 1;                          */
/*                                                                           */
/*                                                                           */
/* ERROR RETURNS =     None                                                  */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

int DST_compar(struct dst_entry *item1, struct dst_entry *item2)
{
	char alpha1[DEV_ID_MAX_LENGTH];	/* alpha part of item1->sdev_id      */
	char alpha2[DEV_ID_MAX_LENGTH];	/* alpha part of item2->sdev_id      */
	char *p_num1;		/* pts to num part of item1->sdev_id */
	char *p_num2;		/* pts to num part of item2->sdev_id */

	int num1;		/* int value for item1 num part      */
	int num2;		/* int value for item2 num part      */
	int result;		/* for result of htx_strcmp()            */

	/*
	 * put alpha portion of sdev_id's in work strings
	 */
	p_num1 = strpbrk(item1->sdev_id, "0123456789");
	p_num2 = strpbrk(item2->sdev_id, "0123456789");

	if (p_num1 != NULL) {	/* numeric part exists?              */
		htx_strncpy(alpha1, item1->sdev_id, (size_t) (p_num1 - item1->sdev_id));
		alpha1[p_num1 - item1->sdev_id] = '\0';	/* concatonate null char     */
	}

	else {		/* all alpha                         */
		 htx_strcpy(alpha1, item1->sdev_id);
	}			/* endif */

	if (p_num2 != NULL) {	/* numeric part exists?              */
		htx_strncpy(alpha2, item2->sdev_id, (size_t) (p_num2 - item2->sdev_id));
		alpha2[p_num2 - item2->sdev_id] = '\0';	/* concatonate null char     */
	}

	else {		/* all alpha                         */
		 htx_strcpy(alpha2, item2->sdev_id);
	}			/* endif */

	if ((result = htx_strcmp(alpha1, alpha2)) != 0)  {	/* alpha parts not equal?   */
		return (result);
	}

	else {			/* alpha parts =, look at numeric    */
		if (p_num1 == NULL) {	/* numeric portion not exist?      */
			num1 = 0;	/* set num to zero            */
		}

		else {	/* numeric portion exists          */
			num1 = atoi(p_num1);	/* convert to integer value   */
		}		/* endif */

		if (p_num2 == NULL) {	/* numeric portion not exist?      */
			num2 = 0;	/* set num to zero            */
		}

		else {	/* numeric portion exists          */
			num2 = atoi(p_num2);	/* convert to integer value   */
		}		/* endif */

		if (num1 == num2) {
			return (0);
		}

		else {
			return ((num1 - num2) / (abs(num1 - num2)));
		}		/* endif */

	}			/* endif */

}				/* DST_compar() */


/*
 * NAME: get_string()
 *
 * FUNCTION: Gets a string from operator keyboard input.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This function is called by other functions in the hxssup program
 *      which need to get a string via operator keyboard input.
 *
 * RETURNS:
 *
 *      Return Codes:
 *      ------------------------------------------------------------------
 *        0 -- Normal exit.
 *       -1 -- Error on wgetch().
 *
 */

 /*
  * curses_win -- Pointer to window structure where the operator will
  *              enter keyboard input.
  * input_row -- The row in the curses window on which the data string
  *              is entered.
  * input_col -- The column in the curses window on which the data string
  *              is entered.
  * input_str -- Pointer to the input string where operator input is to
  *              be stored.
  * max_str_len -- The maximum allowed length of the input string
  *              including the terminating null character.
  * good_chars -- Pointer to a string of allowed input characters.  This
  *              string includes only printable alphanumeric characters.
  * echo_flag -- If TRUE, print input characters.
*/
int get_string(WINDOW * curses_win, int input_row, int input_col,
	       char *input_str, size_t max_str_len, char *good_chars,
	       tbool echo_flag)
{
	int end_index;		/* index to last char in input_str */
	int i;			/* loop index */
	int input_index;	/* input index */
	int key_input;		/* character entered */
	tbool insert_flag;	/* Insert mode flag */

	/*
	 * Beginning of Executable Code
	 */
	insert_flag = FALSE;
	input_index = 0;
	end_index = htx_strlen(input_str) - 1;

	/* htx_strncpy(input_str, "", max_str_len); */
	wmove(curses_win, input_row, input_col);
	wrefresh(curses_win);

	for (;;) {
		/*
		 * Get the next input character -- don't get fooled by a signal...
		 */
		for (; (((key_input = wgetch(curses_win)) == -1) && (errno == EINTR)); )
			;	/* get next character */

		/*
		 * Handle the new character...
		 */
		switch (key_input) {

			case KEY_NEWL:	/* end of string? */
				wmove(curses_win, input_row, input_col);
				for (i = 0; i < (end_index + 1); i++)  {
					waddch(curses_win, ' ');
				}

				wmove(curses_win, input_row, input_col);
				wrefresh(curses_win);
				return (0);

			case -1:
				return (-1);

			case KEY_BACKSPACE:
				if (input_index > 0)  {
					input_index--;
				}
				else if (end_index < 0) {
					beep();
					break;
				}	/* endif */

			case KEY_DC:
				if (end_index >= input_index) {
					end_index--;
					if (end_index >= 0)  {
						htx_strcpy((input_str + input_index), (input_str + input_index + 1));
					}

					*(input_str + end_index + 1) = '\0';
				}

				else  {
					beep();
				}

				if (echo_flag == TRUE) {
					mvwaddch(curses_win, input_row,	(input_col + end_index + 1), ' ');
				}	/* endif */
				break;

			case KEY_LEFT:
				if (input_index > 0)  {
					input_index--;
				}
				else  {
					beep();
				}
				break;

			case KEY_RIGHT:
				if (input_index < (end_index + 1))  {
					input_index++;
				}
				else  {
					beep();
				}
				break;

			case KEY_HOME:
				input_index = 0;
				break;

			case KEY_END:
				input_index = end_index;
				break;

			case KEY_IC:
				if (insert_flag == TRUE)  {
					insert_flag = FALSE;
				}
				else  {
					insert_flag = TRUE;
				}
				break;

			case KEY_EIC:
				insert_flag = FALSE;
				break;

#ifndef	__HTX_LINUX__
			case KEY_SLL:	 /* Locator Select (mouse buttons) */
			case KEY_LOCESC: /* Locator report coming...       */
				fflush(stdin);	/* Discard locator data in input */
				break;
#endif

			default:	/* echo char, position cursor */
				if (key_input < 128) {	/* printable ascii character? */
					if ((good_chars == NULL) || (htx_strchr(good_chars, (char) key_input) != NULL)) {
						if (insert_flag == TRUE) {	/* insert mode? */
							if (end_index != (max_str_len - 2)) {
								end_index++;
								for (i = end_index; i > input_index; i--)  {
									*(input_str + i) = *(input_str + i - 1);
								}

								*(input_str + input_index) = (char) key_input;
								if (input_index < (max_str_len -  2))  {
									input_index++;
								}
							}

							else  {
								beep();	/* no more room in input string */
							}
						}

						else {	/* replace mode */

							if((input_index != end_index) || (input_index != (max_str_len - 2))) {
								*(input_str + input_index) = (char) key_input;
								if (end_index < input_index)  {
								end_index = input_index;
								}

								if (input_index < (max_str_len - 2))  {
									input_index++;
								}
							}

							else  {
								beep();	/* no more room in input string */
							}
						}	/* endif */
					}

					else  {
						beep();	/* char not in good_chars string */
					}
				}

				else  {
					beep();	/* non-printable character */
				}
				break;
		}		/* endswitch */

		if (echo_flag == TRUE) {
			mvwaddstr(curses_win, input_row, input_col, input_str);
			wmove(curses_win, input_row, (input_col + input_index));
			wrefresh(curses_win);
		}	/* endif */
	}		/* endfor */
}			/* get_string() */



/*****************************************************************************/
/*****  l o a d _ c o m p a r ( )  *******************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     load_compar()                                         */
/*                                                                           */
/* DESCRIPTIVE NAME =  Compare load sequence table entries                   */
/*                                                                           */
/* FUNCTION =                                                                */
/*                                                                           */
/*    compare load_tbl structures according to the following precedence:     */
/*                                                                           */
/*            (1) load sequence number;                                      */
/*            (2) relative position in shared memory.                        */
/*                                                                           */
/*    if (item1 < item2) return -1;                                          */
/*                                                                           */
/*    if (item1 == item2) return 0;                                          */
/*                                                                           */
/*    if (item1 > item2) return 1;                                           */
/*                                                                           */
/* INPUT =             item1 - pointer to the first table entry for the      */
/*                         compare                                           */
/*                     item2 - pointer to the second table entry for the     */
/*                         compare                                           */
/*                                                                           */
/* OUTPUT =            Nothing except the return codes listed below.         */
/*                                                                           */
/* NORMAL RETURN =     if (item1 < item2) return -1;                         */
/*                                                                           */
/*                     if (item1 == item2) return 0;                         */
/*                                                                           */
/*                     if (item1 > item2) return 1;                          */
/*                                                                           */
/*                                                                           */
/* ERROR RETURNS =     None                                                  */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

int load_compar(struct load_tbl *item1, struct load_tbl *item2)
{
	if (item1->load_seq < item2->load_seq)  {
		return (-1);
	}
	if (item1->load_seq > item2->load_seq)  {
		return (1);
	}

	/*
	 * If here, load sequence numbers must be equal.
	 */
	if (item1->shm_pos < item2->shm_pos)  {
		return (-1);
	}
	if (item1->shm_pos > item2->shm_pos)  {
		return (1);
	}

	return (0);
}				/* load_compar() */


/*
 * NAME: send_message()
 *
 * FUNCTION: Sends a message to the HTX IPC message queue for inclusion
 *           in the htxmsg/htxerr log files.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This function is called by the SIGCHLD_hdl(), and SIGTERM_hdl()
 *      signal handler functions.
 *
 *      This message handler program, "hxsmsg", is always a child process
 *      of the HTX supervisor program, "hxssup".
 *
 * NOTES:
 *
 *      operation:
 *      ----------
 *      if IPC message queue available
 *        check the message length
 *        build a character time string
 *        set up the htx_data structure and format the message
 *        put the message on the message queue (msgsnd() call)
 *      else
 *        write message to stderr
 *      endif
 *
 *
 * RETURNS:
 *
 *      Return Codes:
 *      ------------------------------------------------------------------
 *       0 (0x0000) -- Normal exit.
 *       1 (0x0001) -- msg_text message was too long (message truncated).
 *       2 (0x0002) -- gettimer() system call failed (used 0).
 *       4 (0x0004) -- msgsnd() system call failed.
 *       8 (0x0008) -- IPC message queue not available.
 *
 *      NOTE: Each of the error codes listed above takes up a single bit
 *            position.  Thus, multiple error conditions are indicated
 *            by "OR'ing" the indicated error codes into a composite
 *            return code.
 *
 *            For example, if the msg_text message was too long (error
 *            code 0x0001) and the msgsnd() system call failed (error
 *            code 0x0004), the return code would be 0x0001 | 0x0004 =
 *            0x0005 (decimal 5).
 *
 *
 */

short send_message(char *msg_text, int errno_val, int severity, mtyp_t msg_type)
     /*
      * msg_text -- the text of the message
      * errno_val -- the error number value
      * sev_code -- the severity code
      * msg_type -- the message type (HTX_SYS_MSG, HTX_SYS_FINAL_MSG)
      */
{
	char char_time[22];	/* variable string for formatted system time */
	char error_msg[128];	/* error message string */
	char *str_time_ptr = NULL;	/* pointer to string format of system time */
	extern char *program_name;	/* this program's name (argv[0]) */
	extern int msgqid;	/* IPC message queue id */
	int errno_save = 0;		/* save area for errno value */
	short exit_code = 0;	/* exit program return code */
	size_t str_length = 0;	/* string length variable */
	struct htx_msg_buf msg_buffer;	/* message buffer for HTX IPC msg queue */

#ifdef	__HTX_LINUX__
	time_t	system_time;
#else
	struct timestruc_t system_time;	/* system time value from gettimer()*/
#endif

	exit_code = GOOD;
	if (msgqid != -1) {	/* message queue set up? */
		/*
		 * Check message length...
		 */
		if (htx_strlen(msg_text) > MAX_TEXT_MSG) {
			msg_text[MAX_TEXT_MSG] = '\0';	/* truncate to max length          */
			exit_code |= MSG_TOO_LONG;	/* set message too log bit         */
		}	/* endif */

		/*
		 * Build the character time string...
		 */
		errno = 0;
#ifdef	__HTX_LINUX__
		system_time = time((time_t *) NULL);
		str_time_ptr = ctime((const time_t *) &system_time);
		htx_strncpy(char_time, "", sizeof(char_time));
		htx_strncpy(char_time, (str_time_ptr + 4), 20);
#else

		if (gettimer(TIMEOFDAY, &system_time) != GOOD) {
			errno_save = errno;
			sprintf(error_msg, "\n%s -- Error in the gettimer() system call of the send_message() function.\nerrno: %d (%s).\n", program_name, errno_save, strerror(errno_save));
			fprintf(stderr, "%s", error_msg);
			fflush(stderr);
			exit_code |= BAD_GETTIMER;
			htx_strcpy(char_time, "gettimer() error");
		}

		else {
			str_time_ptr = ctime((time_t *) (&(system_time.tv_sec)));
			htx_strncpy(char_time, "", sizeof(char_time));
			htx_strncpy(char_time, (str_time_ptr + 4), 20);
		}		/* endif */

#endif

		/*
		 * Set up the "htx_data" portion of the message buffer...
		 */
		 htx_strncpy(msg_buffer.htx_data.sdev_id, "", sizeof(msg_buffer.htx_data.sdev_id));
		 htx_strncpy(msg_buffer.htx_data.sdev_id, "htx_messages",(sizeof(msg_buffer.htx_data.sdev_id) - 1));
		 msg_buffer.htx_data.error_code = errno_val;
		 msg_buffer.htx_data.severity_code = severity;
		 htx_strncpy(msg_buffer.htx_data.HE_name, "", sizeof(msg_buffer.htx_data.HE_name));
		 htx_strncpy(msg_buffer.htx_data.HE_name, program_name, (sizeof(msg_buffer.htx_data.HE_name) - 1));

		/*
		 * Format the message...
		 */
		 sprintf(msg_buffer.htx_data.msg_text, "%-18s%-20s err=%-8.8x sev=%-1.1u %-14s\n%-s\n", msg_buffer.htx_data.sdev_id, char_time, msg_buffer.htx_data.error_code, msg_buffer.htx_data.severity_code, msg_buffer.htx_data.HE_name, msg_text);

		 /*
		  * Make sure that the last two characters are '\n'...
		  */
		 str_length = htx_strlen(msg_buffer.htx_data.msg_text);
		 if (msg_buffer.htx_data.msg_text[str_length - 2] != '\n')  {
			 htx_strcat(msg_buffer.htx_data.msg_text, "\n");
		 }

		 msg_buffer.mtype = msg_type;
		 errno = 0;
		 if (msgsnd(msgqid, &msg_buffer, (sizeof(msg_buffer) - sizeof(mtyp_t)), 0) != GOOD) {
			 errno_save = errno;
			 sprintf(error_msg, "\n%s -- Error in msgsnd() system call of the send_message() function.\nerrno: %d (%s).\n", program_name, errno_save, strerror(errno_save));
			 fprintf(stderr, "%s", error_msg);
			 fflush(stderr);
			 exit_code |= BAD_MSGSND;
		 }		/* endif */
	}

	else {		/* message queue not set up -- write to stderr */
		fprintf(stderr, "%s", msg_text);
		fflush(stderr);
		exit_code |= NO_MSG_QUEUE;
	}			/* endif */

	return (exit_code);
}				/* send_message() */



/*****************************************************************************/
/*****  u n q u o t e ( )  ***************************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     unquote()                                             */
/*                                                                           */
/* DESCRIPTIVE NAME =  strip quotes                                          */
/*                                                                           */
/* FUNCTION =          strip double quotes from string variables             */
/*                                                                           */
/* INPUT =             s - point to the string variable                      */
/*                                                                           */
/* OUTPUT =            formated message which goes to the message handler.   */
/*                                                                           */
/* NORMAL RETURN =     pointer to returned string                            */
/*                                                                           */
/* ERROR RETURNS =     None (I'm an optimist)                                */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*****************************************************************************/

char	*unquote(char *s)
{
	char t[100], *t_ptr = t, *s_save = s;

	do {
		if (*s != '\"') {
			*t_ptr = *s;
			t_ptr++;
		}

		s++;
	} while (*(s - 1) != '\0');

	s = s_save;
	t_ptr = t;

	while ((*s = *t_ptr) != '\0') {
		s++;
		t_ptr++;
	}			/* end while */

	return (s_save);
}				/* unquote() */

/* Function binds a given PID  to any thread of core 0 */
int do_the_bind_proc(pid_t pid)
{
    int lcpu, smt, rc = 0;
    static int th_num = 0;

    smt = get_smt_status(0); /* get smt threads for core 0 */
    lcpu = get_logical_cpu_num(0, 0, 0, th_num); /* Bind to N0P0C0T* */
    rc = bind_process(pid, lcpu, -1);
    if (rc < 0) {

    }
    th_num = (th_num + 1) % smt;
    return rc;
}

