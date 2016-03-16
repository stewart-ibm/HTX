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
