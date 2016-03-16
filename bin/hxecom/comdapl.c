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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <sys/param.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/socketvar.h>

#include "comdapl.h"

/************************************************************************************************************************************************
*                                                                                                            						            *
* DAT_RETURN dapl_open(struct dapl_device *dev, int assync_evd_qlen, struct htx_data * stats);                                                  *
* DAT_RETURN dapl_close(struct dapl_device *dev, struct htx_data * stats);                                                                      *
* DAT_RETURN dapl_query(struct dapl_device *dev, struct htx_data * stats);                                                                      *
* DAT_RETURN dapl_create_pz(struct dapl_device *dev, struct htx_data * stats);                                                                  *
* DAT_RETURN dapl_free_pz(struct dapl_device *dev, struct htx_data * stats);                                                                    *
* DAT_RETURN dapl_register_mem(struct dapl_device *dev, void *addr, unsigned long long length, struct mr_attr *out_mr, struct htx_data * stats);*
* DAT_RETURN dapl_unregister_mem(struct mr_attr *mr, struct htx_data * stats);                                                                  *
* DAT_RETURN dapl_create_event(struct dapl_device *dev, DAT_COUNT qlen, struct htx_data * stats);                                               *
* DAT_RETURN dapl_destroy_events(struct dapl_device *dev, struct htx_data * stats);                                                             *
* DAT_RETURN dapl_create_ep(struct dapl_device *dev, DAT_SERVICE_TYPE service_type, struct htx_data * stats);                                   *
* DAT_RETURN dapl_free_ep(struct dapl_device *dev, struct htx_data * stats);                                                                    *
* DAT_RETURN dapl_connect_ep(struct dapl_device *dev, struct sockaddr_in *daddr, void *priv_data, int data_len, struct htx_data * stats);       *
* DAT_RETURN dapl_listen_ep(struct dapl_device *dev, DAT_EVENT *event, struct htx_data * stats);                                                *
* DAT_RETURN dapl_accept_ep(struct dapl_device *dev, DAT_EVENT *event, void *cmp_pattern, int cmp_len, struct htx_data * stats);                *
* DAT_RETURN dapl_disconnect_ep(struct dapl_device *dev, struct htx_data * stats);                                                              *
* DAT_RETURN dapl_destroy_psp(struct dapl_device *dev, struct htx_data * stats);                                                                *
* DAT_RETURN dapl_addr_exchange(struct dapl_device *dev, DAT_RMR_TRIPLET *rmr_msg, struct htx_data * stats);                                    *
* DAT_RETURN dapl_collect_event(DAT_EVD_HANDLE *dto_evd, DAT_EVENT *event, DAT_TIMEOUT timeout, struct htx_data * stats);                       *
* DAT_RETURN dapl_post_rcv(struct dapl_device *dev, struct mr_attr *rcv_mr, DAT_DTO_COOKIE *k, int num_msg, struct htx_data * stats);           *
* DAT_RETURN dapl_rdma_write(struct dapl_device *dev, unsigned long long buf_len, struct htx_data * stats);                                     *
* DAT_RETURN dapl_rdma_read(struct dapl_device *dev, unsigned long long buf_len, struct htx_data * stats);                                      *
* void dapl_print_addr(struct sockaddr *sa, struct htx_data * stats);                                                                           *
* const char *dapl_ret_tostr(DAT_RETURN ret_value);     					                                                                    *
* const char *dapl_event_tostr(DAT_EVENT_NUMBER event_code);						                                                            *
* void dapl_dump_ia_attr(DAT_IA_ATTR *ia_attr); 						                                                                        *
* void dapl_dump_ep_attr(DAT_EP_ATTR *ep_attr);							                                                                        *
* void dapl_dump_ep_param(DAT_EP_PARAM *ep_param);         							                                                            *
* int dapl_init_instance(char *dev_name,int idx,  unsigned short port_num, struct dapl_device **dev, struct htx_data * stats);                  *
* void dapl_cleanup_instance(struct dapl_device *dev, struct htx_data * stats);                                                                 *
* struct dapl_device *dapl_get_instance(struct htx_data * stats);                                                                         		*
* int dapl_get_provider(char *device_name, char *provider, struct htx_data * stats);                                                            *
* void dapl_flush_evds(struct dapl_device *dev, struct htx_data * stats);                                                                       *
*************************************************************************************************************************************************/

/* hxedapl globals */
static struct dapl_device *dapl_obj = NULL;
/* EEH Global's */ 
extern unsigned int    eeh_enabled ;
extern unsigned int    eeh_retries ;


/*************************hxedapl modules*****************/
/*
 * qlen: Minimum length of the Asynchronous Event Dispatcher queue
 */
DAT_RETURN dapl_open(struct dapl_device *dev, int assync_evd_qlen, struct htx_data * stats)
{
	DAT_RETURN ret;
	char msg_text[1024]; 

	ret = dat_ia_open(dev->provider_name, assync_evd_qlen, &dev->h_evd.async, &dev->h_dev.ia);
	if (ret != DAT_SUCCESS) {
		sprintf(msg_text, "Task[%d]: Error opening device[%s], %s\n", getpid(), dev->provider_name, dapl_ret_tostr(ret));
		hxfmsg(stats, HTXERROR(EX_DAPL_INIT,ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        return (ret);
    } 
  

	return (ret);
}	 

/*
 * flags: Flags for IA closure. Default value of DAT_CLOSE_DEFAULT=DAT_CLOSE_ABRUPT_FALG
 * represents abrupt closure of IA
 */ 
DAT_RETURN dapl_close(struct dapl_device *dev, struct htx_data * stats)
{
	DAT_RETURN ret;
	ret = dat_ia_close(dev->h_dev.ia, DAT_CLOSE_ABRUPT_FLAG);
	return (0);
}

DAT_RETURN dapl_query(struct dapl_device *dev, struct htx_data * stats)
{
	DAT_RETURN ret;
	DAT_IA_ATTR_MASK ia_mask = DAT_IA_FIELD_ALL;
	DAT_PROVIDER_ATTR_MASK provider_mask = 0;
	DAT_PROVIDER_ATTR *provider_attr = NULL;
	char msg_text[1024];

	ret = dat_ia_query(dev->h_dev.ia, &dev->h_evd.async, ia_mask, &dev->ia_attr, provider_mask, provider_attr);
	if (ret != DAT_SUCCESS) {
		sprintf(msg_text, "Task[%d]: Error query device[%s], %s\n", getpid(), dev->provider_name, dapl_ret_tostr(ret));
		hxfmsg(stats, HTXERROR(EX_DAPL_INIT,ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        return (ret);
    } 
    
	return (ret);
}


DAT_RETURN dapl_create_pz(struct dapl_device *dev, struct htx_data * stats)
{
	DAT_RETURN ret;
	char msg_text[1024];

	ret = dat_pz_create(dev->h_dev.ia, &dev->h_dev.pz);
	if (ret != DAT_SUCCESS) {
		sprintf(msg_text, "Task[%d]: Error creating protection zone, %s\n", getpid(), dapl_ret_tostr(ret));
		hxfmsg(stats, HTXERROR(EX_DAPL_CONNECT,ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        return (ret);
    } 
    
	return (ret);
}

DAT_RETURN dapl_free_pz(struct dapl_device *dev, struct htx_data * stats)
{
	DAT_RETURN ret;
	char msg_text[1024];

    /* Free protection domain */
    printf("Task[%d]: Freeing pz\n", getpid());
    ret = dat_pz_free(dev->h_dev.pz);
    if (ret != DAT_SUCCESS) {
        sprintf(msg_text, "Task[%d]: Error freeing PZ: %s\n", getpid(), dapl_ret_tostr(ret));
		hxfmsg(stats, HTXERROR(EX_DAPL_CONNECT,ERRNO), HTX_HE_INFO, msg_text);
		return (ret);
    } 
    dev->h_dev.pz = NULL;
	return DAT_SUCCESS;
}

DAT_RETURN dapl_register_mem(struct dapl_device *dev, void *addr, unsigned long long length, struct mr_attr *out_mr, struct htx_data * stats)
{
	DAT_RETURN ret;
	char msg_text[1024];

	if (addr == NULL) {
		sprintf(msg_text, "Task[%d]: NULL address found for registration\n", getpid());
		hxfmsg(stats, HTXERROR(EINVAL, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
		ret = (-EINVAL);
		return (ret);
	}
	out_mr->region.for_va = addr;
	out_mr->size = (DAT_VLEN)length;
	ret = dat_lmr_create(dev->h_dev.ia, DAT_MEM_TYPE_VIRTUAL, out_mr->region, length, 
						 dev->h_dev.pz, DAT_MEM_PRIV_ALL_FLAG, DAT_VA_TYPE_VA, 
						 &out_mr->h_lmr, &out_mr->lmr_ctx, &out_mr->rmr_ctx, &out_mr->registered_size,
						 &out_mr->registered_addr);
	if (ret != DAT_SUCCESS)  {
		sprintf(msg_text, "Task[%d]: Error registering memory, addr = %p , %s\n", getpid(), addr, dapl_ret_tostr(ret));
		hxfmsg(stats, HTXERROR(EX_DAPL_REGISTER, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        return (ret);
    } 
	printf("[%s]:IN : addr = %#llx, size = %llx, OP : addr = %llx, size = %llx \n", __FUNCTION__, out_mr->region.for_va, length, out_mr->registered_addr, out_mr->registered_size); 

	return (DAT_SUCCESS);
}
	
DAT_RETURN dapl_unregister_mem(struct mr_attr *mr, struct htx_data * stats)
{
	DAT_RETURN ret = DAT_SUCCESS;
	char msg_text[1024];

    if (mr->h_lmr != DAT_HANDLE_NULL) {
        ret = dat_lmr_free(mr->h_lmr);
        if (ret != DAT_SUCCESS) {
            sprintf(msg_text, "%d Error deregistering mr addr %p: %s\n", getpid(), mr->region.for_va, dapl_ret_tostr(ret));
			hxfmsg(stats, HTXERROR(EX_DAPL_REGISTER, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
            return (ret);
        } 
        mr->h_lmr = NULL;
	}
    return ret;
}

/*
 * qlen: evd_min_qlen
 * no CNO i.e dev->h_evd.dto_cno should be DAT_HANDLE_NULL
 */
DAT_RETURN dapl_create_event(struct dapl_device *dev, DAT_COUNT qlen, struct htx_data * stats)
{
	DAT_RETURN ret;
	DAT_EVD_PARAM evd_param;
	char msg_text[1024];
	
	/* create CR EVD */
	ret = dat_evd_create(dev->h_dev.ia, qlen, DAT_HANDLE_NULL, DAT_EVD_CR_FLAG, &dev->h_evd.cr);
	if (ret != DAT_SUCCESS) {
		sprintf(msg_text, "Task[%d]: Error dat_evd_create for CR: %s\n", getpid(), dapl_ret_tostr(ret));
		hxfmsg(stats, HTXERROR(EX_DAPL_INIT, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
		return (ret);
	} 

	
	/* create CONNECTION EVD */
	ret = dat_evd_create(dev->h_dev.ia, qlen, DAT_HANDLE_NULL, DAT_EVD_CONNECTION_FLAG, &dev->h_evd.conn);
	if (ret != DAT_SUCCESS) {
		sprintf(msg_text, "Task[%d]: Error dat_evd_create for CONN: %s\n", getpid(), dapl_ret_tostr(ret));
		hxfmsg(stats,HTXERROR(EX_DAPL_INIT, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
		return (ret);
	} 

#ifdef __USE_CNO_DTO
    /* create CNO */
    ret = dat_cno_create(dev->h_dev.ia, DAT_OS_WAIT_PROXY_AGENT_NULL, &dev->h_dev.dto_cno);
    if (ret != DAT_SUCCESS) {
        sprintf(msg_text, "Task[%d]: Error dat_cno_create: %s\n", getpid(), DT_RetToStr(ret));
		hxfmsg(stats, HTXERROR(EX_DAPL_INIT, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        return (ret);
    } 
    
#endif /*__USE_CNO_DTO */

	/* create DTO SND EVD, with CNO if use_cno is set as above */
    ret = dat_evd_create(dev->h_dev.ia, 2 * qlen, dev->h_dev.dto_cno, DAT_EVD_DTO_FLAG, &dev->h_evd.dto_req);
    if (ret != DAT_SUCCESS) {
        sprintf(msg_text, "Task[%d]: Error dat_evd_create REQ: %s\n", getpid(), dapl_ret_tostr(ret));
		hxfmsg(stats, HTXERROR(EX_DAPL_INIT, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        return (ret);
    } 

    /* create DTO RCV EVD, with CNO if use_cno was set */
    ret = dat_evd_create(dev->h_dev.ia, 2 * qlen, dev->h_dev.dto_cno, DAT_EVD_DTO_FLAG, &dev->h_evd.dto_rcv);
    if (ret != DAT_SUCCESS) {
        sprintf(msg_text, "Task[%d]: Error dat_evd_create RCV: %s\n", getpid(), dapl_ret_tostr(ret));
		hxfmsg(stats, HTXERROR(EX_DAPL_INIT, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        return (ret);
    } 
    

    /* query DTO req EVD and check size */
    ret = dat_evd_query(dev->h_evd.dto_req, DAT_EVD_FIELD_EVD_QLEN, &evd_param);
    if (ret != DAT_SUCCESS) {
        sprintf(msg_text, "Task[%d]: Error dat_evd_query request evd: %s\n", getpid(), dapl_ret_tostr(ret));
		hxfmsg(stats, HTXERROR(EX_DAPL_INIT, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        return (ret);
    } else {
		if (evd_param.evd_qlen < (2 * qlen)) {
        	sprintf(msg_text, "Task[%d]: Error dat_evd qsize too small: %d < %d\n", getpid(), evd_param.evd_qlen, 2 * qlen);
			hxfmsg(stats, HTXERROR(EX_DAPL_INIT, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        	return (ret);
		}
    }

    return (DAT_SUCCESS);
}	

DAT_RETURN dapl_destroy_events(struct dapl_device *dev, struct htx_data * stats)
{
    DAT_RETURN ret;
	char msg_text[1024];

    /* free cr EVD */
    if (dev->h_evd.cr != DAT_HANDLE_NULL) {
        ret = dat_evd_free(dev->h_evd.cr);
        if (ret != DAT_SUCCESS) {
            sprintf(msg_text, "Task[%d]: Error freeing cr EVD: %s\n", getpid(), dapl_ret_tostr(ret));
			hxfmsg(stats, HTXERROR(EX_DAPL_INIT, ERRNO), HTX_HE_INFO, msg_text);
            return (ret);
        } else {
            dev->h_evd.cr = DAT_HANDLE_NULL;
        }
    }

    /* free conn EVD */
    if (dev->h_evd.conn != DAT_HANDLE_NULL) {
        printf("Task[%d]: Free conn EVD %p \n", getpid(), dev->h_evd.conn);
        ret = dat_evd_free(dev->h_evd.conn);
        if (ret != DAT_SUCCESS) {
            sprintf(msg_text, "Task[%d]: Error freeing conn EVD: %s\n", getpid(), dapl_ret_tostr(ret));
			hxfmsg(stats, HTXERROR(EX_DAPL_INIT, ERRNO), HTX_HE_INFO, msg_text);
            return (ret);
        } else {
            dev->h_evd.conn = DAT_HANDLE_NULL;
        }
    }
    /* free RCV dto EVD */
    if (dev->h_evd.dto_rcv != DAT_HANDLE_NULL) {
        ret = dat_evd_free(dev->h_evd.dto_rcv);
        if (ret != DAT_SUCCESS) {
            sprintf(msg_text, "Task[%d]: Error freeing RCV dto EVD: %s\n", getpid(), dapl_ret_tostr(ret));
			hxfmsg(stats, HTXERROR(EX_DAPL_INIT, ERRNO), HTX_HE_INFO, msg_text);
            return (ret);
        } else {
            dev->h_evd.dto_rcv = DAT_HANDLE_NULL;
        }
    }

    /* free REQ dto EVD */
    if (dev->h_evd.dto_req != DAT_HANDLE_NULL) {
        ret = dat_evd_free(dev->h_evd.dto_req);
        if (ret != DAT_SUCCESS) {
            sprintf(msg_text, "Task[%d]: Error freeing REQ dto EVD: %s\n", getpid(), dapl_ret_tostr(ret));
			hxfmsg(stats, HTXERROR(EX_DAPL_INIT, ERRNO), HTX_HE_INFO, msg_text);
            return (ret);
        } else {
            dev->h_evd.dto_req = DAT_HANDLE_NULL;
        }
    }
#ifdef __USE_CNO_DTO
    /* free CNO
	 * No CNO support, the handle must be NULL
     */
    if (dev->h_evd.dto_cno != DAT_HANDLE_NULL) {
        printf("Task[%d]: Free dto CNO %p \n", getpid(), dev->h_evd.dto_cno);
        ret = dat_cno_free(dev->h_evd.dto_cno);
        if (ret != DAT_SUCCESS) {
            sprintf(msg_text, "Task[%d]: Error freeing dto CNO: %s\n", getpid(), dapl_ret_tostr(ret));
			hxfmsg(stats, HTXERROR(EX_DAPL_INIT, ERRNO), HTX_HE_INFO, msg_text);
            return (ret);
        } else {
            dev->h_evd.dto_cno = DAT_HANDLE_NULL;
        }
    }
#endif /*__USE_CNO_DTO */

    return DAT_SUCCESS;
}

/*
 * specify service type in ep_attr, e.g DAT_SERVICE_TYPE_RC
 */
DAT_RETURN dapl_create_ep(struct dapl_device *dev, DAT_SERVICE_TYPE service_type, struct htx_data * stats)
{
	DAT_RETURN ret = DAT_SUCCESS;
	DAT_EP_ATTR ep_attr;
	char msg_text[1024];
	
	memset(&ep_attr, 0, sizeof(ep_attr));
    ep_attr.service_type 				= service_type;
	ep_attr.max_mtu_size     			= USHRT_MAX;
    ep_attr.max_rdma_size 				= USHRT_MAX + 2 * TP_HDR_LEN;
    ep_attr.qos 						= DAT_QOS_BEST_EFFORT;
    ep_attr.max_recv_dtos 				= MSG_BUF_COUNT;
    ep_attr.max_request_dtos 			= MSG_BUF_COUNT + MAX_RDMA_RD;
    ep_attr.max_recv_iov 				= MSG_IOV_COUNT;
    ep_attr.max_request_iov 			= MSG_IOV_COUNT;
    ep_attr.max_rdma_read_in 			= MAX_RDMA_RD;
    ep_attr.max_rdma_read_out 			= MAX_RDMA_RD;
    ep_attr.request_completion_flags 	= DAT_COMPLETION_DEFAULT_FLAG;
	ep_attr.recv_completion_flags    	= DAT_COMPLETION_DEFAULT_FLAG;
    ep_attr.ep_transport_specific_count = 0;
    ep_attr.ep_transport_specific 		= NULL;
    ep_attr.ep_provider_specific_count 	= 0;
    ep_attr.ep_provider_specific 		= NULL;

	ret = dat_ep_create(dev->h_dev.ia, dev->h_dev.pz, dev->h_evd.dto_rcv, dev->h_evd.dto_req, dev->h_evd.conn, &ep_attr, &dev->h_dev.ep);
	if (ret != DAT_SUCCESS) {
        sprintf(msg_text, "Task[%d]: Error dat_ep_create: %s\n", getpid(), dapl_ret_tostr(ret));
		hxfmsg(stats, HTXERROR(EX_DAPL_INIT, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
		return (ret);
    } 
	return (ret);
}
DAT_RETURN dapl_free_ep(struct dapl_device *dev, struct htx_data * stats)
{
	DAT_RETURN ret = DAT_SUCCESS;
	char msg_text[1024];

	if (dev->h_dev.ep != DAT_HANDLE_NULL) {
		ret = dat_ep_free(dev->h_dev.ep);
		if (ret != DAT_SUCCESS) {
            sprintf(msg_text, "Task[%d]: Error freeing EP: %s\n", getpid(), dapl_ret_tostr(ret));
			hxfmsg(stats, HTXERROR(EX_DAPL_INIT, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
			return (ret);
        } else {
            dev->h_dev.ep = DAT_HANDLE_NULL;
        }
	}
	return (ret);
}
/*
 * client call connect while server listen
 */
DAT_RETURN dapl_connect_ep(struct dapl_device *dev, struct sockaddr_in *daddr, void *priv_data, int data_len, struct htx_data * stats)
{
	DAT_RETURN ret;
	DAT_IA_ADDRESS_PTR remote_addr;
	DAT_EVENT event;
	DAT_COUNT nmore;
    int rval;
	char msg_text[1024];

    rval = daddr->sin_addr.s_addr;
    printf("Task[%d]: Server Name: %s.%d \n", getpid(), inet_ntoa(daddr->sin_addr), dev->conn_id);
	remote_addr = (DAT_IA_ADDRESS_PTR)((struct sockaddr *)daddr); /* IP  address */
	
	printf("Task[%d]: Connecting to server\n", getpid());
	ret = dat_ep_connect(dev->h_dev.ep, remote_addr, dev->conn_id, CONN_TIMEOUT, data_len, (DAT_PVOID) priv_data, 0, DAT_CONNECT_DEFAULT_FLAG);
	if (ret != DAT_SUCCESS) {
        sprintf(msg_text, "Task[%d]: Error dat_ep_connect: %s\n", getpid(), dapl_ret_tostr(ret));
		hxfmsg(stats, HTXERROR(EX_DAPL_CONNECT, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        return (ret);
    } 

	ret = dat_evd_wait(dev->h_evd.conn, DAT_TIMEOUT_INFINITE, 1, &event, &nmore);
	if (event.event_number != DAT_CONNECTION_EVENT_ESTABLISHED) {
        sprintf(msg_text, "%d Error unexpected conn event: 0x%x %s\n", getpid(), event.event_number, dapl_event_tostr(event.event_number));
		hxfmsg(stats, HTXERROR(EX_DAPL_CONNECT, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        return (DAT_ABORT);
    }
	dev->conn_state = DAPL_CONNECTED;
	return (ret);
}


/*
 * Server Only
 */
DAT_RETURN dapl_listen_ep(struct dapl_device *dev, DAT_EVENT *event, struct htx_data * stats)
{
	DAT_RETURN ret;
	DAT_COUNT nmore;
	char msg_text[1024];

	/* clear event structure */
    memset(event, 0, sizeof(DAT_EVENT));

	/* create the service point for server listen */
    printf("Task[%d]: Creating service point for listen, port = %#x\n", getpid(), dev->conn_id);

    ret = dat_psp_create(dev->h_dev.ia, dev->conn_id, dev->h_evd.cr, DAT_PSP_CONSUMER_FLAG, &dev->h_dev.psp);
    if (ret != DAT_SUCCESS) {
        sprintf(msg_text, "Task[%d]: Error dat_psp_create: %s\n", getpid(), dapl_ret_tostr(ret));
		hxfmsg(stats, HTXERROR(EX_DAPL_LISTEN, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        return (ret);
    } 
	
	/* 
     * printf("Task[%d]: Reader waiting for connect request on port %lx\n", getpid(), dev->conn_id);
     * ret = dat_evd_wait(dev->h_evd.cr, dev->alarm, 1, event, &nmore);
     * if (ret != DAT_SUCCESS) {
     *   fprintf(stderr, "Task[%d]: Error dat_evd_wait: %s\n", getpid(), dapl_ret_tostr(ret));
     *   return (ret);
     * } else {
     *   printf("Task[%d]: dat_evd_wait for cr_evd completed\n",  getpid());
	 * }
	*/ 
	return (DAT_SUCCESS);
}

/*
 * Server only
 */
DAT_RETURN dapl_accept_ep(struct dapl_device *dev, DAT_EVENT * event, void *cmp_pattern, int cmp_len, struct htx_data * stats)
{
	DAT_RETURN ret;
	void *priv_data;
	DAT_CR_PARAM cr_param = { 0 };
	DAT_COUNT nmore;
	char msg_text[1024];

	printf("Task[%d]: Reader waiting for connect request on port %lx\n", getpid(), dev->conn_id);
	ret = dat_evd_wait(dev->h_evd.cr, DAT_TIMEOUT_INFINITE, 1, event, &nmore);
    if (ret != DAT_SUCCESS) {
        sprintf(msg_text, "Task[%d]: Error dat_evd_wait: %s\n", getpid(), dapl_ret_tostr(ret));
		hxfmsg(stats, HTXERROR(EX_DAPL_ACCEPT, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        return (ret);
    } 

	/* accept connect request from client */
    dev->h_dev.cr = event->event_data.cr_arrival_event_data.cr_handle;

	/* private data - check and send it back */
	ret = dat_cr_query(dev->h_dev.cr, DAT_CSP_FIELD_ALL, &cr_param);
	priv_data = cr_param.private_data;
	if ((cmp_pattern != NULL) && (cmp_len != 0)) {

		if (memcmp(priv_data, cmp_pattern, cmp_len) != 0) {
			sprintf(msg_text, "%d Error with CONNECT REQUEST: private data mismatch\n", getpid());
			hxfmsg(stats, HTXERROR(EX_DAPL_ACCEPT, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
			dat_cr_reject(dev->h_dev.cr, 0, NULL);
			return (DAT_ABORT);
		}
	}
	ret = dat_cr_accept(dev->h_dev.cr, dev->h_dev.ep, cmp_len, cr_param.private_data);
    if (ret != DAT_SUCCESS) {
        sprintf(msg_text, "%d Error dat_cr_accept: %s\n", getpid(), dapl_ret_tostr(ret));
		hxfmsg(stats, HTXERROR(EX_DAPL_ACCEPT, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        return (ret);
    } 

	memset(event, 0, sizeof(DAT_EVENT));
	ret = dat_evd_wait(dev->h_evd.conn, DAT_TIMEOUT_INFINITE, 1, event, &nmore);
	if (event->event_number != DAT_CONNECTION_EVENT_ESTABLISHED) {
        sprintf(msg_text, "%d Error unexpected conn event: 0x%x %s\n", getpid(), event->event_number, dapl_event_tostr(event->event_number));
		hxfmsg(stats, HTXERROR(EX_DAPL_ACCEPT, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        return (DAT_ABORT);
    }
	dev->conn_state = DAPL_CONNECTED;
	return (ret);
}

/*
 * Only the client needs to call disconnect. The server _should_ be able
 * to just wait on the EVD associated with connection events for a
 * disconnect request and then exit.
 */
DAT_RETURN dapl_disconnect_ep(struct dapl_device *dev, struct htx_data * stats)
{
    DAT_RETURN ret = DAT_SUCCESS;
    DAT_EVENT event;
    DAT_COUNT nmore;
	char msg_text[1024];

    if (dev->conn_state == DAPL_CONNECTED) {

        ret = dat_ep_disconnect(dev->h_dev.ep, DAT_CLOSE_DEFAULT);
        if (ret != DAT_SUCCESS) {
            sprintf(msg_text, "%d Error dat_ep_disconnect: %s\n", getpid(), dapl_ret_tostr(ret));
			hxfmsg(stats, HTXERROR(EX_DAPL_INIT, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
			return (ret);
        } 

        ret = dat_evd_wait(dev->h_evd.conn, dev->alarm, 1, &event, &nmore);
        if (ret != DAT_SUCCESS) {
            sprintf(msg_text, "%d Error dat_evd_wait: %s\n", getpid(), dapl_ret_tostr(ret));
			hxfmsg(stats, HTXERROR(EX_DAPL_INIT, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        } else {
            printf("%d dat_evd_wait for h_conn_evd completed\n", getpid());
        }
    }
	return (ret);
}

/*
 * server only
 */
DAT_RETURN dapl_destroy_psp(struct dapl_device *dev, struct htx_data * stats)
{
	DAT_RETURN ret;
	DAT_EVENT event;
    DAT_COUNT nmore;
	char msg_text[1024];

	printf("%d Server waiting for disconnect...\n", getpid());
    ret = dat_evd_wait(dev->h_evd.conn, dev->alarm, 1, &event, &nmore);
    if (ret != DAT_SUCCESS) {
        sprintf(msg_text, "%d Error dat_evd_wait: %s\n", getpid(), dapl_ret_tostr(ret));
		hxfmsg(stats, HTXERROR(EX_DAPL_INIT, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
		return(ret); 
    } else {
        printf("%d dat_evd_wait for h_conn_evd completed\n", getpid());
    }
    /* destroy service point */
    if (dev->h_dev.psp != DAT_HANDLE_NULL) {
        ret = dat_psp_free(dev->h_dev.psp);
        if (ret != DAT_SUCCESS) {
            sprintf(msg_text, "%d Error dat_psp_free: %s\n", getpid(), dapl_ret_tostr(ret));
			hxfmsg(stats, HTXERROR(EX_DAPL_INIT, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
			return(ret);
        } else {
            printf("%d dat_psp_free completed\n", getpid());
        }
    }
	return (ret);
}


DAT_RETURN dapl_collect_event(DAT_EVD_HANDLE *dto_evd, DAT_EVENT *event, DAT_TIMEOUT timeout, struct htx_data * stats)
{
    DAT_COUNT   nmore;
    DAT_RETURN  ret1 = DAT_SUCCESS, ret2 = DAT_SUCCESS;
	char msg_text[1024];
	unsigned int granuality = 1000 * 1000; /* 1 seconds */ 
	unsigned int  loop, i; 
	loop = (timeout / granuality) ; 
	if(eeh_enabled) { 
		loop += ((5 * 1000 * 1000)/ granuality); 
	} 
	for(i = 0; i < (loop + 1); i++) { 
		/* Reduce the timeout granuality, due to scheduling issues, 
		 * We may have valid event for us but we are not envoked. 
		 * So timeout fast and check the queue status 
		 */  
    	ret1 = dat_evd_wait(*dto_evd, granuality, 1, event, &nmore);
		if(ret1 == DAT_SUCCESS) { 
			break; 
		} else if (DAT_GET_TYPE(ret1) == DAT_TIMEOUT_EXPIRED) {  
			/* Check if we still have a valid event in queue, */ 
			ret2 = dat_evd_dequeue(*dto_evd, event); 
			if(ret2 == DAT_SUCCESS) { 
				break; 
			} else if(DAT_GET_TYPE(ret2) == DAT_QUEUE_EMPTY) { 
				if(i < loop) { 
					/* We still didnot get our event, probably we timedout fast, 
				 	 * Lets retry !!!!!!
				 	 */ 
					usleep((i + 1 ) * granuality); 
					continue;
				} else {
					/* Give UP !!! */ 
					sprintf(msg_text, "Error waiting on h_dto_evd %x: %s\n", dto_evd, dapl_ret_tostr(ret1));
                	sprintf((msg_text + strlen(msg_text)), "IO_ALARM_TIMEOUT HIT of %x secs !!! %x: %s\n", timeout, dto_evd, dapl_ret_tostr(ret2));
                	hxfmsg(stats, HTXERROR(EX_DAPL_READ_EVENT, ERRNO), HTX_HE_INFO, msg_text);
                	return(DAT_TIMEOUT_EXPIRED);
				}
			} else { /* Anything else is error for me !!!! */    
        		sprintf(msg_text, "Error waiting on h_dto_evd %x: %s\n", dto_evd, dapl_ret_tostr(ret1));
				sprintf((msg_text + strlen(msg_text)), "Error dat_evd_dequeue on h_dto_evd %x: %s\n", dto_evd, dapl_ret_tostr(ret2));
				hxfmsg(stats, HTXERROR(EX_DAPL_READ_EVENT, ERRNO), HTX_HE_INFO, msg_text); 
				return(ret2);
			}  
		} else {
			sprintf(msg_text, "Error waiting on h_dto_evd %x: rc = %#x, %s\n", dto_evd, ret1,  dapl_ret_tostr(ret1));
			hxfmsg(stats, HTXERROR(EX_DAPL_READ_EVENT, ERRNO), HTX_HE_INFO, msg_text);
			return(ret1);
		}
	} 
	return (DAT_SUCCESS);
}

/*
 * post receive packet, dont wait for receive event, sender will wait for receive
 * recive completion flag should be enabled (default)
 */
DAT_RETURN dapl_post_rcv(struct dapl_device *dev, struct mr_attr *rcv_mr, DAT_DTO_COOKIE *cookie, int num_msg, struct htx_data * stats)
{
    DAT_LMR_TRIPLET iov;
    DAT_RETURN ret;
	char msg_text[1024];

    iov.lmr_context = rcv_mr->lmr_ctx;
    iov.virtual_address = (unsigned long long)rcv_mr->region.for_va;
    iov.segment_length = rcv_mr->size;

    printf("%d calling post_recv_msg\n", getpid());
	ret = dat_ep_post_recv(dev->h_dev.ep, 1 /* 1 segment */, &iov, *cookie, DAT_COMPLETION_DEFAULT_FLAG);
    if (ret != DAT_SUCCESS) {
        sprintf(msg_text, "%d Error posting recv msg buffer: %s\n", getpid(), dapl_ret_tostr(ret));
		hxfmsg(stats, HTXERROR(EX_DAPL_INIT, ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        return (ret);
    } 
    return (ret);
}

DAT_RETURN dapl_rdma_write(struct dapl_device *dev, unsigned long long buf_len, struct htx_data * stats)
{
    DAT_LMR_TRIPLET l_iov;
    DAT_RMR_TRIPLET r_iov;
    DAT_DTO_COOKIE cookie;
    DAT_RETURN ret = DAT_SUCCESS;
	DAT_EVENT event;
	char msg_text[1024];
	unsigned int num_retry = 0, retries = 0; 
	unsigned int severity = HTX_HE_SOFT_ERROR; 

	if(eeh_enabled) {  
		num_retry = eeh_retries; 
		severity = HTX_HE_INFO; 
	} 


    /* RMR Triplet that specifies the remote buffer from which the data is read */
    r_iov.virtual_address = dev->rbuf_info.virtual_address;
    r_iov.segment_length = buf_len;
    r_iov.rmr_context = dev->rbuf_info.rmr_context;



    l_iov.lmr_context = dev->snd_mr.lmr_ctx;
    l_iov.segment_length = buf_len;
    l_iov.virtual_address = (DAT_VADDR)((uintptr_t)(dev->snd_mr.region.for_va));

    cookie.as_64 = MSG_RDMA_WRITE;
	do { 
    	ret = dat_ep_post_rdma_write(dev->h_dev.ep, 1, &l_iov, cookie, &r_iov, DAT_COMPLETION_DEFAULT_FLAG);
		if(eeh_enabled) {
			if( ret != DAT_SUCCESS) {
				/* EEH Running in background, we are expected to hit failures,just
			 	 * dont exit, lets sleep and retry 
			 	 */ 
				sprintf(msg_text, "MSG_RDMA_WRITE : ret = %d, Retry #%d, owing to EEH=%d\n",ret, retries, eeh_enabled); 
				hxfmsg(stats, 0, 7, msg_text);  
				retries++; 
				sleep(5); 
			} else {
				/* ret  == DAT_SUCCESS, with eeh */ 
				break; 
			} 
		} else { 
			/* Not EEH !!!! */ 
			if (ret != DAT_SUCCESS) {  
        		sprintf(msg_text, "MSG_RDMA_WRITE : %d: ERROR: dat_ep_post_rdma_write() %s\n", getpid(), dapl_ret_tostr(ret));
        		hxfmsg(stats, HTXERROR(EX_DAPL_RDMAW, ERRNO), severity, msg_text);
        		return (DAT_ABORT);
    		} else { 
				/* ret  == DAT_SUCCESS, with not eeh */ 
				break; 
			}
		}
	} while(retries < num_retry); 

	if((retries == num_retry) && eeh_enabled && ret != DAT_SUCCESS) {
		/* Give UP */  
		sprintf(msg_text, "MSG_RDMA_WRITE : %d: ERROR: dat_ep_post_rdma_write() %s\n", getpid(), dapl_ret_tostr(ret));
		sprintf(msg_text + strlen(msg_text), " EEH Enabled, could not post rdma_write in %d secs. Exiting !!!! \n", (5 * num_retry));  
        hxfmsg(stats, HTXERROR(EX_DAPL_RDMAW, ERRNO), severity, msg_text);
        return (DAT_ABORT);
	} 	
	
	ret = dapl_collect_event(&(dev->h_evd.dto_req), &event, dev->alarm, stats); 
    if (ret != DAT_SUCCESS) {
        sprintf(msg_text, "MSG_RDMA_WRITE : %d: ERROR: dapl_collect_event() %s\n", getpid(), dapl_ret_tostr(ret));
		hxfmsg(stats, HTXERROR(EX_DAPL_RDMAW, ERRNO), severity, msg_text);
        return (DAT_ABORT);
    }
    if (event.event_number != DAT_DTO_COMPLETION_EVENT) { 
        sprintf(msg_text, "MSG_RDMA_WRITE : %d: ERROR: DTO event number %s\n", getpid(), dapl_event_tostr(event.event_number));
		hxfmsg(stats, HTXERROR(EX_DAPL_RDMAW, ERRNO), severity, msg_text);
		return (DAT_ABORT);        
     }

    if ((event.event_data.dto_completion_event_data.transfered_length != buf_len)
        || (event.event_data.dto_completion_event_data.user_cookie.as_64 != MSG_RDMA_WRITE)) {
        sprintf(msg_text, "MSG_RDMA_WRITE : %d: ERROR: DTO len %d or cookie %lx\n", getpid(),
                  event.event_data.dto_completion_event_data.transfered_length,
                  event.event_data.dto_completion_event_data.user_cookie.as_64);
		hxfmsg(stats, HTXERROR(EX_DAPL_RDMAW, ERRNO), severity, msg_text);
        return (DAT_ABORT);
     }

     if (event.event_data.dto_completion_event_data.status != DAT_DTO_SUCCESS) {
        sprintf(msg_text, "MSG_RDMA_WRITE : %d: ERROR: DTO event status %s\n", getpid(), dapl_event_tostr(event.event_data.dto_completion_event_data.status));
		if(event.event_data.dto_completion_event_data.status == DAT_DTO_ERR_TRANSPORT) { 
			if(eeh_enabled) { 
				sprintf(msg_text+strlen(msg_text), "This error may be due to EEH injection running in background, Will retry !!!"  );
			} else {    
				sprintf(msg_text+strlen(msg_text), "This error may be due to EEH injection running in background,Exerciser will exit. \n"); 
			} 
		}  
		hxfmsg(stats, HTXERROR(EX_DAPL_RDMAW, ERRNO), severity, msg_text);
		return (DAT_ABORT);
        
     }
	 
    return (DAT_SUCCESS);
}

DAT_RETURN dapl_rdma_read(struct dapl_device *dev, unsigned long long buf_len, struct htx_data * stats)
{
    DAT_EVENT event;
    DAT_LMR_TRIPLET l_iov;
    DAT_RMR_TRIPLET r_iov;
    DAT_DTO_COOKIE cookie;
    DAT_RETURN ret;
	char msg_text[1024];
    unsigned int num_retry = 0, retries = 0;
	unsigned int severity = HTX_HE_SOFT_ERROR; 

    if(eeh_enabled) {
		severity = HTX_HE_INFO; 
        num_retry = eeh_retries;
    }



	/* RMR Triplet that specifies the remote buffer from which the data is read */
    r_iov.virtual_address = dev->rbuf_info.virtual_address;
	r_iov.segment_length = buf_len;
	r_iov.rmr_context = dev->rbuf_info.rmr_context;


	/* I/O Vector specifying the local data buffer to fill */
    l_iov.lmr_context = dev->rcv_mr.lmr_ctx;
    l_iov.virtual_address = (DAT_VADDR)dev->rcv_mr.region.for_va;
    l_iov.segment_length = buf_len;


    cookie.as_64 = MSG_RDMA_READ;
    do {
    	ret = dat_ep_post_rdma_read(dev->h_dev.ep, 1, &l_iov, cookie, &r_iov, DAT_COMPLETION_DEFAULT_FLAG);   	
        if(eeh_enabled) {
            if( ret != DAT_SUCCESS) {
                /* EEH Running in background, we are expected to hit failures,just
                 * dont exit, lets sleep and retry
                 */
				sprintf(msg_text, "MSG_RDMA_READ : ret = %d, Retry #%d, owing to EEH=%d\n",ret, retries, eeh_enabled); 
				hxfmsg(stats, 0, 7, msg_text);  
                retries++;
                sleep(5);
            } else {
                /* ret  == DAT_SUCCESS, with eeh */
                break;
            }
        } else {
            /* Not EEH !!!! */
            if (ret != DAT_SUCCESS) {
        		sprintf(msg_text, "MSG_RDMA_READ : %d: ERROR: dat_ep_post_rdma_read() %s\n", getpid(), dapl_ret_tostr(ret));
				hxfmsg(stats, HTXERROR(EX_DAPL_RDMAR, ERRNO), severity, msg_text);
                return (DAT_ABORT);
            } else {
                /* ret  == DAT_SUCCESS, with not eeh */
                break;
            }
		}
    } while(retries < num_retry);

    if((retries == num_retry) && eeh_enabled && ret != DAT_SUCCESS) {
        /* Give UP */
        sprintf(msg_text, "MSG_RDMA_READ : %d: ERROR: dat_ep_post_rdma_read() %s\n", getpid(), dapl_ret_tostr(ret));
        sprintf(msg_text + strlen(msg_text), " EEH Enabled, could not post rdma_write in %d secs. Exiting !!!! \n", (5 * num_retry));
        hxfmsg(stats, HTXERROR(EX_DAPL_RDMAW, ERRNO), severity, msg_text);
        return (DAT_ABORT);
    }


     /* RDMA read completion event */
	ret = dapl_collect_event(&(dev->h_evd.dto_req), &event, dev->alarm, stats);
    if (ret != DAT_SUCCESS) {
        sprintf(msg_text, "MSG_RDMA_READ :%d: ERROR: dapl_collect_event() %s\n", getpid(), dapl_ret_tostr(ret));
		hxfmsg(stats, HTXERROR(EX_DAPL_RDMAR, ERRNO), severity, msg_text);
        return (DAT_ABORT);
	}

    /* validate event number, len, cookie, and status */ 
    if (event.event_number != DAT_DTO_COMPLETION_EVENT) {
        sprintf(msg_text, "MSG_RDMA_READ : %d: ERROR: DTO event number %s\n", getpid(), dapl_event_tostr(event.event_number));
		hxfmsg(stats, HTXERROR(EX_DAPL_RDMAR, ERRNO), severity, msg_text);
        return (DAT_ABORT);
    }

    if (event.event_data.dto_completion_event_data.status != DAT_DTO_SUCCESS) {
        sprintf(msg_text, "MSG_RDMA_READ : %d: ERROR: DTO event status %s\n", getpid(), dapl_event_tostr(event.event_data.dto_completion_event_data.status));
        if(event.event_data.dto_completion_event_data.status == DAT_DTO_ERR_TRANSPORT) {
            if(eeh_enabled) {
                sprintf(msg_text+strlen(msg_text), "This error may be due to EEH injection running in background, Will retry !!!"  );
            } else {
                sprintf(msg_text+strlen(msg_text), "This error may be due to EEH injection running in background, Exerciser would Exit . \n");
            }
        }
		hxfmsg(stats, HTXERROR(EX_DAPL_RDMAR, ERRNO), severity, msg_text);
        return (DAT_ABORT);
    }

    if ((event.event_data.dto_completion_event_data.transfered_length != buf_len) 
         || (event.event_data.dto_completion_event_data.user_cookie.as_64 != MSG_RDMA_READ)) {  
        sprintf(msg_text, "MSG_RDMA_READ : %d: ERROR: DTO len %d or cookie %lx\n", getpid(),
                event.event_data.dto_completion_event_data.transfered_length,
                event.event_data.dto_completion_event_data.user_cookie.as_64);
		hxfmsg(stats, HTXERROR(EX_DAPL_RDMAR, ERRNO), severity, msg_text);
    	return (DAT_ABORT);    
    }

    return (DAT_SUCCESS);
}

/******************utils******************/
int dapl_populate_buf(char *ref_buf, unsigned long long length, char *pattern, struct htx_data * stats)
{
	unsigned long long i;
	unsigned char pat = 0;
	char msg_text[1024];

#if 0
	if (pattern == NULL) {
		pattern = (char *) malloc(strlen("HEX512"));
		strncpy(pattern, "HEX512", strlen("HEX512"));
	}
#endif
	if (ref_buf == NULL){
		sprintf(msg_text, "Task[%d]: ERROR: NULL pointer reference buffer\n", getpid());
		hxfmsg(stats, HTXERROR(EX_MALLOC12, ERRNO),HTX_HE_SOFT_ERROR, msg_text);
        return (DAT_ABORT);
	}
	/* HEX512 */
	for (i = 0; i < length; i++) {
		ref_buf[i] = pat++;
	}
	return (DAT_SUCCESS);
}

void dapl_print_addr(struct sockaddr *sa, struct htx_data * stats)
{
	char str[INET6_ADDRSTRLEN] = {" ??? "};

	switch(sa->sa_family) {
		case AF_INET:
			inet_ntop(AF_INET, &((struct sockaddr_in *)sa)->sin_addr, str, INET6_ADDRSTRLEN);
			printf("%d Local Address AF_INET - %s port %d\n", getpid(), str, SERVER_CONN_QUAL);
			break;
		case AF_INET6:
			inet_ntop(AF_INET6, &((struct sockaddr_in6 *)sa)->sin6_addr, str, INET6_ADDRSTRLEN);
			printf("%d Local Address AF_INET6 - %s flowinfo(QPN)=0x%x, port(LID)=0x%x\n", getpid(), str,
					((struct sockaddr_in6 *)sa)->sin6_flowinfo, ((struct sockaddr_in6 *)sa)->sin6_port);
			break;
		default:
			printf("%d Local Address UNKOWN FAMILY - port %d\n", getpid(), SERVER_CONN_QUAL);
	}
}

/*
 * Map DAT_RETURN values to readable strings,
 * but don't assume the values are zero-based or contiguous.
 */
static char errmsg[DAPL_ERR_MSG_LEN] = { 0 };
const char *dapl_ret_tostr(DAT_RETURN ret_value)
{
    const char *major_msg, *minor_msg;

    dat_strerror(ret_value, &major_msg, &minor_msg);

    strcpy(errmsg, major_msg);
    strcat(errmsg, " ");
    strcat(errmsg, minor_msg);

    return errmsg;
}

/*
 * Map DAT_EVENT_CODE values to readable strings
 */
const char *dapl_event_tostr(DAT_EVENT_NUMBER event_code)
{
    unsigned int i;
    static struct {
        const char *name;
        DAT_RETURN value;
    } dat_events[] = {
#define DATxx(x) { # x, x }
        	DATxx(DAT_DTO_COMPLETION_EVENT),
            DATxx(DAT_RMR_BIND_COMPLETION_EVENT),
            DATxx(DAT_CONNECTION_REQUEST_EVENT),
            DATxx(DAT_CONNECTION_EVENT_ESTABLISHED),
            DATxx(DAT_CONNECTION_EVENT_PEER_REJECTED),
            DATxx(DAT_CONNECTION_EVENT_NON_PEER_REJECTED),
            DATxx(DAT_CONNECTION_EVENT_ACCEPT_COMPLETION_ERROR),
            DATxx(DAT_CONNECTION_EVENT_DISCONNECTED),
            DATxx(DAT_CONNECTION_EVENT_BROKEN),
            DATxx(DAT_CONNECTION_EVENT_TIMED_OUT),
            DATxx(DAT_CONNECTION_EVENT_UNREACHABLE),
            DATxx(DAT_ASYNC_ERROR_EVD_OVERFLOW),
            DATxx(DAT_ASYNC_ERROR_IA_CATASTROPHIC),
            DATxx(DAT_ASYNC_ERROR_EP_BROKEN),
            DATxx(DAT_ASYNC_ERROR_TIMED_OUT),
            DATxx(DAT_ASYNC_ERROR_PROVIDER_INTERNAL_ERROR),
            DATxx(DAT_SOFTWARE_EVENT)
#undef DATxx
    };
#define NUM_EVENTS (sizeof(dat_events)/sizeof(dat_events[0]))

    for (i = 0; i < NUM_EVENTS; i++) {
        if (dat_events[i].value == event_code) {
            return (dat_events[i].name);
        }
    }

    return ("Invalid_DAT_EVENT_NUMBER");
}
void dapl_dump_ia_attr(DAT_IA_ATTR *ia_attr)
{
    printf("------------IA Attributes DUMP START------------\n");
    printf("adapter name: %s\n", ia_attr->adapter_name);
    printf("vendor_name: %s\n", ia_attr->vendor_name);
    printf("hardware_version_major: %d\n", ia_attr->hardware_version_major);
    printf("hardware_version_minor: %d\n", ia_attr->hardware_version_minor);
    printf("firmware_version_major: %d\n", ia_attr->firmware_version_major);
    printf("firmware_version_minor: %d\n", ia_attr->firmware_version_minor);
    printf("ia_address_ptr: %p\n", ia_attr->ia_address_ptr);
    printf("max_eps: %d\n", ia_attr->max_eps);
    printf("max_dto_per_ep: %d\n", ia_attr->max_dto_per_ep);
    printf("max_rdma_read_per_ep_in: %d\n", ia_attr->max_rdma_read_per_ep_in);
    printf("max_rdma_read_per_ep_out: %d\n", ia_attr->max_rdma_read_per_ep_out);
    printf("max_evds: %d\n", ia_attr->max_evds);
    printf("max_evd_qlen: %d\n", ia_attr->max_evd_qlen);
    printf("max_iov_segments_per_dto: %d\n", ia_attr->max_iov_segments_per_dto);
    printf("max_lmrs: %d\n", ia_attr->max_lmrs);
    printf("max_lmr_block_size: 0x%x\n", ia_attr->max_lmr_block_size);
    printf("max_lmr_virtual_address: 0x%lx\n", ia_attr->max_lmr_virtual_address);
    printf("max_pzs: %d\n", ia_attr->max_pzs);
    printf("max_message_size: 0x%x\n", ia_attr->max_message_size);
    printf("max_rdma_size: 0x%x\n", ia_attr->max_rdma_size);
    printf("max_rmrs: %d\n", ia_attr->max_rmrs);
    printf("max_rmr_target_address: 0x%lx\n", ia_attr->max_rmr_target_address);
    printf("max_iov_segments_per_rdma_read: %d\n", ia_attr->max_iov_segments_per_rdma_read);
    printf("max_iov_segments_per_rdma_write: %d\n", ia_attr->max_iov_segments_per_rdma_write);
    printf("------------IA Attributes DUMP END------------\n");
}
void dapl_dump_ep_attr(DAT_EP_ATTR *ep_attr)
{
    printf("------------EP Attributes DUMP START------------\n");
    printf("service_type: %d\n", ep_attr->service_type);
    printf("max_message_size: 0x%x\n", ep_attr->max_message_size);
    printf("max_rdma_size: 0x%x\n", ep_attr->max_rdma_size);
    printf("recv_completion_flags: 0x%x\n", ep_attr->recv_completion_flags);
    printf("request_completion_flags: 0x%x\n", ep_attr->request_completion_flags);
    printf("max_recv_dtos: 0x%x\n", ep_attr->max_recv_dtos);
    printf("max_request_dtos: %d\n", ep_attr->max_request_dtos);
    printf("max_recv_iov: %d\n", ep_attr->max_recv_iov);
    printf("max_request_iov: %d\n", ep_attr->max_request_iov);
    printf("max_rdma_read_in: %d\n", ep_attr->max_rdma_read_in);
    printf("max_rdma_read_out: %d\n", ep_attr->max_rdma_read_out);
    printf("max_rdma_read_iov: %d\n", ep_attr->max_rdma_read_iov);
    printf("max_rdma_write_iov: %d\n", ep_attr->max_rdma_write_iov);
    printf("------------EP Attributes DUMP END------------\n");
}
void dapl_dump_ep_param(DAT_EP_PARAM *ep_param)
{
    printf("------------EP Parameter DUMP START------------\n");
    printf("ia_handle: %p\n", ep_param->ia_handle);
    printf("ep_state: 0x%x\n", ep_param->ep_state);
    printf("local_ia_address_ptr: %p\n", ep_param->local_ia_address_ptr);
    printf("local_port_qual: 0x%x\n", (int)ep_param->local_port_qual);
    printf("remote_ia_address_ptr: %p\n", ep_param->remote_ia_address_ptr);
    printf("remote_port_qual: 0x%x\n", (int)ep_param->remote_port_qual);
    printf("pz_handle: %p\n", ep_param->pz_handle);
    printf("recv_evd_handle: %p\n", ep_param->recv_evd_handle);
    printf("request_evd_handle: %p\n", ep_param->request_evd_handle);
    printf("connect_evd_handle: %p\n", ep_param->connect_evd_handle);
    dapl_dump_ep_attr(&ep_param->ep_attr);
    printf("------------EP Parameter DUMP END------------\n");
}

/*****************************************************************/
/*************hxecom plugins**************************************/
/*****************************************************************/
/*
 * Allocate dapl instance and create end point (QP)
 */
int dapl_init_instance(char *dev_name, int Idx,  unsigned short port_num, struct dapl_device **dev, struct htx_data * stats, DAT_TIMEOUT alarm)
{
	DAT_RETURN ret;
	char provider[PROVIDER_NAME_LEN];

	ret = dapl_get_provider(dev_name, provider, stats);
	if (ret) {
		return (ret);
	}
	if (*dev == NULL) {
    	*dev = malloc(sizeof(struct dapl_device));
	}
    if (*dev == NULL) {
        perror("dapl instance allocation failed\n");
        return(errno);
    }
    memset(*dev, 0, sizeof(struct dapl_device));

	strcpy((*dev)->provider_name, provider);
    (*dev)->conn_id = htons(port_num);
	(*dev)->assync_evd_qlen = ASSYNC_EVD_DEFAULT_QLEN;

	ret = dapl_open(*dev, (*dev)->assync_evd_qlen, stats);
    if (ret != DAT_SUCCESS) {
        goto cleanup0;
    }
	ret = dapl_query(*dev, stats);
    if (ret != DAT_SUCCESS) {
        printf("dat ia query error\n");
        goto cleanup1;
    }
    dapl_dump_ia_attr(&((*dev)->ia_attr));
	ret = dapl_create_pz(*dev, stats);
    if (ret) {
        goto cleanup1;
    }
	(*dev)->alarm = alarm; 
	(*dev)->stats = stats; 
	(*dev)->idx = Idx; 
	ret = dapl_create_event(*dev, (*dev)->assync_evd_qlen, stats);
    if (ret) {
        goto cleanup2;
    }
    ret = dapl_create_ep(*dev, DAT_SERVICE_TYPE_RC, stats);
    if (ret) {
        goto cleanup3;
    } else {
		goto cleanup0;
	}

cleanup3:
	dapl_destroy_events(*dev, stats);
cleanup2:
	dapl_free_pz(*dev, stats);
cleanup1:
	dapl_close(*dev, stats);
cleanup0:
    return (ret);
}
/*
 * Destroy end point and its associated resources, De - allocate dapl instance
 */
void dapl_cleanup_instance(struct dapl_device *dev)
{
	if(!dev) { 
		dev = NULL; 
		return; 
	}

	struct  htx_data * stats = dev->stats; 
	dapl_flush_evds(dev, stats);
	dapl_free_ep(dev, stats);
	dapl_destroy_events(dev, stats);
	dapl_free_pz(dev, stats);
	dapl_unregister_mem(&dev->snd_mr, stats);
	dapl_unregister_mem(&dev->rcv_mr, stats);
	dapl_close(dev, stats);
	free(dev);
	dev = NULL; 
}

struct dapl_device *dapl_get_instance(struct htx_data * stats)
{
	return (dapl_obj);
}

/*
 * get provider name from dat.conf file
 * for the device supplied as an argument (/dev/<dev_name>)
 * to the exerciser
 */

int dapl_get_provider(char *device_name, char *provider, struct htx_data * stats)
{
#define COMMAND_LEN		250
#define DAT_CONF_DEFAULT "/etc/dat.conf"

    
    char cmd[COMMAND_LEN];
	int malloc_flag = 0;
    char *dat_conf_file = NULL;
	char *nl_ptr = NULL; /* for newline stored by gets in the buffer */
    FILE *fp;

    errno = 0;  /* Reset the error number */
    fflush(stdout);

    dat_conf_file = getenv("HXECOM_DAT_CONF");
    if (dat_conf_file == NULL) {
		dat_conf_file = (char *)malloc(COMMAND_LEN);
		if (dat_conf_file == NULL) {
			perror("dapl_get_provider");
			return (-1 * errno);
		}
		malloc_flag = 1;
    	strcpy(dat_conf_file, DAT_CONF_DEFAULT);
    }
    
    if((fp = fopen(dat_conf_file, "r")) == NULL) {
        printf("Error: Could not find the file: %s\n", dat_conf_file);
		if (malloc_flag == 1) {
			free(dat_conf_file);
			malloc_flag = 0;
		}
    	return (-1);
    }else {
    	fclose(fp);
    }
    
    sprintf(cmd, "grep %s %s | awk \' {print $1}\'", device_name, dat_conf_file);
    printf("Cmd - %s\n", cmd);

    if ((fp = popen(cmd, "r")) == NULL) {
		perror("popen failed: ");
		if (malloc_flag == 1) {
			free(dat_conf_file);
			malloc_flag = 0;
		}
		return (errno);
    }
    if (fgets(provider, PROVIDER_NAME_LEN, fp) != NULL) {
		/* check for newline stored in the buffer */
		if ((nl_ptr = strchr(provider, '\n')) != NULL) {
			*nl_ptr = '\0';
		}
        printf("DAT Provider name for %s is %s\n", device_name, provider);
    }
    else {
		perror("fgets failed: ");
		pclose(fp);
		if (malloc_flag == 1) {
			free(dat_conf_file);
			malloc_flag = 0;
		}
        return (-1);
    }
    pclose (fp);
	if (malloc_flag == 1) {
		free(dat_conf_file);
		malloc_flag = 0;
	}
    return (0);
}
void dapl_flush_evds(struct dapl_device *dev, struct htx_data * stats)
{
    DAT_EVENT event;

    /* Flush async error event queue */
    printf("Task[%d] Checking ASYNC EVD[%p]...\n", getpid(), dev->h_evd.async);
    while (dat_evd_dequeue(dev->h_evd.async, &event) == DAT_SUCCESS) {
        printf("ASYNC EVD ENTRY: async evd handle = %p reason = %d\n",
            event.event_data.asynch_error_event_data.dat_handle,
            event.event_data.asynch_error_event_data.reason);
    }
    /* Flush receive event queue */
    printf("Task[%d] Checking RECEIVE EVD[%p]...\n", getpid(), dev->h_evd.dto_rcv);
    while (dat_evd_dequeue(dev->h_evd.dto_rcv, &event) == DAT_SUCCESS) {
        printf("RCV EVD ENTRY: operation = %d status = %d transferred len = %d cookie = %lx\n",
            event.event_data.dto_completion_event_data.operation,
            event.event_data.dto_completion_event_data.status,
            event.event_data.dto_completion_event_data.transfered_length,
            event.event_data.dto_completion_event_data.user_cookie.as_64);
    }
    /* Flush request queue */
    printf("Task[%d] Checking REQUEST EVD[%p]...\n", getpid(), dev->h_evd.dto_req);
    while (dat_evd_dequeue(dev->h_evd.dto_req, &event) == DAT_SUCCESS) {
        printf(" REQ EVD ENTRY: operation = %d status = %d transferred len = %d cookie = %lx\n",
            event.event_data.dto_completion_event_data.operation,
            event.event_data.dto_completion_event_data.status,
            event.event_data.dto_completion_event_data.transfered_length,
            event.event_data.dto_completion_event_data.user_cookie.as_64);
    }
}
