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
/* @(#)09	1.1  src/htx/usr/lpp/htx/bin/hxestorage/devinfo.h, exer_storage, htxubuntu 11/26/14 03:45:57 */

#ifndef _H_DEVINFO
#define _H_DEVINFO

#include <sys/types.h>
#ifdef  __HTX_LINUX__
#undef  uchar
typedef unsigned char uchar;
#endif

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
 * and are the only ones refered to in the HTX-AIX code.
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


#ifdef  __HTX_LINUX__
extern  int     htx_ioctl(int d, int request, struct devinfo *info);
#define IOCINFO 0x01
#define BLKSSZGET  _IO(0x12,104)/* get block device sector size */
#endif

#endif                          /* _H_DEVINFO */
