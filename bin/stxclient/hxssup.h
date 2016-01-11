/* @(#)40	1.3  src/htx/usr/lpp/htx/bin/stxclient/hxssup.h, eserv_gui, htxubuntu 7/10/03 12:40:59 */
#ifndef HXSSUP_H
#define HXSSUP_H

/*
 ******************************************************************************
 ***  hxssup.h -- "hxssup" HTX Supervisor Include File  ***********************
 ******************************************************************************
 */


//#include <curses.h>

#ifdef	__HTX_LINUX__
#include <sys/param.h>
#include <hxiipc.h>
#include <htx_local.h>
#include <hxihtx.h>
#include <hxiconv.h>

#else
#include <htx_local.h>
#include <hxiipc.h>
#include <hxihtx.h>
#include "hxiconv_gui.h"
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

#include "hxssupdef.h"
#include "scr_info.h"

#define htx_strcpy strcpy
#define htx_strncpy strncpy
#define htx_strlen strlen

#endif				/* HXSSUP_H */
