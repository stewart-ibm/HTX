
/* @(#)06	1.4  src/htx/usr/lpp/htx/inc/diag/diag_intr.h, libdiag_lx, htxubuntu 8/5/08 09:27:00 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/autoconf.h>
#include <linux/wait.h>
#include <linux/sched.h>

#include <sys/pdiagex.h>


static inline int pdiag_dd_interrupt_notify(unsigned long handle)
{
	pdiag_struct_t *HANDLE;

	HANDLE = (pdiag_struct_t *) handle;
	if (HANDLE->sleep_flag)
		wake_up_interruptible((wait_queue_head_t *) HANDLE->sleep_word);
        return 0;
}

/* changes done for 2.4.2 */
#define INSTALLINTRHANDLER(key, addr) int init_module(void) \
						 { \
						 inter_module_register(key, THIS_MODULE, addr); \
						  return 0; \
						  }

#define REMOVEINTRHANDLER(key) void cleanup_module(void) \
					  { \
					  inter_module_unregister(key); \
					  return; \
					  }

