
/* @(#)48	1.5  src/htx/usr/lpp/htx/bin/stxclient/misc.c, eserv_gui, htxubuntu 5/24/04 17:07:04 */

/*
 *   FUNCTIONS: AHD_compar
 *    DRT_compar
 *    DST_compar
 *    alarm_signal
 *    child_death
 *    get_string
 *    load_compar
 *    send_message
 *    unquote
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
#include <sys/socket.h>
tnotify_msg notify_rcv;


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

int  AHD_compar(struct ahd_entry *item1, struct ahd_entry *item2)
{
  char alpha1[15];  /* alpha part of item1->sdev_id      */
  char alpha2[15];  /* alpha part of item2->sdev_id      */
  char *p_num1;    /* pts to num part of item1->sdev_id */
  char *p_num2;    /* pts to num part of item2->sdev_id */

  int num1;    /* int value for item1 num part      */
  int num2;    /* int value for item2 num part      */
  int result;    /* for result of strcmp()            */

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

  if (p_num1 != NULL) {  /* numeric part exists?              */
    strncpy(alpha1, item1->sdev_id, (size_t) (p_num1 - item1->sdev_id));
    alpha1[p_num1 - item1->sdev_id] = '\0';  /* concatonate null char      */

  }
  
  else {    /* all alpha                         */
    strcpy(alpha1, item1->sdev_id);
  }      /* endif */

  if (p_num2 != NULL) {  /* numeric part exists?              */
    strncpy(alpha2, item2->sdev_id, (size_t) (p_num2 - item2->sdev_id));
    alpha2[p_num2 - item2->sdev_id] = '\0';  /* concatonate null char      */
  }
  
  else {    /* all alpha                         */
     strcpy(alpha2, item2->sdev_id);
  }      /* endif */

  if ((result = strcmp(alpha1, alpha2)) != 0)  {  /* alpha parts not equal?   */
    return (result);
  }
  
  else {      /* alpha parts =, look at numeric    */
    if (p_num1 == NULL) {  /* numeric portion not exist?      */
      num1 = 0;  /* set num to zero            */
    }
    
    else {  /* numeric portion exists          */
      num1 = atoi(p_num1);  /* convert to integer value   */
    }    /* endif */

    if (p_num2 == NULL) {  /* numeric portion not exist?      */
      num2 = 0;  /* set num to zero            */
    }
    
    else {  /* numeric portion exists          */
      num2 = atoi(p_num2);  /* convert to integer value   */
    }    /* endif */

    if (num1 == num2) {
      return (0);
    }
    
    else {
      return ((num1 - num2) / (abs(num1 - num2)));
    }    /* endif */

  }      /* endif */
}        /* AHD_compar() */


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
  extern int alarm_sig;  /* set to TRUE on receipt of SIGALRM */

  alarm_sig = TRUE;

  return;      /* exit signal handler               */

}        /* alarm_signal() */


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

int  DRT_compar(struct htxshm_HE *item1, struct htxshm_HE *item2)
{
  char alpha1[15];  /* alpha part of item1->sdev_id      */
  char alpha2[15];  /* alpha part of item2->sdev_id      */
  char *p_num1;    /* pts to num part of item1->sdev_id */
  char *p_num2;    /* pts to num part of item2->sdev_id */

  int num1;    /* int value for item1 num part      */
  int num2;    /* int value for item2 num part      */
  int result;    /* for result of strcmp()            */

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

  if (p_num1 != NULL) {  /* numeric part exists?              */
     strncpy(alpha1, item1->sdev_id, (size_t) (p_num1 - item1->sdev_id));
     alpha1[p_num1 - item1->sdev_id] = '\0';  /* concatonate null char      */
  }
  
  else {    /* all alpha                         */
     strcpy(alpha1, item1->sdev_id);
  }      /* endif */

  if (p_num2 != NULL) {  /* numeric part exists?              */
    strncpy(alpha2, item2->sdev_id, (size_t) (p_num2 - item2->sdev_id));
    alpha2[p_num2 - item2->sdev_id] = '\0';  /* concatonate null char    */
  }
  
  else {    /* all alpha                         */
     strcpy(alpha2, item2->sdev_id);
  }      /* endif */

  if ((result = strcmp(alpha1, alpha2)) != 0)  {  /* alpha parts not equal?   */
    return (result);
  }

  else {      /* alpha parts =, look at numeric    */
    if (p_num1 == NULL) {  /* numeric portion not exist?      */
      num1 = 0;  /* set num to zero            */
    }
    
    else {  /* numeric portion exists          */
      num1 = atoi(p_num1);  /* convert to integer value   */
    }    /* endif */

    if (p_num2 == NULL) {  /* numeric portion not exist?      */
      num2 = 0;  /* set num to zero            */
    }
    
    else {  /* numeric portion exists          */
      num2 = atoi(p_num2);  /* convert to integer value   */
    }    /* endif */

    if (num1 == num2) {
      return (0);
    }
    
    else {
      return ((num1 - num2) / (abs(num1 - num2)));
    }    /* endif */

  }      /* endif */
}        /* DRT_compar() */


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
  char alpha1[15];  /* alpha part of item1->sdev_id      */
  char alpha2[15];  /* alpha part of item2->sdev_id      */
  char *p_num1;    /* pts to num part of item1->sdev_id */
  char *p_num2;    /* pts to num part of item2->sdev_id */

  int num1;    /* int value for item1 num part      */
  int num2;    /* int value for item2 num part      */
  int result;    /* for result of strcmp()            */

  /*
   * put alpha portion of sdev_id's in work strings
   */
  p_num1 = strpbrk(item1->sdev_id, "0123456789");
  p_num2 = strpbrk(item2->sdev_id, "0123456789");

  if (p_num1 != NULL) {  /* numeric part exists?              */
    strncpy(alpha1, item1->sdev_id, (size_t) (p_num1 - item1->sdev_id));
    alpha1[p_num1 - item1->sdev_id] = '\0';  /* concatonate null char     */
  }
  
  else {    /* all alpha                         */
     strcpy(alpha1, item1->sdev_id);
  }      /* endif */

  if (p_num2 != NULL) {  /* numeric part exists?              */
    strncpy(alpha2, item2->sdev_id, (size_t) (p_num2 - item2->sdev_id));
    alpha2[p_num2 - item2->sdev_id] = '\0';  /* concatonate null char     */
  }
  
  else {    /* all alpha                         */
     strcpy(alpha2, item2->sdev_id);
  }      /* endif */

  if ((result = strcmp(alpha1, alpha2)) != 0)  {  /* alpha parts not equal?   */
    return (result);
  }

  else {      /* alpha parts =, look at numeric    */
    if (p_num1 == NULL) {  /* numeric portion not exist?      */
      num1 = 0;  /* set num to zero            */
    }
    
    else {  /* numeric portion exists          */
      num1 = atoi(p_num1);  /* convert to integer value   */
    }    /* endif */

    if (p_num2 == NULL) {  /* numeric portion not exist?      */
      num2 = 0;  /* set num to zero            */
    }
    
    else {  /* numeric portion exists          */
      num2 = atoi(p_num2);  /* convert to integer value   */
    }    /* endif */

    if (num1 == num2) {
      return (0);
    }
    
    else {
      return ((num1 - num2) / (abs(num1 - num2)));
    }    /* endif */

  }      /* endif */

}        /* DST_compar() */


/*
 * NAME: get_string()
 *                                                                    
 * FUNCTION: Gets a string from operator keyboard input.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *  This function is called by other functions in the hxssup program
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
  int end_index;    /* index to last char in input_str */
  int i;      /* loop index */
  int input_index;  /* input index */
  int key_input;    /* character entered */
  tbool insert_flag;  /* Insert mode flag */
  fd_set readfds;
  //extern int sockfd;
  extern int lis_fd;
  int numbytes, old_col;
  int cont = 0;
  /*
   * Beginning of Executable Code
   */
  insert_flag = FALSE;
  input_index = 0;
  //input_str[0]=inp;
  end_index = strlen(input_str) - 1;
  old_col = input_col-1;

  /* strncpy(input_str, "", max_str_len); */
  wmove(curses_win, input_row, input_col);
  wrefresh(curses_win);

  for (;;) {
    /*
     * Get the next input character -- don't get fooled by a signal...
     */

    //FD_ZERO(&readfds);
    //FD_SET(STDIN,&readfds);
/****************************/
    //FD_SET(lis_fd,&readfds);
    //select(lis_fd+1,&readfds,NULL,NULL,NULL);
/****************************/
    //select(STDIN+1,&readfds,NULL,NULL,NULL);
    //if (FD_ISSET(STDIN, &readfds)) {
      for (; (((key_input = wgetch(curses_win)) == -1) && (errno == EINTR)); )
      ;  /* get next character */
    /*
     * Handle the new character...
     */
    switch (key_input) {

      case KEY_F(3):
	      		cont = 1;
			strcpy(input_str,"F(3)");
			break;
      case KEY_F(7):
			cont =1;
			strcpy(input_str,"F(7)");
			break;
      case KEY_F(8):	      
			cont = 1;
			strcpy(input_str,"F(8)");
			break;
      case KEY_NEWL:  /* end of string? */
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
	//printf("\n Inside Backspace");fflush(stdout);getch();
	if ((input_index > 0)   || ((input_col + end_index + 1)>old_col)) 
          input_index--;
	//printf("\n Inside the first if ");fflush(stdout);getch();
        else if (end_index < 0) {
          beep();
          break;
        }  /* endif */

      case KEY_DC:
        if (end_index >= input_index) {
	//printf("\n Inside the first if in DC");fflush(stdout);getch();	
          end_index--;
          if (end_index >= 0)  {
	  //  printf("\n Before strcpy");fflush(stdout);getch();		  
            strcpy((input_str + input_index), (input_str + input_index + 1));
          }
	// printf("\n Before null character");fflush(stdout);getch();
          *(input_str + end_index + 1) = '\0';
        }
        
        else  {
          beep();
        }

        if (echo_flag == TRUE) {
          //printf("\n Inside echo flag = true");fflush(stdout);getch();		
          mvwaddch(curses_win, input_row,  (input_col + end_index + 1), ' ');
        }  /* endif */
	if (end_index < -1)
		end_index = -1;
	if (input_index < 0)
		 input_index=0;
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

#ifndef  __HTX_LINUX__
      case KEY_SLL:   /* Locator Select (mouse buttons) */
      case KEY_LOCESC: /* Locator report coming...       */
        fflush(stdin);  /* Discard locator data in input */
        break;
#endif

      default:  /* echo char, position cursor */
		end_index=-1;
	//printf("end_index = %d inp = %d\n", end_index, input_index); fflush(stdout); getch();
        if (key_input < 128) {  /* printable ascii character? */
          if ((good_chars == NULL) || (strchr(good_chars, (char) key_input) != NULL)) {
            if (insert_flag == TRUE) {  /* insert mode? */
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
                beep();  /* no more room in input string */
              }
            }
            
            else {  /* replace mode */

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
                beep();  /* no more room in input string */
              }
            }  /* endif */
          }
          
          else  {
            beep();  /* char not in good_chars string */
          }
        }
        
        else  {
          beep();  /* non-printable character */
        }
        break;
    }    /* endswitch */
    if (cont)
	         return;
    
    if (echo_flag == TRUE) {
      mvwaddstr(curses_win, input_row, input_col, input_str);
      wmove(curses_win, input_row, (input_col + input_index));
      wrefresh(curses_win);
    } 
    /* endif */
    //}
    /*else  {
      //printf("from sock \n");
      //fflush(stdout);
      //getch();
      if ((numbytes = recv(lis_fd,&notify_rcv,100,MSG_WAITALL)) <= 0) {

        clear();
        refresh();
        endwin(); 
        printf("Received %d bytes.\n",numbytes);
        printf("Shutting Down the client\n");
        exit(0);

      }
      PRTMSG(MSGLINE, 0, ("%s.",notify_rcv.msg));
      //getch();
      CLRLIN(MSGLINE, 0);
      break;
    }*/
  }    /* endfor */
}      /* get_string() */



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
}        /* load_compar() */


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

char  *unquote(char *s)
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
  }      /* end while */

  return (s_save);
}        /* unquote() */


