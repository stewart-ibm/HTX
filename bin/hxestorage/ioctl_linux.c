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
/* @(#)73	1.1  src/htx/usr/lpp/htx/bin/hxestorage/ioctl_linux.c, exer_storage, htxubuntu 11/24/14 00:47:55 */

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "devinfo.h"

#ifdef  __HTX_LINUX__

#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <linux/fs.h>
/* #include <sys/mount.h> */
#include <sys/param.h>
#include <errno.h>

int     htx_ioctl(int d, int request, struct devinfo *info)
{
        unsigned long long disk_size = 0;
        unsigned int  sect_sz = 0;
        unsigned long long  no_of_blks = 0;
        int     retval = 0;


        if(info == NULL)  {
                fprintf(stderr, "Unable to access memory, or parameter NULL !\n");
                return  EINVAL;
        }

        /*
         * Only one request, none others are implemented
         */
        switch(request)  {
                case IOCINFO:
                        retval = ioctl(d, BLKSSZGET, &sect_sz);
            if(retval < 0)  {
                 return -1;
            }
                        if(sect_sz == 0) {
                                errno = EINVAL;
                                return(-1);
                        }
                        retval = ioctl(d, BLKGETSIZE64, &disk_size);
                        if(retval < 0)  {
                                return -1;
                        }
                        no_of_blks = (disk_size / sect_sz);
                        if(no_of_blks & 0x01)
                                no_of_blks-- ;

                        printf("sect_sz = %#x, no_of_blks = %#llx \n", sect_sz, no_of_blks);
                        info->devtype = DD_SCDISK;
                        info->un.scdk.blksize = (unsigned int) sect_sz;
                        info->un.scdk.numblks = (unsigned long long ) no_of_blks;
                        info->un.scdk.max_request = 0;
                        info->un.scdk.segment_size = 0;
                        info->un.scdk.segment_count = 0;
                        info->un.scdk.byte_count = 0;
                        break;

                default:
                        return -1;
                        break;
        }

        return 0;
}


#endif  /* __HTX_LINUX__ */
