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
/* @(#)79	1.3  src/htx/usr/lpp/htx/lib/gr64/var.h, htx_libgr, htxubuntu 6/24/04 09:32:32 */

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
	va_start(args, fmt);
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

