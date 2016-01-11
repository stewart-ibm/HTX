/* @(#)47       1.1.4.1  src/htx/usr/lpp/htx/bin/hxefpscr/sigfpe_handler.c, exer_fpscr, htxubuntu 10/28/13 04:41:15 */

/*
 * This file contains the sigfphandler used for debugging the code
 */
#include <stdio.h>
#include <signal.h>
#include <ucontext.h>

void fp_handler(int signo, siginfo_t *sigcode, void *sig_context)
{
	printf("Received SIGFPE  \n");
	//Commented cause breaks the GR build and is not required
	//(((struct pt_regs *)((&(((struct ucontext *)sig_context)->uc_mcontext))->regs))->nip) += 4 ;
	return ;
}
