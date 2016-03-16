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

#ifndef HEADERS_H  
#define HEADERS_H

extern "C" {
#include <pthread.h>
#ifdef __HTX_LINUX__
#include <hxihtx64.h>
#endif
}

#include<ctype.h>
#include<sys/select.h>
	// defines for aix only.
#ifndef __HTX_LINUX__
#undef __STR__		/* needed for c++ in AIX...... */
#include"hxiipc64.h"	
#include<sys/cxma.h>  
#else
extern "C" {
#include <hxiipc64.h>
}
#endif
		

#include<sys/ioctl.h>
#include"cs_term.h"
#include<termios.h>

#ifndef __HTX_LINUX__
#include<sys/termiox.h>
#include<sys/cfgodm.h>
#include<odmi.h>
#include<sys/stropts.h>

#define	DEVNAME_START	8
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PAREXT	0x00100000
#define	DEVNAME_START	9
#endif

	// keep this before the class includes.
#ifndef __HTX_LINUX__
#include"hxihtx64.h"		 
#endif

#include<memory.h>
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<signal.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>

#ifndef __HTX_LINUX__
#include<sys/libcsys.h>
#include"glob.h"  
#else
#include<glob.h>  
#endif


     // defs that class declarations need

	// I need to have prototypes for the thread routines prior to
	// the object header files. So I put them in here. 
void Writer(LPDWORD);

// Don't change the order of these includes.....
#include"Cmsgbase.h"
#include"argdef.h"
#include"hxeasyp.h"	 
#include"Cevent.h"
#include"Cargdata.h"
#include"Cmsg.h" 
#include"Ccom.h"
#include"Cthread.h"
#include"Cgetspace.h"

#endif 
