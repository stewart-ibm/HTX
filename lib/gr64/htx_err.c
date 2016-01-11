
/* @(#)73	1.3  src/htx/usr/lpp/htx/lib/gr64/htx_err.c, htx_libgr, htxubuntu 6/24/04 09:24:47 */

/*****************************************************************************

Function(s) HTX Exerciser Error Handler

Function handles the reporting of errors to HTX via hxfmsg().
Function expects the following variable arg list (IN ORDER):

	1)  pointer to htx_data struct.
	2)  an error number (int), e.g. errno.
	3)  severity code (int)
	4)  a "printf" format string
	5..?)  arguments to be printed as specified by the "printf"
	       format string

                        >>>  W A R N I N G  <<<

As a reminder, recall that variable-argument passing converts some data types
(short, char, and float) to (int, int, and double), respectively.  See
notes in AIX Technical Reference Volume (varargs) in case you modify
this code to ensure you follow the proper protocols.

*****************************************************************************/
#include <stdio.h>
#ifdef LIBC_23
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include "hxihtx.h"
#include "htx_exer.h"
/*#include "var.h"*/




#ifdef LIBC_23
void htx_err (char *fmt, ...)
#else
void htx_err (va_alist)
va_dcl
#endif
   {
        va_list args;
        char *fmtstring;

        struct htx_data *htx_sp;
        int error_code;
        int severity_code;
        char sbuf[HTX_MSG_LEN+1];

        extern void strlencpy(char *destination,char *source,int n_chars);  /* string copying */

        /*
         * Initialize argument list pointer.
         */
#ifdef LIBC_23
	va_start(args,fmt);
#else
        va_start(args);
#endif

        /*
         * Grab the REQUIRED arguments first.
         */
        htx_sp = va_arg(args, struct htx_data *);
        error_code = va_arg(args, int);
        severity_code = va_arg(args, int);


        /*
         * Because the hxfmsg() function requires redundant
         * argument passing, assign the args to the structure
         * members just in case....
         */

        htx_sp->error_code = error_code;
        htx_sp->severity_code = severity_code;

        /*
         * Next, grab expected "printf()" format string
         * along with any args and vsprintf 'em into sbuf.
         */
        fmtstring = va_arg(args, char *);
        vsprintf(sbuf, fmtstring, args);

        /*
         * Again, we copy our message buffer into the member in
         * the htx_data struct due to the required, redundant
         * arg. passing.
         */
         strlencpy(htx_sp->msg_text,sbuf,HTX_MSG_LEN);

        /*
         * Send message to HTX.
         */
        hxfmsg(htx_sp,error_code,severity_code,sbuf);

        /*
         * Perform REQUIRED clean up of the argument list pointer.
         */
        va_end(args);
   }

