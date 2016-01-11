
/* @(#)77	1.2  src/htx/usr/lpp/htx/lib/gr64/strlencpy.c, htx_libgr, htxubuntu 5/24/04 18:04:12 */

/*****************************************************************************

Function(s) String Copying

Function performs strncpy() AND places NULL at end in case source string
too long.  Function expects that destination string has been declared
size of "n_chars + 1".

*****************************************************************************/
#include <string.h>

void strlencpy (char *destination,char *source,int n_chars)
   {
	strncpy(destination,source,n_chars);
	destination[n_chars] = '\0';
   }
