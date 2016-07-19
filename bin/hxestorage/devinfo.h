/* IBM_PROLOG_BEGIN_TAG */
/*
 * Copyright 2003,2016 IBM International Business Machines Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *               http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* IBM_PROLOG_END_TAG */

#ifndef _H_DEVINFO
#define _H_DEVINFO

#include "hxihtx64.h"

#define CACHING_MODE_PAGE       8
#define MODE_SENSE_REPLY        8
#define DISABLE_DESC_BLOCK      8

#define WCE_MASK                0x4
#define WCE_SHIFT               0x2

#undef  uchar
typedef unsigned char uchar;

/*
 * Device information
 */
struct devinfo {
    char devtype;
    char flags;
    char devsubtype;
    union {
		struct {                           /* for SCSI or IDE disks */
		    unsigned int blksize;          /* block size (in bytes) */
		    unsigned  long long numblks;   /* total number of blocks */
		    long max_request;              /* max request to DD */
		    uint segment_size;             /* segment size for transfer stats */
		    uint segment_count;            /* segment count for transfer stats */
		    uint byte_count;               /* byte count for transfer stats */
		} scdk, idedk;
    } un;
};

/* device types */
/*
 * These two are used for SCSI and IDE devices, and also optical devices
 * and are the only ones referred to in the HTX-AIX code.
 */
#define DD_SCDISK  'r'          /* SCSI disk */
#define DD_SCRWOPT 'w'          /* SCSI R/W optical */

/*
 * Other block devices, excluding IDE devices.
 */
#define DD_TMSCSI 'T'           /* SCSI target mode */
#define DD_DISK 'R'             /* disk */
#define DD_CDROM 'C'            /* cdrom */
#define DD_SES    'a'           /* SCSI Enclosure Services Device */

/* device sub-types */
#define DS_LV   'l'             /* logical volume */
#define DS_PV   'p'             /* physical volume - hard disk */
#define DS_SCSI 'S'             /* SCSI adapter */
#define DS_IDE  'I'             /* IDE adapter  */
#define DS_TM   'T'             /* SCSI target mode */


/* flags */
#define DF_FIXED 01             /* non-removable */
#define DF_RAND  02             /* random access possible */
#define DF_FAST  04
#define DF_CONC  0x08           /* Concurrent mode supported */

#define IOCINFO            0x1
#define CHECK_WRITE_CACHE  0x2
#define DISCARD            0x3

extern  int htx_ioctl(struct htx_data *data, int d, int request, void *ptr);

#endif                          /* _H_DEVINFO */
