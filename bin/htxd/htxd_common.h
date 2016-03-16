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
/* @(#)36	1.3  src/htx/usr/lpp/htx/bin/htxd/htxd_common.h, htxd, htxubuntu 9/15/15 20:27:57 */



/*
 * Note:	1. in case of any change in this file, please rebuild both htxd and htxcmd
		 	2. option_list defined in this header file is a global object, so it can not include in multiple file in a same program
        
 */

#ifndef HTXD__COMMON__HEADER
#define HTXD__COMMON__HEADER


#include "htxd_option.h"

/* one option entry : option name, option method, parameter list flag, running ecg list display flag */
option option_list[] =
	{
		{"-createmdt", 0, FALSE, 0},
		{"-listmdt", 0, FALSE, 0},
		{"-run", 0, TRUE, 0},
		{"-getactecg", 0, FALSE, 1},
		{"-shutdown", 0, TRUE, 1},
		{"-refresh", 0, FALSE, 1},
		{"-activate", 0, TRUE, 1},
		{"-suspend", 0, TRUE, 1},
		{"-terminate", 0, TRUE, 1},
		{"-restart", 0, TRUE, 1},
		{"-coe", 0, TRUE, 1},
		{"-soe", 0, TRUE, 1},
		{"-status", 0, TRUE, 1},
		{"-getstats", 0, TRUE, 1},
		{"-geterrlog", 0, FALSE, 1},
		{"-clrerrlog", 0, FALSE, 1},
		{"-cmd", 0, TRUE, 1},
		{"-set_eeh", 0, TRUE, 0},
		{"-set_kdblevel", 0, TRUE, 0},
		{"-set_hxecom", 0, TRUE, 0},
		{"-getvpd", 0, FALSE, 1},
		{"-getecgsum", 0, TRUE, 1},
		{"-getecglist", 0, FALSE, 1},
		{"-select", 0, TRUE, 0},
		{"-exersetupinfo", 0, TRUE, 1},
		{"-bootme", 0, TRUE, 1},
		{"-query", 0, TRUE, 1}
	};

#endif

