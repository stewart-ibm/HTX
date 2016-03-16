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
/* IBM_PROLOG_END_TAG */
#ifndef _h_diag_proto
#define _h_diag_proto

#include <sys/pdiagex.h>

int pdiag_diagnose_state(char *ptr);
int pdiag_restore_state(char *ptr);
/*int pdiag_dd_dma_setup_lpages ( pdiag_struct_t *h_ptr, int flags, unsigned long *baddr,
                unsigned long *p_ptr, unsigned long long count, int minxfer, int operation);*/
int pdiag_dd_dma_setup( pdiag_struct_t *h_ptr, int flags, unsigned long *baddr,
        unsigned long *p_ptr, unsigned int count, int minxfer, int operation);
int pdiag_dd_big_dma_setup ( pdiag_struct_t *h_ptr, int flags, unsigned long *baddr,
                unsigned long *p_ptr, unsigned int count);
/*int pdiag_dd_dma_complete_lpages (pdiag_struct_t *h_ptr, unsigned long *p_addr,
                                            unsigned long long count, int operation);*/
int pdiag_dd_dma_complete (pdiag_struct_t *h_ptr, unsigned long *p_addr, int operation);
int pdiag_dd_big_dma_complete (pdiag_struct_t *h_ptr, unsigned long *p_addr, int operation);
int pdiag_dd_watch_for_interrupt (pdiag_struct_t *handle_ptr, unsigned int mask, unsigned int timeoutsecs);
int pdiag_dd_read(pdiag_struct_t *handle_ptr, int type, int offset, void *user_buf, pdiagex_opflags_t *flags);
int pdiag_dd_write(pdiag_struct_t *handle_ptr, unsigned int type, unsigned int offset, void *user_buf, pdiagex_opflags_t *flags);
/*int pdiag_config_write_old(char *device, unsigned long offset, void *value, int length);*/
int pdiag_config_write(char *device, unsigned long offset, unsigned long value);
/*int pdiag_config_read_old(char *device, unsigned long offset, void *value, int length);*/
int pdiag_config_read(char *device, unsigned long offset, unsigned char *value);
int pdiag_set_eeh_option(char *device,unsigned int function);
int pdiag_inject_error(char *buf, char *err_token, int opn_token);
int pdiag_slot_error_detail(char *device,char * drv_buf,int drv_len,char *rpa_buf,int rpa_len);
int pdiag_read_slot_reset(char *device);
int pdiag_configure_bridge(char *device);
int pdiag_restore_bars(char *device);
int pdiag_errinjct_init(void);
int pdiag_open_error_inject(char *device);
int pdiag_close_error_inject(unsigned long token);
int pdiag_set_slot_reset(char *device);
/*int pdiag_open_old(char *ptr, pdiagex_dds_t *dds_ptr, char *intr_func, pdiag_struct_t *handle_ptr);*/
int pdiag_open(char *ptr, pdiagex_dds_t *dds_ptr, char *intr_func, pdiag_info_handle_t *handle_ptr);
int pdiag_close(pdiag_info_handle_t handle);
int pdiag_cs_open(void);
int pdiag_cs_close(void);
int pdiag_cs_get_attr(char *lname, char *att_name, char **value, char *rep);
void pdiag_cs_free_attr(char **value);
int pdiag_diagnose_sec_device(char *ptr);
int pdiag_restore_sec_device(char *ptr);
#endif
