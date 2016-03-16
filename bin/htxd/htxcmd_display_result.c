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
/* @(#)20	1.1  src/htx/usr/lpp/htx/bin/htxd/htxcmd_display_result.c, htxd, htxubuntu 7/17/13 02:03:59 */



#include <stdio.h>


void htxcmd_display_result(char *result_string)
{
	printf("\n######################## Result Starts Here ################################");
	printf("\n%s\n", result_string);
	printf("######################### Result Ends Here #################################\n");
}
