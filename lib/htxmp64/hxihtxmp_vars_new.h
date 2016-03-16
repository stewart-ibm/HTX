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
/* @(#)78	1.1  src/htx/usr/lpp/htx/lib/htxmp64/hxihtxmp_vars_new.h, htx_libhtxmp, htxubuntu 9/22/10 07:33:04 */
/*
 * COMPONENT_NAME: htx_libhtxmp
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */   

#ifndef __HTX_LINUX__
#include <sys/trchkid.h>
#endif

#include <pthread.h>

#if HTXTHREADED

pthread_mutex_t hxfupdate_mutex;
pthread_once_t hxfupdate_onceblock = PTHREAD_ONCE_INIT;

pthread_mutex_t hxfpat_mutex;
pthread_once_t hxfpat_onceblock = PTHREAD_ONCE_INIT;

pthread_once_t hxfsbuf_onceblock = PTHREAD_ONCE_INIT;
pthread_mutex_t hxfsbuf_mutex;

pthread_once_t hxfcbuf_onceblock = PTHREAD_ONCE_INIT;
pthread_mutex_t hxfcbuf_mutex;

pthread_once_t hxfbindto_a_cpu_onceblock = PTHREAD_ONCE_INIT;
pthread_mutex_t hxfbindto_a_cpu_mutex;

#endif
