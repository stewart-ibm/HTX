/* @(#)91   1.5  src/htx/usr/lpp/htx/bin/hxeasy/headers.h, exer_asy, htxgrsle9 6/23/05 01:29:37 */
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
