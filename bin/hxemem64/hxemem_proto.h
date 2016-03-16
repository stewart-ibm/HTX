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
/* @(#)52	1.4.1.1  src/htx/usr/lpp/htx/bin/hxemem64/hxemem_proto.h, exer_mem64, htxubuntu 10/13/03 12:31:10 */

/**************************************************************************/
/* proto types for hxemem64 */
void set_defaults(void);
int  get_rule(int * line);
int  getline( char s[], int lim);
int  allocate_buffers(int lpage_flag);
int  fill_buffers(void);
int  load_store_buffers( char * bit_pattern);
int  store_readim_buffers( char * bit_pattern);
int  compare_buffers_8(char * bit_pattern);
void release_shm_segs(void);
void print_general_regs(char *msg, long Gprs[32]);
void release_shm_segs(void);
long get_total_pages_memory(void);
long get_free(void);
long get_pspace(void);
int  get_sonoras(void);
void SIGTERM_hdl (int sig, int code, struct sigcontext *scp);
int  run_aix_cmd( char * cmd_name, char * result, int size);
int  get_lpage_type(void);
int  compare_time(char * lp_bufptr, long * nm_time, long *lp_time);
int  check_perf(int num_seg,int lpage_type);
int  fill_byte(char *,char *, long );
int  fill_word(char *,char *, long );
int  fill_dword(char *,char *, long );
int  comp_dword(char *,char *, long , int);
int  wr_cmp_dword(char *,char *, long );
