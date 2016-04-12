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

	htx_err(4, htx_sp, errno, SYSERR, cp);
   }
