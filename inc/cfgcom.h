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
/* @(#)89  1.4  src/htx/usr/lpp/htx/inc/cfgcom.h, htx_libcfgc, htxubuntu 10/19/03 23:37:32 */
/*
 *  Structure for display text specification
 */

#ifndef _H_CFGCOM
#define _H_CFGCOM

#include <cfg04.h>

typedef struct {
     	int type;		/* specification type: message number, pointer
				 * to text, or null (terminator)
				 */
    	union {
		int msgno;	/* message number */
		char *txptr;	/* text pointer   */
    	} u;

	int purpose;	/* use to which this text or message is to be put.
			 * values are: display only, selection list,
			 *   parameter list, prompt, help
			 */

	int selval;	/* value to be returned to the calling routine
			 * if this entry is selected from list
			 */
} cfg__dsp;


/*
 *  Type values for cfg__dsp
 */

#define DSPTERM	0      /* end of array   */
#define DSPMESG 1      /* message number */
#define DSPTEXT 2      /* text pointer   */

/*
 * Purpose values for cfg__dsp
 */

#define DSPONLY 0      /* display only   */
#define DSPSELL 1      /* selection list: shortest unambiguous match */
#define DSPPARM 2      /* parameter list */
#define DSPROMT 3      /* prompt         */
#define DSPHELP 4      /* help           */
#define DSPXSLL 5      /* selection list: exact match */
#define DSPHEDR 6      /* header text    */

/* 
 * Special message number
 */
#define MSGNULL 0	/* empty message  */


/*
 * Return codes for common routines
 */
#define CFG_HELP  4001         /* successful - HELP key hit */
#define CFG_QUIT  4002         /* successful - QUIT key hit */
#define CFG_DO    4003         /* successful - DO key hit */
#define CFG_SHOW  4005         /* successful - SHOW key hit */
#define CFG_PRNT  4006         /* successful - PRINT key hit */
#define CFG_RANG -4010         /* message number out of range */
#define CFG_DSPT -4011         /* invalid dspec->type field */
#define CFG_PURP -4012         /* invalid dspec->purpose field */
#define CFG_MAXT -4014         /* maximum text entities exceeded */

#define CFG_VRMC -4027         /* vrmconfig failure */
#define CFG_MNFD -1            /* match not found */
#define CFG_KWNF -4031         /* keyword not found */
#define CFG_BADFLG -4033       /* bad flag value */
#define CFG_VLBF -4034         /* not enough room in value-list buffer */
#define CFG_VNF  -4035         /* value to delete not found in value list */
#define CFG_MDSN -4036         /* minidisk name could not be generated */


/*
 * Input error generic message number for cfgcldsp()
 */
extern  int ldsperr;

/*
 * Global variable for activating optional keys
 */
extern  unsigned short cfgkeys;

/*
 * Key definitions - each key is defined as a single bit in the cfgkeys
 * variable
 */
#define SHOWKEY 0x0100
#define PRNTKEY 0x0010

/*
 * Macro for activating a key
 */
#define key_on(key) cfgkeys |= key

/*
 * Macro for de-activating a key
 */
#define key_off(key) cfgkeys &= (0xffff - key)

/*
 * Global filename variables for error message inserts
 */
extern char	msgc2[];


/*
 * structure for interface to checking routines
 */
struct chkstr {
 	char adapter[10];     /* adapter field */
 	CFG__SFT *KAFptr;     /* ptr to open KAF file */
};

/*
 * structure for ddi descriptions table
 */
struct kel {
    	char key[10];
    	char desc[30];
};

#define KELSIZE sizeof(struct kel)

/*
 * structure for ddi possible options table
 */
struct oel {
    	char key[20];
    	char opt[30];
};

#define OELSIZE sizeof(struct oel)

#define DDIDESC	"/etc/ddi/descriptions"
#define DDIOPTS	"/etc/ddi/options"

#define NONATIVE 6	/* max number of native (internal) disks */

#endif


