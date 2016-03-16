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
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <asm/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/pdiagex_dds.h>
#include <diag/diag_err.h>
#include <errno.h>

#define MAXSIZE 100

#define EEH_SOFTWARE_ERR  -2
#define EEH_HARDWARE_ERR  -1

static int fd = -1;     /* File Discriptor for accessing the /dev/pdiagchar file */
char DEVICE_NAME[10];   /* Device name */

/*
 * Function :
 *     pdiag_diagnose_state
 * Arguments :
 *     ptr - contains the device name to be diagnosed.
 * Return Value :
 *     Success (0) or Error Value (resulted).
 * Description :
 *     HTX needs exlusive access to any particular device that it's accessing or
 * testing. This function removes the entry of the device from the kernel data
 * structure and holds it. Thus removing the device from the kernel. Thus giving
 * exclusive access to the HTX exercisers.
 */
int pdiag_diagnose_state(char *ptr)
{
    int rc;
    char *temp, *temp1, str[MAXSIZE], resource_name[20], bus_num[3], dev_num[3], fn_num[3];

    char  irq_str[6];
    FILE *fp;

    struct diagnose_struct diag_struct;


    if(fd == -1) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_diagnose_state\n");
            return -1;
        }
    }

    fp = fopen("/tmp/devlist.txt", "r");
    if (fp == NULL) {
        printf("\n sorry device info file not available \n");
        return -1;
    }


    resource_name[0] = 0;

    /* Reading the necessary ionformations for the specified devices
       ( /dev/<device_n> ) from the file device filr ( /tmp/devlist.txt ) */
    while (fgets(str, MAXSIZE, fp)) {
        if ((char *) strstr(str, ptr)) {
            temp = (char *) strstr(str, "-");
            temp+=2;
            temp1 = (char *) strstr(temp,"-");
            temp1-=1;
            strncpy(resource_name, temp, temp1-temp);
            resource_name[temp1-temp] = 0;
            break;
        }
    }

    /* Seperating the bus:dev:fn:irq read from the /dev/devlist.txt and
        saving in the data structure ( struct diagnose_struct ) */
    if (strlen(resource_name)) {
        diag_struct.resource_name = ptr;    /* Saving the device name */
#ifndef __64_BIT_MODE__
        diag_struct.resource_name64 = NULL; /* Saving the device name */
#endif
        temp = (char *) strstr(resource_name, ":");
        strncpy(bus_num, resource_name, temp - resource_name);
        bus_num[temp - resource_name] = 0;
        diag_struct.bus_num = atoi(bus_num);    /* Saving Bus No. */
        temp++;
        temp1 = (char *) strstr(temp, ":");
        strncpy(dev_num, temp, temp1 - temp);
        dev_num[temp1-temp] = 0;
        diag_struct.dev_num = atoi(dev_num);    /* Saving Device No. */
        temp = temp1+1;
        temp1 = (char *) strstr(temp, ":");
        strncpy(fn_num, temp, temp1 - temp);
        fn_num[temp1-temp] = 0;
        diag_struct.fn_num = atoi(fn_num);      /* Saving Function No. */
        temp = temp1 + 1;
        strcpy(irq_str, temp);
        diag_struct.irq = atoi(irq_str);        /* Saving the Interrupt No. */
        rc = ioctl(fd, PDIAGEX_DD_DIAGNOSE, &diag_struct);
        if (!rc) strcpy(DEVICE_NAME,ptr); else DEVICE_NAME[0] = 0;
    }
    else {
        printf("\nNo device found \n");
        rc = DGX_DEVICE_NOT_AVAILABLE_FOR_DIAGNOSIS;
    }

    fclose(fp);

    return rc;
}

/*
 * Function :
 *     pdiag_restore_state
 * Arguments :
 *     ptr - contains the device name to be diagnosed.
 * Return Value :
 *     Success (0) or Error Value (resulted).
 * Description :
 *     Once the HTX test is over, it needs to return back the device access to
 * kernel again. This function removes the entry of the device from the pdiagex
 * kernel extension and gives back to the kernel again. Thus giving the device
 * back to the kernel again.
 */
int pdiag_restore_state(char *ptr)
{
    int rc;
    int fd_flag=-1;

    if (strcmp(ptr, DEVICE_NAME)) {
        rc = DGX_DEVICE_NOT_AVAILABLE_FOR_DIAGNOSIS;
        return rc;
    }

    if(fd == -1) fd_flag = 1;
    if(!fd_flag) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_dd_restore_state\n");
            return -1;
        }
    }
    rc = ioctl(fd, PDIAGEX_DD_RESTORE, ptr);

    close(fd);
    fd = -1;

    return rc;
}

/*
 * Function :
 *     pdiag_dd_dma_setup_lpages
 * Arguments :
 *     h_ptr - The handle pointer for accessing the device.
 *     flags - The operation to be performed.
 *     baddr - The base address of the buffer for which the PCI address has tobe obtained.
 *     p_ptr - Pointer to store the PCI address of the buffer.
 *     count - Size of the buffer.
 *     operation - This specifies the type of operation.
 * Return Value :
 *     Success (0) or Error Value (resulted).
 * Description :
 *     This sets up the DMA environment for multiple page length buffers.
 */
int pdiag_dd_dma_setup_lpages ( pdiag_struct_t *h_ptr, int flags, unsigned long *baddr,
                unsigned long *p_ptr, unsigned long long count, int minxfer, int operation)
{
    int rc;
    int fd_flag=-1;
    char device_name[10];
    struct pdiag_dma_struct dma_struct;

#ifdef __HTX_LINUX__
    strcpy(device_name,DEVICE_NAME);
    dma_struct.device = device_name;
#endif
    dma_struct.handle_ptr = h_ptr;
    dma_struct.flags = flags;
    dma_struct.virt_addr = baddr;
    dma_struct.phy_addr = p_ptr;
    dma_struct.count = count;
    dma_struct.minxfer = minxfer;
    dma_struct.op = operation;
#ifndef __64_BIT_MODE__
    dma_struct.device64 = NULL;
    dma_struct.handle_ptr64 = NULL;
    dma_struct.virt_addr64 = NULL;
    dma_struct.phy_addr64 = NULL;
#endif

    if(fd == -1) fd_flag = 1;
    if(!fd_flag) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_dd_dma_setup\n");
            return -1;
        }
    }


    rc = ioctl(fd, PDIAGEX_DD_DMA_SETUP, &dma_struct);
    if(!fd_flag) {
        close(fd);
        fd = -1;
    }

    return rc;
}

/*
 * Function :
 *     pdiag_dd_dma_setup
 * Arguments :
 *     h_ptr - The handle pointer for accessing the device.
 *     flags - The operation to be performed.
 *     baddr - The base address of the buffer for which the PCI address has tobe obtained.
 *     p_ptr - Pointer to store the PCI address of the buffer.
 *     count - Size of the buffer.
 *     operation - This specifies the type of operation.
 * Return Value :
 *     Success (0) or Error Value (resulted).
 * Description :
 *     This sets up the DMA environment for the specific adapter. This function
 * gets in the buffer virtiual address and the size of the Buffer which is used to
 * store DMA data and returns its hardware ( PCI ) address back to be programmed
 * for initiating the DMA operation.
 */
int pdiag_dd_dma_setup( pdiag_struct_t *h_ptr, int flags, unsigned long *baddr,
        unsigned long *p_ptr, unsigned int count, int minxfer, int operation)
{
    return pdiag_dd_dma_setup_lpages( h_ptr, flags, baddr, p_ptr, (unsigned long long)count, minxfer, operation);
}

/*
 * Function :
 *     pdiag_dd_big_dma_setup
 * Arguments :
 *     h_ptr - The handle pointer for accessing the device.
 *     flags - The operation to be performed.
 *     baddr - The base address of the buffer for which the PCI address has tobe obtained.
 *     p_ptr - Pointer to store the PCI address of the buffer.
 *     count - Size of the buffer.
 *     operation - This specifies the type of operation.
 * Return Value :
 *     Success (0) or Error Value (resulted).
 * Description :
 *     This sets up the DMA environment for 16 K Fixed page length buffers.
 */
int pdiag_dd_big_dma_setup ( pdiag_struct_t *h_ptr, int flags, unsigned long *baddr,
                unsigned long *p_ptr, unsigned int count)
{
    return pdiag_dd_dma_setup_lpages( h_ptr, flags, baddr, p_ptr, (unsigned long long)count, 0, PDIAG_DMA_MASTER);
}

/*
 * Function :
 *     pdiag_dd_dma_complete_lpages
 * Arguments :
 *     h_ptr - The handle pointer for accessing the device.
 *     p_addr - Pointer to the stored PCI address.
 *     count - Size of the buffer.
 *     operation - This specifies the type of operation.
 * Return Value :
 *     Success (0) or Error Value (resulted).
 * Description :
 *     This releases the DMA environment for multiple page length buffers.
 */
int pdiag_dd_dma_complete_lpages (pdiag_struct_t *h_ptr, unsigned long *p_addr,
                                            unsigned long long count, int operation)
{
    int rc;
    int fd_flag=-1;
    char device_name[10];
    struct pdiag_dma_complete_struct dma_struct;

#ifdef __HTX_LINUX__
    strcpy(device_name,DEVICE_NAME);
    dma_struct.device = device_name;
#endif
    dma_struct.handle_ptr = h_ptr;
    dma_struct.phy_addr = p_addr;
    dma_struct.count = count;
    dma_struct.op = operation;
#ifndef __64_BIT_MODE__
    dma_struct.device64 = NULL;
    dma_struct.handle_ptr64 = NULL;
    dma_struct.phy_addr64 = NULL;
#endif

    if(fd == -1) fd_flag = 1;
    if(!fd_flag) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_dd_dma_complete\n");
            return -1;
        }
    }
    rc = ioctl(fd, PDIAGEX_DD_DMA_COMPLETE, &dma_struct);
    if(!fd_flag) {
        close(fd);
        fd = -1;
    }

    return rc;
}

/*
 * Function :
 *     pdiag_dd_dma_complete
 * Arguments :
 *     h_ptr - The handle pointer for accessing the device.
 *     p_addr - Pointer to the stored PCI address.
 *     operation - This specifies the type of operation.
 * Return Value :
 *     Success (0) or Error Value (resulted).
 * Description :
 *     This releases the DMA environment set up by the function pdiag_dd_dma_setup.
 */
int pdiag_dd_dma_complete (pdiag_struct_t *h_ptr, unsigned long *p_addr, int operation)
{
    return pdiag_dd_dma_complete_lpages( h_ptr, p_addr, (unsigned long long)4 KByte, operation);
}

/*
 * Function :
 *     pdiag_dd_big_dma_complete
 * Arguments :
 *     h_ptr - The handle pointer for accessing the device.
 *     p_addr - Pointer to the stored PCI address.
 *     operation - This specifies the type of operation.
 * Return Value :
 *     Success (0) or Error Value (resulted).
 * Description :
 *     This releases the DMA environment for 16 K Fixed page lenght buffers.
 */
int pdiag_dd_big_dma_complete (pdiag_struct_t *h_ptr, unsigned long *p_addr, int operation)
{
    return pdiag_dd_dma_complete_lpages( h_ptr, p_addr, (unsigned long long)16 KByte, operation);
}

/*
 * Function :
 *     pdiag_dd_watch_for_interrupt
 * Arguments :
 *     handle_ptr - The handle pointer for accessing the device.
 *     mask - Command for interrupt usage.
 *     timeoutsecs - Timeout value in secs..
 * Return Value :
 *     Success (0) or Error Value (resulted).
 * Description :
 *     This releases the DMA environment set up by the function pdiag_dd_dma_setup.
 */
int pdiag_dd_watch_for_interrupt (pdiag_struct_t *handle_ptr, unsigned int mask, unsigned int timeoutsecs)
{
    int rc;
    int fd_flag=-1;
    char device_name[10];
    struct pdiag_intr_struct intr_struct;

#ifdef __HTX_LINUX__
    strcpy(device_name,DEVICE_NAME);
    intr_struct.device = device_name;
#endif
    intr_struct.handle_ptr = handle_ptr;
    intr_struct.mask = mask;
    intr_struct.timeoutsecs = timeoutsecs;
#ifndef __64_BIT_MODE__
    intr_struct.device64 = NULL;
    intr_struct.handle_ptr64 = NULL;
#endif

    if(fd == -1) fd_flag = 1;
    if(!fd_flag) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_dd_watch_for_interrupt\n");
            return -1;
        }
    }

    rc = ioctl(fd, PDIAGEX_DD_WATCH_FOR_INTR, &intr_struct);
    if(!fd_flag) {
        close(fd);
        fd = -1;
    }

    return rc;
}

/*
 * Function :
 *     pdiag_dd_read
 * Arguments :
 *     handle_ptr - contains the handle to the device to read from.
 *     type - contains the type of operation.
 *     offset - offset from the base address to read from.
 *     user_buf - buffer in which the read value is returned back.
 *     flags - Flags for reading the memory region.
 * Return Value :
 *     Success (0) or Error Value (resulted).
 * Description :
 *     This function is used to read value from the Adapter's registers.
 */
int pdiag_dd_read(pdiag_struct_t *handle_ptr, int type, int offset, void *user_buf, pdiagex_opflags_t *flags)
{
    int rc;
    int fd_flag=-1;
    char device_name[10];
    struct pdiag_rw_struct mem_read_struct;

#ifdef __HTX_LINUX__
    strcpy(device_name,DEVICE_NAME);
    mem_read_struct.device = device_name;
#endif
    mem_read_struct.handle_ptr = handle_ptr;
    mem_read_struct.length = type;
    mem_read_struct.offset = offset;
    mem_read_struct.user_buf = user_buf;
    mem_read_struct.flags = flags;
#ifndef __64_BIT_MODE__
    mem_read_struct.device64 = NULL;
    mem_read_struct.handle_ptr64 = NULL;
    mem_read_struct.user_buf64 = NULL;
    mem_read_struct.flags64 = NULL;
#endif

    if(fd == -1) fd_flag = 1;
    if(!fd_flag) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_dd_read\n");
            return -1;
        }
    }


    rc = ioctl(fd, PDIAGEX_DD_READ, &mem_read_struct);
    if(!fd_flag) {
        close(fd);
        fd = -1;
    }

    return rc;
}

/*
 * Function :
 *     pdiag_dd_write
 * Arguments :
 *     handle_ptr - contains the handle to the device to read from.
 *     type - contains the type of operation.
 *     offset - offset from the base address the value is written to.
 *     user_buf - Value to be written to the specified address.
 *     flags - Flags for reading the memory region.
 * Return Value :
 *     Success (0) or Error Value (resulted).
 * Description :
 *     This function is used to write value to the Adapter's registers.
 */
int pdiag_dd_write(pdiag_struct_t *handle_ptr, unsigned int type, unsigned int offset, void *user_buf, pdiagex_opflags_t *flags)
{
    int rc;
    int fd_flag=-1;
    char device_name[10];
    struct pdiag_rw_struct mem_write_struct;

#ifdef __HTX_LINUX__
    strcpy(device_name,DEVICE_NAME);
    mem_write_struct.device = device_name;
#endif
    mem_write_struct.handle_ptr = handle_ptr;
    mem_write_struct.length = type;
    mem_write_struct.offset = offset;
    mem_write_struct.user_buf = user_buf;
    mem_write_struct.flags = flags;
#ifndef __64_BIT_MODE__
    mem_write_struct.device64 = NULL;
    mem_write_struct.handle_ptr64 = NULL;
    mem_write_struct.user_buf64 = NULL;
    mem_write_struct.flags64 = NULL;
#endif

    if(fd == -1) fd_flag = 1;
    if(!fd_flag) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_dd_write\n");
            return -1;
        }
    }

    rc = ioctl(fd, PDIAGEX_DD_WRITE, &mem_write_struct);
    if(!fd_flag) {
        close(fd);
        fd = -1;
    }
    return rc;
}

/*
 * Function :
 *     pdiag_config_write_old
 * Arguments :
 *     device - device to whose config. space the value is to be written.
 *     offset - offset from the base address the value is written to.
 *     value - Value to be written to the specified address.
 * Return Value :
 *     Success (0) or Error Value (resulted).
 * Description :
 *     This function is used to write value to the Adapter's configuration space.
 * of user desired length
 */
int pdiag_config_write_old(char *device, unsigned long offset, void *value, int length)
{
    int rc;
    int fd_flag=-1;
    struct pdiag_rw_struct config_write_struct;
    pdiagex_opflags_t flags = { PDIAG_CFG_OP, 1, PDIAG_SING_LOC_ACC, PROCLEV, NULL };

    config_write_struct.device = device;
    config_write_struct.offset = offset;
    config_write_struct.user_buf = value;
    config_write_struct.length = length;
    config_write_struct.flags = &flags;
#ifndef __64_BIT_MODE__
    config_write_struct.device64 = NULL;
    config_write_struct.handle_ptr = NULL;
    config_write_struct.handle_ptr64 = NULL;
    config_write_struct.user_buf64 = NULL;
    config_write_struct.flags64 = NULL;
#endif

    if(fd == -1) fd_flag = 1;
    if(!fd_flag) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_config_write\n");
            return -1;
        }
    }
    rc = ioctl(fd, PDIAGEX_CONFIG_WRITE, &config_write_struct);
    if(!fd_flag) {
        close(fd);
        fd = -1;
    }

    return rc;
}

/*
 * Function :
 *     pdiag_config_write
 * Arguments :
 *     device - device to whose config. space the value is to be written.
 *     offset - offset from the base address the value is written to.
 *     value - Value to be written to the specified address.
 * Return Value :
 *     Success (0) or Error Value (resulted).
 * Description :
 *     This function is used to write value to the Adapter's configuration space.
 */
int pdiag_config_write(char *device, unsigned long offset, unsigned long value)
{
    return pdiag_config_write_old(device,offset,(void *)&value,2);
}

/*
 * Function :
 *     pdiag_config_read_old
 * Arguments :
 *     device - device to whose config. space the value is to be written.
 *     offset - offset from the base address the value is written to.
 *     value - buffer to be store the read value of the config's space.
 * Return Value :
 *     Success (0) or Error Value (resulted).
 * Description :
 *     This function is used to read value from the Adapter's configuration space
 * of user desired length
 */
int pdiag_config_read_old(char *device, unsigned long offset, void *value, int length)
{
    int rc;
    int fd_flag=-1;
    struct pdiag_rw_struct config_read_struct;
    pdiagex_opflags_t flags = { PDIAG_CFG_OP, 1, PDIAG_SING_LOC_ACC, PROCLEV, NULL };

    config_read_struct.device = device;
    config_read_struct.offset = offset;
    config_read_struct.user_buf = (void *)value;
    config_read_struct.length = length;
    config_read_struct.flags = &flags;
#ifndef __64_BIT_MODE__
    config_read_struct.device64 = NULL;
    config_read_struct.handle_ptr = NULL;
    config_read_struct.handle_ptr64 = NULL;
    config_read_struct.user_buf64 = NULL;
    config_read_struct.flags64 = NULL;
#endif

    if(fd == -1) fd_flag = 1;
    if(!fd_flag) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_config_read\n");
            return -1;
        }
    }
    rc = ioctl(fd, PDIAGEX_CONFIG_READ, &config_read_struct);
    if(!fd_flag) {
        close(fd);
        fd = -1;
    }

    return rc;
}

/*
 * Function :
 *     pdiag_config_read
 * Arguments :
 *     device - device to whose config. space the value is to be written.
 *     offset - offset from the base address the value is written to.
 *     value - buffer to be store the read value of the config's space.
 * Return Value :
 *     Success (0) or Error Value (resulted).
 * Description :
 *     This function is used to read value from the Adapter's configuration space.
 */
int pdiag_config_read(char *device, unsigned long offset, unsigned char *value)
{
    return pdiag_config_read_old(device,offset,(void *)value,2);
}


int pdiag_set_eeh_option(char *device,unsigned int function)
{
    int rc;
    int fd_flag=-1;
    struct Set_EEH seteeh;

    seteeh.device = device;
    seteeh.function = function;
#ifndef __64_BIT_MODE__
    seteeh.device64 = NULL;
#endif

    if(fd == -1) fd_flag = 1;
    if(!fd_flag) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_set_eeh_option\n");
            return -1;
        }
    }
    rc = ioctl(fd, PDIAGEX_SET_EEH, &seteeh);
    if(!fd_flag) {
        close(fd);
        fd = -1;
    }

    return rc;
}

int pdiag_inject_error(char *buf, char *err_token, int opn_token)
{
    int rc;
    int fd_flag=-1;
    struct ERR_Injct err_inject;

    err_inject.buf = buf;
    err_inject.err_token = err_token;
    err_inject.opn_token = opn_token;
#ifndef __64_BIT_MODE__
    err_inject.buf64 = NULL;
    err_inject.err_token64 = NULL;
#endif

    if(fd == -1) fd_flag = 1;
    if(!fd_flag) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_set_eeh_option\n");
            return -1;
        }
    }
    rc = ioctl(fd, PDIAGEX_INJECT_ERROR, &err_inject);
    if(!fd_flag) {
        close(fd);
        fd = -1;
    }

    return rc;
}

int pdiag_slot_error_detail(char *device,char * drv_buf,int drv_len,char *rpa_buf,int rpa_len)
{
    int rc;
    int fd_flag=-1;
    struct error_detail errbuf;

    errbuf.device = device;
    errbuf.drv_buf = drv_buf;
    errbuf.rpa_buf = rpa_buf;
    errbuf.drv_len = drv_len;
    errbuf.rpa_len = rpa_len;
#ifndef __64_BIT_MODE__
    errbuf.device64 = NULL;
    errbuf.drv_buf64 = NULL;
    errbuf.rpa_buf64 = NULL;
#endif

    if(fd == -1) fd_flag = 1;
    if(!fd_flag) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_set_eeh_option\n");
            return -1;
        }
    }
    rc = ioctl(fd, PDIAGEX_SLOT_ERR_DETAIL, &errbuf);
    if(!fd_flag) {
        close(fd);
        fd = -1;
    }

    return rc;
}

int pdiag_read_slot_reset(char *device)
{
    int rc;
    int fd_flag=-1;

#ifdef NO_RTAS
    return 0;
#endif

    if(fd == -1) fd_flag = 1;
    if(!fd_flag) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_read_slot_reset \n");
            return -1;
        }
    }
    rc = ioctl(fd, PDIAGEX_READ_SLOT_RESET, device);
    if(!fd_flag) {
        close(fd);
        fd = -1;
    }

    return rc;
}

int pdiag_configure_bridge(char *device)
{
    int rc;
    int fd_flag=-1;

    if(fd == -1) fd_flag = 1;
    if(!fd_flag) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_configure_bridge \n");
            return -1;
        }
    }
    rc = ioctl(fd, PDIAGEX_CONFIGURE_BRIDGE, device);
    if(!fd_flag) {
        close(fd);
        fd = -1;
    }

    return rc;
}


int pdiag_errinjct_init(void)
{
    int rc;
    int dummy;
    int fd_flag=-1;

    if(fd == -1) fd_flag = 1;
    if(!fd_flag) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_errinjct_init \n");
            return -1;
        }
    }
    rc = ioctl(fd, PDIAGEX_INIT_INJECT_ERROR, &dummy);
    if(!fd_flag) {
        close(fd);
        fd = -1;
    }

    return rc;
}

int pdiag_open_error_inject(char *device)
{
    int rc;
    int fd_flag=-1;

    if(fd == -1) fd_flag = 1;
    if(!fd_flag) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_open_error_inject \n");
            return -1;
        }
    }
    rc = ioctl(fd, PDIAGEX_OPEN_ERROR_INJECT, device);
    if(!fd_flag) {
        close(fd);
        fd = -1;
    }

    return rc;
}

int ndiag_close_error_inject(unsigned long token)
{
    int rc;
    int fd_flag=-1;

    if(fd == -1) fd_flag = 1;
    if(!fd_flag) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_open_error_inject \n");
            return -1;
        }
    }
    rc = ioctl(fd, PDIAGEX_CLOSE_ERROR_INJECT, token);
    if(!fd_flag) {
        close(fd);
        fd = -1;
    }

    return rc;
}

int pdiag_set_slot_reset(char *device)
{
    int rc;
    int fd_flag=-1;

    if(fd == -1) fd_flag = 1;
    if(!fd_flag) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_set_slot_reset \n");
            return -1;
        }
    }
    rc = ioctl(fd, PDIAGEX_SET_SLOT_RESET, device);
    if(!fd_flag) {
        close(fd);
        fd = -1;
    }

    return rc;
}

int pdiag_open_old(char *ptr, pdiagex_dds_t *dds_ptr, char *intr_func, pdiag_struct_t *handle_ptr)
{
    int rc = 0;
    int fd_flag=-1;

    struct pdiag_open_struct {
        char *ptr;
        pdiagex_dds_t *dds_ptr;
        char *intr_func;
        pdiag_struct_t *handle_ptr;
    };

    struct pdiag_open_struct pdiag_open_struct;

    if (strcmp(ptr, DEVICE_NAME)) {
        rc = DGX_DEVICE_NOT_AVAILABLE_FOR_DIAGNOSIS;
        return rc;
    }


    if(fd == -1) fd_flag = 1;
    if(!fd_flag) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_open\n");
            return -1;
        }
    }

    pdiag_open_struct.ptr = ptr;
    pdiag_open_struct.dds_ptr = dds_ptr;
    pdiag_open_struct.intr_func = intr_func;
    pdiag_open_struct.handle_ptr = handle_ptr;

    rc = ioctl(fd, PDIAGEX_OPEN, &pdiag_open_struct);

    return rc;
}

/*
 * Function :
 *     pdiag_open
 * Arguments :
 *     ptr - contains the device name to be diagnosed.
 *     dds_ptr - pointer to fill-in the dds structure.
 *     ptr - contains the intr. name of the device.
 *     ptr - pointer to fill in the handle to the device.
 * Return Value :
 *     Success (0) or Error Value (resulted).
 * Description :
 *     This provides an interface to access the harware directly by the exerciser.
 * The handle is used to read/write data from/to the hardware directly.
 */
int pdiag_open(char *ptr, pdiagex_dds_t *dds_ptr, char *intr_func, pdiag_info_handle_t *handle_ptr)
{
    int rc = 0;
    int fd_flag=-1;

    struct pdiag_open_struct open_struct;
#ifndef __HTX_LINUX__
    char command[100],intrfunc[100],buf[1024];
    FILE *fp;

    if(intr_func != NULL) {
        if ((fp = popen("ls /proc/kallsyms 2> /dev/null", "r")) == NULL)
            printf("popen error \n");
        fgets(buf, 1024, fp);
        if ( (pclose(fp)) == -1 )
            printf("pclose failed\n");
        if(strlen(buf)) {
            if(!strcmp(intr_func,"atminterrupt"))
                strcpy(intrfunc,"atm_interrupt");
            else if(!strcmp(intr_func,"goent_intr"))
                strcpy(intrfunc,"goliad_interrupt");
            else if(!strcmp(intr_func,"scurry_intr"))
                strcpy(intrfunc,"scurry_intr");

            sprintf(command,"cat /proc/kallsyms | awk '/d %s/ { print $1 }'",intrfunc);
            if ((fp = popen(command, "r")) == NULL)
                printf("popen error \n");
            printf("command is %s",command);

            if ( fgets(buf, 1024, fp) == NULL )
                printf("fgets failed\n");
            if ( (pclose(fp)) == -1 )
                printf("pclose failed\n");
            printf("%s\n",buf);
            if(!strlen(buf))
                printf("%s module not loaded\n",intrfunc);
        }
    }
#endif

    if (strcmp(ptr, DEVICE_NAME)) {
        rc = DGX_DEVICE_NOT_AVAILABLE_FOR_DIAGNOSIS;
        return rc;
    }

    *handle_ptr = NULL;
    (*handle_ptr) = (pdiag_struct_t *) malloc(sizeof(pdiag_struct_t));
    if ((*handle_ptr) == NULL) {
        return(-ENOMEM);
    }

    open_struct.ptr = ptr;
    open_struct.dds_ptr = dds_ptr;
#ifndef __HTX_LINUX__
    if(strlen(buf)) {
        open_struct.intr_func = buf;
        open_struct.intr_func64 = NULL;
    }
    else
#endif
        open_struct.intr_func = intr_func;

    open_struct.handle_ptr = (pdiag_struct_t *) (*handle_ptr);
#ifndef __64_BIT_MODE__
    open_struct.ptr64 = NULL;
    open_struct.dds_ptr64 = NULL;
    open_struct.intr_func64 = NULL;
    open_struct.handle_ptr64 = NULL;
#endif

    if(fd == -1) fd_flag = 1;
    if(!fd_flag) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_open\n");
            return -1;
        }
    }

    rc = ioctl(fd, PDIAGEX_OPEN, &open_struct);

    if(!fd_flag) {
        close(fd);
        fd = -1;
    }
    return rc;
}


/*
 * Function :
 *     pdiag_close
 * Arguments :
 *     handle - pointer to fill in the handle to the device.
 * Return Value :
 *     Success (0) or Error Value (resulted).
 * Description :
 *     This removes the interface to access the harware directly by the exerciser.
 * and destroy the handle created by the open call for the device.
 */
int pdiag_close(pdiag_info_handle_t handle)
{
    int rc;
    int fd_flag=-1;
    char device_name[10];
    struct pdiag_close_struct close_struct;

#ifdef __HTX_LINUX__
    strcpy(device_name,DEVICE_NAME);
    close_struct.device = device_name;
#endif
    close_struct.handle_ptr = (pdiag_struct_t *)handle;
#ifndef __64_BIT_MODE__
    close_struct.device64 = NULL;
    close_struct.handle_ptr64 = NULL;
#endif

    if(fd == -1) fd_flag = 1;
    if(!fd_flag) {
        fd = open("/dev/pdiagchar", O_RDWR);
        if (fd < 0) {
            printf("\nerror opening file in user fn pdiag_open\n");
            return -1;
        }
    }

    rc = ioctl(fd, PDIAGEX_CLOSE, &close_struct);
    if(!fd_flag) {
        close(fd);
        fd = -1;
    }

    /*
    free((pdiag_struct_t *)handle);
    */

    return rc;
}

int pdiag_cs_open(void)
{
    return 0;
}

int pdiag_cs_close(void)
{
    return 0;
}

int pdiag_cs_get_attr(char *lname, char *att_name, char **value, char *rep)
{
    return 0;
}

void pdiag_cs_free_attr(char **value)
{
    return;
}

/*
 * Function :
 *     pdiag_diagnose_sec_device
 * Arguments :
 *     ptr - contains the device name to be diagnosed.
 * Return Value :
 *     Success (0) or Error Value (resulted).
 * Description :
 *     This Diagnoses second device if any device.
 */
int pdiag_diagnose_sec_device(char *ptr)
{
    int rc=0;
    char first_device[10];

    strcpy(first_device,DEVICE_NAME);
    rc = pdiag_diagnose_state(ptr);
    strcpy(DEVICE_NAME,first_device);

    return rc;
}

/*
 * Function :
 *     pdiag_restore_sec_device
 * Arguments :
 *     ptr - contains the device name to be diagnosed.
 * Return Value :
 *     Success (0) or Error Value (resulted).
 * Description :
 *     This Restores second device if any device.
 */
int pdiag_restore_sec_device(char *ptr)
{
    int rc=0;
    char first_device[10];

    strcpy(first_device,DEVICE_NAME);
    strcpy(DEVICE_NAME,ptr);
    rc = pdiag_restore_state(ptr);
    strcpy(DEVICE_NAME,first_device);

    return rc;
}


