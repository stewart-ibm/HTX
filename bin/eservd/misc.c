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


#include "eservd.h"
#include "global.h"

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

int AHD_compar(struct ahd_entry *item1, struct ahd_entry *item2)
{
    char              alpha1[DEV_ID_MAX_LENGTH];        /* alpha part of item1->sdev_id      */
    char              alpha2[DEV_ID_MAX_LENGTH];        /* alpha part of item2->sdev_id      */
    char              *p_num1;           /* pts to num part of item1->sdev_id */
    char              *p_num2;           /* pts to num part of item2->sdev_id */

    int               num1;              /* int value for item1 num part      */
    int               num2;              /* int value for item2 num part      */
    int               result;            /* for result of strcmp()            */
    DBTRACE(DBENTRY,("enter misc.c AHD_compar\n"));

  /*
   ***  beginning of executable code  *****************************************
   */
    if (item1->slot < item2->slot)
    {
	DBTRACE(DBEXIT,("return/a -1 misc.c AHD_compar\n"));
	return (-1);
    }
    if (item1->slot > item2->slot)
    {
	DBTRACE(DBEXIT,("return/b 1 misc.c AHD_compar\n"));
	return (1);
    }

  /*
   ***  If here, slot numbers must be equal.  *********************************
   */
    if (item1->port < item2->port)
    {
	DBTRACE(DBEXIT,("return/c -1 misc.c AHD_compar\n"));
	return (-1);
    }
    if (item1->port > item2->port)
    {
	DBTRACE(DBEXIT,("return/d 1 misc.c AHD_compar\n"));
	return (1);
    }

  /*
   ***  If here, port numbers must be equal.  *********************************
   */
  /*
   ***  put alpha portion of sdev_id's in work strings  ***********************
   */
    p_num1 = strpbrk(item1->sdev_id, "0123456789");
    p_num2 = strpbrk(item2->sdev_id, "0123456789");

    if (p_num1 != NULL)
    { /* numeric part exists?              */
	(void) strncpy(alpha1, item1->sdev_id,
		       (size_t) (p_num1 - item1->sdev_id));
	alpha1[p_num1 - item1->sdev_id] = '\0';  /* concatonate null char      */
    }
    else
    { /* all alpha                         */
	(void) strcpy(alpha1, item1->sdev_id);
    } /* endif */

    if (p_num2 != NULL)
    { /* numeric part exists?              */
	(void) strncpy(alpha2, item2->sdev_id,
		       (size_t) (p_num2 - item2->sdev_id));
	alpha2[p_num2 - item2->sdev_id] = '\0';  /* concatonate null char      */
    }
    else
    { /* all alpha                         */
	(void) strcpy(alpha2, item2->sdev_id);
    } /* endif */

    if ((result = strcmp(alpha1, alpha2)) != 0)   /* alpha parts not equal?   */
    {
	DBTRACE(DBEXIT,("return/e misc.c AHD_compar\n"));
	return(result);
    }
    else
    { /* alpha parts =, look at numeric    */

	if (p_num1 == NULL)
	{ /* numeric portion not exist?      */
	    num1 = 0;                            /* set num to zero            */
	}
	else
	{ /* numeric portion exists          */
	    num1 = atoi(p_num1);                 /* convert to integer value   */
	} /* endif */

	if (p_num2 == NULL)
	{ /* numeric portion not exist?      */
	    num2 = 0;                            /* set num to zero            */
	}
	else
	{ /* numeric portion exists          */
	    num2 = atoi(p_num2);                 /* convert to integer value   */
	} /* endif */

	if (num1 == num2)
	{
	    DBTRACE(DBEXIT,("return/f misc.c AHD_compar\n"));
	    return(0);
	}
	else
	{
	    DBTRACE(DBEXIT,("return/g misc.c AHD_compar\n"));
	    return((num1 - num2)/(abs(num1 - num2)));
	} /* endif */

    } /* endif */
    DBTRACE(DBEXIT,("leave misc.c AHD_compar\n"));
}  /* AHD_compar() */


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

void   alarm_signal(int sig, int code, struct sigcontext *scp)
{
  /*
   ***  variable declarations  ***********************************************
   */
    extern  int   alarm_sig;             /* set to TRUE on receipt of SIGALRM */
    DBTRACE(DBENTRY,("enter misc.c alarm_signal\n"));

  /*
   ***  beginning of executable code  ****************************************
   */
    alarm_sig = TRUE;

    DBTRACE(DBEXIT,("return misc.c alarm_signal\n"));
    return;                              /* exit signal handler               */

} /* alarm_signal() */


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
/*                     Also, this function will modify the external          */
/*                     semaphore instruction array "wait_sops" to reflect    */
/*                     the decrease in the number of child processes.        */
/*                                                                           */
/* INPUT =             SIGCLD signal                                         */
/*                                                                           */
/* OUTPUT =            Explanatory messages to the message handler and an    */
/*                     updated wait_sops[] semaphore operations array.       */
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

void   child_death(int sig, int code, struct sigcontext *scp)
{
  /*
   ***  variable declarations  ************************************************
   */
  char    workstr[81];                 /* work string for messages          */
  char    tracemsg[80];
  char    tag[81];                     /* tag on to work string             */

  extern  int   shell_PID;             /* shell process id                  */
  extern  int   hang_mon_PID;
#ifdef _DR_HTX_
  extern  int   DR_child_PID;
#endif
  //extern  int   wait_flag;

  int     child_PID;                   /* process id of terminated child    */
  int     i;                           /* loop counter                      */
  int     status;                      /* status from terminated child      */
  int     wait_count = 0;
  static int     prev_wait_count = 0;
  (void) sprintf(tracemsg, "enter misc.c child_death, processing signal = %d\n", sig);
  DBTRACE(DBENTRY,(tracemsg));

  /*
   ***  beginning of executable code  *****************************************
   */
  /* print_log(LOGMSG,"going to do a wait to get the child status.cur_ecg_pos = %d\n", cur_ecg_pos); */

#if defined(__OS400__)
  while ((child_PID = waitpid(-1, &status, WNOHANG)) > 0)
#else
    while ((child_PID = wait3(&status, WNOHANG, (struct rusage *) NULL)) > 0)
#endif
    {
      wait_count ++;
      if ((status & 0x000000ff) == 0x00000000) /* terminated by exit() call? */
	(void) sprintf(tag, "exit(%d) call.", ((status & 0x0000ff00)>>8) );
      else if ((status & 0x0000ff00) == 0x00000000)
	(void) sprintf(tag, "signal %d.", (status & 0x0000007f) );

      if (child_PID == hxsmsg_PID) {
	hxsmsg_PID = 0;
	(void) msgctl(msgqid, IPC_RMID, (struct msqid_ds *) NULL);
	msgqid = -1;
	if (editor_PID == 0) {       /* no editor running on this term? */
	  /* print_log(LOGMSG,"hxsmsg program terminated by %s", tag); */
	} /* endif */
      }

      else if (child_PID == hxsdst_PID) {
	(void) sprintf(workstr, "hxsdst program terminated by %s\n", tag);
	hxsdst_PID = 0;
	if (status == 0x00000000) { /* exit(0) termination?        */
	  (void) send_message(workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	}
	else {
	  (void) send_message(workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	} /* endif */
	/* print_log(LOGMSG,"%s", workstr); */

      }
      else if (child_PID == -1) {
	(void) sprintf(workstr,
		       "Error on wait() in child_death().  errno = %d.",
		       errno);
	(void) send_message(workstr, errno, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
	if (editor_PID == 0) {
	  /* print_log(LOGMSG,"%s",workstr); */
	} /* endif */
      }
      else if (child_PID == editor_PID)
	editor_PID = 0;
      else if (child_PID == shell_PID)
	shell_PID = 0;
      else if (child_PID == hxstats_PID)
	hxstats_PID = 0;
      else if (child_PID == hang_mon_PID)
    hang_mon_PID = 0;
  #ifdef _DR_HTX_
      else if (child_PID == DR_child_PID)
    DR_child_PID = 0;
  #endif
      else {
	for (i = 0; i < NUM_EXERS ; i++) {
	  if( ecg_info[0].exer_list[i].exer_addr.HE_addr == 0 )
	    continue;
	  /*print_log(LOGMSG,"the entry in loc %d is %s with hdr_addr:0x%x\n",
                i, (ecg_info[0].exer_list[i].exer_addr.HE_addr)->HE_name,
                (ecg_info[0].exer_list[i].ecg_exer_addr.hdr_addr)); */
	  if( (ecg_info[0].exer_list[i].exer_pid) == child_PID )
	    break;

	}
	if (i < NUM_EXERS) { /* found it?                       */
	  /* print_log(LOGMSG,"got the child pid child_PID:%d, index:%d",child_PID,i);
	  print_log(LOGMSG,"name:%s, dev_id:%s shut flag: %d\n",
           (ecg_info[0].exer_list[i].exer_addr.HE_addr)->HE_name,
           (ecg_info[0].exer_list[i].exer_addr.HE_addr)->sdev_id,
           (ecg_info[0].exer_list[i].ecg_exer_addr.hdr_addr)->shutdown); */
	  (void) sprintf(workstr, "%s HE program for %s terminated by %s",
			 (ecg_info[0].exer_list[i].exer_addr.HE_addr)->HE_name,
			 (ecg_info[0].exer_list[i].exer_addr.HE_addr)->sdev_id, tag);
	if ( (ecg_info[0].exer_list[i].ecg_exer_addr.hdr_addr)->shutdown == 0 ) {
	  if (((ecg_info[0].exer_list[i].exer_addr.HE_addr)->user_term) || ((ecg_info[0].exer_list[i].exer_addr.HE_addr)->DR_term)) { /* terminated by user request? */
	    (void) send_message(workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	  } else {
	    (void) send_message(workstr, 0, HTX_HE_HARD_ERROR, HTX_SYS_MSG);
	  } /* endif */
	} else {
                /* printf("exerciser killed on shutdown: %s\n",workstr ); */
          }
	  /* print_log(LOGMSG,"decreasing the entries adr = 0x%x entries = %d cur = %d\n",(int)(ecg_info[0].exer_list[i].ecg_exer_addr.hdr_addr), (ecg_info[0].exer_list[i].ecg_exer_addr.hdr_addr)->num_entries, cur_ecg_pos); */
	  fflush(stdout);
	  ((ecg_info[0].exer_list[i].ecg_exer_addr.hdr_addr)->num_entries)--;
	  (ecg_info[0].exer_list[i].exer_addr.HE_addr)->PID = 0;            /* set PID to zero */
	  ecg_info[0].exer_list[i].exer_pid = 0;            /* set PID to zero */

	  if (editor_PID == 0) {
	    /* print_log(LOGMSG,"%s", workstr); */
	  } /* endif */

	} /* endif */

      } /* endif */
    }


  /* if ((wait_count == 0) && ((prev_wait_count < 2) && !wait_flag)) { */
  if ((wait_count == 0) && ((prev_wait_count < 2))) {
    if (child_PID < 0) {
      (void) sprintf(workstr, "SIGCHLD signal received by hxssup.  wait3() failed.  errno = %d.", errno);
      /* print_log(LOGMSG,"%s", workstr); */
      (void) send_message(workstr, errno, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
    }
    else if (child_PID == 0 && equaliser_flag == 0) {
      if (system_call == FALSE) {
	(void) sprintf(workstr,
		       "wait3() system call returned 0 -- No child "
		       "processes have stopped or exited. %d", prev_wait_count);
	(void) send_message(workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	if (editor_PID == 0) {     /* no editor running on this term? */
	  /* print_log(LOGMSG,"%s", workstr); */
	} /* endif */
      } /* endif */
    }
  } /* endif */
  prev_wait_count = wait_count;
    //wait_flag = 0;
  /* print_log(LOGMSG,"Returning from child death... pos = %d\n", cur_ecg_pos); fflush(stdout); */
  (void) sprintf(tracemsg, "return misc.c child_death, processing signal = %d\n", sig);
  DBTRACE(DBENTRY,(tracemsg));

  return;                              /* exit signal handler               */
} /* child_death() */


int
external_child_death(int child_PID)
{
    char    workstr[81];                 /* work string for messages          */
    char    tag[81];                     /* tag on to work string             */
    int prev_cur_ecg_pos;
    int i;
    DBTRACE(DBENTRY,("enter misc.c external_child_death\n"));

    prev_cur_ecg_pos = cur_ecg_pos;
    cur_ecg_pos = 0;
    for (i = 0; ((i < ECG_MAX_ENTRIES) && (ECGEXER_ADDR(i)) &&
		 ((ECGEXER_ADDR(i)->PID) != child_PID ));
    i++)
	;
    if (i < ECG_MAX_ENTRIES) { /* found it?                       */
	(void) sprintf(workstr, "%s HE program for %s terminated by %s",
		       (ECGEXER_ADDR(i))->HE_name,
		       (ECGEXER_ADDR(i))->sdev_id, tag);
	if (((ECGEXER_ADDR(i))->user_term) || ((ECGEXER_ADDR(i))->DR_term)) { /* terminated by user request? */
	    (void) send_message(workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	}
	else {
	    (void) send_message(workstr, 0, HTX_HE_HARD_ERROR,
				HTX_SYS_MSG);
	} /* endif */
	(ECGEXER_HDR(i)->num_entries)--;
	(ECGEXER_ADDR(i))->PID = 0;            /* set PID to zero */
	EXER_PID(i) = 0;
	cur_ecg_pos = prev_cur_ecg_pos;
    }
    DBTRACE(DBEXIT,("return 0 misc.c external_child_death\n"));
    return 0;
}


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

int DRT_compar(struct htxshm_HE *item1, struct htxshm_HE *item2)
{
    char              alpha1[DEV_ID_MAX_LENGTH];        /* alpha part of item1->sdev_id      */
    char              alpha2[DEV_ID_MAX_LENGTH];        /* alpha part of item2->sdev_id      */
    char              *p_num1;           /* pts to num part of item1->sdev_id */
    char              *p_num2;           /* pts to num part of item2->sdev_id */

    int               num1;              /* int value for item1 num part      */
    int               num2;              /* int value for item2 num part      */
    int               result;            /* for result of strcmp()            */
    DBTRACE(DBENTRY,("enter misc.c DRT_compar\n"));

  /*
   ***  beginning of executable code  *****************************************
   */
    if (item1->slot < item2->slot)
    {
	DBTRACE(DBEXIT,("return/a -1 misc.c DRT_compar\n"));
	return (-1);
    }
    if (item1->slot > item2->slot)
    {
	DBTRACE(DBEXIT,("return/b 1 misc.c DRT_compar\n"));
	return (1);
    }
  /*
   ***  If here, slot numbers must be equal.  *********************************
   */
    if (item1->port < item2->port)
    {
	DBTRACE(DBEXIT,("return/c -1 misc.c DRT_compar\n"));
	return (-1);
    }
    if (item1->port > item2->port)
    {
	DBTRACE(DBEXIT,("return/d 1 misc.c DRT_compar\n"));
	return (1);
    }
  /*
   ***  If here, port numbers must be equal.  *********************************
   */
  /*
   ***  put alpha portion of sdev_id's in work strings  ***********************
   */
    p_num1 = strpbrk(item1->sdev_id, "0123456789");
    p_num2 = strpbrk(item2->sdev_id, "0123456789");

    if (p_num1 != NULL)
    { /* numeric part exists?              */
	(void) strncpy(alpha1, item1->sdev_id,
		       (size_t) (p_num1 - item1->sdev_id));
	alpha1[p_num1 - item1->sdev_id] = '\0';  /* concatonate null char      */
    }
    else
    { /* all alpha                         */
	(void) strcpy(alpha1, item1->sdev_id);
    } /* endif */

    if (p_num2 != NULL)
    { /* numeric part exists?              */
	(void) strncpy(alpha2, item2->sdev_id,
		       (size_t) (p_num2 - item2->sdev_id));
	alpha2[p_num2 - item2->sdev_id] = '\0';  /* concatonate null char    */
    }
    else
    { /* all alpha                         */
	(void) strcpy(alpha2, item2->sdev_id);
    } /* endif */

    if ((result = strcmp(alpha1, alpha2)) != 0)   /* alpha parts not equal?   */
    {
	DBTRACE(DBEXIT,("return/e misc.c DRT_compar\n"));
	return(result);
    }
    else
    { /* alpha parts =, look at numeric    */

	if (p_num1 == NULL)
	{ /* numeric portion not exist?      */
	    num1 = 0;                            /* set num to zero            */
	}
	else
	{ /* numeric portion exists          */
	    num1 = atoi(p_num1);                 /* convert to integer value   */
	} /* endif */

	if (p_num2 == NULL)
	{ /* numeric portion not exist?      */
	    num2 = 0;                            /* set num to zero            */
	}
	else
	{ /* numeric portion exists          */
	    num2 = atoi(p_num2);                 /* convert to integer value   */
	} /* endif */

	if (num1 == num2)
	{
	    DBTRACE(DBEXIT,("return/f 0 misc.c DRT_compar\n"));
	    return(0);
	}
	else
	{
	    DBTRACE(DBEXIT,("return/g misc.c DRT_compar\n"));
	    return((num1 - num2)/(abs(num1 - num2)));
	} /* endif */

    } /* endif */
    DBTRACE(DBEXIT,("leave misc.c DRT_compar\n"));
}  /* DRT_compar() */


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
    char              alpha1[DEV_ID_MAX_LENGTH];        /* alpha part of item1->sdev_id      */
    char              alpha2[DEV_ID_MAX_LENGTH];        /* alpha part of item2->sdev_id      */
    char              *p_num1;           /* pts to num part of item1->sdev_id */
    char              *p_num2;           /* pts to num part of item2->sdev_id */

    int               num1;              /* int value for item1 num part      */
    int               num2;              /* int value for item2 num part      */
    int               result;            /* for result of strcmp()            */
    DBTRACE(DBENTRY,("enter misc.c DST_compar\n"));

  /*
   ***  beginning of executable code  *****************************************
   */
  /*
   ***  put alpha portion of sdev_id's in work strings  ***********************
   */
    p_num1 = strpbrk(item1->sdev_id, "0123456789");
    p_num2 = strpbrk(item2->sdev_id, "0123456789");

    if (p_num1 != NULL)
    { /* numeric part exists?              */
	(void) strncpy(alpha1, item1->sdev_id,
		       (size_t) (p_num1 - item1->sdev_id));
	alpha1[p_num1 - item1->sdev_id] = '\0';  /* concatonate null char     */
    }
    else
    { /* all alpha                         */
	(void) strcpy(alpha1, item1->sdev_id);
    } /* endif */

    if (p_num2 != NULL)
    { /* numeric part exists?              */
	(void) strncpy(alpha2, item2->sdev_id,
		       (size_t) (p_num2 - item2->sdev_id));
	alpha2[p_num2 - item2->sdev_id] = '\0';  /* concatonate null char     */
    }
    else
    { /* all alpha                         */
	(void) strcpy(alpha2, item2->sdev_id);
    } /* endif */

    if ((result = strcmp(alpha1, alpha2)) != 0)   /* alpha parts not equal?   */
    {
	DBTRACE(DBEXIT,("return/a misc.c DST_compar\n"));
	return(result);
    }
    else
    { /* alpha parts =, look at numeric    */

	if (p_num1 == NULL)
	{ /* numeric portion not exist?      */
	    num1 = 0;                            /* set num to zero            */
	}
	else
	{ /* numeric portion exists          */
	    num1 = atoi(p_num1);                 /* convert to integer value   */
	} /* endif */

	if (p_num2 == NULL)
	{ /* numeric portion not exist?      */
	    num2 = 0;                            /* set num to zero            */
	}
	else
	{ /* numeric portion exists          */
	    num2 = atoi(p_num2);                 /* convert to integer value   */
	} /* endif */

	if (num1 == num2)
	{
	    DBTRACE(DBEXIT,("return/b 0 misc.c DST_compar\n"));
	    return(0);
	}
	else
	{
	    DBTRACE(DBEXIT,("return/c misc.c DST_compar\n"));
	    return((num1 - num2)/(abs(num1 - num2)));
	} /* endif */

    } /* endif */

    DBTRACE(DBEXIT,("leave misc.c DST_compar\n"));
}  /* DST_compar() */


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
    DBTRACE(DBENTRY,("enter misc.c load_compar\n"));
    if (item1->load_seq < item2->load_seq)
    {
	DBTRACE(DBEXIT,("return/a -1 misc.c load_compar\n"));
	return -1;
    }
    if (item1->load_seq > item2->load_seq)
    {
	DBTRACE(DBEXIT,("return/b 1 misc.c load_compar\n"));
	return 1;
    }
  /*
   ***  If here, load sequence numbers must be equal.  ***********************
   */
    if (item1->shm_pos < item2->shm_pos)
    {
	DBTRACE(DBEXIT,("return/c -1 misc.c load_compar\n"));
	return -1;
    }
    if (item1->shm_pos > item2->shm_pos)
    {
	DBTRACE(DBEXIT,("return/d 1 misc.c load_compar\n"));
	return 1;
    }

    DBTRACE(DBEXIT,("return 0 misc.c load_compar\n"));
    return(0);
}  /* load_compar() */


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

short send_message(char *msg_text, int errno_val, int  severity,
		   mtyp_t msg_type)
     /*
      * msg_text -- the text of the message
      * errno_val -- the error number value
      * sev_code -- the severity code
      * msg_type -- the message type (HTX_SYS_MSG, HTX_SYS_FINAL_MSG)
      */

{
  /*
   ***  Data and Functions Definitions/Declarations  **************************
   */
    char char_time[22];        /* variable string for formatted system time    */
    char error_msg[128];       /* error message string                         */
    char *str_time_ptr;        /* pointer to string format of system time      */

    extern char *program_name;  /* this program's name (argv[0])               */

    extern int msgqid;         /* IPC message queue id                         */

    int errno_save;            /* save area for errno value                    */

    short exit_code;           /* exit program return code                     */

    size_t str_length;         /* string length variable                       */


    struct htx_msg_buf msg_buffer;  /* message buffer for HTX IPC msg queue    */

#if defined(__HTX_LINUX__) || defined(__OS400__)   /* 400 */
    time_t  system_time;
#else
    struct timestruc_t system_time;  /* system time value from gettimer()      */
#endif
    DBTRACE(DBENTRY,("enter misc.c send_message\n"));

  /*
   ***  Beginning of Executable Code  *****************************************
   */
    exit_code = GOOD;

    if (msgqid != -1)          /* message queue set up?                        */
    {
      /*
       * Check message length...
       */
	if (strlen(msg_text) > MAX_TEXT_MSG)
	{
	    msg_text[MAX_TEXT_MSG] = '\0';  /* truncate to max length          */
	    exit_code |= MSG_TOO_LONG;      /* set message too log bit         */
	} /* endif */


      /*
       * Build the character time string...
       */
	errno = 0;
#if defined(__HTX_LINUX__) || defined(__OS400__)   /* 400 */
	system_time = time((time_t *) NULL);
	str_time_ptr = ctime((const time_t *) &system_time);
	htx_strncpy(char_time, "", sizeof(char_time));
	htx_strncpy(char_time, (str_time_ptr + 4), 20);
#else
	if (gettimer(TIMEOFDAY , &system_time) != GOOD)
	{
	    errno_save = errno;

	    (void) sprintf(error_msg,
			   "\n%s -- Error in the gettimer() system call of the \
   send_message() function.\nerrno: %d (%s).\n",
			   program_name,
			   errno_save,
			   strerror(errno_save));

	    (void) fprintf(stderr, "%s", error_msg);
	    (void) fflush(stderr);
	    exit_code |= BAD_GETTIMER;
	    (void) strcpy(char_time, "gettimer() error");
	}
	else
	{
	    str_time_ptr = ctime((time_t *) (&(system_time.tv_sec)));
	    (void) strncpy(char_time, "", sizeof(char_time));
	    (void) strncpy(char_time, (str_time_ptr + 4), 20);
	} /* endif */

#endif

      /*
       * Set up the "htx_data" portion of the message buffer...
       */
	(void) strncpy(msg_buffer.htx_data.sdev_id, "",
		       sizeof(msg_buffer.htx_data.sdev_id));
	(void) strncpy(msg_buffer.htx_data.sdev_id, "htx_messages",
		       (sizeof(msg_buffer.htx_data.sdev_id) - 1));
	msg_buffer.htx_data.error_code = errno_val;
	msg_buffer.htx_data.severity_code = severity;
	(void) strncpy(msg_buffer.htx_data.HE_name, "",
		       sizeof(msg_buffer.htx_data.HE_name));
	(void) strncpy(msg_buffer.htx_data.HE_name, program_name,
		       (sizeof(msg_buffer.htx_data.HE_name) - 1));

      /*
       * Format the message...
       */
	(void) sprintf(msg_buffer.htx_data.msg_text,
		       "---------------------------------------------------------------------\nDevice id      : %-18s\nTimestamp      : %-20s\nerr            : %-8.8x\nsev            : %d\nExerciser Name : %-14s\nError Text     : -%s\n---------------------------------------------------------------------\n",
		       msg_buffer.htx_data.sdev_id,
		       char_time,
		       msg_buffer.htx_data.error_code,
		       msg_buffer.htx_data.severity_code,
		       msg_buffer.htx_data.HE_name,
		       msg_text);

      /*
       * Make sure that the last two characters are '\n'...
       */
	str_length = strlen(msg_buffer.htx_data.msg_text);
	if (msg_buffer.htx_data.msg_text[str_length - 2] != '\n')
	    (void) strcat(msg_buffer.htx_data.msg_text, "\n");

	msg_buffer.mtype = msg_type;

	errno = 0;
	if (msgsnd(msgqid, &msg_buffer, (sizeof(msg_buffer) - sizeof(mtyp_t)), IPC_NOWAIT)
	    != GOOD)
	{
	    errno_save = errno;

	    (void) sprintf(error_msg,
			   "\n%s -- Error in msgsnd() system call of the \
   send_message() function.\nerrno: %d (%s).\n",
			   program_name,
			   errno_save,
			   strerror(errno_save));

	    (void) fprintf(stderr, "%s", error_msg);
	    (void) fflush(stderr);
	    exit_code |= BAD_MSGSND;
	} /* endif */
    }
    else                       /* message queue not set up -- write to stderr */
    {
	(void) fprintf(stderr, "%s", msg_text);
	(void) fflush(stderr);
	exit_code |= NO_MSG_QUEUE;
    } /* endif */

    DBTRACE(DBEXIT,("return misc.c send_message\n"));
    return(exit_code);
} /* send_message() */



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

char *unquote(char *s)
{
    char t[100], *t_ptr = t, *s_save = s;
    DBTRACE(DBENTRY,("enter misc.c unquote\n"));

    do
    {
	if (*s != '\"')
	{
	    *t_ptr = *s;
	    t_ptr++;
	}
	s++;
    } while (*(s - 1) != '\0');

    s = s_save;
    t_ptr = t;

    while ((*s = *t_ptr) != '\0')
    {
	s++;
	t_ptr++;
    } /* end while */

    DBTRACE(DBEXIT,("return misc.c unquote\n"));
    return(s_save);
} /* unquote() */

int find_nap_enable(char *msg)
{
    int rc;
    FILE *fp;

    fp = fopen("/proc/sys/kernel/powersave-nap", "r");
    if (fp == NULL) {
        print_log(LOGMSG, "fopen failed for file /proc/sys/kernel/powersave-nap with errno: %d\n", errno);
        sprintf (msg, "Could not run the mdt/ecg %s.\nfopen failed for file /proc/sys/kernel/powersave-nap with errno: %d.\n", ECGNAME, errno);
        return(-1);
    }
    fscanf(fp, "%d", &rc);
    if ( rc == 0) {
        sprintf(msg, "Could not run the mdt/ecg %s, To run this, powersave mode should be enabled.\n"
                     "Current value in /proc/sys/kernel/powersave-nap is 0.\n"
                     "Please set it to non-zero value (1 - Nap, 2 - sleep).\n", ECGNAME);
    }
    return rc;
}

/* Function binds a given PID  to any thread of core 0 */
int do_the_bind_proc(pid_t pid)
{
    int lcpu, smt, rc = 0;
    static int th_num = 0;

    smt = get_smt_status(0); /* get smt threads for core 0 */
    lcpu = get_logical_cpu_num(0, 0, 0, th_num); /* Bind to N0P0C0T* */
    rc = bind_process(pid, lcpu, -1);
    th_num = (th_num + 1) % smt;
    return rc;
}
