/* @(#)92   1.4  src/htx/usr/lpp/htx/bin/hxeasy/hxeasyp.h, exer_asy, htxubuntu 6/13/02 23:36:05 */
/*
 *   COMPONENT_NAME: exer_asy
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *   OBJECT CODE ONLY SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 88,93
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
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
