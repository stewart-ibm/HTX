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

/* @(#)41	1.9.4.1  src/htx/usr/lpp/htx/bin/stxclient/disp_dst.c, eserv_gui, htxubuntu 3/16/15 06:24:18 */

#include "hxssup.h"
//#include <sys/poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <libgen.h>


int glbl_cycles_flag = 1;
extern tfull_info info_rcv; 
extern tnotify_msg notify_rcv; 
char *hdr,*s2[512];
extern char ecg_name[20];


/*****************************************************************************/
/*****  d i s p _ d s t ( )  *************************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     disp_dst()                                            */
/*                                                                           */
/* DESCRIPTIVE NAME =  Display the Device Status Table                       */
/*                                                                           */
/* FUNCTION =          Displays all devices defined by the mdt               */
/*                     (/usr/lpp/htx/mdt/mdt attribute file) and the time    */
/*                     of their last update and last error                   */
/*                                                                           */
/* INPUT =             Operator keyboard and/or mouse input                  */
/*                                                                           */
/* OUTPUT =            Device Status screen(s)                               */
/*                                                                           */
/* NORMAL RETURN =     0 to indicate successful completion                   */
/*                                                                           */
/* ERROR RETURNS =     -1 to indicate no hardware exerciser programs         */
/*                         currently in memory.                              */
/*                     -2 to indicate an unsuccessful attempt to allocate    */
/*                         memory for the device status table sort.          */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = display_scn -  displays screen image                  */
/*                     init_dst_tbl - initializes and sorts the list of      */
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

int  disp_dst(void)
{
  char status[3];    /* HE status.                        */
  char workstr[128];  /* work string                       */
  char result_msg[80];

  extern char level[];  /* HTX and AIx levels.               */

  extern int COLS;  /* number of cols in CURSES screen   */
  extern int LINES;  /* number of lines in CURSES screen  */
  extern WINDOW *stdscr;  /* standard screen window            */

  int col = 0;    /* first column displayed on screen  */
  int errno_save;    /* errno save area. */
  int error = 0;    /* error flag (1 = not ack'ed)       */
  int i;      /* loop counter                      */
  int j;      /* loop counter                      */
  int key_input;    /* input from keyboard               */
  int max_page;    /* max dst page number               */
  int num_disp;    /* number of dst entries to show     */
  int num_entries = 0;  /* local number of shm HE entries    */
  int page = 1;    /* current dst page                  */
  int prev_page = 1;
  int row = 0;    /* first row displayed on screen     */
  int min_cycles_done = 0x7fffffff;
  int max_cycles_done = 0, ch_no = 0;
   char fno[4] = "";
   int fno1, sorry = 0;
   int curr_num, old_page=1;

  static int dst_beep = 1;  /* beep on error flag (0=no, 1=yes)  */

  struct tm *p_tm;  /* pointer to tm structure           */
  struct timeval tv;
  fd_set read_fds;
  extern int sockfd;

  /*
   * find out how many entries we need to display
   */
  if (info_rcv.cur_info.max_entries == 0) {
    CLRLIN(MSGLINE, 0);
    PRTMSG(MSGLINE, 0, ("There are no Hardware Exerciser programs defined in the ecg file."));
    //getch();
    return (-1);
  }

  else {
    num_entries = info_rcv.cur_info.num_entries;
    if (num_entries <= 0) {
      return (num_entries);
    }
    
    else {
      max_page = ((num_entries - 1) / 34) + 1;
    }    /* endif */
  }      /* endif */

  /*
   * loop until operator says to quit
   */
  for (;;){
    clear();
    refresh();
    display_scn(stdscr, 0, 0, LINES, COLS, "dst_scn", 1, &row, &col, 23, 81, 'n');
    num_entries = info_rcv.cur_info.num_entries;

    sprintf(workstr, " %s ",info_rcv.sys_hdr_info.str_curr_time);
#ifdef  __HTX_LINUX__
    wcolorout(stdscr, GREEN_BLACK | NORMAL);
#else
    wcolorout(stdscr, NORMAL | F_GREEN | B_BLACK);
#endif
     mvwaddstr(stdscr, 0, 55, workstr);
     //mvwaddstr(stdscr, 0, 57, workstr);

/********* level ******/
    #ifdef  __HTX_LINUX__
      wcolorout(stdscr, GREEN_BLACK | BOLD| NORMAL);
    #else
      wcolorout(stdscr, NORMAL | F_GREEN | B_BLACK);
    #endif
    mvwaddstr(stdscr,2,0,level);
/********* level ******/

    sprintf(workstr, " %s ", info_rcv.cur_info.str_start_time);
#ifdef  __HTX_LINUX__
    wcolorout(stdscr, GREEN_BLACK | NORMAL);
#else
    wcolorout(stdscr, NORMAL | F_GREEN | B_BLACK);
#endif
    mvwaddstr(stdscr, 1, 55, workstr);
    wmove(stdscr, 5, 38);

    sprintf(workstr, "ecg = %s (Total Devices = %d)",basename(ecg_name), num_entries);
#ifdef  __HTX_LINUX__
    wcolorout(stdscr, RED_BLACK | NORMAL);
#else
    wcolorout(stdscr, NORMAL | F_RED | B_BLACK|UNDERSCORE);
#endif
    mvwaddstr(stdscr, 2, 24, workstr);

    sprintf(workstr, "5.%d", page);
    mvwaddstr(stdscr, 22, 74, "     ");
    mvwaddstr(stdscr, 23, 73, workstr);

    min_cycles_done = 0x7fffffff;
    max_cycles_done = 0;

    if (glbl_cycles_flag) {
      sprintf(workstr, "Cycle Curr. ");
      mvwaddstr(stdscr, 4, 26, workstr);
      mvwaddstr(stdscr, 4, 68, workstr);
      sprintf(workstr, "Count Stanza");
#ifdef  __HTX_LINUX__
      wcolorout(stdscr, GREEN_BLACK | BOLD | UNDERSCORE);
#else
      wcolorout(stdscr, F_GREEN | B_BLACK | BOLD | UNDERSCORE);
#endif
      mvwaddstr(stdscr, 5, 26, workstr);
#ifdef  __HTX_LINUX__
      wcolorout(stdscr, GREEN_BLACK | BOLD | UNDERSCORE);
#else
      wcolorout(stdscr, F_GREEN | B_BLACK | BOLD | UNDERSCORE);
#endif
      mvwaddstr(stdscr, 5, 68, workstr);
    }
    
    else {
      sprintf(workstr, "Last Error");
      mvwaddstr(stdscr, 4, 26, workstr);
      mvwaddstr(stdscr, 4, 68, workstr);
      sprintf(workstr, "Day   Time");
#ifdef  __HTX_LINUX__
      wcolorout(stdscr, GREEN_BLACK | BOLD | UNDERSCORE);
#else
      wcolorout(stdscr, F_GREEN | B_BLACK | BOLD | UNDERSCORE);
#endif
      mvwaddstr(stdscr, 5, 26, workstr);
#ifdef  __HTX_LINUX__
      wcolorout(stdscr, GREEN_BLACK | BOLD | UNDERSCORE);
#else
      wcolorout(stdscr, F_GREEN | B_BLACK | BOLD | UNDERSCORE);
#endif
      mvwaddstr(stdscr, 5, 68, workstr);
    }

    /*
     * build screen data for the current page
     */

    num_disp = num_entries - ((page - 1) * 34);

    if (num_disp > 34)  {
      num_disp = 34;
    }

    for (i = 1; i <= 2; i++) {
      for (j = 1; j <= 17; j++) {

        if ((((i - 1) * 17) + j) <= num_disp) {  /* something there?       */
          
          strcpy(status, info_rcv.scn_num.scn_5_info[((i - 1) * 17) + j -1].status);

          wmove(stdscr, (j + 5), ((i - 1) * 42));
          if ((strcmp(status, "DD") == 0) || (strcmp(status, "HG") == 0) || (strcmp(status, "ER") == 0)||
(strcmp(status,"PR")==0)) {
#ifdef  __HTX_LINUX__
            wcolorout(stdscr, BLACK_RED | BOLD | STANDOUT);
#else
            wcolorout(stdscr, STANDOUT | F_BLACK | B_RED | BOLD);
#endif
          }

          else {
#ifdef  __HTX_LINUX__
            wcolorout(stdscr, GREEN_BLACK | NORMAL);
#else
            wcolorout(stdscr, NORMAL | F_GREEN | B_BLACK);
#endif
          }

          waddstr(stdscr, status);
          wmove(stdscr, (j + 5), (((i - 1) * 42) + 3));
          if (strcmp(status, "HG") == 0) {
#ifdef  __HTX_LINUX__
            wcolorout(stdscr, BLACK_RED | STANDOUT | BOLD);
#else
            wcolorout(stdscr, STANDOUT | F_BLACK | B_RED | BOLD);
#endif
          }
          
          else {
#ifdef  __HTX_LINUX__
            wcolorout(stdscr, GREEN_BLACK | NORMAL);
#else
            wcolorout(stdscr, NORMAL | F_GREEN | B_BLACK);
#endif
          }
          waddstr(stdscr, info_rcv.scn_num.scn_5_info[((i - 1) * 17) + j -1].sdev_id);

          /*
           * time of last update
           */
          wmove(stdscr, (j + 5), (((i - 1) * 42) + 12));
          /*if(((strcmp(status, "ST") == 0) || (strcmp(status, "TM") == 0)) && (info_rcv.scn_num.scn_5_info[((i - 1) * 17) + j -1].tm_last_upd != 0)) {
#ifdef  __HTX_LINUX__
            wcolorout(stdscr, BLACK_RED | STANDOUT | BOLD);
#else
            wcolorout(stdscr, STANDOUT | F_BLACK | B_RED | BOLD);
#endif
          }
          
          else {
#ifdef  __HTX_LINUX__
            wcolorout(stdscr, GREEN_BLACK | NORMAL);
#else
            wcolorout(stdscr, NORMAL | F_GREEN | B_BLACK);
#endif
          }*/

            //wmove(stdscr, (j + 5), (((i - 1) * 42) + 12));
            sprintf(workstr, "%s", info_rcv.scn_num.scn_5_info[((i - 1) * 17) + j -1].upd_time);
            waddstr(stdscr, workstr);

          /*
           * time of last error or cycle count
           */
          if (glbl_cycles_flag) {
            sprintf(workstr, "%9d %4d", info_rcv.scn_num.scn_5_info[((i - 1) * 17) + j -1].cycles, info_rcv.scn_num.scn_5_info[((i - 1) * 17) + j -1].test_id);
            if (info_rcv.scn_num.scn_5_info[((i - 1) * 17) + j -1].test_id == 0) {
              workstr[13] = '-';
            }  /* endif */
            
#ifdef  __HTX_LINUX__
            wcolorout(stdscr, GREEN_BLACK | NORMAL);
#else
            wcolorout(stdscr, NORMAL | F_GREEN | B_BLACK);
#endif
            waddstr(stdscr, workstr);
          }
          
          else { //if (info_rcv.scn_num.scn_5_info[((i - 1) * 17) + j -1].tm_last_err != 0) {
            wmove(stdscr, (j + 5), (((i - 1) * 42) + 26));
            if ((info_rcv.scn_num.scn_5_info[((i - 1) * 17) + j -1].err_ack == 0) || (strcmp(status, "ER") == 0)) {
#ifdef  __HTX_LINUX__
              wcolorout(stdscr, BLACK_RED | BOLD | STANDOUT);
#else
              wcolorout(stdscr, STANDOUT | F_BLACK | B_RED | BOLD);
#endif
            }
            
            else {
#ifdef  __HTX_LINUX__
              wcolorout(stdscr, GREEN_BLACK | NORMAL);
#else
              wcolorout(stdscr, NORMAL | F_GREEN | B_BLACK);
#endif
            }

            sprintf(workstr, "%s",info_rcv.scn_num.scn_5_info[((i - 1) * 17) + j -1].err_time);
            waddstr(stdscr, workstr);
          }
        }

        //p_into_table++;
      }  /* endfor */
    }    /* endfor */

    sprintf(workstr, "Cycle Count(Min/Max)=%d/%d", info_rcv.cur_info.min_cycles_done, info_rcv.cur_info.max_cycles_done);
#ifdef  __HTX_LINUX__
    wcolorout(stdscr, GREEN_BLACK | NORMAL);
#else
    wcolorout(stdscr, NORMAL | F_GREEN | B_BLACK);
#endif
    mvwaddstr(stdscr, 1, 0, workstr);

    /*
     * check for unack'ed errors
     */

    error = error|info_rcv.cur_info.error_flag;

    wmove(stdscr, 0, 0);

    if (error != 0) {  /* un-ack'ed error? */
#ifdef  __HTX_LINUX__
      wcolorout(stdscr, BLACK_RED | BOLD | STANDOUT);
#else
      wcolorout(stdscr, STANDOUT | F_BLACK | B_RED | BOLD);
#endif
      waddstr(stdscr, "*** ERROR ***");
      if (dst_beep != 0) {  /* beep on error? */
        beep();
      }  /* endif */
    }
    
    else {  /* no un-ack'ed error */
#ifdef  __HTX_LINUX__
      wcolorout(stdscr, GREEN_BLACK | NORMAL);
#else
      wcolorout(stdscr, NORMAL | F_GREEN | B_BLACK);
#endif
      waddstr(stdscr, "             ");
    }    /* endif */


    /*
     * update screen
     */

    wmove(stdscr, MSGLINE, 0);
#ifdef  __HTX_LINUX__
    wcolorout(stdscr, GREEN_BLACK | NORMAL);
#else
    wcolorout(stdscr, NORMAL | F_GREEN | B_BLACK);
#endif
    if (wrefresh(stdscr) == -1) {
      errno_save = errno;
      PRTMSG(MSGLINE, 0, ("Error on wrefresh().  errno = %d.",errno_save));
      sleep((unsigned) 10);
    }    /* endif */

    if (fflush(stdout) == -1) {
      errno_save = errno;
      PRTMSG(MSGLINE, 0, ("Error on fflush(stdout).  errno = %d.", errno_save));
      sleep((unsigned) 10);
      clearerr(stdout);  /* clear error flag on stdout stream */
    }    /* endif */

    /*
     * read input from keyboard
     */
    FD_ZERO(&read_fds);
    FD_SET(STDIN,&read_fds);
    FD_SET(sockfd,&read_fds);
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    select(sockfd+1,&read_fds,NULL,NULL,&tv);
    if (FD_ISSET(STDIN, &read_fds)) {
    //if((poll_rc = poll(poll_fd, (unsigned long) 1, (long) 10000)) > 0) {
      sorry = 0;
      fno[0] = '\0';
      fno1 = 0;
      ch_no = 0;
      key_input = getch();
      CLRLIN(MSGLINE, 0);

      switch (key_input) {
        case KEY_NEWL:
        case ' ':
          break;
        case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
         fno[ch_no] = key_input;
         ch_no++;
         while (key_input = getch ()) {
            if (key_input == '1' || key_input == '2' || key_input == '3'
                || key_input == '4' || key_input == '5' || key_input == '6'
                || key_input == '7' || key_input == '8' || key_input == '9'
                || key_input == '0') {
               fno[ch_no] = key_input;
               ch_no++;
            }
            else if (key_input == 'f' || key_input == 'F'
                     || key_input == 'b' || key_input == 'B') {
               fno[ch_no] = '\0';
               break;
            }
            else {
               beep ();
               sorry = 1;
               PRTMSG (MSGLINE, 0,
                       ("Sorry, that key is not valid with this screen."));
               break;
            }
         }
         if (sorry)
            break;
         fno1 = atoi (fno);
         //printf("fno1 = :%d: max = :%d:\n", fno1, max_page);fflush(stdout);getch();
         if (key_input == 'f' || key_input == 'F') {
            curr_num = page * 34;
            fno1 = (curr_num + (fno1 * 34)) / 34;
            if (fno1 < max_page) {
	       prev_page = page;
               page = fno1;
               //send_sockmsg (SCREEN_5, 1, page, ecg_name, result_msg);
               //clear ();
               //refresh ();
               break;

            }
            else {
                 prev_page = page;
               page = max_page;
               //send_sockmsg (SCREEN_5, 1, page, ecg_name, result_msg);
               //clear ();
               //refresh ();
               break;
            }
         }
         else if (key_input == 'b' || key_input == 'B') {
            curr_num = page * 34;
            fno1 = (curr_num - (fno1 * 34)) / 34;
            if (fno1 > 1) {
	       prev_page = page;
               page = fno1;
               //send_sockmsg (SCREEN_5, 1, page, ecg_name, result_msg);
               //clear ();
               //refresh ();
               break;
            }
            else {
		prev_page = page;
               page = 1;
               //send_sockmsg (SCREEN_5, 1, page, ecg_name, result_msg);
               //clear ();
               //refresh ();
               break;
            }

         }
         break;
        case 'f':
        case 'F':
          if (page < max_page)  {
		prev_page = page;
            page++;
          }
          else  {
            beep();
          }
          //send_sockmsg(SCREEN_5,1,page,ecg_name,result_msg);
          //clear();
          //refresh();
          break;

        case 'b':
        case 'B':
          if (page > 1)  {
		prev_page = page;
            page--;
          }
          else  {
            beep();
          }
          //send_sockmsg(SCREEN_5,1,page,ecg_name,result_msg);
          //clear();
          //refresh();
          break;
	 case 'u':
         case 'U':
                clear();
                refresh();
		prev_page = page;
                page = 1;
                break;
         case KEY_F(8):
                if (page < max_page)  {
			prev_page = page;
                   	page++;
                   }
                else  {
            	beep();
          	 }
          	//send_sockmsg(SCREEN_5,1,page,ecg_name,result_msg);
          	//clear();
         	//refresh();
          	break;
	 case KEY_F(7):
		if (page > 1)  {
		prev_page = page;
            	page--;
          	}
          	else  {
            	beep();
          	}
          	//send_sockmsg(SCREEN_5,1,page,ecg_name,result_msg);
          	//clear();
          	//refresh();
         	 break;
	 case KEY_F(3):
                if (prev_page == page)
                page = old_page;
                else {
                old_page = page;
                page = prev_page;
                }
 		//send_sockmsg(SCREEN_5,1,page,ecg_name,result_msg);
		break;
         case 'l':
         case 'L':
                 clear();
                refresh();
                page = max_page;
                break;

        case 'e':
        case 'E':
#ifndef __HTX_LINUX__
          erpt();
          clear();
          refresh();
#else

        PRTMSG(MSGLINE, 0, ("Sorry, This option is currently not supported in LINUX"));
#endif
          break;

        case 'h':
        case 'H':
          help(16, 50, 2, 2, "ddhelp_scn", 8);
          clear();
          break;

        case 'r':
        case 'R':
          //send_sockmsg(SCREEN_5,1,page,ecg_name,result_msg);
          //clear();
          //refresh();
          break;

        case 'q':
        case 'Q':
          return (0);

        case 'a':
        case 'A':
          error = 0;  /* clear error flag */
          send_sockmsg(SCREEN_5,'a',page,ecg_name,result_msg);
          break;

        case 'c':
        case 'C':
          if (glbl_cycles_flag)  {
            glbl_cycles_flag = 0;
          }
          else  {
            glbl_cycles_flag = 1;
          }
          break;

        case 's':
        case 'S':
          if (dst_beep != 0) {  /* currently set to beep? */
            dst_beep = 0;  /* turn beep off. */
            PRTMSG(MSGLINE, 0, ("Beeping turned off by operator request."));
          }
          
          else {  /* currently set for NO beep */
            dst_beep = 1; /* turn beep on.*/
            PRTMSG(MSGLINE, 0, ("Beeping turned on by operator request."));
          }  /* endif */
          break;

#ifndef  __HTX_LINUX__
        case KEY_SLL:  /* Locator select (mouse buttons) */
        case KEY_LOCESC:  /* Locator report following...    */
          fflush(stdin);  /* Discard following locator data in the input buffer. */
          break;

#endif
        default:
          beep();
          PRTMSG(MSGLINE, 0, ("Sorry, that key is not valid with this screen."));
          break;
      }  /* endswitch */
    //}

    }
    else if (FD_ISSET(sockfd, &read_fds)) {
      if (recv(sockfd,&notify_rcv,100,MSG_WAITALL) == -1) {
         printf("Error receiving command from the server\n");
      }
      PRTMSG(MSGLINE, 0, ("%s.",notify_rcv.msg));
      //getch();
      //CLRLIN(MSGLINE, 0);
    }
    //printf("Sending again\n");
    //fflush(stdout);
    send_sockmsg(SCREEN_5,1,page,ecg_name,result_msg);

    /* endif */
  }      /* endfor */
 
}  /* disp_dst() */
