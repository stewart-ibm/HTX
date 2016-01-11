
/* @(#)04	1.9  src/htx/usr/lpp/htx/bin/stxclient/disp_ecg.c, eserv_gui, htxubuntu 5/24/04 17:09:11 */

#include "hxssup.h"

#define NUM_COLS  (23)
#define NUM_ROWS  (81)

#define AH_IN_ROW  (20)
#define AH_IN_COL  (19)

extern char ecg_name[];
extern tfull_info info_rcv;
int  selected=0;

int disp_ecg(void)
{
  char  input[80];    /* input string                      */
  char  ret[3];      /* return array for regex()    */
  char  *work = NULL;    /* work int */
  char  workstr[128];    /* work string                       */
  char result_msg[80];
  char * fno;
  int len;
  int fno1;
  int inp;
  extern int COLS;    /* number of cols in CURSES screen   */
  extern int LINES;    /* number of lines in CURSES screen  */
  extern WINDOW *stdscr;    /* standard screen window            */


  int  col = 0;    /* first column displayed on screen  */
  int  i = 0,rc;      /* loop counter                      */
  int  max_strt_ent = 0;  /* max starting entry                */
  int  num_disp = 0;    /* number of ahd entries to show     */
  int  num_entries = 0;  /* local number of shm HE entries    */
  int  row = 0;    /* first row displayed on screen     */
  int  strt_ent = 1;    /* first entry on screen             */
  int  update_screen = 0;  /* update screen flag         */
  char edit_name[56];
  char addecg_name[80];
  char *ptr1, *ptr2;
  extern int run_type_disp;
  int  cur_page=1;
  int prev_page=1;
  int old_page = 1;
  char tempstr[80];


  /******************************************************************/
  /**  display screen outline  **************************************/
  /******************************************************************/
  if (info_rcv.cur_info.num_entries <= 0) {  /* no HE programs? */
     PRTMSG(MSGLINE, 0, ("There are no Hardware Exerciser programs currently defined."));
     getch();
     fflush(stdin);
     CLRLIN(MSGLINE, 0);
     return (-1);
  }

  clear();
  refresh();
  display_scn(stdscr, 0, 0, LINES, COLS, "ecg_scn", 1, &row, &col, NUM_COLS, NUM_ROWS, 'n');

  /********************************************************************/
  /**  loop until operator says to quit  ******************************/
  /********************************************************************/

  for (;;) {
      
      num_entries = info_rcv.cur_info.num_entries;
      if (num_entries <= 14) {
         max_strt_ent = 1;
      }
      /*
      * more than 14 entries
      */
      else {
         max_strt_ent = num_entries;
      }  /* endif */
      sprintf(workstr, "%s",info_rcv.sys_hdr_info.str_curr_time); 
      #ifdef  __HTX_LINUX__
        wcolorout(stdscr, GREEN_BLACK | BOLD| NORMAL);
      #else
        wcolorout(stdscr, NORMAL | F_GREEN | B_BLACK);
      #endif
        mvwaddstr(stdscr, 1, 61, workstr);
        wmove(stdscr, 5, 38);
       
	old_page = prev_page;
     prev_page = cur_page;
      cur_page = ((strt_ent-1)/14)+1;
	mvwaddstr(stdscr, 22, 74, "     ");
      sprintf(workstr, "0.%d", cur_page);
      mvwaddstr(stdscr, 22, 74, workstr);


 
      num_disp = num_entries - strt_ent + 1;

      if (num_disp > 14)  {
        num_disp = 14;
      }
      for (i = 1; i <= 14; i++) {
        if (i <= num_disp) {  /* something there? */

#ifdef  __HTX_LINUX__
          wcolorout(stdscr, WHITE_BLUE | NORMAL);
          /*if (i == selected)*/
	     if (strcmp(ecg_name,info_rcv.scn_num.scn_0_info[i].ecg_abs_name) == 0)
             wcolorout(stdscr, GREEN_BLUE | BOLD|NORMAL);
#else
          wcolorout(stdscr, NORMAL | F_WHITE | B_BLUE);
         /* if (i == selected)*/
	    if (strcmp(ecg_name,info_rcv.scn_num.scn_0_info[i].ecg_abs_name) == 0)
             wcolorout(stdscr, BOLD| F_GREEN | B_BLUE);
#endif
          wmove(stdscr, (i + 4), 2);
          wprintw(stdscr, "%2d", i);

#ifdef  __HTX_LINUX__
          wcolorout(stdscr, WHITE_BLUE | NORMAL);
         /* if (i == selected) */
	    if (strcmp(ecg_name,info_rcv.scn_num.scn_0_info[i].ecg_abs_name) == 0)
             wcolorout(stdscr, GREEN_BLUE | BOLD|NORMAL);
#else
          wcolorout(stdscr, NORMAL | F_WHITE | B_BLUE);
         /* if (i == selected) */
	    if (strcmp(ecg_name,info_rcv.scn_num.scn_0_info[i].ecg_abs_name) == 0)
             wcolorout(stdscr, BOLD| F_GREEN | B_BLUE);
#endif
          sprintf(workstr, "%-7s", info_rcv.scn_num.scn_0_info[i].ecg_status);
          mvwaddstr(stdscr, (i + 4), 6, workstr);
          sprintf(workstr, "%-26s", info_rcv.scn_num.scn_0_info[i].ecg_abs_name);
          mvwaddstr(stdscr, (i + 4), 16, workstr);
          sprintf(workstr, " %-16s ", info_rcv.scn_num.scn_0_info[i].ecg_desc);
          mvwaddstr(stdscr, (i + 4), 45, workstr);
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
          mvwaddstr(stdscr, (i + 4), 2, "  ");             // Serial # 
          mvwaddstr(stdscr, (i + 4), 6, "        ");        // Status of ECG
          mvwaddstr(stdscr, (i + 4), 16, "                         ");    // ECG name
          mvwaddstr(stdscr, (i + 4), 45, "                         "); // ECG Description
       }
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
strncpy(input, "", DIM(input));  /* clear input */
 get_string(stdscr, AH_IN_ROW, AH_IN_COL, input, (size_t) DIM(input), (char *) NULL, (tbool) TRUE);
	
	if((strcmp(input,"F(7)"))==0)
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
			    send_sockmsg(2000,'b',strt_ent,"FFF",result_msg);
                         }
                      else {
                             beep();
                           }
    
                }
	else if((strcmp(input,"F(8)"))==0)
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
			     send_sockmsg(2000,'f',strt_ent,"BBB",result_msg);
                     }
                     else
                        {
                          beep();
                        }

                 }
	else if((strcmp(input,"F(3)"))==0)
      {
	        	if (prev_page == cur_page && cur_page != old_page)
		        		{
			                prev_page = old_page;
			           }
         else {
          old_page = prev_page;
               }
                   strt_ent=(prev_page*14)-13;
                   if(strt_ent<max_strt_ent)
                   {
                    update_screen= TRUE;
		     send_sockmsg(2000,'p',strt_ent,"PPP",result_msg);
                   }
                  else
                        {
                        beep();
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
                     update_screen = TRUE;
		     send_sockmsg(2000,'f',strt_ent,"FFF",result_msg);
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
                            update_screen = TRUE;
			   send_sockmsg(2000,'l',strt_ent,"FFF",result_msg);

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
			send_sockmsg(2000,'f',strt_ent,"FFF",result_msg);

                       }
                      else
                        {
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
			 send_sockmsg(2000,'b',strt_ent,"BBB",result_msg);

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
			   send_sockmsg(2000,'u',strt_ent,"BBB",result_msg);
	
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
		      send_sockmsg(2000,'b',strt_ent,"BBB",result_msg);

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
           if(strt_ent < max_strt_ent)
            {
              while (strt_ent < max_strt_ent)
                    strt_ent += 14;

	            if (strt_ent > max_strt_ent)
	              {
	                 strt_ent -= 14;
	              }
	              update_screen = TRUE;
	              send_sockmsg(SCREEN_2,'l',strt_ent,ecg_name,result_msg);																			          }
			     else {											       
							    beep();
							  }
            }
     }


else if((strcmp(input,"q")==0)||(strcmp(input,"Q"))==0)
      {
       if (strlen(input) != 1) {
              PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
              getch();
              CLRLIN(MSGLINE, 0);
            }
	else
	 return (0);
      }
else if((strcmp(input,"r")==0)||(strcmp(input,"R"))==0)
     {
 	     if (strlen(input) != 1) {
              PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
              getch();
              CLRLIN(MSGLINE, 0);
               }
             else
		{
 		  clear();
            refresh();
            send_sockmsg(2000,0,strt_ent,"Refresh eServer screen 2",result_msg);
            display_scn(stdscr, 0, 0, LINES,COLS, "ecg_scn", 1, &row, &col, NUM_COLS, NUM_ROWS, 'n');
            update_screen = TRUE;
               }
     }
else if((input[0]=='e')||(input[0]=='E'))
{
	    run_type_disp = 0 ;
            PRTMSG(MSGLINE, 0, ("Editing %s", &input[2]));
            sprintf(edit_name, "%s", &input[2]);
            edit(edit_name);
            run_type_disp = 1 ;
            send_sockmsg(2000,0,strt_ent,"Refresh eServer screen 2",result_msg);
            display_scn(stdscr, 0, 0, LINES,COLS, "ecg_scn", 1, &row, &col, NUM_COLS, NUM_ROWS, 'n');
            update_screen = TRUE;
}
else if(input[0] == 'a')
{
             ptr1 = strtok(input," ");
             if (strcmp("addecg",ptr1) == 0 ) {
                   strcpy(addecg_name,&input[strlen(ptr1)+1]);
                   if ( strcmp(addecg_name,"") != 0 ) {
                      clear();
                      refresh();
                      send_sockmsg(2010,0,strt_ent,addecg_name,result_msg);
                                   num_entries = info_rcv.cur_info.num_entries;
                      strt_ent = (num_entries/14)*14 + 1;
                      display_scn(stdscr, 0, 0, LINES,COLS, "ecg_scn", 1, &row, &col, NUM_COLS, NUM_ROWS, 'n');
                       update_screen = TRUE;
                   } else {
                       PRTMSG(MSGLINE, 0, ("please provide the ecg name"));
                       getch();
                       CLRLIN(MSGLINE, 0);
                   }
              }
}
else if((strcmp(input,"h")==0)||(strcmp(input,"H"))==0)
{
	   help(16, 50, 2, 2, "s0help_scn", 6);
            clear();
            display_scn(stdscr, 0, 0, LINES,COLS, "ecg_scn", 1, &row, &col, NUM_COLS, NUM_ROWS, 'n');
            update_screen = TRUE;

}
else
   { 
     if ((atoi(input) < 0) || (atoi(input) > 14)) {
               PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
               getch();
               CLRLIN(MSGLINE, 0);
            }
     else  {
            update_screen = TRUE;
rc = send_sockmsg(2000,atoi(input),strt_ent,info_rcv.scn_num.scn_0_info[atoi(input)].ecg_abs_name,result_msg);
            if (rc != 0) {
               PRTMSG(MSGLINE, 0, ("%s", result_msg));
               getch();
               CLRLIN(MSGLINE, 0);
               strcpy(ecg_name, "/usr/lpp/htx/ecg/ecg.bu");
            }
            else {
               selected = atoi(input);
               strcpy(ecg_name,info_rcv.scn_num.scn_0_info[atoi(input)].ecg_abs_name);
               ecg_name[strlen(info_rcv.scn_num.scn_0_info[atoi(input)].ecg_abs_name)] = '\0';
            }}
  }
      }    /* endfor */
    }    /* endfor */
}        /* disp_ecg() */
