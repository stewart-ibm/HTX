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
/* @(#)94  1.3  src/htx/usr/lpp/htx/inc/cfgpest.h, htx_libcfgc, htxubuntu 10/19/03 23:39:16 */

#ifndef _H_CFGPEST
#define _H_CFGPEST

#ifdef BUG

#define TRACE 0

#define BUGLEV atoi(getenv("DBUGL"))

/*
 * Values of "DBUGL"
 */

#define BUGNFO	1        /* information: e.g., file open/close */
#define BUGACT	3        /* statement of program action */
#define BUGNTF	5        /* interfaces: names, data, return codes */
#define BUGNTA	6        /* interfaces for subordinate routines */
#define BUGNTX	7        /* detailed interface data */
#define BUGGID	9        /* gory internal detail */

#define bugpr(prspec) \
	{fprintf(stdout,"[%s #%d]  ", __FILE__, __LINE__); \
	   	printf prspec;}

#define buglpr(bl,prspec) {if (bl <= BUGLEV) \
    	{fprintf(stdout,"[%s #%d]  ", __FILE__, __LINE__); \
	   	printf prspec;}}

#define bugc(expr,comnt) {if (!(expr)) \
    	{fprintf(stderr,"[%s #%d]  %s\n", \
		    	__FILE__, __LINE__,comnt);}}

#define buglc(bl,expr,comnt) {if ((!(expr)) && (bl <= BUGLEV)) \
    	{fprintf(stderr,"[%s #%d]  %s\n", \
		    	__FILE__, __LINE__,comnt);}}

#define bugx(expr,funct) {if (!(expr)) \
    	{fprintf(stderr,"[%s #%d]\n", \
		    	__FILE__, __LINE__);funct;}}

#define buglx(bl,expr,funct) {if ((!(expr)) && (bl <= BUGLEV)) \
    	{fprintf(stderr,"[%s #%d]\n", \
		    	__FILE__, __LINE__);funct;}}

#define bugcx(expr,comnt,funct) {if (!(expr)) \
    	{fprintf(stderr,"[%s #%d]  %s\n", \
		    	__FILE__, __LINE__,comnt);funct;}}

#define buglcx(bl,expr,comnt,funct) {if ((!(expr)) && (bl <= BUGLEV)) \
    	{fprintf(stderr,"[%s #%d]  %s\n", \
		    	__FILE__, __LINE__,comnt);funct;}}

#define bugvt(variable,type)  \
	{fprintf(stderr,"[%s #%d]  variable = %type\n", \
		    	__FILE__, __LINE__,variable);}

#define buglvt(bl,variable,type)  {if (bl <= BUGLEV) \
    	{fprintf(stderr,"[%s #%d]  variable = %type\n", \
		    	__FILE__, __LINE__,variable);}}

#define bugrt(comnt,variable,type)  \
	{fprintf(stderr,"[%s #%d]  %s...variable = %type\n", \
		    	__FILE__, __LINE__,comnt,variable);}

#define buglrt(bl,comnt,variable,type)  {if (bl <= BUGLEV) \
    	{fprintf(stderr,"[%s #%d]  %s...variable = %type\n", \
		    	__FILE__, __LINE__,comnt,variable);}}

#define bugdm(comnt,dumpaddr,dumpl) \
	{fprintf(stderr,"[%s #%d]  %s\n", \
		    	__FILE__, __LINE__,comnt);cmxdump(dumpaddr,dumpl);}

#define bugldm(bl,comnt,dumpaddr,dumpl) {if (bl <= BUGLEV) \
    	{fprintf(stderr,"[%s #%d]  %s\n", \
		    	__FILE__, __LINE__,comnt);cmxdump(dumpaddr,dumpl);}}

/*
 * Don't use bugstop if stdin is not from the terminal
 */

#define bugstop()  \
	{fprintf(stderr,"[%s #%d]  PRESS ENTER TO CONTINUE", \
		    	__FILE__, __LINE__);while (getchar() != '\n');}

/*
 * Don't use bugstop if stdin is not from the terminal
 */
#define buglstop(bl) {if (bl <= BUGLEV) \
    	{fprintf(stderr,"[%s #%d]  PRESS ENTER TO CONTINUE", \
		    	__FILE__, __LINE__);while (getchar() != '\n');}}

/*
 * Don't use bugstop if stdin is not from the terminal
 */

#else

#define	bugpr(prspec);
#define buglpr(bl,prspec);
#define bugc(expr,comnt);
#define buglc(bl,expr,comnt);
#define bugx(expr,funct);
#define buglx(bl,expr,funct);
#define bugvt(variable,type);
#define buglvt(bl,variable,type);
#define bugrt(comnt,variable,type);
#define buglrt(bl,comnt,variable,type);
#define bugcx(expr,comnt,funct);
#define buglcx(bl,expr,comnt,funct);
#define bugdm(comnt,dumpaddr,dumpl);
#define bugldm(bl,comnt,dumpaddr,dumpl);
#define bugstop();
#define buglstop(bl);

#endif


#endif /* _H_CFGPEST */


