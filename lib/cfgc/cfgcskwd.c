
/* @(#)93	1.5  src/htx/usr/lpp/htx/lib/cfgc/cfgcskwd.c, htx_libcfgc, htxubuntu 5/24/04 18:02:16 */

#include <cfgclib.h>
#include <bufupd.h>
#include <cfgpest.h>
#include <cfgcom.h>


/*
 * keyword -- keyword to scan for
 * buf -- pointer to buffer containing stanza
 * ptr -- ptr to string of max value length
 */

int  cfgcskwd(char *keyword, char *buf, char *ptr)
{                          /* begin scan for keyword & return value fn */

	int	rc = CFG_SUCC;         /* return code */
	char	*returnval = NULL;           /* ptr to string to return value */
	int	len = 0;                   /* length of keyword */
	int	match = NO;            /* match found -- yes or no */
	int	i = 0;                     /* working index */
	char	*sptr = NULL;                /* working pointer */


	returnval = ptr;           /* return ptr to string or NULL */
	len = htx_strlen (keyword);    /* len is length of keyword, not counting \0 */

	sptr = buf;                /* start at beginning of stanza */
	bugvt(sptr,x);
	sptr = htx_strchr (sptr,NEWLN);	/* scan past stza name to first newline */
	sptr++;                    /* point to first character after newline */

	/*
	 * loop through stanza looking for match
	 */
	while ((*sptr != NULLC) && (match == NO))  {
		bugvt(sptr,x);             /* scan past blanks and whatever */
	     	while ((*sptr <= BLANK) && (*sptr != NEWLN) && (*sptr != NULLC))  {
			sptr++;
		}

		/*
		 * ignore comment lines and blank lines */
	     	if ((*sptr != ASTRSK) && (*sptr != NEWLN) && (*sptr != NULLC))  {
			bugvt (*sptr,c);
	       		bugvt (keyword,s);
	       		bugvt (len,d);

	       		if ( htx_strncmp(sptr,keyword,len) == 0 )  {                 /* entire string matches */

      				/*
				 * check that blank or `=' follows keyword in stanza
				 */
		    		i = 0;
		    		while ((i < len) && (*sptr != NULLC)) {
					sptr++;
					i++;
				}

		    		if ((*sptr == EQLSGN) || (*sptr == BLANK))  {             /* match found */
			 		match = YES;
	     				/* scan to value */

			 		while ((*sptr == BLANK) || (*sptr == EQLSGN))  {
						sptr ++;
					}

	     				/* write value to return string */
			 		i = 0;
			 		while ((*sptr > BLANK) && (i < MAXVAL))  {
			      			returnval[i++] = *sptr;
			      			sptr++;
			 		}

			 		if (i < MAXVAL)  {
			      			returnval[i] = NULLC;     /* add terminating \0 */
					}
			 		else  {
			      			returnval[--i] = NULLC;   /* add terminating \0 */
					}

		    		}

		    		else if(*sptr != NULLC)  {
			 		sptr = htx_strchr(sptr,NEWLN);
			 		sptr++;
		    		}
	       		}

	       		else if (*sptr != NULLC)  {
		    		sptr = htx_strchr(sptr,NEWLN);
		    		sptr++;
	       		}
	     	}

	     	else if (*sptr != NULLC)  {
		  	sptr = htx_strchr(sptr,NEWLN);
		  	sptr++;
	     	}
	}

	if (match == NO)  {
	     	rc = CFG_MNFD;
	}


	bugvt(rc,d);
	return (rc);
}


