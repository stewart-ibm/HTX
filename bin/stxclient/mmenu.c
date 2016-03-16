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

/* @(#)49	1.15.4.1  src/htx/usr/lpp/htx/bin/stxclient/mmenu.c, eserv_gui, htxubuntu 2/19/15 10:06:57 */

/*
 *   FUNCTIONS: do_option
 *              mmenu
 */


/*****************************************************************************/
/*                                                                           */
/* MODULE NAME    =    mmenu.c                                               */
/* COMPONENT NAME =    hxssup (supervisor)                                   */
/* LPP NAME       =    HTX                                                   */
/*                                                                           */
/* DESCRIPTIVE NAME =  Main Menu Functions                                   */
/*                                                                           */
/* COPYRIGHT =         (C) COPYRIGHT IBM CORPORATION 1988                    */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM           */
/*                                                                           */
/* STATUS =            Release 1 Version 0                                   */
/*                                                                           */
/* FUNCTION =          Main Menu Functions                                   */
/*            mmenu - displays the main menu and handles operator input      */
/*            mm_action - handles the response for "Action" key and mouse    */
/*                button input                                               */
/*            mm_cur_down - handles the response for cursor down key and     */
/*                mouse down movement input                                  */
/*            mm_cur_up - handles the response for cursor up key and mouse   */
/*                up movement input                                          */
/*                                                                           */
/* COMPILER OPTIONS =  -I/src/master/htx/common -g -Nn3000 -Nd4000 -Np1200   */
/*                     -Nt2000                                               */
/*                                                                           */
/* CHANGE ACTIVITY =                                                         */
/*    DATE    :LEVEL:PROGRAMMER:DESCRIPTION                                  */
/*    MMMMMMMMNMMMMMNMMMMMMMMMMNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM */
/*    06/17/88:1.0  :J BURGESS :INITIAL RELEASE                              */
/*    01/21/00:1.11 :R GEBHARDT:Feature 290676 Add/Terminate Exercisers      */
/*            :     :          :                                             */
/*                                                                           */
/*****************************************************************************/

#include "hxssup.h"
#ifndef __HTX_LINUX__
#include <sys/access.h>
#endif

#define  MM_IN_ROW      20                 /* input row for main menu       */
#define  MM_IN_COL      20                 /* input column for main menu    */

char ecg_name[56] = "/usr/lpp/htx/ecg/ecg.bu";
char temp_ecg_name[20];
extern char stat_fname[40];
extern char err_fname[40];
extern char sum_fname[40];
extern char sysdata_fname[40];
extern pid_t editor_PID;
//extern tscn_1 scn_1_in;
int sockfd;
#define AD_NAME_GOOD_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@%^,.0123456789+=-_"


/*****************************************************************************/
/*****  m m e n u ( )  *******************************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     mmenu()                                               */
/*                                                                           */
/* DESCRIPTIVE NAME =  Main Menu Function                                    */
/*                                                                           */
/* FUNCTION =          Formats and displays the HTX main menu.               */
/*                                                                           */
/* INPUT =             keyboard input                                        */
/*                     autostart - auto-startup flag                         */
/*                                                                           */
/* OUTPUT =            Formatted main menu screen                            */
/*                     Miscellaneous operator requested functions.           */
/*                                                                           */
/* NORMAL RETURN =     0 to indicate succussful completion - normal shutdown */
/*                       requested.                                          */
/*                     1 - fast shutdown requested.                          */
/*                                                                           */
/*                                                                           */
/* ERROR RETURNS =     None (I'm an optimist)                                */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = do_option - performs requested option.                */
/*                     mmenu_scn - displays main menu screen (screens.c)     */
/*                     send_message - outputs a message to the screen and    */
/*                                    the message log file                   */
/*                                                                           */
/*    DATA AREAS =     HTXPATH - pointer to a string which contains the path */
/*                         to the top level of the HTX file hierarchy        */
/*                         (usually "/usr/lpp/htx").                         */
/*                     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                                                                           */
/*    MACROS =         CLRLIN - Clear line CURSES macro                      */
/*                              (defined in common/hxiconv.h)                */
/*                     PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                                                                           */
/*****************************************************************************/

int  mmenu(int autostart)
{
  /*
   ***  variable definitions  *************************************************
   */
  char                input1[2];       /* input string                      */
  char                input2[3];       /* input string                      */
  char                workstr[128];    /* work string                       */
  char result_msg[200];

  extern char         HTXPATH[];       /* HTX file system path spec         */

  extern union shm_pointers shm_addr;  /* shared memory union pointers      */

  int                 main_option;     /* main option integer value         */
  int                 page = 1;        /* main menu page number             */
  int                 pseudo_key;      /* fake keyboard input for auto-start*/
   int rc;
   extern int run_type_disp;

  /*
   ***  beginning of executable code  *****************************************
   */

  /*
   ***  display main menu  ****************************************************
   */
   rc = send_sockmsg(SCREEN_1,0,0,ecg_name,result_msg);
   if((strcmp(ecg_name,"/")==0))
   strcpy(result_msg,"No ecg directory existent");
   if (rc !=0 )
      PRTMSG(MSGLINE, 0, ("%s", result_msg));

  (void) mmenu_scn(&page);

  if (autostart == 0)
    pseudo_key = 'n';
  else
    pseudo_key = 'y';

  /*
   * Keyboard input loop...
   */
  for (; ; )
    {

      (void) refresh();                  /* refresh screen                  */
      switch(pseudo_key)
        {
        case 'y':
          input2[0] = '1'; input2[1] = '\0';
          pseudo_key = '1';
          break;
        case '1':
          input2[0] = '5'; input2[1] = '\0';
          pseudo_key = '5';
          break;
        default:
		  strncpy(input2, "", DIM(input2));	 /* clear input */
          (void) get_string(stdscr, MM_IN_ROW, MM_IN_COL, input2,
                            (size_t) DIM(input2), "0123456789aAbBeEhHmMqQrRsS",
                            (tbool) TRUE);
          CLRLIN(MSGLINE, 0);
          break;
        } /* endswitch */

      switch (input2[0])
        {
        case '\0':
          break;
        /*case 'b':
        case 'B':
          page--;
          (void) mmenu_scn(&page);
          break;*/

    case 'a':    /* Edit eServer system specific data */
    case 'A':    /* Edit eServer system specific data */
      rc = send_sockmsg(SCREEN_A,1,0,"Get htx_sysdata file...",result_msg);
      if (rc == -99)
         return rc;
      if (rc !=0) {
         PRTMSG(MSGLINE, 0, ("%s",result_msg));
         getch();
      }
      run_type_disp = 0 ;
      edit(sysdata_fname);
      run_type_disp = 1 ;
      mmenu_scn(&page);
      break;

      case 'b':    /* Start $SHELL (shell-out) */
          run_type_disp = 0 ;
          editor_PID = 111;
          shell();
          run_type_disp = 1 ;
          mmenu_scn(&page);
          break;

        case 'e':
        case 'E':
#ifndef __HTX_LINUX__
          erpt();
          (void) clear();
          (void) refresh();
          (void) mmenu_scn(&page);
#else
         PRTMSG(MSGLINE, 0, ("Sorry, This option is currently not supported in LINUX"));
#endif
          break;
        case 'f':
        case 'F':
          page++;
          (void) mmenu_scn(&page);
          break;
        case 'h':
        case 'H':
          (void) help(16, 50, 2, 2, "mmhelp_scn", 4);
          (void) clear();
          (void) mmenu_scn(&page);
          break;
        case 'r':
        case 'R':
          (void) clear();
          (void) refresh();
          rc = send_sockmsg(SCREEN_1,0,0,ecg_name,result_msg);
          if (rc !=0 )
             PRTMSG(MSGLINE, 0, ("%s", result_msg));
          (void) mmenu_scn(&page);
          break;

      case 'q':
      case 'Q':
        htx_strncpy(workstr, "Do you really want to quit? ", DIM(workstr) - 1);
        PRTMSG(MSGLINE, 0, ("%s", workstr));
        htx_strncpy(input1, "", DIM(input1));  /* clear input */
        get_string(stdscr, MSGLINE, htx_strlen(workstr), input1, (size_t) DIM(input1), "yYnN", (tbool) TRUE);
        CLRLIN(MSGLINE, 0);
        if ((input1[0] == 'Y') || (input1[0] == 'y')) {
           PRTMSG(MSGLINE, 0, ("Shutting down eServer GUI will.  Please wait....  "));
           usleep(500000);
           clear();                /*  clean up screen               */
           refresh();              /*  update screen                 */
           endwin();               /*  end CURSES */
           exit(0);
        }
        else {
          PRTMSS(MSGLINE, 0, "                  ");
        }  /* endif */
        break;


        case 's':
        case 'S':
          (void) strncpy(workstr, "Do you really want me to shut down the "
                         "system? ", DIM(workstr) - 1);
          PRTMSG(MSGLINE, 0, ("%s", workstr));
		  strncpy(input1, "", DIM(input1));	 /* clear input */
          (void) get_string(stdscr, MSGLINE, strlen(workstr), input1,
                            (size_t) DIM(input1), "yYnN",
                            (tbool) TRUE);
          CLRLIN(MSGLINE, 0);
          if ((input1[0] == 'Y') || (input1[0] == 'y'))
            {
            rc = 1;
            rc = send_sockmsg(SHUTDOWN,(int)NULL,(int)NULL,"shutdown",result_msg);
            if (rc !=0 ) {
               PRTMSG(MSGLINE, 0, ("%s", result_msg));
	       strcat(result_msg,"\ttest wasn't shutdown, closing the client");
            }
            else {
	       strcpy(result_msg,"Test was shutdown successfully. \n\t Closing the client, daemon is stil active....");
               PRTMSG(MSGLINE, 0, ("System shutdown successful...."));
            }
           sleep(1);
           clear();                /*  clean up screen               */
           refresh();              /*  update screen                 */
           endwin();               /*  end CURSES */
	   printf("\n\n\n\t*****************************************************\n");
           printf("\t%s\n",result_msg);
	   printf("\n\n\n\t*****************************************************\n");
           fflush(stdout);
           sleep(1);
           exit(0);
        }

        else {
          PRTMSS(MSGLINE, 0, "                  ");
        }  /* endif */
        break;

        case 'm':
        case 'M':	/* display man pages */
		  edit("");
          (void) mmenu_scn(&page);
		  break;
        default:
	  rc = send_sockmsg(SCREEN_1,0,0,ecg_name,result_msg);
          main_option = -1;         /* set to invalid value */
          main_option = atoi(input2);
          //do_option(main_option, autostart);
          do_option(input2, autostart);
          (void) mmenu_scn(&page);
        } /* endswitch */

    } /* endfor */

} /* mmenu() */

/*****************************************************************************/
/*****  d o _ o p t i o n ( )  ***********************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     do_option()                                           */
/*                                                                           */
/* DESCRIPTIVE NAME =  Main Menu Option handler                              */
/*                                                                           */
/* FUNCTION =          This function calls the appropriate function(s) for   */
/*                     the option selected.                                  */
/*                                                                           */
/* INPUT =             option - integer value of the selected option.        */
/*                     autoflag - auto startup flag for AH_system()          */
/*                                                                           */
/* OUTPUT =            Miscellaneous status messages.                        */
/*                     Miscellaneous operator requested functions.           */
/*                                                                           */
/* NORMAL RETURN =     None - void function.                                 */
/*                                                                           */
/* ERROR RETURNS =     None - void function.                                 */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = AH_device - Activates/Halts individual devices        */
/*                     AH_system - Activates/Halts the entire HTX system     */
/*                     disp_dst - displays the Device Status Table           */
/*                     send_message - outputs a msg to the screen and the    */
/*                         message log file                                  */
/*                                                                           */
/*    DATA AREAS =     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                                                                           */
/*    MACROS =         PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                                                                           */
/*****************************************************************************/

//int do_option(int option, int autoflag)
int do_option(char *inp, int autoflag)
{
  /***************************************************************************/
  /***  variable/function declarations  **************************************/
  /***************************************************************************/
  int rc, option;
  extern int run_type_disp;
  char result_msg[80];
  char input1[2];    /* input string                      */
  char input2[3];    /* input string                      */

  char         workstr[128];           /* work string area.        */

  extern char  HTXPATH[];              /* HTX file system path spec         */

  extern int   hxstats_PID;            /* stats program process id          */

  extern union shm_pointers shm_addr;  /* shared memory union pointers      */

  /*
   ***  beginning of executable code  *****************************************
   */

  option = inp[0]-48;
  switch (option)
    {

    case 0:    /* Activate/Halt system          */
      //mvwaddstr(stdscr, 3,53,"                ");
      memset(temp_ecg_name,0,20);
      if((strcmp(ecg_name,"/")==0))
        strcpy(result_msg,"No ecg directory existent");
      rc = send_sockmsg(2000,0,0," ",result_msg);
      if (rc != 0)
         PRTMSG(MSGLINE, 0, ("%s",result_msg));
      disp_ecg();
      rc = send_sockmsg(SCREEN_1,0,0,ecg_name,result_msg);
      break;

    case 1:                            /* Activate/Halt system          */
      //rc = send_sockmsg(SCREEN_1,1,0,ecg_name,result_msg);
      //printf("ecg_name = %s\n",ecg_name); fflush(stdout); getch();
      if (strcmp(inp,"1a") == 0) {
         rc = send_sockmsg(2011,0,0,ecg_name,result_msg);
      }
      else if (strcmp(inp,"1s") == 0) {
         rc = send_sockmsg(2021,0,0,ecg_name,result_msg);
      }
      else if (strcmp(inp,"1") == 0) {
         rc = send_sockmsg(SCREEN_1,1,0,ecg_name,result_msg);
      }
      else {
         PRTMSG(MSGLINE, 0, ("Sorry, you entered an invalid option.  Please try again."));
         break;
      }
      if (rc == -99)
         return rc;
      if (rc != 0)
         PRTMSG(MSGLINE, 0, ("%s",result_msg));
      break;
    case 2:                            /* Activate/Halt Devices         */
      rc = send_sockmsg(SCREEN_2,0,0,ecg_name,result_msg);
      if (rc == -99)
         return rc;
      if (rc != 0) {
         PRTMSG(MSGLINE, 0, ("%s",result_msg));
         getch();
         CLRLIN(MSGLINE, 0);
         break;
      }
      AH_device();
      break;

    case 3:                            /* Set Run Type (REG/EMC)        */
      rc = send_sockmsg(SCREEN_3,0,0,ecg_name,result_msg);
      if (rc == -99)
         return rc;
      if (rc != 0) {
         PRTMSG(MSGLINE, 0, ("%s",result_msg));
         getch();
         CLRLIN(MSGLINE, 0);
         break;
      }
      COE_device();
      break;

    case 4:
         htx_strncpy(workstr, "Do you really want me to shut down the ECG? ", DIM(workstr) - 1);
        PRTMSG(MSGLINE, 0, ("%s", workstr));
        htx_strncpy(input1, "", DIM(input1));  /* clear input */
        get_string(stdscr, MSGLINE, htx_strlen(workstr), input1, (size_t) DIM(input1), "yYnN", (tbool) TRUE);
        CLRLIN(MSGLINE, 0);
        if ((input1[0] == 'Y') || (input1[0] == 'y')) {
           rc = send_sockmsg(SCREEN_4,1,0,ecg_name,result_msg);
           PRTMSG(MSGLINE, 0, ("ECG shutdown started. Please Wait..."));
           if (rc !=0 ) {
              PRTMSG(MSGLINE, 0, ("%s", result_msg));
           }
           else {
              PRTMSG(MSGLINE, 0, ("ECG shutdown Successful..."));
           }
        }
      break;
    case 5:                            /* Display Device Status Table */
      rc = send_sockmsg(SCREEN_5,1,0,ecg_name,result_msg);
      if (rc == -99)
         return rc;
      if (rc != 0) {
         CLRLIN(MSGLINE, 0);
         PRTMSG(MSGLINE, 0, ("%s",result_msg));
         getch();
         CLRLIN(MSGLINE, 0);
      }
      disp_dst();
      break;
    case 6:                            /* Update & Display Statistics */
     PRTMSG(MSGLINE, 0, ("Updating HTX statistics file..."));
      rc = send_sockmsg(SCREEN_6,1,0,ecg_name, result_msg);
      if (rc == -99)
         return rc;
      if (rc != 0) {
         PRTMSG(MSGLINE, 0, ("%s",result_msg));
         getch();
         CLRLIN(MSGLINE, 0);
         break;
      }
      run_type_disp = 0 ;
      edit(stat_fname);
      run_type_disp = 1 ;
      break;

    case 7:                            /* Edit HTX Error Log */
      rc = send_sockmsg(SCREEN_7,1,0,"Get htxerr file...",result_msg);
      if (rc == -99)
         return rc;
      if (rc !=0) {
         PRTMSG(MSGLINE, 0, ("%s",result_msg));
         getch();
         CLRLIN(MSGLINE, 0);
         break;
      }
      run_type_disp = 0 ;
      edit(err_fname);
      run_type_disp = 1 ;
      break;

    case 8:                            /* Edit HTX Message Log */
      rc = send_sockmsg(SCREEN_8,1,0,"Get htxsum file...",result_msg);
      if (rc == -99)
         return rc;
      if (rc !=0) {
         PRTMSG(MSGLINE, 0, ("%s",result_msg));
         getch();
      }
      run_type_disp = 0 ;
      edit(sum_fname);
      run_type_disp = 1 ;
      break;
    case 9:                            /* Edit MDT file */
      //func_menu();
      ART_device();
      break;

    default:
      PRTMSG(MSGLINE, 0, ("Sorry, you entered an invalid option.  Please try again."));
      break;
  }      /* endswitch */

  return;
}        /* do_option() */
