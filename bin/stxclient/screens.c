
/* @(#)53	1.12  src/htx/usr/lpp/htx/bin/stxclient/screens.c, eserv_gui, htxubuntu 5/24/04 17:08:22 */

/*
 *   FUNCTIONS: display_scn
 *    htx_scn
 *    mmenu_scn
 */


/*****************************************************************************/
/*                                                                           */
/* MODULE NAME    =    screens.c                                             */
/* COMPONENT NAME =    hxssup (supervisor)                                   */
/* LPP NAME       =    HTX                                                   */
/*                                                                           */
/* DESCRIPTIVE NAME =  Display Screen Functions                              */
/*                                                                           */
/* COPYRIGHT =         (C) COPYRIGHT IBM CORPORATION 1988                    */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM           */
/*                                                                           */
/* STATUS =            Release 1 Version 0                                   */
/*                                                                           */
/* FUNCTION =          Miscellaneous Display Screen Functions                */
/*            display_scn - displays all or a portion of a given screen data */
/*                file.                                                      */
/*            htx_scn - displays the HTX logo screen.                        */
/*            mmenu_scn - displays the HTX main menu screens.                */
/*                                                                           */
/*                                                                           */
/* COMPILER OPTIONS =  -I/src/master/htx/common -g -Nn3000 -Nd4000 -Np1200   */
/*                     -Nt2000                                               */
/*                                                                           */
/* CHANGE ACTIVITY =                                                         */
/*    DATE    :LEVEL:PROGRAMMER:DESCRIPTION                                  */
/*    MMMMMMMMNMMMMMNMMMMMMMMMMNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM */
/*    06/17/88:1.0  :J BURGESS :INITIAL RELEASE                              */
/*    01/18/00:1.16 :R GEBHARDT:Feature 290676 Add/Terminate Exercisers      */
/*            :     :          :                                             */
/*                                                                           */
/*****************************************************************************/

#include "hxssup.h"
#include "scr_info.h"

#ifdef  __HTX_LINUX__
#include <wchar.h>
typedef  wchar_t NLSCHAR;
#endif

/*struct htxmessage {
int com;
int subcom;
int loc;
char str[80];
};*/
extern char ecg_name[20];

extern tsys_hdr sys_hdr_in;
extern tscn_1 scn_1_in;
extern tscn_5   scn_5_in[];



/*****************************************************************************/
/*****  d i s p l a y _ s c n ( )  *******************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     display_scn()                                         */
/*                                                                           */
/* DESCRIPTIVE NAME =  Display a screen                                      */
/*                                                                           */
/* FUNCTION =          Based on the parameters passed from the calling       */
/*                     function, display all or some portion of the passed   */
/*                     screen data file.                                     */
/*                                                                           */
/* INPUT =             winscr - CURSES window for display.                   */
/*                     wrow - Physical place on the display screen where the */
/*                         CURSES window, winscr, begins.                    */
/*                     wcol - Physical place on the display screen where the */
/*                         CURSES window, winscr, begins.                    */
/*                     wmaxrow - Number of rows on the CURSES window, winscr.*/
/*                     wmaxcol - Number of cols on the CURSES window, winscr.*/
/*                                                                           */
/*                     scn_file - pointer to the name of the screen data     */
/*                         file.                                             */
/*                     page - page number in the screen file to be shown.    */
/*                     arow - beginning row in the screen file to be shown.  */
/*                     acol - beginning col in the screen file to be shown.  */
/*                     amaxrow - number of rows per page in the screen file. */
/*                     amaxcol - number of cols per page in the screen file. */
/*                     refresh - logical variable:                           */
/*                                 'y' == refresh the screen when done.      */
/*                                 'n' == do not refresh.                    */
/*                                                                           */
/* OUTPUT =            All or some portion of the screen data file           */
/*                     displayed on the screen.                              */
/*                                                                           */
/* NORMAL RETURN =     0 - Successful completion.                            */
/*                                                                           */
/* ERROR RETURNS =     10 - Invalid wrow (beginning row of window) value.    */
/*                     11 - Invalid wcol (beginning column of window) value. */
/*                     12 - Invalid wmaxrow (last row of window) value.      */
/*                     13 - Invalid wmaxcol (last column of window) value.   */
/*                                                                           */
/*                     20 - Unable to access scn_file file (character file). */
/*                     21 - Unable to access scn_file.a file (attribute      */
/*                          file).                                           */
/*                                                                           */
/*                     30 - Invalid data/format of scn_file file (character  */
/*                          file).                                           */
/*                     31 - Invalid data/format of scn_file.a file           */
/*                          (attribute file).                                */
/*                                                                           */
/*                     40 - Unable to allocate scn_file buffer.              */
/*                     41 - Unable to allocate scn.a_file buffer.            */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    DATA AREAS =     LINES - number of lines on CURSES screen.             */
/*                     COLS  - number of columns on CURSES screen.           */
/*                                                                           */
/*****************************************************************************/

int display_scn(WINDOW * winscr, int wrow, int wcol, int wmaxrow,
    int wmaxcol, char *scn_file, int page, int *arow,
    int *acol, int amaxrow, int amaxcol, char refrsh)

     /* WINDOW *         winscr;        -- window specification              */
     /* int              wrow;          -- row where the window starts       */
     /* int              wcol;          -- col where the window starts       */
     /* int              wmaxrow;       -- number of rows in the window      */
     /* int              wmaxcol;       -- number of columns in the window   */
     /* char *           scn_file;      -- screen character file             */
     /* int              page;          -- page number in file to be shown   */
     /* int *            arow;          -- beginning row to be shown         */
     /* int *            acol;          -- beginning column to be shown      */
     /* int              amaxrow;       -- number of rows per page in file   */
     /* int              amaxcol;       -- number of cols per page in file   */
     /* char             refrsh;        -- logical refresh screen variable   */
{

  extern char level[];
  char *scn_array;  /* pointer to scn_file char buffer   */
  char *scna_array;  /* pointer to .a file attr buffer    */
  char last_mode;    /* last known screen mode            */
  char workstr[128];  /* work string                       */

/*  extern char HTXPATH[];   HTX file system path              */
  extern char HTXSCREENS[];  /* HTX file system path             */
  
  extern int COLS;  /* number of cols in CURSES screen   */
  extern int LINES;  /* number of lines in CURSES screen  */

  int disp_cols;    /* number of cols to display         */
  int disp_rows;    /* number of rows to display         */
  int i;      /* loop counter                      */
  int j;      /* loop counter                      */
  int scn_id;    /* file id for scn_file (char. file) */
  int scna_id;    /* file id for .a file (attributes)  */

  off_t fileptr;    /* pointer into a file (seek)        */

  /*
   * check window parameters
   */
  if ((wrow < 0) || (wrow > (LINES - 5)))  {
    return (10);
  }
  if ((wcol < 0) || (wcol > (COLS - 5)))  {
    return (11);
  }
  if ((wmaxrow < 5) || (wmaxrow > (LINES - wrow)))  {
    return (12);
  }
  if ((wmaxcol < 5) || (wmaxcol > (COLS - wcol)))  {
    return (13);
  }

  /*
   * try to open scn_file (character file)
   */

  /* remove hard coded paths */
  /*
      strcpy(workstr, HTXPATH);
      strcat(workstr, "/screens_stx/");
   */

//  strcpy(HTXSCREENS, getenv("STXSCREENS"));
//  if(strlen(HTXSCREENS) == 0)  {
    strcpy(HTXSCREENS, "/usr/lpp/htx/etc/screens_stx/");
//  }

         strcpy(workstr, HTXSCREENS);
  strcat(workstr, scn_file);
  if ((scn_id = open(workstr, O_RDONLY)) == -1)  {
    return (20);
  }

  /*
   * try to open scn_file.a (attribute file)
   */
  strcat(workstr, ".a");
  if ((scna_id = open(workstr, O_RDONLY)) == -1) {
    close(scn_id);
    return (21);
  }  /* endif */

  /*
   * check *arow value and adjust if necessary
   */
  if ((amaxrow - *arow) < (wmaxrow - 1)) {
    if (amaxrow > wmaxrow)  {
      *arow = amaxrow - wmaxrow;
    }
    else  {
      *arow = 0;
    }
  }  /* endif */

  /*
   * check *column value and adjust if necessary
   */

  if (((amaxcol - 1) - *acol) < (wmaxcol - 1)) {
    if ((amaxcol - 1) > wmaxcol)  {
      *acol = (amaxcol - 1) - wmaxcol;
    }
    else  {
      *acol = 0;
    }
  }  /* endif */

  /*
   * Go to proper starting point in the char and attr files.
   */
  fileptr = (off_t) (((page - 1) * (amaxrow * amaxcol)) + (*arow * amaxcol) + *acol);
  if (lseek(scn_id, fileptr, SEEK_SET) == (off_t) - 1) {
    close(scn_id);
    close(scna_id);
    return (30);
  }  /* endif */

  if (lseek(scna_id, fileptr, SEEK_SET) == (off_t) - 1) {
    close(scn_id);
    close(scna_id);
    return (31);
  }  /* endif */

  /*
   * set the number of columns to be displayed for the given screen
   */
  if ((amaxcol - 1) < wmaxcol)  {
    disp_cols = amaxcol - 1;
  }
  else  {
    disp_cols = wmaxcol;
  }

  /*
   * set the number of rows to be displayed for the given screen
   */
  if (amaxrow < wmaxrow)  {
    disp_rows = amaxrow;
  }
  else  {
    disp_rows = wmaxrow;
  }

  /*
   * allocate character arrays for both the character and attribute files
   */
  if ((scn_array = (char *) calloc((size_t) amaxcol, (size_t) sizeof(char))) == NULL) {
    close(scn_id);
    close(scna_id);
    return (40);
  }  /* endif */

  if ((scna_array = (char *) calloc((size_t) amaxcol, (size_t) sizeof(char))) == NULL) {
    close(scn_id);
    close(scna_id);
    free(scn_array);
    return (41);
  }  /* endif */

  /*
   * force the first call to colorout()
   */
  last_mode = (char) 0xff;

  /*
   * display the screen
   */
  if (strlen(level) >= 20)
     level[20] = '\0';
  else 
     level[strlen(level)] = '\0';

  //printf("12 %c 13 = %c 14 = %c 21=%c len = %d \n",level[12], level[13], level[14], level[21],strlen(level));
  //mvwaddstr(stdscr, 1, 3, level,strlen(level));
  mvwaddstr(stdscr, 1, 3, level);

  for (i = *arow; i < disp_rows; i++) {
    if (read(scn_id, scn_array, (unsigned int) amaxcol) !=  amaxcol) {
      close(scn_id);
      close(scna_id);
      free(scn_array);
      free(scna_array);
      return (30);
    }

    if (read(scna_id, scna_array, (unsigned int) amaxcol) !=  amaxcol) {
      close(scn_id);
      close(scna_id);
      free(scn_array);
      free(scna_array);
      return (31);
    }

    wmove(winscr, i, 0);
    for (j = *acol; j < disp_cols; j++) {
      if (*(scna_array + j) != last_mode) {
        switch (*(scna_array + j)) {

#ifdef  __HTX_LINUX__
          case '0':
                   wcolorout(winscr, NORMAL);
            break;

          case '1':
            wcolorout(winscr, BLACK_BLUE | NORMAL);
            break;

          case '2':
                   wcolorout(winscr, BLACK_BLUE | NORMAL | BOLD | UNDERSCORE);
            break;

          case '3':
               wcolorout(winscr, BLACK_RED | NORMAL | BOLD);
            break;

          case '4':
               wcolorout(winscr, BLUE_BLACK | NORMAL);
            break;

          case '5':
                   wcolorout(winscr, BLUE_BLACK | NORMAL | BOLD | UNDERSCORE);
            break;

          case '6':
               wcolorout(winscr, GREEN_BLACK | NORMAL);
            break;

          case '7':
                   wcolorout(winscr, GREEN_BLACK | BOLD | UNDERSCORE | NORMAL);
            break;

          case '8':
               wcolorout(winscr, RED_BLACK | NORMAL);
            break;

          case '9':
                   wcolorout(winscr, RED_BLUE | BOLD | UNDERSCORE | NORMAL);
            break;

          case 'A':
               wcolorout(winscr, WHITE_BLUE | NORMAL);
            break;

          case 'a':
               wcolorout(winscr, BLACK_RED | STANDOUT);
            break;

          case 'b':
               wcolorout(winscr, BLACK_RED | BOLD | STANDOUT);
            break;

          case 'c':
                   wcolorout(winscr, BLACK_RED | BOLD | UNDERSCORE | STANDOUT);
            break;

          default:
                   wcolorout(winscr, NORMAL);
            break;


#else
          case '0':
                   wcolorout(winscr, NORMAL);
            break;

          case '1':
            wcolorout(winscr, NORMAL | F_BLACK | B_BLUE);
            break;

          case '2':
                   wcolorout(winscr, NORMAL | F_BLACK | B_BLUE | BOLD | UNDERSCORE);
            break;

          case '3':
               wcolorout(winscr, NORMAL | F_BLACK | B_RED | BOLD);
            break;

          case '4':
               wcolorout(winscr, NORMAL | F_BLUE | B_BLACK);
            break;

          case '5':
                   wcolorout(winscr, NORMAL | F_BLUE | B_BLACK | BOLD | UNDERSCORE);
            break;

          case '6':
               wcolorout(winscr, NORMAL | F_GREEN | B_BLACK);
            break;

          case '7':
                   wcolorout(winscr, NORMAL | F_GREEN | B_BLACK | BOLD | UNDERSCORE);
            break;

          case '8':
               wcolorout(winscr, NORMAL | F_RED | B_BLACK);
            break;

          case '9':
                   wcolorout(winscr, NORMAL | F_RED | B_BLUE | BOLD | UNDERSCORE);
            break;

          case 'A':
               wcolorout(winscr, NORMAL | F_WHITE | B_BLUE);
            break;

          case 'a':
               wcolorout(winscr, STANDOUT | F_BLACK | B_RED);
            break;

          case 'b':
               wcolorout(winscr, STANDOUT | F_BLACK | B_RED | BOLD);
            break;

          case 'c':
                   wcolorout(winscr, STANDOUT | F_BLACK | B_RED | BOLD | UNDERSCORE);
            break;

          default:
                   wcolorout(winscr, NORMAL);
            break;
#endif
        }  /* endswitch */

        last_mode = *(scna_array + j);
      }    /* endif */

      waddch(winscr, (NLSCHAR) * (scn_array + j));
    }    /* endfor */
  }      /* endfor */

  //mvwaddnstr(stdscr, 1, 3, level,strlen(level));
  mvwaddstr(stdscr, 1, 3, level);
  wmove(winscr, i, 0);
#ifdef  __HTX_LINUX__
  wcolorout(winscr, NORMAL);
#else
  wcolorout(winscr, NORMAL);
#endif
  if ((refrsh == 'y') || (refrsh == 'Y'))  {
    wrefresh(winscr);
  }

  close(scn_id);
  close(scna_id);
  free(scn_array);
  free(scna_array);

  return (0);
}        /* display_scn() */


/*****************************************************************************/
/*****  h t x _ s c n ( )  ***************************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     htx_scn()                                             */
/*                                                                           */
/* DESCRIPTIVE NAME =  Display the HTX logo screen.                          */
/*                                                                           */
/* FUNCTION =          This function displays the HTX logo screen via the    */
/*                     display_scn() function.                               */
/*                                                                           */
/* INPUT =             None.                                                 */
/*                                                                           */
/* OUTPUT =            The displayed HTX logo screen.                        */
/*                     level_str - external string which has the HTX and LINUX */
/*                                 levels.                                   */
/*                                                                           */
/* NORMAL RETURN =     None - void function.                                 */
/*                                                                           */
/* ERROR RETURNS =     None - void function.                                 */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = display_scn - the display screen function.            */
/*                                                                           */
/*    DATA AREAS =     LINES - number of lines on CURSES screen.             */
/*                     COLS  - number of columns on CURSES screen.           */
/*                     level_str - contains LINUX and HTX levels.              */
/*                                                                           */
/*****************************************************************************/

void  htx_scn(void)
{
  extern char level[];  /* HTX and LINUX levels. */
  extern int COLS;  /* number of cols in CURSES screen   */
  extern int LINES;  /* number of lines in CURSES screen  */
  extern WINDOW *stdscr;  /* standard screen window            */

  int column = 0;    /* column number                     */
  //int fileid = 0;    /* file id.                          */
  int row = 0;    /* row number                        */


  display_scn(stdscr, 0, 0, LINES, COLS, "htx_scn", 1, &row, &column, 23, 81, 'n');


  /*
   * write LINUX and HTX levels on htx screen and refresh
   */
 // mvwaddstr(stdscr, 21, 1, "       ");
  //mvwaddstr(stdscr, 21, 1, level);
  wrefresh(stdscr);

  return;
}        /* htx_scn */


/*****************************************************************************/
/*****  m m e n u _ s c n ( )  ***********************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     mmenu_scn()                                           */
/*                                                                           */
/* DESCRIPTIVE NAME =  Display the HTX main menu screen.                     */
/*                                                                           */
/* FUNCTION =          This function displays the HTX main menu screen via   */
/*                     the display_scn() function.                           */
/*                                                                           */
/* INPUT =             page - indicates which page of the main menu to show. */
/*                                                                           */
/* OUTPUT =            The displayed HTX main menu screen.                   */
/*                                                                           */
/* NORMAL RETURN =     None - void function.                                 */
/*                                                                           */
/* ERROR RETURNS =     None - void function.                                 */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = display_scn - the display screen function.            */
/*                                                                           */
/*    DATA AREAS =     LINES - number of lines on CURSES screen.             */
/*                     COLS  - number of columns on CURSES screen.           */
/*                                                                           */
/*                     level_str - HTX and LINUX levels.                       */
/*                     semhe_id - main htx system semaphore id.              */
/*                     shm_addr - shared memory address.                     */
/*                     stdscr - standard CURSES screen.                      */
/*                                                                           */
/*****************************************************************************/

void  mmenu_scn(int *page)
{
  extern char level[];  /* HTX and LINUX levels                */
  extern int COLS;  /* number of cols in CURSES screen   */
  extern int LINES;  /* number of lines in CURSES screen  */
  extern WINDOW *stdscr;  /* standard screen window       */

  static int col = 0;  /* column number                     */
  static int row = 0;  /* row number                        */

  extern void end_it(int );  

  char workstr[128];  /* work string                       */
  char file_ecgname[56], *filename_ptr;  /*the ecg_name in the full ecg path */

  if (*page < 1)  {
    *page = 1;
  }
  if (*page > 1)  {
    *page = 1;
  }
  
  if(LINES < 24 || COLS < 80)
  {
    PRTMSG(1,0,("Increase the window resolution to 23 - 81\n"));
    sleep(2);
  }
  display_scn(stdscr, 0, 0, LINES, COLS, "mmenu_scn", *page, &row, &col, 23, 81, 'n');

  if (strlen(level) >= 20)
     level[20] = '\0';
  else
     level[strlen(level)] = '\0';

#ifdef  __HTX_LINUX__
    wcolorout(stdscr, GREEN_BLACK | BOLD| NORMAL);
#else
    wcolorout(stdscr, NORMAL | F_GREEN | B_BLUE);
#endif

  mvwaddstr(stdscr, 3, 53, "                ");
  mvwaddstr(stdscr, 4, 35, "         ");

#ifdef  __HTX_LINUX__
    wcolorout(stdscr, GREEN_BLACK | BOLD| NORMAL);
#else
    wcolorout(stdscr, NORMAL | F_GREEN | B_BLACK);
#endif

  mvwaddstr(stdscr, 1, 3, level);
  strcpy(file_ecgname, ecg_name);
  filename_ptr = basename(file_ecgname); 
  mvwaddstr(stdscr, 3, 53, filename_ptr);
  sprintf(workstr, "%s", scn_1_in.sys_hdr_info.str_curr_time);
  mvwaddstr(stdscr, 1, 61, workstr);

  //if (scn_1_in.sys_hdr_info.running_halted == 0)  // active
  //   mvwaddstr(stdscr, 4, 35, "Active");
  //else if(scn_1_in.sys_hdr_info.running_halted == 1) // suspended
  //   mvwaddstr(stdscr, 4, 35, "Suspended");          
  //else if(scn_1_in.sys_hdr_info.running_halted == 99)

  //   mvwaddstr(stdscr, 4, 35, "Inactive");
  if (strcmp(scn_1_in.sys_hdr_info.running_halted,"UNLOADED") ==0)
     strcpy(scn_1_in.sys_hdr_info.running_halted,"INACTIVE");
  mvwaddstr(stdscr, 4, 35, scn_1_in.sys_hdr_info.running_halted);

#ifdef  __HTX_LINUX__
    wcolorout(stdscr, CYAN_BLACK | BOLD | UNDERSCORE | NORMAL);
#else
    wcolorout(stdscr, NORMAL | F_CYAN | B_BLACK | BOLD | UNDERSCORE);
#endif

#ifdef  __HTX_LINUX__
  wcolorout(stdscr, NORMAL);
#else
  wcolorout(stdscr, NORMAL);
#endif

  wmove(stdscr, MSGLINE, 0);
  wrefresh(stdscr);
  return;
}        /* mmenu_scn */

