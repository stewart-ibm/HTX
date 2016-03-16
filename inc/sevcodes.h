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
/* @(#)31	1.3  src/htx/usr/lpp/htx/inc/sevcodes.h, htx_libgr, htxubuntu 6/7/04 14:20:46 */

#ifndef SEVCODES_H
#define SEVCODES_H

/*
 * FUNCTIONS: Get Rule Header File
 */

/*****************************************************************************

Header HTX Severity Codes
Module Name :  sevcodes.h
Header file that specifies severity_codes as defined by HTX.

*****************************************************************************/

#define NO_ERRNO 0		/* severity codes */
#define SYSERR   0
#define HARDERR  1
#define SOFTERR  4
#define INFO     7

#endif
