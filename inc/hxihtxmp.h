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

/* @(#)42	1.2  src/htx/usr/lpp/htx/inc/hxihtxmp.h, htx_libhtxmp, htxsles11 5/24/04 14:24:23 */

#if HTXTHREADED

#include <pthread.h>
#define MAX_NUMBER_OF_THREADS 32

/* hxfmsg is thread safe but because we need it to call hxfupdate_t
   instead of hxfupdate(which is not thread safe) we need the 
   following macro */

/* #define hxfmsg(p,err,sev,text)  hxfmsg_tsafe(p,err,sev,text) */ /* thread safe hxfmsg() is disabled */
#define hxfupdate(type,arg)     hxfupdate_tsafe(type,arg)

/* This function is called from exerciser, to tell library how many
   resources it needs */
   int mp_initialize(int num_resources, struct htx_data * htx_ds);

/* This function will return back a unique number on each invoke,
   preferably these numbers would be incremental. Library would use these
   unique numbers to index into the each calling threads resources */
   int mp_start(struct htx_data * htx_ds);

/* Cleanup all the resources created by htxmp library */
   int mp_destroy(struct htx_data * htx_ds);

/* This function is called directly by hxehd.c and also from hxfcbuf.
   We need to make sure it is thread safe from both directions. It is
   not thread safe because it in turn calls hxfmsg instead of hxfmsg_t */

#define hxfsbuf(ps,wbuf,rbuf,len) hxfsbuf_tsafe(ps,wbuf,rbuf,len)


/* hxfpat_t had to be included in libhtxmp.a because it is using
   read. That read must be compiled with thread safe libc etc */

#define hxfpat(filename, pattern_buf, num_chars) hxfpat_tsafe(filename, pattern_buf, num_chars)


/* hxfcbuf is not thread safe instead call hxfcbuf_tsafe with an extra parameter
   for message pointer.*/
/* hxfcbuf had to be included in libhtxmp.a because it is using
   read. That read must be compiled with thread safe libc etc */

#endif /* HTXTHREADED */
