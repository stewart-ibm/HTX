
/* @(#)87	1.6  src/htx/usr/lpp/htx/lib/cfgc/cfgcdflt.c, htx_libcfgc, htxubuntu 5/24/04 17:59:47 */

/********************************************************************
* Begin (Process default stanza)
*   If default already exists then
*     Free the space allocated for the default stanza
*     Free the space allocated for keyword index array
*   Endif
*   Scan default stanza, counting total bytes and newline characters
*   Allocate storage for array of keyword indexes using number
*       of newlines as basis
*   Allocate storage for the default stanza
*   Address the start of the buffer
*   While all characters not moved
*     Move characters to the default buffer, saving index of
*         the start of each line that is not a comment
*   Endwhile
* End (Process default stanza)
********************************************************************/

#include <cfgclib.h>
#include <cfgcom.h>
#include <cfgpest.h>

#define NEWLINE "\n"
#define TAB "\t"
#define BLANK ' '

/*      Global filename variables for error message inserts  */
char msgc2[MAXDIR];

/*
 * sfptr -- file pointer
 * buf -- default buffer
 */
int	cfgcsvdflt(CFG__SFT *sfptr, char *buf)
{
    	unsigned int	i;	/* buffer index variable */
	unsigned int	j;	/* map index variable */
	unsigned int	nl;	/* number of newlines in default stanza */
	unsigned int 	len;	/* length of default stanza */

    	nl = len = 0;
    	while (buf[len] != '\0')  {	/* scan through default stanza */
	 	if (buf[len++] == '\n')  {	/* counting new lines */
			nl++;
		}
	}

	len += 2;			/* make sure there's enough */
	nl += 2;			/* space */

    	if (sfptr->defbuf != NULL) {	/* if default stanza already */
	 	free(sfptr->defbuf);	/* exists, get rid of it */
	 	free(sfptr->defmap);
    	}

	/*
	 * get new buffers for defbuf
	 */
    	sfptr->defbuf = (char *)malloc(len);
    	sfptr->defmap = (short *)calloc(nl,sizeof(short));	/* move default to defbuf and   */


	i = j = 0;			/* build keyword line index */
    	while ((sfptr->defbuf[i] = buf[i]) != '\0')  {
		if ((buf[i++] == '\n') && (buf[i] != '*'))  {
	    		sfptr->defmap[j++] = i;
		}
	}

	sfptr->defmap[j] = 0;               /* indicate end of newlines     */

	
    	bugldm(BUGGID,"defbuf",sfptr->defbuf,96);
    	bugldm(BUGGID,"defmap",sfptr->defmap,32);

    	return(0);
}

/*
 *  Begin (Merge default)
 */
/*
 * buf -- buffer containing the stanza
 * sfptr -- file pointer
 * nbyte -- maximum size of the stanza
 */
int	cfgcmrgdflt(char *buf, CFG__SFT *sfptr, int nbyte)
{
    	char	value[MAXVAL];	/* temporary garbage storage */
	char	name[MAXKWD];	/* keyword of a stanza line */
	char	*orgbuf = NULL;	/* copy of original buf */
    	int	i = 0;		/* temporary index variable */
	int	j = 0;		/* temporary index variable */
	int	rc = 0;		/* return code */
	int	lng = 0;	/* length of stanza */
	int	linlen = 0;	/* length of default stanza line */

    	rc = CFG_SUCC;		/* initialize return code       */
    	lng = htx_strlen(buf);	/* set initial length of stanza */
    	orgbuf = (char *)malloc(lng+1);     /* save the buf for searches    */
    	htx_strcpy(orgbuf,buf);
    	lng--;                              /* back out newline at buf end  */
    	nbyte--;                            /* leave space for null at end  */

	/*
	 * for each default keyword
	 */

    	for (i = 0;(j = sfptr->defmap[i]) != 0; i++) {
		/*
		 * copy keyword into buffer
		 */
		cfgckwd(&sfptr->defbuf[j],name);
		bugldm(BUGGID,"defbuf keyword", name, 16);
		if (*name == '\0')  {
			break;       /* no more interesting data     */
		}

		/*
		 * if keyword is not in stanza
		 */
		if (cfgcskwd(name,orgbuf,value) != CFG_SUCC) {
			/*
			 * if lines will fit in buffer
			 */
	    		linlen = (sfptr->defmap[i+1] - j);
	    		if (nbyte > (lng + linlen)) {
				/*
				 * add line to stanza
				 */
				htx_strncpy(&buf[lng],&sfptr->defbuf[j],linlen);
				lng += linlen;
	    		}

			else {                      /* else stanza is now too big   */
				rc = CFG_SZBF;
				break;
	    		}
		}
    	}

	htx_strcpy(&buf[lng],"\n");             /* put back the last <CR>       */
    	free(orgbuf);                       /* free the malloc'd space      */

    	return(rc);
}

/*
 *  Begin (Copy keyword)
 */

int	cfgckwd(char *line, char *keyword)      /* get keyword from a line */
{
     	int	i = 0, j = 0;

	/*
	 * scan past initial garbage
	 */
     	while ((line[i] <= BLANK) && (line[i] != '\0'))  {
	  	i++;
	}

	/*
	 * copy first non-blank field
	 */
     	while ((line[i] > BLANK) && (line[i] != '='))  {
	  	keyword[j++] = line[i++];
	}

     	keyword[j] = '\0';                   /* end keyword string */

     	return(0);
}

/*
 * remove current default information from a stanza
 */
/*
 * sfptr -- stanza file pointer
 * buf -- stanza being updated
 */

int	cfgcunmrgdft(CFG__SFT *sfptr, char *buf)
{
     	char key[MAXKWD];                    /* present keyword */
     	char val[MAXVAL];                    /* present value */
     	char defval[MAXVAL];                 /* default buffer value */
     	char *linptr, *nxtlinptr;            /* pointer to the lines of the stanza */

	/*
	 * if there is a default stanza
	 */
     	if ((sfptr->defbuf != NULL) && (*sfptr->defbuf != '\0')) {
	   	linptr = buf;		/* initialize line pointer to beginning of the stanza */

		/*
		 * address first keyword line
		 */
		do  {
#ifdef NLS
			linptr += NLstrcspn(linptr,NEWLINE) + 1;
#else
			linptr += htx_strcspn(linptr,NEWLINE) + 1;
#endif
		} while (*linptr == '*');

		/*
		 * while not at end of stanza
		 */
	  	while ((*linptr != '\n') && (*linptr != '\0')) {
			bugldm(BUGGID,"stanza keyword",linptr,16);

			/*
			 * address next keyword line
			 */
#ifdef NLS
			nxtlinptr = linptr + NLstrcspn(linptr,NEWLINE) + 1;
#else
			nxtlinptr = linptr + htx_strcspn(linptr,NEWLINE) + 1;
#endif

			while (*nxtlinptr == '*')  {
#ifdef NLS
				nxtlinptr = nxtlinptr + NLstrcspn(nxtlinptr,NEWLINE) + 1;
#else
				nxtlinptr = nxtlinptr + htx_strcspn(nxtlinptr,NEWLINE) + 1;
#endif
			}

			cfgcprsln(linptr, key, val);	/* get keyword and value of current line */
			if(cfgcskwd(key, sfptr->defbuf, defval) == 0) {
                                        /* if values match */
		     		if (htx_strcmp(val, defval) == 0) {
					/* get next line & delete it */
			  		htx_strcpy(linptr, nxtlinptr);
			  		continue;                 /* no need to advance ptr */
		     		}	/* end if values match */
			}		/* end if keyword found */
			linptr = nxtlinptr;	/* go to next line */
	  	}			/* end while */

	  	htx_strcpy(linptr,NEWLINE);	/* add terminating newline */

     	}		/* end if default stanza exists */

     	return(0);
}			/* end function */


/*
 * get keyword and value from a line
 */
int	cfgcprsln(char *line, char *keyword, char *value)
{
     	int	i = 0, j = 0;

	/*
	 * scan past initial blanks
	 */
     	while ((line[i] <= BLANK) && (line[i] != '\0'))  {
	  	i++;
	}

	/*
	 * copy first non-blank field
	 */
     	while ((line[i] > BLANK) && (line[i] != '='))  {
	  	keyword[j++] = line[i++];
	}
     
	keyword[j] = '\0';                   /* end keyword string */

	/*
	 * scan past ' = 'field
	 */
     	while (((line[i] <= BLANK) && (line[i] != '\0')) || (line[i] == '='))  {
	  	i++;
	}


     	j = 0;
	/*
	 * copy second non-blank field
	 */
	while (line[i] > BLANK)  {
	  	value[j++] = line[i++];
	}

     	value[j] = '\0';                     /* end value string */

     	return(0);
}



