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

#ifndef CPROTO_H   
#define CPROTO_H

							// AIX STUFF...........
extern "C" { int *_Errno(); }

extern "C" {void crash_sys(int,int,int,int);}
void sig_handler(int sig);


void PrtSyntax(char *);





int SanityCkBaud(	enum wtype wrap_type, 
					int cbaud[],
					ARGS_T *ArgDataPtr);

int get_rule(	FILE *fd, 
				int *line,
				RULE_T * r_ptr,
				ARGS_T *ArgDataPtr);

void pat_to_buf(char *pattern, 
				char *wbuf, 
				int num_chars, 
				int chsize,
				ARGS_T *ArgDataPtr);

int ReadPattern(ARGS_T *ArgDataPtr);

int sync_ports(ARGS_T *ArgDataPtr);

int Reader(ARGS_T *ArgDataPtr);


#endif	
