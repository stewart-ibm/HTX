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
#ifndef	__DAPL_H
	#define __DAPL_H
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/mman.h>
#include <inttypes.h>
#ifdef _64BIT_
#include "hxihtx64.h"
#else 
#include "hxihtx.h"
#endif 
#include "exit.h"
#include "global.h" 
/* Header files needed for DAT/uDAPL */
#include <dat2/udat.h>


#define DEFAULT_PROVIDER			"ct3_0"
#define PROVIDER_NAME_LEN			(64)
#define HOSTNAME_LEN				(256)
#define ASSYNC_EVD_DEFAULT_QLEN     (16)
#define MSG_BUF_COUNT     			(256)
#define MSG_IOV_COUNT     			(1)
#define SERVER_CONN_QUAL  			(45248)
#define MAX_RDMA_RD					(1)
#define DTO_TIMEOUT       			(1000*1000*5)
#define CNO_TIMEOUT       			(1000*1000*1)
#define DTO_FLUSH_TIMEOUT 			(1000*1000*2)
#define CONN_TIMEOUT      			(1000*1000*100)
#define SERVER_TIMEOUT    			DAT_TIMEOUT_INFINITE
#define RDMA_BUFFER_SIZE  			(64)
#define DAPL_CONNECTED				(1)
#define DAPL_ERR_MSG_LEN 			(512)
#define ERRNO errno 

/* MSG for verification */
#define MSG_SND_RCV		((DAT_UINT64)(100))
#define MSG_RMR_EXCHG	((DAT_UINT64)(101))
#define MSG_RDMA_READ	((DAT_UINT64)(200))
#define MSG_RDMA_WRITE	((DAT_UINT64)(201))
#define HTXERROR(ex,no) ((ex<<16) +no)

struct dapl_msg_hdr {
    unsigned int msg_type;
    unsigned int msg_hdr_len;
    unsigned int msg_len;
};

struct rbuf_info {
    struct dapl_msg_hdr hdr;
    unsigned long addr;
    unsigned long length;
    unsigned int rkey;
};



struct mr_attr{
	DAT_REGION_DESCRIPTION region;			/* contains user provided MR base address */
	DAT_VLEN               size;			/* size of MR requested */
	DAT_LMR_HANDLE         h_lmr;			/* MR handle */
	DAT_LMR_CONTEXT        lmr_ctx;			/* local MR context */
	DAT_RMR_CONTEXT        rmr_ctx;			/* remote MR context */
	DAT_VLEN               registered_size;	/* registered MR size */
	DAT_VADDR              registered_addr;	/* registered MR address */
};

struct dev_handle{
	DAT_IA_HANDLE ia;
    DAT_PZ_HANDLE pz;
    DAT_EP_HANDLE ep;
    DAT_PSP_HANDLE psp;
    DAT_CR_HANDLE cr;
	DAT_CNO_HANDLE dto_cno;	
};

struct event_handle{
	DAT_EVD_HANDLE async;
    DAT_EVD_HANDLE dto_req;
    DAT_EVD_HANDLE dto_rcv;
    DAT_EVD_HANDLE cr;
    DAT_EVD_HANDLE conn;
	DAT_CNO_HANDLE dto_cno;	
};

struct dapl_device {
	/* DAT resource handles */
	struct dev_handle h_dev;

	/* DAT event handles */
	struct event_handle h_evd;

	/* memory region attributes for local buffers */
	struct mr_attr snd_mr;				/* send buffer memory region attributes */
	struct mr_attr rcv_mr;				/* receive buffer memory region attributes */

	/* Remote buffer info */ 
	DAT_RMR_TRIPLET rbuf_info;	/* remote buffer translation info */

	/* miscellaneous for device management */
	DAT_EVENT event;
	DAT_DTO_COOKIE cookie;
	DAT_CONN_QUAL conn_id;						/* similar to port in TCP domain, default = 45248 */
	int conn_state;
	char dst_host[HOSTNAME_LEN];
	int idx;   
	DAT_SERVICE_TYPE service_type; 	/* RC, UD, UC */
	DAT_IA_ATTR ia_attr;
	DAT_EP_ATTR ep_attr;
	DAT_EP_PARAM ep_param;
	DAT_COUNT assync_evd_qlen;
	char provider_name[PROVIDER_NAME_LEN];
	DAT_TIMEOUT  alarm;			/* Holds Rules alarm timeout in microseconds */ 
	struct htx_data * stats; 
};

/* timers */
/*double start, stop, total_us, total_sec;*/

struct tc_time {
	double total;
   	double open;
	double reg;
	double unreg;
	double pzc;
	double pzf;
	double evdc;
	double evdf;
	double cnoc;
	double cnof;
	double epc;
	double epf;
	double rdma_wr;
	double rdma_rd[MAX_RDMA_RD];
	double rdma_rd_total;
	double rtt;
	double close;
	double conn;
};

struct exer_dapl_rule {
	int max_evd_qlen;
	int service_type;
	int max_rdma_size;
	int recv_completion_flagso;
	int use_polling;
	int use_cno;
	int msg_count;
	int segment_count;
};

                                                                                                                                              
DAT_RETURN dapl_open(struct dapl_device *dev, int assync_evd_qlen, struct htx_data * stats);                                                  
DAT_RETURN dapl_close(struct dapl_device *dev, struct htx_data * stats);                                                                     
DAT_RETURN dapl_query(struct dapl_device *dev, struct htx_data * stats);                                                                      
DAT_RETURN dapl_create_pz(struct dapl_device *dev, struct htx_data * stats);                                                                  
DAT_RETURN dapl_free_pz(struct dapl_device *dev, struct htx_data * stats);                                                                    
DAT_RETURN dapl_register_mem(struct dapl_device *dev, void *addr, unsigned long long length, struct mr_attr *out_mr, struct htx_data * stats);
DAT_RETURN dapl_unregister_mem(struct mr_attr *mr, struct htx_data * stats);                                                                  
DAT_RETURN dapl_create_event(struct dapl_device *dev, DAT_COUNT qlen, struct htx_data * stats);                                               
DAT_RETURN dapl_destroy_events(struct dapl_device *dev, struct htx_data * stats);                                                             
DAT_RETURN dapl_create_ep(struct dapl_device *dev, DAT_SERVICE_TYPE service_type, struct htx_data * stats);                                   
DAT_RETURN dapl_free_ep(struct dapl_device *dev, struct htx_data * stats);                                                                    
DAT_RETURN dapl_connect_ep(struct dapl_device *dev, struct sockaddr_in *daddr, void *priv_data, int data_len, struct htx_data * stats);       
DAT_RETURN dapl_listen_ep(struct dapl_device *dev, DAT_EVENT *event, struct htx_data * stats);                                                
DAT_RETURN dapl_accept_ep(struct dapl_device *dev, DAT_EVENT *event, void *cmp_pattern, int cmp_len, struct htx_data * stats);                
DAT_RETURN dapl_disconnect_ep(struct dapl_device *dev, struct htx_data * stats);                                                              
DAT_RETURN dapl_destroy_psp(struct dapl_device *dev, struct htx_data * stats);                                                                
DAT_RETURN dapl_addr_exchange(struct dapl_device *dev, DAT_RMR_TRIPLET *rmr_msg, struct htx_data * stats);                                    
DAT_RETURN dapl_collect_event(DAT_EVD_HANDLE *dto_evd, DAT_EVENT *event, DAT_TIMEOUT timeout, struct htx_data * stats);                       
DAT_RETURN dapl_post_rcv(struct dapl_device *dev, struct mr_attr *rcv_mr, DAT_DTO_COOKIE *k, int num_msg, struct htx_data * stats);           
DAT_RETURN dapl_rdma_write(struct dapl_device *dev, unsigned long long buf_len, struct htx_data * stats);                                     
DAT_RETURN dapl_rdma_read(struct dapl_device *dev, unsigned long long buf_len, struct htx_data * stats);                                      
void dapl_print_addr(struct sockaddr *sa, struct htx_data * stats);                                                                           
const char *dapl_ret_tostr(DAT_RETURN ret_value);                                                                                             
const char *dapl_event_tostr(DAT_EVENT_NUMBER event_code);                                                                                    
void dapl_dump_ia_attr(DAT_IA_ATTR *ia_attr);                                                                                                 
void dapl_dump_ep_attr(DAT_EP_ATTR *ep_attr);                                                                                                 
void dapl_dump_ep_param(DAT_EP_PARAM *ep_param);                                                                                              
int dapl_init_instance(char *dev_name,int Idx, unsigned short port_num, struct dapl_device **dev, struct htx_data * stats, DAT_TIMEOUT alarm);                           
void dapl_cleanup_instance(struct dapl_device *dev);                                                                 
struct dapl_device *dapl_get_instance(struct htx_data * stats);                                                                               
int dapl_get_provider(char *device_name, char *provider, struct htx_data * stats);                                                            
void dapl_flush_evds(struct dapl_device *dev, struct htx_data * stats);                                                                      

#endif /* __DAPL_H */
