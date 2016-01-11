/*@(#)39  1.7  src/htx/usr/lpp/htx/bin/hxstats/hxstatsdef.h, htx_stats, htxubuntu 10/19/03 23:36:59*/
/*
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *   OBJECT CODE ONLY SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
 *  hxstatsdef.h -- HTX Statistics Program (hxstats) Function Declarations
 *          
 *    This include file is the respository for all HTX Statistics Program
 *    (hxstats) function declarations.
 *
 */                                                          

#ifndef HXSTATSDEF_H
#define HXSTATSDEF_H

#include <hxihtx.h>

#include <sys/signal.h>

#ifdef __OS400__
struct sigcontext; /*400 predeclare struct*/
#endif


/*
 *  function to update the checkpoint file
 */
short send_message PARMS((char *, int, int, mtyp_t));

/*
 *  function to update the checkpoint file
 */
short set_signal_hdl PARMS((int, void (*)(int, int, struct sigcontext *)));



/*
 *  function to handle SIGTERM (terminate) signal
 */
void SIGTERM_hdl PARMS((int, int, struct sigcontext *));

/*
 *  function to handle SIGUSR1 (first user defined) signal
 */
void SIGUSR1_hdl PARMS((int, int, struct sigcontext *));

#endif  /* HXSTATSDEF_H */
