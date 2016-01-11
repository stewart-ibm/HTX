/*@(#)49  1.5.3.1  src/htx/usr/lpp/htx/bin/hxssup/hxssup.h, htx_sup, htxubuntu 6/24/14 01:19:21*/
#ifndef HXSSUP_H
#define HXSSUP_H

/*
 ******************************************************************************
 ***  hxssup.h -- "hxssup" HTX Supervisor Include File  ***********************
 ******************************************************************************
 */


#include <curses.h>

#ifdef	__HTX_LINUX__
#include <sys/param.h>
#include <htx_local.h>
#include <hxiipc.h>
#include <hxihtx.h>
#include <hxiconv.h>
/*union semun{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
	void *__pad;
};*/

#else
#include <htx_local.h>
#include <hxiipc.h>
#include <hxihtx.h>
#include <hxiconv.h>
#endif


/* undefine TRUE and FALSE to prevent lint errors -- redefined in cfgdb.h */
#undef TRUE
#undef FALSE

#ifdef	__HTX_LINUX__
#define	TRUE	1
#define	FALSE	0
#endif

#ifndef	__HTX_LINUX__
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#endif

#ifdef __HTX_LINUX_
    #define ENTER_KEY 10
#else
    #define ENTER_KEY 13
#endif

#define ESC_KEY 27
#include "hxssupdef.h"
#define HTXSHMKEY 10601

#define RULE_NAME_LENGTH_EXCEEDED 5001

#endif				/* HXSSUP_H */
