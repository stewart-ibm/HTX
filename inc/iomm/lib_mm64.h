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

#ifndef _LIB_MM_H
#define _LIB_MM_H

#define  B     1
#define  K     1024*B
#define  M     1024*K
#define PG_SZ_16M (16*M)
#define PG_SZ_64K (64*K)
#define PG_SZ_4K  (4 *K)

#include <stdio.h>
#include <ctype.h>
#include <sys/errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <stddef.h>

#ifdef DEBUG
   #define  PRINT(x)   {printf x;}
#else
   #define  PRINT(x)
#endif

/*
 * memory pool management
 */

#define NUM_BUF_SIZES  1024

struct buf_node {
	struct           buf_node *next;
    char * 	         unaligned_addr;    /* Unaligned Address                                      */
	char *	         aligned_addr;      /* Aligned   Address                                      */	
	char *	         bus_addr;          /* Bus       Address                                      */
	char * 	         real_addr;         /* Real      Address                                      */

	int              shm_id;            /* Shared memory id, while creating segment for 16M pages */

	unsigned long    size;              /* Size of the node buffer                                */
    unsigned long    page_size;         /* Base page size of the pool                             */
};

struct buf_pool {
    unsigned long     size;             /* Size of the node buffer in bytes       	      		  */
    unsigned long     page_size;        /* Base page size of the pool                             */
    struct buf_node   *free;            /* Pointer to the head of the free buf_node linked list   */
    struct buf_node   *inuse;           /* Pointer to the head of the inuse buf_node linked list  */
    int               num_free;	        /* Number of buffers of size "size" that are free         */
    int               num_elements;     /* Total number of allocated buffers for size "size"     
				                         * num_elements = num_free + number of buffers in use     */
    int	              alignment_size;   /* Alignment size                                         */
    char *            shm_start_addr;   /* Address where the segment is attached                  */
	int               shm_id;           /* Shared memory id, while creating segment for 16M pages */
    unsigned 	      PCI_addr:1,       /* Bit field to indicate if PCI addr has to be generated  */
 				      real_addr:1,      /* Bit field to indicate if real addr has to be generated */
				      align:1;          /* Bit field to indicate if the addr has to be aligned    */
    unsigned          shm_pool:1;       /* Bit field to indicate if the memory was allocated 
                                         * using shmget or malloc                                 */
};

/*
 * HTX descriptor pool management routines
 * creates, manages and destroy descriptor pool memory
 * for Tx and Rx operations
 */
int iomm_init_pool(long *pool_handle, long buf_size, long align_size, int num_elements);

int iomm_free_pool_list(long index);

int iomm_resize_pool(long pool_handle, long size, int num_nodes);

int iomm_get_buf_node(long pool_handle, long size, int num_nodes, struct buf_node **node);

int iomm_get_buf_page_node(long pool_handle, long page_size, long size, int num_nodes, struct buf_node **node);

int iomm_put_buf_node(long pool_handle, long size, struct buf_node *buf);

long iomm_check_buf_size(long pool_handle);

int iomm_init_page_pool(long *pool_handle, void *shmat_addr, long page_size, long buf_size, int num_elements, int *num_pages_allocated);

int iomm_cleanup_page_pool(int index, long psize);

void *iomm_get_shmat_addr(long index);

#endif
