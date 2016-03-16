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

/* @(#)32	1.11  src/htx/usr/lpp/htx/bin/stxclient/AH_device.c, eserv_gui, htxubuntu 5/24/04 17:02:47 */

/*****************************************************************************/
/*                                                                           */
/* MODULE NAME    =    AH_device.c                                           */
/* COMPONENT NAME =    hxssup (supervisor)                                   */
/* LPP NAME       =    HTX                                                   */
/*                                                                           */
/* DESCRIPTIVE NAME =  Activate/Halt devices                                 */
/*                                                                           */
/* COPYRIGHT =         (C) COPYRIGHT IBM CORPORATION 1988                    */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM           */
/*                                                                           */
/* STATUS =            Release 1 Version 0                                   */
/*                                                                           */
/* FUNCTION =          Activates/Halts one or more devices.                  */
/*                                                                           */
/* COMPILER OPTIONS =  -I/src/master/htx/common -g -Nn3000 -Nd4000 -Np1200   */
/*                     -Nt2000                                               */
/*                                                                           */
/*****************************************************************************/

#include "hxssup.h"

#define NUM_COLS  (23)
#define NUM_ROWS  (81)

#define AH_IN_ROW  (20)
#define AH_IN_COL  (20)

/*
 * Define this so that the rest of the code fails.
 */
extern tfull_info info_rcv;
extern char ecg_name[20];
char * fno;
int len;
int fno1;

#ifdef  __HTX_LINUX__

char  *__loc1;
int tmpp;

#ifndef  BUFSIZ
#define  BUFSIZ  (1024)
#endif

char *regcmp(char *arg1, char *arg2)
{
  if(arg1 == NULL)  {
    return NULL;
  }

  if(arg2 == NULL)  {
    return arg1;
  }

  strcat(arg1, arg2);
  return arg1;
}

char *regex(char *arg1, char *arg2)
{
  if(arg1 == NULL || arg2 == NULL)  {
    return NULL;
  }
  else  {
    while(*arg1++ != *arg2++)
      ;

    if(arg1 != NULL && arg2 != NULL)  {
      __loc1 = (--arg1);
    }
    else  {
      __loc1 = NULL;
      return NULL;
    }
    while(*arg1++ == *arg2++)
      ;

    return (++arg1);
  }

  return NULL;
}
#endif



/*****************************************************************************/
/*****  A H _ d e v i c e ( )  ***********************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     AH_device.c                                           */
/*                                                                           */
/* DESCRIPTIVE NAME =  Activate/Halt devices                                 */
/*                                                                           */
/* FUNCTION =          Activates/Halts one or more devices.                  */
/*                                                                           */
/* INPUT =             p_loc_thres: pointer to the hft data structure which  */
/*                         contains the current locator (mouse) threshold    */
/*                         values                                            */
/*                                                                           */
/*                     Operator keyboard and/or mouse input                  */
/*                                                                           */
/* OUTPUT =            Device Active/Halted Status screen(s)                 */
/*                                                                           */
/*                     Updated system semaphore structure to activate/halt   */
/*                     individual hardware exerciser programs                */
/*                                                                           */
/* NORMAL RETURN =     0 to indicate successful completion                   */
/*                                                                           */
/* ERROR RETURNS =     -1 to indicate no hardware exerciser programs         */
/*                         currently in memory.                              */
/*                     -2 to indicate an unsuccessful attempt to allocate    */
/*                         memory for the activate/halt device table.        */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = display_scn -  displays screen image                  */
/*                     init_ahd_tbl - initializes and sorts the list of      */
/*                                    devices                                */
/*                     send_message - outputs a message to the screen and    */
/*                                    the message log file                   */
/*                                                                           */
/*    DATA AREAS =     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                     SEMHEKEY semaphores (identified by the semhe_id       */
/*                        variable)                                          */
/*                                                                           */
/*    MACROS =         CLRLIN - Clear line CURSES macro                      */
/*                              (defined in common/hxiconv.h)                */
/*                     PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                                                                           */
/*****************************************************************************/

int AH_device(void)
{
  //boolean_t run_device = FALSE;  /* TRUE = activate, FALSE = halt    */
  char  *cmp_regexp = NULL;  /* ptr to compiled reg expression    */
  char  input[32];    /* input string                      */
  char  ret[3];      /* return array for regex()    */
  char  *work = NULL;    /* work int */
  char  workstr[128];    /* work string                       */
  char result_msg[80];

  extern char *__loc1;    /* beginning of reg exp match */

  extern int COLS;    /* number of cols in CURSES screen   */
  extern int LINES;    /* number of lines in CURSES screen  */
  extern WINDOW *stdscr;    /* standard screen window            */

  int inp;
  int  col = 0;    /* first column displayed on screen  */
  int  i = 0;      /* loop counter                      */
  int  max_strt_ent = 0;  /* max starting entry                */
  int  num_disp = 0;    /* number of ahd entries to show     */
  int  num_entries = 0;  /* local number of shm HE entries    */
  int  row = 0;    /* first row displayed on screen     */
  int  strt_ent = 1;    /* first entry on screen             */
  int  update_screen = 0;  /* update screen flag         */
  struct tm *p_tm;
  int yr2000;
  int  cur_page=1;
  int old_page = 1;
  int prev_page=1;
  char tempstr[80];
  /******************************************************************/
  /**  display screen outline  **************************************/
  /**********`********************************************************/

    if (info_rcv.cur_info.num_entries == 0) {  /* no HE programs? */
      PRTMSG(MSGLINE, 0, ("There are no Hardware Exerciser programs currently defined."));
      getch();
      fflush(stdin);
      CLRLIN(MSGLINE, 0);
      return (-1);
    }


  /********************************************************************/
  /**  loop until operator says to quit  ******************************/
  /********************************************************************/

  for (;;) {
  clear();
  refresh();
  display_scn(stdscr, 0, 0, LINES, COLS, "AHD_scn", 1, &row, &col, NUM_COLS, NUM_ROWS, 'n');
      
    /*
     * some HE's defined.
     */
        /*
         * # HE change?
         */
          num_entries = info_rcv.cur_info.num_entries;
          if (num_entries <= 0)  {
            return (num_entries);
          }
          

          else  {          

            if (num_entries <= 14) {
              max_strt_ent = 1;

            }
            
            /*
             * more than 14 entries
             */

            else {
              max_strt_ent = num_entries;
            }  /* endif */
        }  /* endif */
      

      /*
       * build screen data for the current strt_ent
       */


      sprintf(workstr, "%s",info_rcv.sys_hdr_info.str_curr_time); 
      #ifdef  __HTX_LINUX__
        wcolorout(stdscr, GREEN_BLACK | BOLD| NORMAL);
      #else
        wcolorout(stdscr, NORMAL | F_GREEN | B_BLACK);
      #endif
        mvwaddstr(stdscr, 1, 61, workstr);

      sprintf(workstr, "ecg = %s. (Total Devices = %d)",ecg_name, num_entries);
      #ifdef  __HTX_LINUX__
        wcolorout(stdscr, RED_BLACK | BOLD| NORMAL);
      #else
        wcolorout(stdscr, NORMAL | F_RED | B_BLACK);
      #endif
        mvwaddstr(stdscr, 2, 21, workstr);
        wmove(stdscr, 5, 38);
      old_page = prev_page;
      prev_page = cur_page;
      cur_page = ((strt_ent-1)/14)+1;
      sprintf(workstr, "2.%d", cur_page);
      mvwaddstr(stdscr, 22, 74, "     ");
      mvwaddstr(stdscr, 22, 74, workstr);


      //p_into_table = p_ahd_table + strt_ent - 1;
      num_disp = num_entries - strt_ent + 1;

      if (num_disp > 14)  {
        num_disp = 14;
      }
      for (i = 1; i <= 14; i++) {
        if (i <= num_disp) {  /* something there? */

#ifdef  __HTX_LINUX__
          wcolorout(stdscr, WHITE_BLUE | NORMAL);
#else
          wcolorout(stdscr, NORMAL | F_WHITE | B_BLUE);
#endif
          wmove(stdscr, (i + 4), 3);
          wprintw(stdscr, "%2d", i);



          if (strcmp(info_rcv.scn_num.scn_2_4_info[i].status,"ACTIVE") == 0) {
#ifdef  __HTX_LINUX__
            wcolorout(stdscr, RED_BLACK | NORMAL | BOLD);
#else
            wcolorout(stdscr, NORMAL | F_RED | B_BLACK | BOLD);
#endif
            mvwaddstr(stdscr, (i + 4), 6, "ACTIVE ");
          }
          
          else if (strcmp(info_rcv.scn_num.scn_2_4_info[i].status,"Suspend") == 0) {
#ifdef  __HTX_LINUX__
            wcolorout(stdscr, RED_BLACK | STANDOUT | BOLD);
#else
            wcolorout(stdscr, STANDOUT | F_RED | B_BLACK | BOLD);
#endif
            mvwaddstr(stdscr, (i + 4), 6, "Suspend");
          }  /* endif */
 
          else if (strcmp(info_rcv.scn_num.scn_2_4_info[i].status,"ERROR") == 0) {
#ifdef  __HTX_LINUX__
              wcolorout(stdscr, RED_BLACK | STANDOUT | BOLD);
#else
              wcolorout(stdscr, STANDOUT | F_RED | B_BLACK | BOLD);
#endif

              mvwaddstr(stdscr, (i + 4), 6, "ERROR  ");


          }


 else if (strcmp(info_rcv.scn_num.scn_2_4_info[i].status,"PRTLYRN") == 0) {
#ifdef  __HTX_LINUX__
              wcolorout(stdscr, RED_BLACK | STANDOUT | BOLD);
#else
              wcolorout(stdscr, STANDOUT | F_RED | B_BLACK | BOLD);
#endif

              mvwaddstr(stdscr, (i + 4), 6, "PRTLYRN");


          }
          
/* Slot, Port, /dev/id, Adapt Desc,
           * and Device Desc
           */
          wmove(stdscr, i + 4, 14);
#ifdef  __HTX_LINUX__
          wcolorout(stdscr, WHITE_BLUE | NORMAL);
#else
          wcolorout(stdscr, NORMAL | F_WHITE | B_BLUE);
#endif
          sprintf(workstr, " %-7s ", info_rcv.scn_num.scn_2_4_info[i].sdev_id);
          mvwaddstr(stdscr, (i + 4), 14, workstr);
          sprintf(workstr, " %-11s ", info_rcv.scn_num.scn_2_4_info[i].adapt_desc);
          mvwaddstr(stdscr, (i + 4), 25, workstr);
          sprintf(workstr, "  %-16s ", info_rcv.scn_num.scn_2_4_info[i].device_desc);
          mvwaddstr(stdscr, (i + 4), 38, workstr);
          sprintf(workstr, "%-13s", info_rcv.scn_num.scn_2_4_info[i].slot_port);
          mvwaddstr(stdscr, (i + 4), 57, workstr);
          /*sprintf(workstr, "%.4d", info_rcv.scn_num.scn_2_4_info[i].slot);
          mvwaddstr(stdscr, (i + 4), 57, workstr);
          sprintf(workstr, "-%.4d ", info_rcv.scn_num.scn_2_4_info[i].port);
          mvwaddstr(stdscr, (i + 4), 61, workstr);*/
        }
        
        /*
         * no HE entry for this area of screen
         */
        else  {
#ifdef  __HTX_LINUX__
          wcolorout(stdscr, WHITE_BLUE | NORMAL);
#else
          wcolorout(stdscr, NORMAL | F_WHITE | B_BLUE);
#endif
          /*
           * scn entry num
           */
          mvwaddstr(stdscr, (i + 4), 3, "  ");
          mvwaddstr(stdscr, (i + 4), 6, "       ");

          /*
           * COE Flag slot fld
           */
          mvwaddstr(stdscr, (i + 4), 14, "          ");
          /*
           * port fld
           */
          mvwaddstr(stdscr, (i + 4), 25, "             ");
          /*
           * sdev_id
           */
          mvwaddstr(stdscr, (i + 4), 38, "                  ");
          /*
           * adapt_desc field
           */
          mvwaddstr(stdscr, (i + 4), 56, "                       ");
          /*
           * device_desc field
           */
        }  /* endif */

      }    /* endfor */

#ifdef  __HTX_LINUX__
      wcolorout(stdscr, NORMAL);
#else
      wcolorout(stdscr, NORMAL);
#endif

      for (update_screen = FALSE; (update_screen == FALSE); ) {

        /*
         * Read input from keyboard...
         */
	strncpy(input,"", DIM(input));  /* clear input */
        get_string(stdscr, AH_IN_ROW, AH_IN_COL, input, (size_t) DIM(input), (char *) NULL, (tbool) TRUE);
           /* activate all devices
           */
	if ((strcmp(input,"F(3)"))==0)
	{
	 if (prev_page == cur_page && cur_page != old_page)
	   {	
	 	prev_page = old_page;
	   } 
	 else {
	 old_page = prev_page;	 
	 }
	 strt_ent=(prev_page*14)-13;
         if(strt_ent<=max_strt_ent)
         {
             update_screen= TRUE;
	   send_sockmsg(SCREEN_2,'p',strt_ent,ecg_name,result_msg);
        }
        else
        { 
	      beep();
          
	}		  
	}
        else if ((strcmp(input,"F(7)"))==0)	
	{
		if(strt_ent > 1)
                 {
                         if (strt_ent > 1)
                         strt_ent -= 14;
                        if (strt_ent < 1)
                          {
                          strt_ent = 1;
                         }
                        update_screen = TRUE;
                      send_sockmsg(SCREEN_2,'u',strt_ent,ecg_name,result_msg);
		}
                else {
	              beep();
		     }
		      
	}
	else if ((strcmp(input,"F(8)"))==0)
	{
	  if(strt_ent < max_strt_ent)
           {
             if (strt_ent < max_strt_ent)
             strt_ent += 14;
            if (strt_ent > max_strt_ent)
             {
              strt_ent -= 14;
	     }
           update_screen = TRUE;
          send_sockmsg(SCREEN_2,'l',strt_ent,ecg_name,result_msg);
            }
         else
           {
		   beep();
   	   }	
	}		
	else  if((strcmp(input,"h")==0)||(strcmp(input,"H")==0))
             {
             if (strlen(input) != 1)
                {
                  PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
                  getch();
                  CLRLIN(MSGLINE, 0);
                }
             else
                {
                  help(16, 50, 2, 2, "s2help_scn", 4);
                  clear();
                  refresh();
                 //  display_scn(stdscr, 0, 0, LINES,COLS, "AHD_scn", 1, &row, &col, NUM_COLS, NUM_ROWS, 'n');
                   update_screen = TRUE;
                }
            } else if((strcmp(input,"a")==0)||(strcmp(input,"A")==0))
             {
             if (strlen(input) != 1)
                {
                  PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
                  getch();
                  CLRLIN(MSGLINE, 0);
                }
             else
                {
                  update_screen = TRUE;
                  send_sockmsg(SCREEN_2,'a',strt_ent,ecg_name,result_msg);
                }
             }
         
 else if(input[strlen(input)-1]=='f'||input[strlen(input)-1]=='F')
            {
            if (strlen(input) != 1)
               {
                 fno = (char *)strtok(input,"f");
		if(fno!=NULL)
		 {
		 len = strlen(fno);
		 for(i=0 ; i<len ; i++)
 		   {
                      if ((fno[i] < 48) || (fno[i] > 57))
 			{
			PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
			 getch();
	                 CLRLIN(MSGLINE, 0);
                         break;
			}
                   }
                 if (i==len)
 		   { 
                 fno1= (int) atoi(fno);
                 if(strt_ent + (fno1 * 14) < max_strt_ent)
                   {
                     strt_ent += (fno1 * 14);
                     send_sockmsg(SCREEN_2,'f',strt_ent,ecg_name,result_msg);
                     update_screen = TRUE;
                   }
                 else
                   {
                     update_screen = TRUE;
                     if(strt_ent < max_strt_ent)
                       {
                            while (strt_ent < max_strt_ent)
                            strt_ent += 14;
                            if (strt_ent > max_strt_ent)
                               {
                                  strt_ent -= 14;
                               }
                            send_sockmsg(SCREEN_2,'l',strt_ent,ecg_name,result_msg);
                            update_screen = TRUE;
                       }
                     else
                       {
                            beep();
                       }
                   }
       
              }
	 }
    else {
       PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
        getch();
        CLRLIN(MSGLINE, 0);
    }
}
             else
                  {
                      update_screen = TRUE;
                      if (strt_ent < max_strt_ent)
                       {
                          strt_ent += 14;
                          if (strt_ent > max_strt_ent)
                             {
                               strt_ent -= 14;
                             }
                          update_screen = TRUE;
			  clear();
			  refresh();
                          send_sockmsg(SCREEN_2,'f',strt_ent,ecg_name,result_msg);
                       }
                      else
                        {
                          beep();
                       }  
                  }
        }
       else if((strcmp(input,"l")==0)||(strcmp(input,"L")==0))
         {
              if (strlen(input) != 1)
                {
                   PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
                   getch();
                   CLRLIN(MSGLINE, 0);
                }
              else
                {
                   update_screen = TRUE;
                   if(strt_ent <= max_strt_ent)
                     {
                       while (strt_ent < max_strt_ent)
                       strt_ent += 14;

                       if (strt_ent > max_strt_ent)
                         {
                            strt_ent -= 14;
                         }
                       update_screen = TRUE;
                       send_sockmsg(SCREEN_2,'l',strt_ent,ecg_name,result_msg);
                     }
                  else {
                         beep();
                       }  
                 }
         }

else if(input[strlen(input)-1]=='b'||input[strlen(input)-1]=='B')
           {
             if (strlen(input) != 1)
                {
                   fno = (char *)strtok(input,"b");
		if(fno!=NULL)
                 {
                 len = strlen(fno);
                 for(i=0 ; i<len ; i++)
                   {
                      if ((fno[i] < 48) || (fno[i] > 57))
                        {
                        PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
                         getch();
                         CLRLIN(MSGLINE, 0);
                         break;
                        } 
                   }      
                 if (i==len) 
                   {           

                   fno1= (int) atoi(fno);
                   if(strt_ent - (fno1 * 14) > 1)
                     {
                       strt_ent -= (fno1 * 14);
                       update_screen = TRUE;
                       send_sockmsg(SCREEN_2,'b',strt_ent,ecg_name,result_msg);
                     }
                   else
                     {
                       update_screen = TRUE;
                       if(strt_ent > 1)
                         {
                           while (strt_ent > 1)
                           strt_ent -= 14;
                           if (strt_ent < 1)
                              {
                                strt_ent = 1;
                              }
                           update_screen = TRUE;
                           send_sockmsg(SCREEN_2,'u',strt_ent,ecg_name,result_msg);
                         }
                      else {
                             beep();
                           } 
                    }
                }}
           else 
              {
       		PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
        	getch();
       		 CLRLIN(MSGLINE, 0);
   	     } 
       }
         else
                {
                if(strt_ent > 1)
                  {
                     strt_ent -= 14;
                     if (strt_ent < 1)
                        {
                          strt_ent = 1;
                        }
                     update_screen = TRUE;
                     send_sockmsg(SCREEN_2,'b',strt_ent,ecg_name,result_msg);
                  }
                else
                  {
                     beep();
                     }  
                }
        }
       else if((strcmp(input,"u")==0)||(strcmp(input,"U")==0))
            {
              if (strlen(input) != 1)
                 {
                    PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
                    getch();
                    CLRLIN(MSGLINE, 0);
                 }
              if(strt_ent > 1)
                {
                   while (strt_ent > 1)
                   strt_ent -= 14;
                   if (strt_ent < 1)
                      {
                        strt_ent = 1;
                      }
                   update_screen = TRUE;
                   send_sockmsg(SCREEN_2,'u',strt_ent,ecg_name,result_msg);
               }
              else
                {
                 beep();
                }  /* endif */
           }
else   if((strcmp(input,"s")==0)||(strcmp(input,"S")==0))
              {
                if (strlen(input) != 1)
                   {
                      PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
                      getch();
                      CLRLIN(MSGLINE, 0);
                   }
                else
                   {
                      update_screen = TRUE;
                      send_sockmsg(SCREEN_2,'s',strt_ent,ecg_name,result_msg);
                   }
             }
      else if((strcmp(input,"q")==0)||(strcmp(input,"Q")==0))
            {
              if (strlen(input) != 1)
                 {
                   PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
                   getch();
                   CLRLIN(MSGLINE, 0);
                 }
                   return (0);
             }
       else if((strcmp(input,"r")==0)||(strcmp(input,"R")==0))
             {
               if (strlen(input) != 1)
                  {
                      PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
                      getch();
                      CLRLIN(MSGLINE, 0);
                  }
               else
                  {
                      clear();
                      refresh();
                      send_sockmsg(SCREEN_2,0,strt_ent,ecg_name,result_msg);
                 //  display_scn(stdscr, 0, 0, LINES,COLS, "AHD_scn", 1, &row, &col, NUM_COLS, NUM_ROWS, 'n');
                   update_screen = TRUE;
                  }
             }
else {
        update_screen = TRUE;
	if (strlen(input) > 0) {
        sprintf(workstr,"%s@%s",ecg_name,input);
        send_sockmsg(2012,'w',strt_ent,workstr,result_msg);
	}
     }


//      }    /* end of new else*/
    } /* end of for*/  
  }   /*end of for*/
}        /* AH_device() */
