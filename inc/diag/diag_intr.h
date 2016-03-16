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

