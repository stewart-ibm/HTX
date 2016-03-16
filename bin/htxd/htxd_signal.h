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
/* @(#)66	1.1  src/htx/usr/lpp/htx/bin/htxd/htxd_signal.h, htxd, htxubuntu 7/17/13 09:10:11 */



#ifndef HTXD__SIGNAL__HEADER
#define HTXD__SIGNAL__HEADER


extern void register_signal_handlers(void);
extern int htxd_send_SIGTERM(pid_t);
extern int htxd_send_SIGKILL(pid_t);
extern int htxd_send_SIGUSR1(pid_t);


#endif
