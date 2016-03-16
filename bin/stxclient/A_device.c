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

/* @(#)34	1.5  src/htx/usr/lpp/htx/bin/stxclient/A_device.c, eserv_gui, htxubuntu 5/24/04 17:03:03 */

#include "hxssup.h"

#ifndef  __HTX_LINUX__
#include <string.h>
#endif

/*#ifdef  __HTX_LINUX__
#include "cfgclibdef.h"
#else
#include "cfgclibdef.h"
#endif*/
extern char ecg_name[20];

extern  int  AD_scn(char menu_add_name[],    /* I/O device or file name */
    enum t_add_method *addMethod,  /* I/O method to generate MDT */
    enum t_start_now *startNow,  /* I/O for start_halted attribute */
    boolean_t * forceRedef,    /* I/O Overwrite existing dev */
    char *num_devs_to_add,    
    char op_info_msg[]);    /* I/O operator message */


extern tprobe_msg probe_rcv;
extern char HTXPATH[];
//extern unsigned int max_wait_tm;  /* maximum semop wait time      */

/*****************************************************************************/
/*****  A _ d e v i c e  *****************************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     A_device                                              */
/*                                                                           */
/* DESCRIPTIVE NAME =  Add device and exerciser to test environment.         */
/*                                                                           */
/* FUNCTION =          Top level Add Device function, called from            */
/*                     ART_device().  Puts up the screen and gets user       */
/*                     input then calls subordinate functions to             */
/*                     add the device to the share memory structures,        */
/*                     initialized semaphores, and start the exerciser       */
/*                     process.                                              */
/*                                                                           */
/* INPUT =             Gets operator input from AD_scn().  This could        */
/*                     include an MDT attribute file.                        */
/*                                                                           */
/* OUTPUT =            Updated semaphores and shared memory.                 */
/*                                                                           */
/* NORMAL RETURN =     N/A - void function                                   */
/*                                                                           */
/* ERROR RETURNS =     N/A - void function                                   */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*                                                                           */
/*    OTHER ROUTINES =                                                       */
/*                     send_message() - outputs a message to the screen and  */
/*                                     the message log file                  */
/*                     AD_scn() - Gets operator input.                       */
/*                     cfgcclsf() - close MDT file.                          */
/*                     cfgcopsf() - open MDT file.                           */
/*                     process_mdt_to_shm()                                  */
/*                     exec_HE_script()                                      */
/*                                                                           */
/*    DATA AREAS =                                                           */
/*                     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                                                                           */
/*    MACROS =         PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                                                                           */
/*****************************************************************************/
void  A_device(void)
{

  boolean_t confirm_errmsg;
  char msg[81];
  char *p_msg = NULL;
  int  AD_scn_rc = 0;
  int indx;
  char result_msg[80], workstr[80], num_devs_to_add[5];

  /*
   * Menu Items
   */
  static char add_name[26];  /* device to add or MDT file name */
  static enum t_add_method addMethod = AD_Method_Default;
  static enum t_start_now startNow = AD_Start_Default;
  static boolean_t forceRedef = FALSE;

  /*
   * set file descriptor to close on exec
   */
  fcntl(fileno(stdin), F_SETFD, 1);  /* stdin to close on exec  */
  fcntl(fileno(stdout), F_SETFD, 1);  /* stdout to close on exec */
  fcntl(fileno(stderr), F_SETFD, 1);  /* stderr to close on exec */

  strncpy(add_name, "", DIM(add_name));  /* empty name field */
  p_msg = NULL;              /* no message */
  confirm_errmsg = FALSE;

  for (;;)  {

      /*
       * Do the cleanup from the previous loop.  The conditions are
       * always false the first time through.
       */

      if (confirm_errmsg) {
          PRTMSG(MSGLINE - 2, 0, ("Press any key to continue..."));
          getch();
          fflush(stdin);
      }

      CLRLIN(MSGLINE, 0);
      confirm_errmsg = FALSE;

      if (AD_scn_rc != 0)  {
          break;
      }

      /* 
       *              Display menu
       */
      AD_scn_rc = AD_scn(add_name, &addMethod, &startNow, &forceRedef, num_devs_to_add, p_msg);

      /*
       * skip the rest of the loop if the user wants to quit.
       */
      if (AD_scn_rc == 1)  {
          continue;  /* operator quit the menu */
      }

      if (strlen(add_name) == 0) {
        p_msg = strcpy(msg, "Specify device or MDT file name (option 2).");
          continue;
      }    /* if */

      p_msg = NULL;  /* init message pointer */

      indx = startNow;
      if (startNow == 4)
         indx = 2;
      indx = (indx<<8)|(forceRedef&0xFF);
      sprintf(workstr,"%s@%s",ecg_name, add_name);
      send_sockmsg(SCREEN_9_A,atoi(num_devs_to_add),indx,workstr,result_msg);
      //send_sockmsg(SCREEN_9_A,addMethod,forceRedef,add_name);
      //if (probe_rcv.err != 0)
      CLRLIN(MSGLINE, 0);
      PRTMSG((MSGLINE-1), 0, ("%s", probe_rcv.msg_text));
      PRTMSG(MSGLINE, 0, ("Press any key to continue..."));
      getch();
         

  }    /* for ever */


  fcntl(fileno(stdin), F_SETFD, 0);  /* stdin NOT to close on exec */
  fcntl(fileno(stdout), F_SETFD, 0);  /*stdout NOT to clse on exec */
  fcntl(fileno(stderr), F_SETFD, 0);  /*stderr NOT to clse on exec */

}        /* A_device */



