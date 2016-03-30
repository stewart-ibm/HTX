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

/*
 *   FUNCTIONS: CLRLIN
 *              PROMPT
 *              PRTMSG
 *              PRTMSS
*/

/*
****************************************************************************
*                                                                          *
*  Module Name: hxiconv.h                                                  *
*                                                                          *
*  Description: HTX convenience definitions and declarations.              *
*                                                                          *
****************************************************************************
*/

#include <stdio.h>
#include <sys/types.h>
#undef TRUE
#undef FALSE

#ifdef	__HTX_LINUX__
#include <curses.h>
#else
#include <cur01.h>
#include <cur02.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>

#include <time.h>
#ifdef HTX_REL_tu320
#include <sys/hft.h>
#endif
#include <sys/signal.h>
#include <sys/stat.h>

/*
 * We define one macro for each foreground-background color combination 
 * for use with the init_color() and wcolorout() functions. wcolorout() is
 * itself provided as a macro that calls wattrset().
 *
 * Ashutosh S. Rajekar
 */

#ifdef	__HTX_LINUX__

#define	BLACK_BLACK	COLOR_PAIR(1)
#define	BLACK_RED	COLOR_PAIR(2)
#define	BLACK_GREEN	COLOR_PAIR(3)
#define	BLACK_YELLOW	COLOR_PAIR(4)
#define	BLACK_BLUE	COLOR_PAIR(5)
#define	BLACK_MAGENTA	COLOR_PAIR(6)
#define	BLACK_CYAN	COLOR_PAIR(7)
#define	BLACK_WHITE	COLOR_PAIR(8)

#define	RED_BLACK	COLOR_PAIR(9)
#define	RED_RED		COLOR_PAIR(10)
#define	RED_GREEN	COLOR_PAIR(11)
#define	RED_YELLOW	COLOR_PAIR(12)
#define	RED_BLUE	COLOR_PAIR(13)
#define	RED_MAGENTA	COLOR_PAIR(14)
#define	RED_CYAN	COLOR_PAIR(15)
#define	RED_WHITE	COLOR_PAIR(16)

#define	GREEN_BLACK	COLOR_PAIR(17)
#define	GREEN_RED	COLOR_PAIR(18)
#define	GREEN_GREEN	COLOR_PAIR(19)
#define	GREEN_YELLOW	COLOR_PAIR(20)
#define	GREEN_BLUE	COLOR_PAIR(21)
#define	GREEN_MAGENTA	COLOR_PAIR(22)
#define	GREEN_CYAN	COLOR_PAIR(23)
#define	GREEN_WHITE	COLOR_PAIR(24)

#define	YELLOW_BLACK	COLOR_PAIR(25)
#define	YELLOW_RED	COLOR_PAIR(26)
#define	YELLOW_GREEN	COLOR_PAIR(27)
#define	YELLOW_YELLOW	COLOR_PAIR(28)
#define	YELLOW_BLUE	COLOR_PAIR(29)
#define	YELLOW_MAGENTA	COLOR_PAIR(30)
#define	YELLOW_CYAN	COLOR_PAIR(31)
#define	YELLOW_WHITE	COLOR_PAIR(32)

#define	BLUE_BLACK	COLOR_PAIR(33)
#define	BLUE_RED	COLOR_PAIR(34)
#define	BLUE_GREEN	COLOR_PAIR(35)
#define	BLUE_YELLOW	COLOR_PAIR(36)
#define	BLUE_BLUE	COLOR_PAIR(37)
#define	BLUE_MAGENTA	COLOR_PAIR(38)
#define	BLUE_CYAN	COLOR_PAIR(39)
#define	BLUE_WHITE	COLOR_PAIR(40)

#define	MAGENTA_BLACK	COLOR_PAIR(41)
#define	MAGENTA_RED	COLOR_PAIR(42)
#define	MAGENTA_GREEN	COLOR_PAIR(43)
#define	MAGENTA_YELLOW	COLOR_PAIR(44)
#define	MAGENTA_BLUE	COLOR_PAIR(45)
#define	MAGENTA_MAGENTA	COLOR_PAIR(46)
#define	MAGENTA_CYAN	COLOR_PAIR(47)
#define	MAGENTA_WHITE	COLOR_PAIR(48)

#define	CYAN_BLACK	COLOR_PAIR(49)
#define	CYAN_RED	COLOR_PAIR(50)
#define	CYAN_GREEN	COLOR_PAIR(51)
#define	CYAN_YELLOW	COLOR_PAIR(52)
#define	CYAN_BLUE	COLOR_PAIR(53)
#define	CYAN_MAGENTA	COLOR_PAIR(54)
#define	CYAN_CYAN	COLOR_PAIR(55)
#define	CYAN_WHITE	COLOR_PAIR(56)

#define	WHITE_BLACK	COLOR_PAIR(57)
#define	WHITE_RED	COLOR_PAIR(58)
#define	WHITE_GREEN	COLOR_PAIR(59)
#define	WHITE_YELLOW	COLOR_PAIR(60)
#define	WHITE_BLUE	COLOR_PAIR(61)
#define	WHITE_MAGENTA	COLOR_PAIR(62)
#define	WHITE_CYAN	COLOR_PAIR(63)
#define	WHITE_WHITE	COLOR_PAIR(64)
	
#define	wcolorout(window, attr)	wattrset(window, attr)

#endif		/* __HTX_LINUX__  */


/*
 * various mneumonics used thoughout htx
 */
#define  EXTRA_ENTRIES	(unsigned int) (0)  /* # of spare entries in htx shm */
#define  PSEUDO_EXTRA_ENTRIES	(unsigned int) (100)
#define  MSGLINE	(23)	/* htx system message line #     */

#define  SD_DEFAULT	(15)	/* time from SIGTERM to SIGKILL for fast shutdown */
#define  SLOW_SD_DEFAULT (60)	/* time from SIGTERM to SIGKILL for slow shutdown  */

#define  END_MSG	(200)	/* causes msg handler to exit    */
#define  REG_MSG	(100)	/* regular supervisor message    */

/*
 * miscellaneous key code definitions not in /usr/include/cur02.h
 */

#define  KEY_ACTION     0x281	/* Action Key                        */
#define	KEY_NEWL	'\r'

/*
 * Seperate macros defined for Linux. Here, the function keys, and also the
 * other attributes like the NORMAL, STANDALONE, etc are given new definitions.
 */

#ifdef	__HTX_LINUX__
#define  KEY_F1         KEY_F(1)	/* F1 Key                            */
#define  KEY_F2         KEY_F(2)	/* F2 Key                            */
#define  KEY_F3         KEY_F(3)	/* F3 Key                            */
#define  KEY_F4         KEY_F(4)	/* F4 Key                            */
#define  KEY_F5         KEY_F(5)	/* F5 Key                            */
#define  KEY_F6         KEY_F(6)	/* F6 Key                            */
#define  KEY_F7         KEY_F(7)	/* F7 Key                            */
#define  KEY_F8         KEY_F(8)	/* F8 Key                            */
#define  KEY_F9         KEY_F(9)	/* F9 Key                            */
#define  KEY_F10        KEY_F(10)	/* F10 Key                           */
#define  KEY_F11        KEY_F(11)	/* F11 Key                           */
#define  KEY_F12        KEY_F(12)	/* F12 Key                           */
					/* KEY_MOUSE defined in curses.h */

#define	ATTRIBUTES	WA_ATTRIBUTES
#define NORMAL		WA_NORMAL
#define STANDOUT	WA_STANDOUT
#define UNDERLINE	WA_UNDERLINE
#define UNDERSCORE	WA_UNDERLINE
#define REVERSE		WA_REVERSE
#define BLINK		WA_BLINK
#define BOLD		WA_BOLD
#define ALTCHARSET	WA_ALTCHARSET
#define INVIS		WA_INVIS
#define PROTECT		WA_PROTECT
#define HORIZONTAL	WA_HORIZONTAL
#define LEFT		WA_LEFT
#define LOW		WA_LOW
#define RIGHT		WA_RIGHT
#define TOP		WA_TOP
#define VERTICAL	WA_VERTICAL
#else		/* Not Linux */


#define  KEY_F1         0x181	/* F1 Key                            */
#define  KEY_F2         0x182	/* F2 Key                            */
#define  KEY_F3         0x183	/* F3 Key                            */
#define  KEY_F4         0x184	/* F4 Key                            */
#define  KEY_F5         0x185	/* F5 Key                            */
#define  KEY_F6         0x186	/* F6 Key                            */
#define  KEY_F7         0x187	/* F7 Key                            */
#define  KEY_F8         0x188	/* F8 Key                            */
#define  KEY_F9         0x189	/* F9 Key                            */
#define  KEY_F10        0x18a	/* F10 Key                           */
#define  KEY_F11        0x18b	/* F11 Key                           */
#define  KEY_F12        0x18c	/* F12 Key                           */
#define  KEY_MOUSE      0x289	/* signals mouse input report        */
#endif

/*
 * curses interface macros
 */
#define  CLRLIN(_j, _i)          { (void) move(_j, _i);\
                                   (void) printw("%-80s", " ");\
                                   (void) refresh(); }

#define  PRTMSG(_j, _i, _a)      { CLRLIN(_j, _i);\
                                   (void) move(_j, _i);\
                                   (void) standout();\
                                   (void) printw _a;\
                                   (void) refresh();\
                                   (void) beep();\
                                   (void) standend(); }

#define  PRTMSG1(_j, _i, _a)      { CLRLIN(_j, _i);\
                                   (void) move(_j, _i);\
                                   (void) standout();\
                                   (void) printw _a;\
                                   (void) refresh();\
                                   (void) standend(); }

#define  PRTMSS(_j, _i, _a)      { (void) move(_j, _i);\
                                   (void) printw (_a);\
                                   (void) refresh(); }

#define  PROMPT(_j, _i, _a, _b)  { CLRLIN(_j, _i);\
                                   (void) move(_j, _i);\
                                   (void) printw(_a);\
                                   (void) refresh();\
                                   _b = getch(); }

#define DEV_ID_MAX_LENGTH 40  /* defined in  hxiconv.h,hxihtx64.h, hxihtx.h  */

/*
 * Activate/Halt device table entry
 */
struct ahd_entry {
	int shm_pos;		/* shm relative position number      */
	unsigned short slot;	/* slot number of adapter            */
	unsigned short port;	/* port number of device             */
	char sdev_id[DEV_ID_MAX_LENGTH];	/* /dev/???? id                      */
};

/*
 * device status table entry
 */
struct dst_entry {
	int shm_pos;		/* shm relative position number      */
	char sdev_id[DEV_ID_MAX_LENGTH];	/* /dev/???? id                      */
};

/*
 * load sequence table entry
 */
struct load_tbl {
	int shm_pos;		/* shm relative position number      */
	unsigned short load_seq;	/* load sequence number              */
};

/*
 * enums for menu items in the Add New Device menu
 */
/*
 * values tied to menu_options array in ART_device.c, use them as indices
 */
enum t_add_method { AD_Method_Default = 4, AD_Method_Name = 3, AD_Method_File = 2 };
enum t_start_now { AD_Start_Default = 4, AD_Start_Active = 1, AD_Start_Halted = 0  };

/*
 * enums for device status as use by get_dst() in T_device.c
 */
enum t_dev_status {
	/* Unknown  */ DST_UK = 0,
	/* Complete */ DST_CP,
	/* Dead Dev */ DST_DD,
	/* Error    */ DST_ER,
	/* Hung     */ DST_HG,
	/* Running  */ DST_RN,
	/* Signalled*/ DST_SG,
	/* Stopped  */ DST_ST,
	/* Term'd   */ DST_TM,
	/* DR Term  */ DST_DT,
	/* Inactive */ DST_IN
};

