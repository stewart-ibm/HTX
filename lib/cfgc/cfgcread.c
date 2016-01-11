
/* @(#)92	1.5  src/htx/usr/lpp/htx/lib/cfgc/cfgcread.c, htx_libcfgc, htxubuntu 5/24/04 18:02:03 */

/********************************************************************/
/*                                                                  */
/* Module Name:  cfgcread                                           */
/* Component Name:  CFG - System Configuration                      */
/* LPP Name:  BOS                                                   */
/*                                                                  */
/* Descriptive Name:  Attribute File Read Routine                   */
/*                                                                  */
/* Copyright:  Copyright IBM Corporation 1984                       */
/*                                                                  */
/* Status:  Unit tested                                             */
/*                                                                  */
/* Function:  This routine will read the next stanza from an open   */
/*      attribute file.                                             */
/*                                                                  */
/* Compiler Options:                                                */
/*      -DBUG turns on debug print statements                       */
/*                                                                  */
/* Input:                                                           */
/*      sfptr  - pointer to an open file table                      */
/*      buf    - pointer to buffer where stanza will be stored      */
/*      nbyte  - size in bytes of buf                               */
/*                                                                  */
/* Output:                                                          */
/*      Requested stanza is stored in buf.                          */
/*                                                                  */
/* Normal Return:                                                   */
/*      CFG_SUCC - successful completion                            */
/*      CFG_EOF  - end of file reached                              */
/*                                                                  */
/* Error Return:                                                    */
/*      I/O errors returned by system calls                         */
/*      CFG_SZNF - requested stanza not found                       */
/*      CFG_BFSZ - requested stanza larger than nbyte               */
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

/*********************************************************************
* Begin (Read Attribute File)
*
*   Search for stanza name to define start of stanza
*   Move stanza to caller's buffer
*
* End (Read Attribute File)
*********************************************************************/
#include <cfgclib.h>
#include <cfgcom.h>
#include <cfgpest.h>

#define SPACE   '\040'
#define DEFAULT "default:"

int cfgcread(CFG__SFT *sfptr, char *buf, int nbyte)
{
    	int c = 0;                     /* temporary character storage */
    	int i = 0;                  /* index variable */
    	int rc = 0;                 /* return code */
    	char namebuf[MAXVAL];       /* stanza name buffer */
    	struct {                    /* flags */
		unsigned nmfnd : 1;     /* name found */
		unsigned snfnd : 1;     /* stanza end found */
		unsigned one_nl : 1;    /* one newline found */
    	} flag;

    	bugldm(BUGNTA,"entry",&sfptr,6);
    	flag.nmfnd = 0;                     /* initialize loop flag         */
    	while (!flag.nmfnd) {               /* while name not found         */
		i = 0;                          /* initialize name buffer index */
                                        /* while delimiter not found    */
		while ((c = getc(sfptr->sfile)) != SPACE && (c != '\n')) {
	    		if (feof(sfptr->sfile)) {   /* return on end of file        */
				flag.nmfnd = 1;
				break;
	    		}

			/*
			 *  if 1st character is a *, this is a comment. so read to end of line.
			 */
	    		if (c == '*' && i == 0) {
		   		while ((c = getc(sfptr->sfile)) != '\n') {
			 		if (feof(sfptr->sfile)) {   /* return on end of file */
						flag.nmfnd = 1;
						break;
			 		}
		   		}
		   		break;
	    		}

                                        /* if name hasn't overflowed    */
	    		if (i < sizeof(namebuf) - 2) {
				namebuf[i++] = c;       /* add character to name        */
				if (c == ':')  {           /* if name terminator           */
		    			flag.nmfnd = 1;     /* set up to terminate loop     */
				}
	    		}
	    		else  {                        /* else                         */
				flag.nmfnd = 0;         /* continue looping             */
			}
		}
    	}

	if (feof(sfptr->sfile))  {             /* return on end of file        */
		rc = CFG_EOF;
	}

    	else {
		ungetc(c,sfptr->sfile);         /* put back space or newline    */
		namebuf[i] = '\0';              /* terminate stanza name        */

		buglpr(BUGGID,("  Name found. Name = %s\n",namebuf));
		if (i < nbyte) {                /* if stanza name < buffer size */
	    		htx_strcpy(buf,namebuf);        /* move stanza name to buffer   */
	    		flag.snfnd = 0;             /* initialize loop flags        */
	    		flag.one_nl = 0;

	    		buglvt(BUGGID, i, d);
	    		buglvt(BUGGID, nbyte, d);

			/*
			 * while end of stanza not found and buffer space left
			 */
	    		while ((!flag.snfnd) && (i < nbyte)) {
				c = getc(sfptr->sfile); /* get character from file      */

				/* if end of file               */
				if (feof(sfptr->sfile)) {
		    			flag.snfnd = 1;     /* set up loop termination      */
		    			buf[i++] = '\n';    /* terminate stanza correctly   */
				}

				else {
		    			buf[i++] = c;       /* store character in buffer    */
		    			if (c == '\n')  {      /* if char is newline           */
						/* if second in a row set up    */
						/*   to terminate               */
						if (flag.one_nl)  {
							flag.snfnd = 1;
						}
					/* else indicate one found      */
						else  {
							flag.one_nl = 1;
						}
					}
						/* else not a newline           */
		    			else  {
						if (c > ' ')  {
							flag.one_nl = 0;
						}
					}
						
				}

	    		}

	    		buglvt(BUGGID, buf, s);
	    		buglvt(BUGGID, i, d);

	    		if (i == nbyte) {           /* if buffer was filled         */
				rc = CFG_SZBF;          /* return buffer too small      */
				i--;                    /* prepare to insert terminator */
	    		}

	    		else  {
				rc = CFG_SUCC;          /* indicate successful read     */
			}

	    		buf[i] = '\0';              /* terminate buffer with null   */
	    		buglpr(BUGGID,("  Stanza read successfully.\n"));

		}

		else  {
	    		rc = CFG_SZBF;              /* return buffer too small      */
		}

    	}

    	if (ferror(sfptr->sfile)) {         /* check for error on read      */
		rc = CFG_UNIO;                  /* set return code              */
		clearerr(sfptr->sfile);         /* reset error indication       */
		htx_strcpy(msgc2,sfptr->spath);     /* set message insert variable  */
	}

    	buglrt(BUGNTA,"exit", rc, d);
    	return(rc);
}


