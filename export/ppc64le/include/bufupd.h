/* @(#)85  1.4  src/htx/usr/lpp/htx/inc/bufupd.h, htx_libcfgc, htxubuntu 11/21/03 11:00:14 */
/* Component = htx_libcfgc_lx */

#ifndef _H_BUFUPD
#define _H_BUFUPD

/* bufupd.h  -- include file for all buffer update routines --  May 1, 1984 */


/* characters used in attribute file update routines */

#define BLANK ' ' /* was 040 - Use character literals, not ASCII codes */
#define COLON ':'
#define NEWLN '\n'
#define ASTRSK '*'
#define EQLSGN '='

/* values for loop control of search for match */

#ifndef YES
#define YES 1
#endif

#ifndef NO
#define NO 0
#endif

#define DELIMNUM	32  /* # of value delimiters +1; set to 33 for blank */
#define NOTFOUND     0

#ifndef TRUE
#define TRUE         1
#endif

#define MINBASE		2	/* minimum base for base conversions */
#define MAXBASE		32	/* maximum base for base conversions */
#define DELIM		1	/* token types */
#define COMMENT		2	/* used in   */
#define KEYWORD		3	/* stanza   */
#define VALUE		4	/* parser   */
#define INIT		1	/* new stanza indicator for stanza parser */

/*
 * values for string manipulation
 */

#define NULLC '\0'

#define CFG_NAIU -1

#endif /* _H_BUFUPD */


