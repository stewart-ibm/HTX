
/* @(#)33	1.4  src/htx/usr/lpp/htx/bin/stxclient/AH_system.c, eserv_gui, htxubuntu 5/24/04 17:08:55 */

/*****************************************************************************/
/*                                                                           */
/* MODULE NAME    =    AH_system.c                                           */
/* COMPONENT NAME =    hxssup (supervisor)                                   */
/* LPP NAME       =    HTX                                                   */
/*                                                                           */
/* DESCRIPTIVE NAME =  Activate/Halt system                                  */
/*                                                                           */
/* COPYRIGHT =         (C) COPYRIGHT IBM CORPORATION 1988                    */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM           */
/*                                                                           */
/* STATUS =            Release 1 Version 0                                   */
/*                                                                           */
/* FUNCTION =          Activates/Halts the HTX system including all hardware */
/*                     exercisers defined in the mdt.                        */
/*                                                                           */
/* COMPILER OPTIONS =  -I/src/master/htx/common -g -Nn3000 -Nd4000 -Np1200   */
/*                     -Nt2000                                               */
/*                                                                           */
/* CHANGE ACTIVITY =                                                         */
/*    DATE    :LEVEL:PROGRAMMER:DESCRIPTION                                  */
/*    MMMMMMMMNMMMMMNMMMMMMMMMMNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM */
/*    06/14/88:1.0  :J BURGESS :INITIAL RELEASE                              */
/*    11/08/99:1.23 :R GEBHARDT:Feature 290676 - Add/Restart/Term device     */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*                                                                           */
/*****************************************************************************/

#include "hxssup.h"

/*
 * For Linux compatibility
 */
#define	SIGMAX	(SIGRTMAX)

/*
 ***  Externals global to this module **************************************
 */
extern union shm_pointers shm_addr;	/* shared memory union pointers      */
extern int semhe_id;		/* semaphore id                      */
extern char HTXPATH[];		/* HTX file system path         */
extern int load_exerciser(struct htxshm_HE *p_HE, struct sigaction *sigvec);
extern union semun semctl_arg;

/*
 * The following sembuf structure must be global for correct child death
 * processing.
 */
struct sembuf wait_sops[2] = {	/* wait for all HE's to stop sop     */
	{1, -1, SEM_UNDO},
	{1, 1, SEM_UNDO}
};
