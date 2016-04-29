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
/* @(#)78	1.2  src/htx/usr/lpp/htx/lib/htx64/hxihtxmp_vars_new.h, htx_libhtxmp, htxubuntu, htxubuntu_390 4/21/16 23:54:23 */

#ifndef __HTX_LINUX__
#include <sys/trchkid.h>
#endif

#include <pthread.h>

pthread_rwlockattr_t              rwlattr_update;
