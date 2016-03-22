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

/* @(#)73	1.2  src/htx/usr/lpp/htx/bin/hxestorage/ioctl_linux.c, exer_storage, htxubuntu 2/15/16 22:55:03 */

#include <stdio.h>
#include <string.h>
#include <scsi/sg.h>
#include <scsi/scsi.h>
#include "devinfo.h"

#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <linux/fs.h>
#include <errno.h>

int htx_ioctl(struct htx_data *data, int d, int request, void *ptr)
{
    int i, retval = 0;
    char msg[256];

    switch(request)  {
        case IOCINFO:
        {
            struct devinfo *info = (struct devinfo *) ptr;
            unsigned long long disk_size = 0, no_of_blks = 0;
            unsigned int  sect_sz = 0;

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
            if(no_of_blks & 0x01) {
                no_of_blks-- ;
		    }
            printf("sect_sz = %#x, no_of_blks = %#llx \n", sect_sz, no_of_blks);
            info->devtype = DD_SCDISK;
            info->un.scdk.blksize = (unsigned int) sect_sz;
            info->un.scdk.numblks = (unsigned long long ) no_of_blks;
            info->un.scdk.max_request = 0;
            info->un.scdk.segment_size = 0;
            info->un.scdk.segment_count = 0;
            info->un.scdk.byte_count = 0;
            break;
	    }

	    case CHECK_WRITE_CACHE:
	    {
            /* IOCTL output - first 4 bytes will be HEADER, following which
               will start mode page data. There will be no Descriptor block
               in the output after the header as in cdb we have set the bit
               to disable descriptor block i.e. cdb[1]

               Below is the format of mode page 8 output data (after skipping first
               4 bytes of header):

               BYTE | 7  |  6   |  5  |   4  |   3  |  2  |  1 |  0  |
               ------------------------------------------------------|
                 4  | PS | SPF  |        PAGE CODE (0x8)             |
               ------------------------------------------------------|
                 5  |                PAGE LENGTH (0x12)              |
               ------------------------------------------------------|
                 6  | IC | ABPF | CAP | DISC | SIZE | WCE | MF | RCD |
               ------------------------------------------------------|

               WCE bit in 3rd byte i.e. byte[2] will tell whether Write
               Cache is enabled for the disk or not.
            */
            int WCE_bit = 0;
            unsigned char cdb[6] = {MODE_SENSE, DISABLE_DESC_BLOCK, CACHING_MODE_PAGE, 0, MODE_SENSE_REPLY, 0};
            unsigned char mode_sense_resp[MODE_SENSE_REPLY];
            struct sg_io_hdr io_hdr;

            if ((ioctl(d, SG_GET_VERSION_NUM, &i) < 0) || (i < 30000)) {
                sprintf(msg, "%s is not a sg device OR older sg driver version\n. Will not run thread for sync_cache", data->sdev_id);
                hxfmsg(data, 0, HTX_HE_INFO, msg);
                return (-1);
		    }
		    memset(&io_hdr, 0, sizeof(struct sg_io_hdr));

		    io_hdr.interface_id = 'S';
		    io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
		    io_hdr.cmdp = cdb;
		    io_hdr.cmd_len = sizeof(cdb);
		    io_hdr.dxfer_len = MODE_SENSE_REPLY;
		    io_hdr.dxferp = mode_sense_resp;
		    /* printf("cdb: %02x %02x %02x %02x %02x %02x\n", cdb[0], cdb[1], cdb[2], cdb[3], cdb[4], cdb[5]); */

		    retval = ioctl(d, SG_IO, &io_hdr);
		    if (retval < 0) {
                sprintf(msg, "MODE_SENSE command to check write cache enablement failed. errno: %d\n"
                             "Will not run thread for sync_cache", errno);
                hxfmsg(data, errno, HTX_HE_INFO, msg);
                return (-1);
		    }
            /* printf("SCSI status: %d, host_status: %d, driver_status: %d\n", io_hdr.status, io_hdr.host_status, io_hdr.driver_status);
            for (i = 0; i < MODE_SENSE_REPLY; i++) {
                printf("%02x ", mode_sense_resp[i]);
		    }
		    */
            WCE_bit = (int) ((mode_sense_resp[6] & WCE_MASK) >> WCE_SHIFT);
            /* printf("WCE_bit: %d\n", WCE_bit); */
            return WCE_bit;

            break;
	    }

            default:
                return -1;
                break;
        }

        return 0;
}

