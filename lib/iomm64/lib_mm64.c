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

#include <sys/types.h>
#include <iomm/lib_mm64.h>
#include <hxihtx64.h>
#include <sevcodes.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <memory.h>
#include <stdarg.h>

#ifdef __HTX_LINUX__
#include <sys/stat.h>
#endif

#ifndef __HTX_LINUX__
#include <sys/vminfo.h>
#endif

extern struct htx_data htx_d;

/********************************************************
 * HTX IO descriptor pool management routines           *
 * creates, manages and destroy descriptor pool memory	*
 * written for IO adapters exerciser development		*
 ********************************************************/

/* global instance to handle pool management
 * each index to buflist points to a pool of
 * buffers of same size, specified by user
 * for each index or size there is a list of "free"
 * and "inuse" pool of buffers
 */
static struct buf_pool buflist[NUM_BUF_SIZES];
/* call not exposed to user */
int iomm_free_pool(struct buf_pool *pool);


/*
 * Function to cleanup the the page (4K, 64K, 16M) pool shared memory
 */
int iomm_cleanup_page_pool(int free_index, long psize)
{
    int rc, index = -1, free_count = 0, i;
    struct buf_node *tmp = NULL, *bn = NULL;
    char *shm_start_addr;

    PRINT(("IN %s: freeing the buf nodes for page size = 0x%lx\n", __FUNCTION__, psize))

    /* check free index for psize */
    for(i = 0; i < NUM_BUF_SIZES; i++) {
        if (buflist[i].page_size == psize) {
            index = i;
            break;
        }
    }

    PRINT(("index = %d, free_index = %d\n", index, free_index))

    if (index == -1 || index != free_index) {
        sprintf(htx_d.msg_text,"IN %s: Large Page Pool Not Found", __FUNCTION__);
        hxfmsg(&htx_d, -1, HTX_HE_HARD_ERROR, htx_d.msg_text);
        return -1;
    }

    PRINT(("IN %s: index = %d, shm_start_addr = %p\n", __FUNCTION__, index, buflist[index].shm_start_addr))
    shm_start_addr = buflist[index].shm_start_addr;

	if (shm_start_addr) {
    	PRINT(("IN %s: Calling shmdt for addr = %p\n", __FUNCTION__, shm_start_addr))
    	rc = shmdt(shm_start_addr);
    	if (rc != 0) {
        	sprintf(htx_d.msg_text,"%s: shmdt failed with %d", __FUNCTION__, errno);
        	hxfmsg(&htx_d, rc, HTX_HE_HARD_ERROR, htx_d.msg_text);
        	return(-1);
    	}
	}


    if (buflist[index].shm_id != 0) {
        rc = shmctl(buflist[index].shm_id, IPC_RMID, (struct shmid_ds *)NULL);
        if (rc != 0) {
            sprintf(htx_d.msg_text,"%s: shmctl for IPC_RMID failed with %d",__FUNCTION__, errno);
            hxfmsg(&htx_d, rc, HTX_HE_HARD_ERROR, htx_d.msg_text);
            return(-1) ;
        }
        buflist[index].shm_id = 0 ;
    }

    buflist[index].num_free = 0;
    buflist[index].num_elements = 0;
    buflist[index].size = 0;
    buflist[index].page_size = 0;
    buflist[index].shm_id = 0;
    buflist[index].shm_pool = 0;

    for(bn = buflist[index].free, free_count = 0; bn;) {
        tmp = bn;
        bn = bn->next;
        free(tmp);
        free_count++;
    }

    PRINT(("%d nodes of free list freed\n", free_count))
    PRINT(("Freeing inuse list\n"))

    for(bn = buflist[index].inuse, free_count = 0; bn;) {
        tmp = bn;
        bn = bn->next;
        free(tmp);
        free_count++;
    }

    PRINT(("%d nodes of inuse list freed\n", free_count))
    PRINT(("OUT %s: done\n", __FUNCTION__))
	return (0);
}

/*
 * creates a pool of buffers backed by the page size supplied
 * constituting num_elements for the specified buffer size
 * returns handle to the pool to user
 * Support for 4K, 64K, 16M pages
 * ******************************
 * e.g. user can request for buffer backed by 16M page size.
 * Depending upton num_elements, a shared memory segment is created of size (16M * num_elements ).
 * If successful, then  create as many nodes as num_pages. The unaligned_addr pointer of each node
 * points to the starting address of a 16M page.
 * Feature# Add shmat addr to shmat call : 13/10/2009
 */

int iomm_init_page_pool(long *pool_handle, void *shmat_addr, long page_size, long buf_size, int num_bufs, int *out_npages)
{
    int i, rc;
    int node_count = 0;
    long free_index = -1;

    unsigned long num_lpage_requested = 0;
    unsigned long num_lpage_available = 0;
    unsigned long sys_pg_flag = 0;

    size_t shm_size;
    unsigned int shm_flag = (IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    struct shmid_ds shm_buf;

    struct buf_node *tmp = NULL;

    #ifndef __HTX_LINUX__
    	struct vminfo vmi;
    	int num_psizes;
	#else
    	unsigned long num_lpages_rsvd = 0;
    	FILE *fp;
	#endif

    memset(&shm_buf, 0, sizeof(struct shmid_ds));

	PRINT(("-------------------Entry %s----------------------------\n", __FUNCTION__))
    PRINT(("IN %s: base page size = %d, input page size = %ld\n", __FUNCTION__, getpagesize(), page_size))

    if (page_size == getpagesize()) {
    	sys_pg_flag = 1; /* true */
		PRINT(("IN %s: page_size 0x%lx is the same as system_page_size\n", __FUNCTION__, page_size))
    }
	else {
        #ifdef __HTX_LINUX__
        if ((page_size == PG_SZ_4K) && (getpagesize() == PG_SZ_64K)) {
            rc = -1;
            sprintf(htx_d.msg_text, "iomm_init_page_pool: Not supported: page size %ld, where base page size is %d", page_size, getpagesize());
            hxfmsg(&htx_d, rc, HTX_HE_HARD_ERROR, htx_d.msg_text);
            return (rc);
        }
        #endif
    }


    PRINT(("IN %s: shmat addr = %p page size = 0x%lx buf_size = 0x%lx num_bufs = %d\n", __FUNCTION__, shmat_addr, page_size, buf_size, num_bufs))

    *pool_handle = free_index; /* mark handle invalid */

    for (i = 0; i < NUM_BUF_SIZES; i++) {
         /* check for a free index for new size */
         if (buflist[i].size == 0) {
             free_index = i;
             break;
         }
    }
    if (free_index == -1) {
        sprintf(htx_d.msg_text, "iomm_init_page_pool: pool initialization failed for buf size = %ld", buf_size);
        hxfmsg(&htx_d, -ENOMEM, HTX_HE_HARD_ERROR, htx_d.msg_text);
        return (-ENOMEM);
    }

    *pool_handle = free_index;
    PRINT(("IN %s: got pool_handle [%ld] for page size: 0x%lx buf size = 0x%lx\n", __FUNCTION__, free_index, page_size, buf_size))

    *out_npages = 0;
    buflist[free_index].shm_pool = 1;
    buflist[free_index].page_size = page_size;
    buflist[free_index].size = buf_size;
    buflist[free_index].alignment_size = 0; /*align_size;*/
    buflist[free_index].num_free = buflist[free_index].num_elements = 0;
    buflist[free_index].free = buflist[free_index].inuse = NULL;

    PRINT(("iomm_init_page_pool: buf_size = %ld, num_bufs = %d, page size = %lx\n", buf_size, num_bufs, page_size))
    num_lpage_requested = (buf_size * num_bufs) / page_size;
    if (((buf_size * num_bufs) % page_size)  != 0) {
        num_lpage_requested++;
    }
   	PRINT(("IN %s: Num pages to be allocated = %ld with page size = %ld\n", __FUNCTION__, num_lpage_requested, page_size))

    if (!sys_pg_flag && (page_size != PG_SZ_64K) && (page_size != PG_SZ_4K)) {
	#ifndef __HTX_LINUX__
	  #if !defined(SET_PAGESIZE)
	  	shm_flag |= (SHM_LGPAGE | SHM_PIN);
	    PRINT(("iomm_init_page_pool : In AIX part, memflg = SHM_LGPAGE, SHM_PIN\n"))
	  #endif
    	num_psizes = vmgetinfo(NULL, VMINFO_GETPSIZES, 0);
    	if (num_psizes == -1) {
        	*pool_handle = -1;
        	sprintf(htx_d.msg_text, "iomm_init_page_pool: vmgetinfo call for VMINFO_GETPSIZES failed to get num_psizes, errno = %d\n", errno);
        	hxfmsg(&htx_d, errno, HTX_HE_HARD_ERROR, htx_d.msg_text);
        	return (-1);
    	}
    	PRINT(("IN %s: num page sizes enabled = %d\n", __FUNCTION__, num_psizes))

    	rc = vmgetinfo(&vmi, VMINFO, sizeof(struct vminfo));
    	if (rc != 0) {
        	*pool_handle = -1;
        	sprintf(htx_d.msg_text, "vmgetinfo call for VMINFO cmd failed with errno = %d\n", errno);
        	hxfmsg(&htx_d, errno, HTX_HE_HARD_ERROR, htx_d.msg_text);
        	return (-1);
    	}
    	PRINT(("lpage size from vmgetinfo = %d\n", vmi.lgpg_size))
    	PRINT(("Num 16M present = 0x%llx\n", vmi.lgpg_cnt))
    	PRINT(("Num 16M free = 0x%llx\n", vmi.lgpg_numfrb))
    	num_lpage_available = vmi.lgpg_numfrb;
        PRINT(("In AIX part, num_lpages_available = %ld\n", num_lpage_available))
	#else
		shm_flag |= (SHM_HUGETLB | IPC_EXCL);
	    PRINT(("In Linux part, memflg = SHM_HUGETLB, IPC_EXCL\n"))
        fp = popen("cat /proc/meminfo | grep HugePages_Free | awk '{print $2}' ","r");
        if (fp == NULL || fp == -1) {
            sprintf(htx_d.msg_text, "popen failed: errno(%d)\n", errno);
            hxfmsg(&htx_d, errno, HTX_HE_HARD_ERROR, htx_d.msg_text);
            return (-1);
        }
        rc = fscanf(fp, "%ld\n", &num_lpage_available);
        fclose(fp);
        if (rc == 0 || rc == EOF) {
            sprintf(htx_d.msg_text, "Could not read num free huge pages from pipe");
            hxfmsg(&htx_d, errno, HTX_HE_HARD_ERROR, htx_d.msg_text);
            return (-1);
        }
        fp = popen("cat /proc/meminfo | grep HugePages_Rsvd | awk '{print $2}' ","r");
		if (fp == NULL || fp == -1 ) {
	    	sprintf(htx_d.msg_text, "popen failed: errno(%d)", errno);
	    	hxfmsg(&htx_d, errno, HTX_HE_HARD_ERROR, htx_d.msg_text);
	    	return (-1);
		}
		rc = fscanf(fp,"%ld\n", &num_lpages_rsvd);
        fclose(fp);
		if (rc == 0 || rc == EOF) {
	    	sprintf(htx_d.msg_text, "Could not read num reserved huge pages from pipe");
	    	hxfmsg(&htx_d, errno, HTX_HE_HARD_ERROR, htx_d.msg_text);
	    	return (-1);
		}
		num_lpage_available -= num_lpages_rsvd;

        PRINT(("In Linux part, num_lpages_available = %ld\n", num_lpage_available))
	#endif

        if (num_lpage_requested > num_lpage_available) {
            *pool_handle = -1;
            *out_npages = 0;
            sprintf(htx_d.msg_text, "Error: lg page free = %ld requested = %ld", num_lpage_available, num_lpage_requested);
            hxfmsg(&htx_d, -1, HTX_HE_HARD_ERROR, htx_d.msg_text);
            return (-1);
        }
        PRINT(("num_lpage_requested = %ld, num_lpage_available = %ld\n", num_lpage_requested, num_lpage_available))
    }

    /* alocate node and correspomding shared memory */
    buflist[free_index].num_free = buflist[free_index].num_elements = num_bufs;
    buflist[free_index].free = (struct buf_node *) malloc(sizeof(struct buf_node));
    if (NULL == buflist[free_index].free) {
        sprintf(htx_d.msg_text, "malloc failed size: %lx, Line:%d, File:%s", sizeof(struct buf_node), __LINE__, __FILE__);
        hxfmsg(&htx_d, -ENOMEM, HTX_HE_HARD_ERROR, htx_d.msg_text);
        iomm_free_pool_list(free_index);
        return (-ENOMEM);
    }
    bzero(buflist[free_index].free, sizeof(struct buf_node));

    shm_size = page_size * num_lpage_requested;
    PRINT(("calling shmget for page size: %ld, shm_size = 0x%x\n", page_size, shm_size))
    buflist[free_index].shm_id = buflist[free_index].free->shm_id = shmget(IPC_PRIVATE, shm_size, shm_flag);
    if (buflist[free_index].free->shm_id == -1) {
        printf("shmget failed with %d ! ... calling cleanup...\n", errno);
        sprintf(htx_d.msg_text, "shmget failed with %d !", errno);
        hxfmsg(&htx_d, errno, HTX_HE_HARD_ERROR, htx_d.msg_text);
        iomm_cleanup_page_pool(free_index, page_size);
        return(-1);
    }


    #if !defined(__HTX_LINUX__) && defined(SET_PAGESIZE)
        PRINT(("In AIX part, calling shmctl\n"))
        if (shm_size > 256*M) {
            PRINT(("Before calling shmctl for SHM_GETLBA\n"))
            if ((rc = shmctl(buflist[free_index].free->shm_id, SHM_GETLBA, &shm_buf)) == -1) {
                iomm_cleanup_page_pool(free_index, page_size);
                sprintf(htx_d.msg_text,"shmctl failed to get minimum alignment requirement - %d", errno);
                hxfmsg(&htx_d, rc, HTX_HE_HARD_ERROR, htx_d.msg_text);
                return(-1);
            }
        }
        shm_buf.shm_pagesize = page_size;
        PRINT(("shm_buf.shm_pagesize = %llx, calling shmctl now for SHM_PAGESIZE\n", shm_buf.shm_pagesize))
        if ((rc = shmctl(buflist[free_index].free->shm_id, SHM_PAGESIZE, &shm_buf)) == -1) {
        	sprintf(htx_d.msg_text,"shmctl failed with %d while setting page size",errno);
            hxfmsg(&htx_d, rc, HTX_HE_HARD_ERROR, htx_d.msg_text);
            iomm_cleanup_page_pool(free_index, page_size);
            return(-1);
       	}

    #endif

    /* attach to shmid */
    buflist[free_index].free->unaligned_addr = buflist[free_index].free->aligned_addr = (char *) shmat(buflist[free_index].free->shm_id, shmat_addr, 0);
    if (buflist[free_index].free->unaligned_addr == (char *)-1) {
        sprintf(htx_d.msg_text, "shmat failed with %d", errno);
        hxfmsg(&htx_d, -1, HTX_HE_HARD_ERROR, htx_d.msg_text);
        iomm_cleanup_page_pool(free_index, page_size);
        return(-1);
    }

#if 0
    #if !defined(__HTX_LINUX__)
        PRINT(("In AIX part, calling mlock\n"))
        rc = mlock(buflist[free_index].free->unaligned_addr, shm_size);
        if (rc == -1) {
            sprintf(htx_d.msg_text,"mlock failed with %d", errno);
            hxfmsg(&htx_d, rc, HTX_HE_HARD_ERROR, htx_d.msg_text);
            iomm_cleanup_page_pool(free_index, page_size);
            return(-1);
        }
	#else
        PRINT(("In linux part, calling shmctl with flag SHM_LOCK\n"))
        if ((rc = shmctl(buflist[free_index].free->shm_id, SHM_LOCK, &shm_buf)) == -1) {
            sprintf(htx_d.msg_text, "shmctl failed to get minimum alignment requirement - %d", errno);
            hxfmsg(&htx_d, rc, HTX_HE_HARD_ERROR, htx_d.msg_text);
            iomm_cleanup_page_pool(free_index, page_size);
            return(-1);
        }
	#endif
#endif

    PRINT(("IN %s: shm attached at: %p\n", __FUNCTION__, buflist[free_index].free->unaligned_addr))
	memset(buflist[free_index].free->unaligned_addr, 0, shm_size);

    buflist[free_index].shm_start_addr = buflist[free_index].free->unaligned_addr;
    buflist[free_index].free->page_size = page_size;
    buflist[free_index].free->size = buf_size;
    buflist[free_index].free->next = NULL;

    tmp = buflist[free_index].free;
    node_count++;

    for(i = 1; i < num_bufs; i++) {
        tmp->next = (struct buf_node *) malloc(sizeof(struct buf_node));
        if (NULL == tmp->next) {
            sprintf(htx_d.msg_text, "malloc failed Size: %lx, Line:%d, File:%s", sizeof(struct buf_node),__LINE__, __FILE__);
            hxfmsg(&htx_d, -ENOMEM, HTX_HE_HARD_ERROR, htx_d.msg_text);
            iomm_cleanup_page_pool(free_index, page_size);
            return(-ENOMEM);
        }
        bzero(tmp->next, sizeof(struct buf_node));
        /*PRINT(("i = %d, addr = %p\n", i, (char *)shm_start_addr + (buf_size * (i - 1)) ))*/
        tmp->next->unaligned_addr = tmp->next->aligned_addr = buflist[free_index].shm_start_addr + (buf_size * i);
        tmp->next->page_size = page_size;
		tmp->next->size = buf_size;
        tmp = tmp->next;
        node_count++;
    }

    *out_npages = num_lpage_requested;

    PRINT(("Num nodes: %d size %ld\n", node_count, buflist[free_index].size))
    for(i = 0, tmp = buflist[free_index].free; i < node_count; i++) {
        PRINT(("Node assig addr[%d] = %p, buf size = %ld, page size = %ld\n", i, tmp->unaligned_addr, buf_size, page_size))
        tmp = tmp->next;
    }
	PRINT(("-------------------Exit %s----------------------------\n", __FUNCTION__))

    return (0);
}

/*
 * creates a pool of buffers constituting num_elements for the specified buffer size
 * returns handle to the pool to user
 */

int iomm_init_pool(long *pool_handle, long buf_size, long align_size, int num_elements)
{
    int i, j, node_count = 0;
    struct buf_node *tmp;
    unsigned long tmp1;
	long free_index = -1;

	*pool_handle = free_index; /* mark handle invalid */

	/* check for a free index for new size */
	for(i = 0; i < NUM_BUF_SIZES; i++) {
    	if (buflist[i].size == 0) {
			free_index = i;
			break;
		}
	}

/*	sprintf(htx_d.msg_text, "iomm_init_pool: free_index =%ld\n", free_index); */
/*	hxfmsg(&htx_d,0, HTX_HE_INFO, htx_d.msg_text); */


	if (free_index == -1) {
   		sprintf(htx_d.msg_text, "iomm_init_pool: pool initialization failed for buf size = %ld", buf_size);
       	hxfmsg(&htx_d, -ENOMEM, HTX_HE_HARD_ERROR, htx_d.msg_text);
       	return (-ENOMEM);
    }

    buflist[free_index].page_size = getpagesize();
    buflist[free_index].size = buf_size;
    buflist[free_index].alignment_size = align_size;
	buflist[free_index].num_free = buflist[free_index].num_elements = num_elements;
	buflist[free_index].free = buflist[free_index].inuse = NULL;

	for(j = 0; j < buflist[free_index].num_elements; j++) {
	    if (j == 0) {
			buflist[free_index].free = (struct buf_node *)malloc(sizeof(struct buf_node));
			if (NULL == buflist[free_index].free) {
			   	sprintf(htx_d.msg_text, "iomm_init_pool: malloc failed Size: %lx, Line:%d, File:%s", sizeof(struct buf_node), __LINE__, __FILE__);
				hxfmsg(&htx_d, -ENOMEM, HTX_HE_HARD_ERROR, htx_d.msg_text);
		    	iomm_free_pool_list(free_index);
		    	return(-ENOMEM);
			}
			bzero(buflist[free_index].free, sizeof(struct buf_node));
			buflist[free_index].free->unaligned_addr = (char *)malloc(buflist[free_index].size + buflist[free_index].alignment_size);
			if (NULL == buflist[free_index].free->unaligned_addr) {
				sprintf(htx_d.msg_text, "iomm_init_pool: malloc failed Size: %ld, Line:%d, File:%s", buflist[free_index].size,__LINE__, __FILE__);
				hxfmsg(&htx_d, -ENOMEM, HTX_HE_HARD_ERROR, htx_d.msg_text);
		    	iomm_free_pool_list(free_index);
		    	return(-ENOMEM);
			}
			bzero(buflist[free_index].free->unaligned_addr, buflist[free_index].size + buflist[free_index].alignment_size);

            buflist[free_index].free->page_size = getpagesize();

			if (buflist[free_index].alignment_size != 0) {
		    	tmp1 = (((unsigned long )buflist[free_index].free->unaligned_addr + (buflist[free_index].alignment_size - 1)) & \
		    	(~(buflist[free_index].alignment_size - 1)));

		    	buflist[free_index].free->aligned_addr = (char *)tmp1;
		    	PRINT(("iomm_init_pool: alignment_size = %d, unaligned_addr = %p, aligned_addr = %p\n", buflist[free_index].alignment_size,  \
		    	buflist[free_index].free->unaligned_addr, buflist[free_index].free->aligned_addr))
		    	buflist[free_index].align = 1;
			}

			tmp = buflist[free_index].free;
			node_count++;
	    }
	    else {
			tmp->next = (struct buf_node *)
			malloc(sizeof(struct buf_node));
			if (NULL == tmp->next) {
				sprintf(htx_d.msg_text, "iomm_init_pool: malloc failed Size: %lx, Line:%d, File:%s", sizeof(struct buf_node),__LINE__, __FILE__);
				hxfmsg(&htx_d, ENOMEM, HTX_HE_HARD_ERROR, htx_d.msg_text);
		    	iomm_free_pool_list(free_index);
		    	return(ENOMEM);
			}
			bzero(tmp->next, sizeof(struct buf_node));
			tmp->next->unaligned_addr = (char *)malloc(buflist[free_index].size + buflist[free_index].alignment_size);
			if (NULL == tmp->next->unaligned_addr) {
				sprintf(htx_d.msg_text, "iomm_init_pool: malloc failed Sixe: %ld, Line:%d, File:%s", buflist[free_index].size,__LINE__, __FILE__);
				hxfmsg(&htx_d, ENOMEM, HTX_HE_HARD_ERROR, htx_d.msg_text);
		    	iomm_free_pool_list(free_index);
		    	return(ENOMEM);
			}
			bzero(tmp->next->unaligned_addr, buflist[free_index].size + buflist[free_index].alignment_size);

            tmp->next->page_size = getpagesize();

			if (buflist[free_index].alignment_size != 0) {
		    	tmp1 = (((unsigned long )tmp->next->unaligned_addr + (buflist[free_index].alignment_size - 1)) & \
		    	(~(buflist[free_index].alignment_size - 1)));
		    	tmp->next->aligned_addr = (char *)tmp1;
		    	PRINT(("iomm_init_pool: alignment_size = %d, unaligned_addr = %p, aligned_addr = %p\n", buflist[free_index].alignment_size, \
		    	tmp->next->unaligned_addr, tmp->next->aligned_addr))
			}

			tmp = tmp->next;
			node_count++;
	    }
	}
	/* return handle to user */
	*pool_handle = free_index;
    PRINT(("\niomm_init_pool: pool handle = %ld:no of nodes in the pool are %d\n", *pool_handle, node_count))

    return (0);
}

/*
 * Actually free memory allocated for node and buffer
 * called by buf_pool_free_list, should not be exposed
 * to user
 */
int iomm_free_pool(struct buf_pool *pool)
{
    unsigned long i, free_count = 0;
	struct buf_node *bn, *tmp;
	int num_use;

	if (pool == NULL) {
		PRINT(("In %s: pool addr passed is NULL\n", __FUNCTION__))
		sprintf(htx_d.msg_text, "iomm_free_pool: Error pool is NULL\n");
		hxfmsg(&htx_d, 0, HTX_HE_HARD_ERROR, htx_d.msg_text);
		return (-1);
	}

	/* free nodes in inuse list */
	num_use = pool->num_elements - pool->num_free;

/*	sprintf(htx_d.msg_text, "iomm_free_pool: Freeing malloc'ed buffer num_use=%d\n", num_use);*/
/*	hxfmsg(&htx_d, 0, HTX_HE_INFO, htx_d.msg_text);*/

	tmp = bn = pool->inuse;
	if (tmp != NULL){
		for(i = 0; i < num_use; i++) {
            tmp = bn->next;
            if(bn->unaligned_addr) {
              PRINT(("iomm_free_pool: Freeing malloc'ed buffer in inuse list, starting addr = %p\n", bn->unaligned_addr))

/*				sprintf(htx_d.msg_text, "iomm_free_pool: Freeing malloc'ed buffer in inuse list, starting addr = %p\n", bn->unaligned_addr);*/
/*				hxfmsg(&htx_d, 0, HTX_HE_INFO, htx_d.msg_text);*/

               free(bn->unaligned_addr);
            }
            PRINT(("iomm_free_pool : Freeing the node[%ld] in inuse list\n", i))
            free(bn);
            free_count++;
            if(tmp == NULL){
                break;
            }
            bn = tmp;
        }
	}
	PRINT(("iomm_free_pool: 1 Total inuse nodes freed = %ld\n", free_count))
/*	sprintf(htx_d.msg_text, "iomm_free_pool: 1 Total inuse nodes freed = %ld\n", free_count);*/
/*	hxfmsg(&htx_d, 0, HTX_HE_INFO, htx_d.msg_text);*/

	/* free nodes in free list */
	free_count = 0;
	tmp = bn = pool->free;
	if (tmp != NULL){
		for(i = 0; i < pool->num_free; i++) {
            tmp = bn->next;
            if(bn->unaligned_addr) {
               PRINT(("iomm_free_pool: Freeing malloc'ed buffer in free list, starting addr = %p\n", bn->unaligned_addr))
               free(bn->unaligned_addr);
            }
            PRINT(("iomm_free_pool: Freeing the node[%ld] in free list\n", i))
            free(bn);
            free_count++;
            if(tmp == NULL){
                break;
            }
            bn = tmp;
        }
	}
	PRINT(("iomm_free_pool: Total free nodes freed = %ld\n", free_count))

/*	sprintf(htx_d.msg_text, "iomm_free_pool: 2 Total free nodes freed = %ld\n", free_count);*/
/*	hxfmsg(&htx_d, 0, HTX_HE_INFO, htx_d.msg_text);*/

	return (0);
}

/*
 * free entire list of buffer pool having different buffer sizes
 * if index = -1, else just free one pool given the pool index
 * from a list of pools, this index is returned to user after
 * the pool is initialized
 */
int iomm_free_pool_list(long index)
{
    int i, rc;

/*	sprintf(htx_d.msg_text, "iomm_free_pool_list: Start  index=%ld\n", index);*/
/*	hxfmsg(&htx_d, 0, HTX_HE_INFO, htx_d.msg_text); */

	if (index == -1) {
		/* clean entire pool */

		/* check inuse list and free buffers */
    	for(i = 0; i < NUM_BUF_SIZES; i++) {
            /* If the page_size is of the type lpage, then call iomm__cleanup_lpage_pool() */
            if (buflist[i].shm_pool == 1) {
/*				sprintf(htx_d.msg_text, "iomm_free_pool_list: Calling iomm_cleanup_page_pool for handle = %d, page_size = %ld\n", i, buflist[i].page_size);*/
/*				hxfmsg(&htx_d, 0, HTX_HE_INFO, htx_d.msg_text);*/
                PRINT(("Calling iomm_cleanup_page_pool for handle = %d, page_size = %ld\n", i, buflist[i].page_size))
                iomm_cleanup_page_pool(i, buflist[i].page_size);
            }
            else { /* For 4K buf pools */
			    if (buflist[i].size) {
			        rc = iomm_free_pool(&buflist[i]);
        	        if (rc) {
						sprintf(htx_d.msg_text, "iomm_free_pool_list: iomm_free_pool clean entire pool Error:  index=%ld, rc=%d i=%d\n", index, rc,i);
						hxfmsg(&htx_d, rc, HTX_HE_HARD_ERROR, htx_d.msg_text);
            	        return (rc);
        	        }
				    memset(&buflist[i], 0, sizeof(struct buf_pool));
				    buflist[i].size = 0;

/*					sprintf(htx_d.msg_text, "iomm_free_pool_list: clean entire pool : Done index=%ld, i=%d \n", index, i);*/
/*					hxfmsg(&htx_d, 0, HTX_HE_INFO, htx_d.msg_text);*/
                }
            }
		}
	}
	else {

		/* clean the specified pool */
		rc = iomm_free_pool(&buflist[index]);
		if (rc) {
			sprintf(htx_d.msg_text, "iomm_free_pool_list : iomm_free_pool() specified pool returned Error index=%ld, rc=%d\n", index, rc);
			hxfmsg(&htx_d, rc, HTX_HE_HARD_ERROR, htx_d.msg_text);
			return (rc);
		}
		memset(&buflist[index], 0, sizeof(struct buf_pool));
		buflist[index].size = 0;

/*		sprintf(htx_d.msg_text, "iomm_free_pool_list: clean specified pool Done index=%ld\n", index);*/
/*		hxfmsg(&htx_d, 0, HTX_HE_INFO, htx_d.msg_text);*/
	}

    return (0);
}

int iomm_resize_pool(long index, long size, int num_nodes)
{
    int i, free_count = 0;
	long buf_size;
    struct buf_node *temp, *bn;

    /* First start with error checking.
     * a) Check if "size" exists in buflist[index]
     */

    buf_size = iomm_check_buf_size(index);
    if (buf_size != size) {
		sprintf(htx_d.msg_text, "%ld size does not belong to buflist[%ld]", size, index);
		hxfmsg(&htx_d, -EINVAL, HTX_HE_HARD_ERROR, htx_d.msg_text);
		return (-EINVAL);
    }

    PRINT(("In buf_pool_resize, num_free = %d, num_nodes = %d\n", buflist[index].num_free, num_nodes))
    /* b) Now, resize the free list according to the value specifies through num_nodes
     * if num_nodes == 0
     * do nothing
     *    else if num_nodes < 0 i.e reduce pool
     *           if ( abs(num_nodes) - buflist[index].num_free ) < 0
     *                return EINVAL
     *           else if ( abs(num_nodes) - buflist[index].num_free ) == 0
     *                   free all the nodes in free list
     *                else
     *                   free "num_nodes" number of nodes from free list
     *    else expand pool
     *        Allocate "num_nodes" number of nodes and assign them to free list
     */

    if (num_nodes == 0) {
		PRINT(("No resizing done\n"))
		return (0);
    }
 	if (num_nodes < 0) {
 		if ((num_nodes + buflist[index].num_free) < 0) {
			sprintf(htx_d.msg_text, "There are only %d nodes in the free list", buflist[i].num_free);
			hxfmsg(&htx_d, EINVAL, HTX_HE_HARD_ERROR, htx_d.msg_text);
	    	return (-EINVAL);
		}
		else {
			if ( ( num_nodes + buflist[index].num_free ) == 0 ) {
				/* free pool */
				iomm_free_pool_list(index);
        	}
			else {
	    		PRINT(("Freeing nodes in buf_pool_resize, in else part\n"))
	    		temp = bn = buflist[index].free;
	    		for( i = 0; i < num_nodes; i++ ) {
					temp = bn->next;
                	if(bn->unaligned_addr) {
                    	PRINT(("Freeing malloc'ed buffer, starting addr = %p\n", bn->unaligned_addr))
                    	free(bn->unaligned_addr);
                	}
                	if(bn) {
                    	PRINT(("Freeing the node\n"))
                    	free(bn);
                	}
                	bn = temp;
                	free_count++;
            	}
	    		buflist[index].free = temp->next;
	    		buflist[index].num_free -= num_nodes;
	   	 		buflist[index].num_elements -= num_nodes;

			}
    	}
	}
    else { /* num_nodes > 0, i.e add num_nodes into free pool */
        for(i = 0; i < num_nodes; i++) {
	    	temp = (struct buf_node *) malloc(sizeof(struct buf_node));
	    	if ( temp == NULL ) {
				sprintf(htx_d.msg_text, "malloc failed Size: %lx, Line:%d", sizeof(struct buf_node),__LINE__);
				hxfmsg(&htx_d, ENOMEM, HTX_HE_HARD_ERROR, htx_d.msg_text);
				iomm_free_pool_list(index);
				return ( ENOMEM );
	    	}
        	bzero(temp, sizeof(struct buf_node));
        	temp->unaligned_addr = (char *)malloc(buflist[index].size);
        	if(NULL == temp->unaligned_addr) {
				sprintf(htx_d.msg_text, "malloc failed Size: %ld, Line:%d", buflist[index].size,__LINE__);
				hxfmsg(&htx_d, ENOMEM, HTX_HE_HARD_ERROR, htx_d.msg_text);
				iomm_free_pool_list(index);
           		return(ENOMEM);
        	}
        	bzero(temp->unaligned_addr, buflist[index].size);

	    	temp->next = buflist[index].free;
	   	 	buflist[index].free = temp;
	    	buflist[index].num_free++;
	    	buflist[index].num_elements++;
		}
    }
  	return (0);
}

/*
 * returns a list of num_nodes to user from pool, use the get_buf version if the nodes
 * are allocated using iomm_init_page_pool(), which is a pool of shared memory
 * updates pointer to node as in arg3, in case of success
 */
int iomm_get_buf_page_node(long index, long page_size, long size, int num_nodes, struct buf_node **node)
{
    int i, rc = 0;
	long buf_size;
    struct buf_node *head, *tail;

    /* First check if "size" exists in buflist[index] */

    buf_size = iomm_check_buf_size(index);
    PRINT(("In %s: buf_size = %ld, index = %ld\n", __FUNCTION__, buf_size, index))
    if (buf_size != size) {
		sprintf(htx_d.msg_text, "reqested buf size = %ld, buflist[%ld] is a pool of %ld size buffer", size, index, buf_size);
		hxfmsg(&htx_d, -EINVAL, HTX_HE_HARD_ERROR, htx_d.msg_text);
		return (-EINVAL);
    }

    /* Check if there are atleast "num_nodes" number of nodes.
     * If there are less free nodes than what was requested, call iomm_resize_pool() to extend the free list.
     * If the resize was successful, return the head pointer of the list
     * else, return err ENOMEM, update buf_node * to NULL
     */

    PRINT(("In %s: buflist[%ld].num_free = %d\n", __FUNCTION__, index, buflist[index].num_free))
    if (num_nodes > buflist[index].num_free) {
        if (page_size == PG_SZ_4K) {
		    sprintf(htx_d.msg_text, "Free nodes: %d size %ld, call iomm_resize_pool()", buflist[index].num_free, size);
		    hxfmsg(&htx_d, 0, HTX_HE_INFO, htx_d.msg_text);
		    rc = iomm_resize_pool(index, size, num_nodes - buflist[index].num_free);
		    if (rc != 0) {
	    	    return(rc);
		    }
        }
        else {
            sprintf(htx_d.msg_text, "Number of nodes requested is more than that available in 16M pool\n. Invoke iomm_cleanup_page_pool and then iomm_init_16M_pool with the desired number of 16M pages\n");
            hxfmsg(&htx_d, 0, HTX_HE_HARD_ERROR, htx_d.msg_text);
            return (-ENOMEM);
        }
    }

	PRINT(("In %s: nodes req = %d, buflist[%ld].num_free = %d\n", __FUNCTION__, num_nodes, index, buflist[index].num_free))

    /* Now there are enough nodes in free list, return the head pointer of the free list. Assign this to
     * inuse list. Leave the rest of the nodes intact in the free list.
     * Make sure that head points to the first node and tail points to the last node of the list that
     * is to be returned.
     */

    tail = head = buflist[index].free;
    for(i = 0; i < num_nodes - 1 && tail->next; i++) {
		tail = tail->next;
    }

    buflist[index].free = tail->next;
    buflist[index].num_free -= num_nodes;

    /* Now insert the list to inuse list */
    tail->next = buflist[index].inuse;
    buflist[index].inuse = head;

    *node =  buflist[index].inuse;
    rc = 0;
    PRINT(("In %s, rc = %d\n", __FUNCTION__, rc));

    return (rc);
}

/*
 * returns a list of num_nodes to user from pool, use this version of get_buf if the nodes are
 * allocated by iomm_init_pool(), which is pool of maloced memory
 * updates pointer to node as in arg3, in case of success
 */
int iomm_get_buf_node(long index, long size, int num_nodes, struct buf_node **node)
{
    int i, rc = 0;
	long buf_size;
    struct buf_node *head, *tail;

    /* First check if "size" exists in buflist[index] */

    buf_size = iomm_check_buf_size(index);
    if (buf_size != size) {
		sprintf(htx_d.msg_text, "reqested buf size = %ld, buflist[%ld] is a pool of %ld size buffer", size, index, buf_size);
		hxfmsg(&htx_d, -EINVAL, HTX_HE_HARD_ERROR, htx_d.msg_text);
		return (-EINVAL);
    }

    /* Check if there are atleast "num_nodes" number of nodes.
     * If there are less free nodes than what was requested, call iomm_resize_pool() to extend the free list.
     * If the resize was successful, return the head pointer of the list
     * else, return err ENOMEM, update buf_node * to NULL
     */

    if ( num_nodes > buflist[index].num_free ) {
		sprintf(htx_d.msg_text, "Free nodes: %d size %ld, call iomm_resize_pool()", buflist[index].num_free, size);
		hxfmsg(&htx_d, 0, HTX_HE_INFO, htx_d.msg_text);
		rc = iomm_resize_pool(index, size, num_nodes - buflist[index].num_free);
		if ( rc != 0 ) {
	    	return( rc );
		}
    }
	PRINT(("In %s: num_nodes = %d, buflist[%ld].num_free = %d\n", __FUNCTION__, num_nodes, index, buflist[index].num_free))

    /* Now there are enough nodes in free list, return the head pointer of the free list. Assign this to
     * inuse list. Leave the rest of the nodes intact in the free list.
     * Make sure that head points to the first node and tail points to the last node of the list that
     * is to be returned.
     */

    tail = head = buflist[index].free;
    for( i = 0; i < num_nodes - 1; i++ ) {
		tail = tail->next;
    }

    buflist[index].free = tail->next;
    buflist[index].num_free -= num_nodes;



    /* Now insert the list to inuse list */
    tail->next = buflist[index].inuse;
    buflist[index].inuse = head;

    *node =  buflist[index].inuse;
    rc = 0;
    PRINT(("In buf_get, rc = %d\n", rc));

    return (rc);
}

int iomm_put_buf_node(long index, long size, struct buf_node *buf)
{
    if (iomm_check_buf_size(index) != size) {
        sprintf(htx_d.msg_text, "buf size = %ld does not belong to pool buflist[%ld]", size, index);
        hxfmsg(&htx_d, -EINVAL, HTX_HE_HARD_ERROR, htx_d.msg_text);
        return (-EINVAL);
    }
	buf->next = buflist[index].free;
	buflist[index].free = buf;

    buflist[index].num_free++;

	return(0);
}

long iomm_check_buf_size(long index)
{
	PRINT(("In %s: buf_size = 0x%lx, page_size = 0x%lx\n", __FUNCTION__, buflist[index].size, buflist[index].page_size))
	return (buflist[index].size);
}

void *iomm_get_shmat_addr(long index)
{
	PRINT(("In %s: buf_size = 0x%lx, page_size = 0x%lx\n", __FUNCTION__, buflist[index].size, buflist[index].page_size))
	return (buflist[index].shm_start_addr);
}
