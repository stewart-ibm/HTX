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
	This file has defs that the class declarations might use. So include
  	it before the class includes
*/


#ifndef _GLOBAL_H
#define _GLOBAL_H 

//*******************************************************************
//	THIS IS AIX STUFF ONLY.............
//*******************************************************************

#define RETURN(rc) 	return(rc)
#define ERRNO   	errno

// typedefs needed 
typedef signed long  DWORD;
typedef signed long * LPDWORD;
typedef unsigned char  BYTE;

#define BASEKEY	0xeeab0000   

#define MAX_TTYS 2000
/* Used by Ltcsetattr to determine sleep time.                   */
#define TC_TCSET1               1
#define TC_TCSET2               2
#define TC_VMIN                 4
#define TC_DEFAULTS             5
  
#define EX_Abort                1
#define SLASH_CHR       '/'
#define SLASH_TXT       "/"
#define RULE_PATH   (stats->run_type[0] == 'E') ? "../rules/emc/hxeasy" : "../rules/reg/hxeasy"
#define PATTERN_PATH    "../pattern"

// note if you give timeout to read that is larger than 1000 sec the 
// readcom code takes the input to be usec not sec. 
#define READ_TIMEOUT   6      // set for 6 seconds , it seems with network 
								// attached ports it needs more time.
								// NOTE: If you set this too long the thread 
								// stays asleep waiting for read data and will 
								// not see the htx stop command in a timely 
								// manner. 
#define WRITE_TIMEOUT  6 
//Use this timeout value when timeing reads for flush and connnect port reads
//and sync reads.. And if you change it the ReadComm routine uses it to
//see if it should retry read0 on select or quit.
//  NOTE:  don't make FLUSH_TIMEOUT and READ_TIMEOUT the same or some data 
//  may not be read, As long as select is failing to notify us when read data 
//  is available. 
#define FLUSH_TIMEOUT  1
// this is not used in aix, the sems wait forever.......
#define COMM_TIMEOUT	900		// set for 15 minutes. sem event wait timeout.
#define WAIT_TIMEOUT  	0xFF00  // return value from waitevent that times out.
#define WAIT_OBJECT_0 	0xFF01  // return value from waitevent that occurred. 


#define BADBOY  -1          // must be -1...

#define HXEASY_HOSTNAMELEN      32
enum wtype {ePlug, eCable, eUnKnown, eWtError};
enum atype {e128p, e64p, e16p, e16p422, e8p, e8p422, eNative, e731, e8isa, e8422, e8med, e8med422, e128_232, e128_422, ejas, ejas_linux, ejas422, ects3_16_linux, ects3_16, ects3_16422, evcon, eAtUnknown};

#define MAX_CHARS       32767
#define FNAME_MAX       50

 
typedef struct rule_format {
   char            rule_id[9];  /* Rule Id                                    */
   char            pattern_id[9];       /* /htx/pattern_lib/xxxxxxxx          */
   int             num_oper;    /* number of operations to be performed       */
/*                               1 = default for invalid or no value          */
   char            clocal;      /* Y - ignore modem line condition.           */
/*                               N - do not ignore modem line condition       */
/*                               N = default for invalid or no value          */
   char         special[4];     /* Only used for forced read/writes.  Not     */
/*                               supported with synchronization methods.       */
/*                               should not be used for normal cable/wrap     */
/*                               tests.  Possible values are permutations of  */
/*                               "WRC".  i.e. "R", "W", "WRC", "RC"           */
   char            ign_brk;     /* Y - ignore break condition.  Char          */
/*                               not put on input queue and thus is           */
/*                               not read.                                    */
/*                               N - do not ignore break condition            */
/*                               see following key word                       */
/*                               N = default for invalid or no value          */
   char            hupcl;       /* Y - disconnect when last proc done         */
/*                               N - do not disconnect                        */
/*                               Y = default for invalid or no value          */
   char            brk_int;     /* Y - break condition generates an           */
/*                               interrupt flushes both input and             */
/*                               output queues                                */
/*                               N - break does not generate intr             */
/*                               N = default for invalid or no value          */
   char            ign_par;     /* Y - characters with other framing &        */
/*                               parity errors are ignored.                   */
/*                               N - do not ignore other frame or par         */
/*                               errors.  See following key word              */
/*                               N = default for invalid or no value          */
   char            par_mark;    /* Y - character with framing or parity       */
/*                               error is read as 3-character seq             */
/*                               0377, 0, x. x is data of character           */
/*                               received in error.                           */
/*                               N - character with framing or parity         */
/*                               error is read as the character NULL          */
/*                               N = default for invalid or no value          */
   char            ixon;        /* Y - enable start/stop output control       */
/*                               N - disable start/stop output ctrl           */
/*                               N = default for invalid or no value          */
   char            ixoff;       /* Y - transmit start/stop char when          */
/*                               input queue near empty                       */
/*                               N - do not transmit start/stop char          */
/*                               N = default for invalid or no value          */
   int             cbaud[16];   /* Baud rates - Following is a list of        */
/*                               the current valid rates: 50, 75,110,         */
/*                               134, 150, 200, 300, 600, 1200, 1800,         */
/*                               2400, 4800, 9600, 19200, 38400, 57600,       */
/*                               76800, 115200.                               */
/*                               9600 = default for bad or no value           */
/*                               for baud rate                                */
   int             chsize[4];   /* Character bit size - enter Y or N          */
/*                               respectively for the following char          */
/*                               sizes 5, 6, 7, 8.                            */
/*                               7 = default for bad or no value for          */
/*                               5, 6, 8.                                     */
   int             cstopb[2];   /* stop bits - enter Y or N for               */
/*                               1 and 2 stop bits                            */
/*                               1 - one stop bit                             */
/*                               2 - two stop bits                            */
/*                               B - send both 1 and 2 stop bits              */
/*                               2 = default for invalid or no value          */
   char            parodd[3];   /* parity check - enter Y or N                */
/*                               for, odd, even, and no parity check          */
/*                               O - generate parity bit for odd par          */
/*                               E - generate parity bit for even par         */
/*                               N - no parity bit generation                 */
/*                               B - generate par bit for both odd &          */
/*                               even parity check                            */
/*                               N = default for invalid or no value          */
   int             num_chars;   /* 0 - MAX_CHARS: read or write # of chars    */
   int             vmin;        /* value for VMIN setting  on reads           */
   int             bufsize;     /* write buffer size                          */
   int             ioctl_sleep; /* usleep time after set ioctls               */
   int             write_sleep; /* usleep time after set ioctls               */
   int             ihog;        /* ihog > 512 or 0                            */
   int             crash;       /* if set crash on miscompare.                */
   unsigned  trace_flag  : 1;   /* for activating trace                       */
   unsigned default_dir  : 1;   /* Initial direction of half-duplex data flow */
   unsigned         rts  : 1;   /* Set if using RTS line discipline.          */
   unsigned         dtr  : 1;   /* Set if using DTR line discipline.          */
   unsigned    fastcook  : 1;   /* 128port Specific mode only.                */
   unsigned      altpin  : 1;   /* 128port Specific mode only.                */
   unsigned     dtrpace  : 1;   /* 128port Specific mode only.                */
   char    slewrate[12];   	/* for 731 server */
} RULE_T;


#define RULES			ArgDataPtr->Rules
#define PARMPTR			ArgDataPtr->TestParms
#define RULESPATH       ArgDataPtr->RulesPath
#define	PATTERNPATH		ArgDataPtr->PatternPath
#define	PATTERNDATA		ArgDataPtr->PatternDataPtr
#define	IM_THE_WRITER	ArgDataPtr->MyInfo.writer


#endif
