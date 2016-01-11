
/* @(#)78	1.3  src/htx/usr/lpp/htx/lib/gr64/sys_err.c, htx_libgr, htxubuntu 6/24/04 09:30:21 */

/*****************************************************************************

Function(s) System Error Message

Function sends error message to HTX via "htx_err()" based on a
system error (errno).

*****************************************************************************/
#include <stdio.h>
#include "hxihtx.h"
#ifdef LIBC_23
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include "sevcodes.h"
#include "htx_exer.h"

void sys_err (struct htx_data *htx_sp)
   {
	char *cp;

	extern int errno, sys_nerr;
	char *sys_errlist[100];
	if ((errno > 0) && (errno < sys_nerr))
		cp = sys_errlist[errno];
	else
		cp = "Undefined errno.";

	htx_err(htx_sp, errno, SYSERR, cp);
   }
