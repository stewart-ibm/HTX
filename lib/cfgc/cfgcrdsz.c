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

/* @(#)91	1.5  src/htx/usr/lpp/htx/lib/cfgc/cfgcrdsz.c, htx_libcfgc, htxubuntu 5/24/04 18:00:50 */

/********************************************************************/
/*                                                                  */
/* Module Name:  cfgcrdsz                                           */
/* Component Name:  CFG - System Configuration                      */
/* LPP Name:  BOS                                                   */
/*                                                                  */
/* Descriptive Name:  Read Stanza Routine                           */
/*                                                                  */
/* Copyright:  Copyright IBM Corporation 1984                       */
/*                                                                  */
/* Function:  This routine will read one stanza from an attribute   */
/*      file.  A specific stanza may be requested, or the next      */
/*      stanza in the file will be read.  When a stanza is read,    */
/*      any information contained in a default stanza preceding     */
/*      it will be added to the information returned.               */
/*                                                                  */
/* Compiler Options:                                                */
/*      -DBUG turns on debug print statements                       */
/*                                                                  */
/* Input:                                                           */
/*      sfptr   - pointer to an open file table                     */
/*      buf    - pointer to buffer where stanza will be stored      */
/*      nbyte  - size in bytes of buf                               */
/*      stanza - pointer to the name of the stanza to be read       */
/*                                                                  */
/* Output:                                                          */
/*      Requested stanza is stored in buf.                          */
/*                                                                  */
/* Normal Return:                                                   */
/*      CFG_SUCC - successful completion                            */
/*      CFG_EOF  - end of file ("next stanza" request only)         */
/*                                                                  */
/* Error Return:                                                    */
/*      I/O errors returned by system calls                         */
/*      CFG_SZNF - requested stanza not found                       */
/*      CFG_SZBF - requested stanza larger than nbyte               */
/*                                                                  */
/* External References:                                             */
/*                                                                  */
/*   Other Routines:                                                */
/*                                                                  */
/*   Data Areas:                                                    */
/*                                                                  */
/* Change Activity:                                                 */
/*                                                                  */
/********************************************************************/

/********************************************************************
* Begin (Read a stanza)
*
*   While stanza not found and entire file not searched
*     Call read routine to get a stanza
*     Switch (read return code)
*       Case (Successful):
*       Case (Buffer too small):
*         Note: For buffer too small, process as normal.
*               Return code will indicate condition to caller.
*         If "default" stanza read then
*           If default stanza was requested then
*             Set up to exit the loop
*           Else
*             Call routine to save the default stanza
*           Endif
*         Else
*           If (stanza read = stanza requested) or
*               ("next stanza" requested) then
*             Set up to exit the loop
*             If default information is available then
*               Merge the default information
*             Endif
*           Else
*             If all of file searched then
*               Set return code = stanza not found
*               Set up to exit the loop
*             Endif
*           Endif
*         Endif
*       Case (End of file):
*         If a specific stanza was requested then
*           Rewind the file
*         Else
*           Set up to exit the loop
*         Endif
*       Case (Default):
*         Set up to exit the loop
*     Endswitch
*   Endwhile
*
* End (Read a stanza)
********************************************************************/

#include <cfgclib.h>
#include <cfgcom.h>
#include <cfgpest.h>

#define DEFAULT "default:"
#define BLANK ' '

int	cfgcrdsz(CFG__SFT *sfptr, char *buf, int nbyte, char *stanza)
{
    	long	startloc = 0;	/* stanza search start location */
    	int	nameln = 0;	/* length of stanza name */
    	int	retc = 0;	/* return code */
    	struct {			/* flags */
		unsigned quit: 1;	/* loop control */
		unsigned rewound: 1;	/* search has wrapped to start of file */
	} flag;

    	bugldm(BUGNTA,"entry",&sfptr,8);
    	startloc = ftell(sfptr->sfile); /* get current location in file */

    	buglvt(BUGGID,startloc,ld);
    	buglvt(BUGGID,stanza,s);

    	buf[0] = '\0';			/* initialize buffer */
    	nameln = htx_strlen(stanza);	/* set length of stanza name */
	flag.quit = 0;			/* initialize loop control */
    	flag.rewound = 0;		/* initialize wrap indicator */

    	while (!flag.quit) {		/* while searching read one stanza */
		retc = cfgcread(sfptr,buf,nbyte);

		buglvt(BUGGID,retc,d);

		switch (retc) {		/* switch on return from read   */
			case CFG_SUCC:	/* case: read successful */
			case CFG_SZBF:	/* successful but stanza too big */
                                        /* if default stanza read       */

		    		bugldm(BUGGID,"buf from cfgcread",buf,32);
		    		if ((htx_strncmp(DEFAULT, buf, sizeof(DEFAULT) - 1)) == 0) {
					cfgcsvdflt(sfptr, buf); /* save the default info */
					/*
					 * if default stanza requested
					 */
					if (((htx_strncmp(stanza,buf,nameln)) == 0) && (buf[nameln] == ':'))
			    			flag.quit = 1;      /* return the default stanza    */
		    		}

				else {		/* if match found */
					if ((stanza == NULL) || ((buf[nameln] == ':') && (htx_strncmp(stanza,buf,nameln) == 0)))  { /* if stanza not too big */
						if (retc == CFG_SUCC)  {
							/* if there is a default stanza */
							if (sfptr->defbuf != NULL && *sfptr->defbuf != '\0')  {
								/* add default stanza to stanza */
								retc = cfgcmrgdflt(buf,sfptr,nbyte);
							}

							flag.quit = 1;      /* set up loop termination      */
						}
					}

					/* else if wrap has occurred    */
					else if (flag.rewound) {
						/* if all of file searched      */
			    			if (ftell(sfptr->sfile) >= startloc) {
							buglpr(BUGACT,("Stanza not found.\n"));
							/* return stanza not found      */
							retc = CFG_SZNF;
							flag.quit = 1;
			    			}
					}
		    		}

		    		break;

			case CFG_EOF:               /* case: end of file reached    */
		    		if (stanza != NULL) {
					if (flag.rewound) {     /* if wrap has occurred         */
			    			flag.quit = 1;      /* set up loop termination      */
			    			retc = CFG_SZNF;    /* return stanza not found      */
					}

					else {                  /* else wrap to start of file   */
			    			cfgcrwnd(sfptr);    /* rewind, invalidate default   */
			    			flag.rewound = 1;   /* indicate wrap has occurred   */
					}

					buglvt(BUGGID,flag.rewound,d);
		    		}

				else flag.quit = 1;
		    		break;

			default:                    /* case: other                  */
		    		flag.quit = 1;          /* set up loop termination      */
		    		break;                  /*  (return read return code)   */
		}
    	}

    	buglrt(BUGNTA,"exit",retc,d);
    	return(retc);
}



