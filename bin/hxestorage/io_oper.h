/* @(#)07	1.5  src/htx/usr/lpp/htx/bin/hxestorage/io_oper.h, exer_storage, htxubuntu 8/10/15 05:30:22 */

/********************************************************************/
/* File name - io_oper.h                                            */
/* Header file to include all the strcuture/variables/functions     */
/* declaration associated with io_oper.c                            */
/********************************************************************/

#include "hxestorage_utils.h"

#define DUMP_PATH "/tmp/"
#define MAX_MSG_DUMP 20
#define MAX_DUMP_DATA_LEN MAX_TEXT_MSG
#define MAX_MISCOMPARES 11

#ifdef __HTX_LINUX__
typedef loff_t  offset_t;
#endif

extern char fsync_flag;

#ifdef __CAPI_FLASH__


#define BITS_USED               12
#define THREAD_ID_MASK          0xFFF0000000000000ULL
#define INDEX_MASK              0x000FFFFFFFFFFFFFULL

#define GENERATE_TAG(__thread_id__, __index__) 	 ( {               \
    uint64_t mask = 0;                                              \
    mask |= __thread_id__ << (sizeof(uint64_t) - BITS_USED);         \
    mask |= __index__;                                              \
    mask;                                                   \
    } );                                                              

#define GET_INDEX_FROM_TAG(__tag__) (__tag__ & INDEX_MASK) \


#endif


struct cache_thread {
    int th_num;
    int parent_th_num;
    struct htx_data cache_htx_ds;
};

/*************************/
/* Function Declarations */
/*************************/
int do_ioctl (struct htx_data *, int, int, void *);
int set_addr(struct htx_data *, struct thread_context *, int);
int open_disk (struct htx_data *, const char *, struct thread_context *);
int close_disk (struct htx_data *, struct thread_context *);

/* SYNC IO functions */
int read_disk (struct htx_data *, struct thread_context *, int);
int write_disk (struct htx_data *, struct thread_context *, int);
int compare_buffers (struct htx_data *, struct thread_context *, int);

/* ASYNC IO functions */
int read_async_disk(struct htx_data *, struct thread_context *, int);
int write_async_disk(struct htx_data *, struct thread_context *, int);
int cmp_async_disk (struct htx_data *, struct thread_context *, int);

/* PASSTHROUGH functions */
int open_passth_disk (struct htx_data *, const char *, struct thread_context *);
int close_passth_disk (struct htx_data *, struct thread_context *);
int read_passth_disk(struct htx_data *, struct thread_context *, int);
int write_passth_disk(struct htx_data *, struct thread_context *, int);
int verify_passth_disk (struct htx_data *, struct thread_context *, int);

/* CACHE functions */
int read_cache (struct htx_data *, struct thread_context *, int);
int write_cache (struct htx_data *, struct thread_context *, int);
int compare_cache (struct htx_data *, struct thread_context *, int);
int read_cache_disk(struct htx_data *htx_ds, struct thread_context *tctx);
int write_cache_disk(struct htx_data *htx_ds, struct thread_context *tctx);

#ifdef __CAPI_FLASH__
int open_lun(struct htx_data *htx_ds, const char * capi_device, struct thread_context * ); 
int close_lun(struct htx_data *htx_ds, struct thread_context * ); 
int cflsh_write_operation(struct htx_data * htx_ds, struct thread_context *tctx, int loop ); 
int cflsh_read_operation(struct htx_data * htx_ds, struct thread_context *tctx, int loop ); 
int cflsh_awrite_operation(struct htx_data * htx_ds, struct thread_context *tctx, int loop ); 
int cflsh_aread_operation(struct htx_data * htx_ds, struct thread_context *tctx, int loop );
int cflsh_aresult_operation(struct htx_data * htx_ds, struct thread_context *tctx, int loop ); 
#endif 

