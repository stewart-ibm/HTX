
/* @(#)46     1.2  src/htx/usr/lpp/htx/lib/misc64/xscom.c, libmisc, htxubuntu 8/20/15 01:48:55 */

#define _LARGEFILE64_SOURCE
#include <sys/mman.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <dirent.h>
#include <assert.h>
#include <ctype.h>

#include <xscom.h>
#include <hxihtx64.h>

extern struct htx_data	*misc_htx_data;
#define XSCOM_BASE_PATH "/sys/kernel/debug/powerpc/scom"

struct xscom_chip {
	struct xscom_chip	*next;
	uint32_t			chip_id;
	int					fd;
};
static struct xscom_chip *xscom_chips;

void xscom_for_each_chip(void (*cb)(uint32_t chip_id))
{
	struct xscom_chip *c;

	for (c = xscom_chips; c; c = c->next)
		cb(c->chip_id);
}

static uint32_t xscom_add_chip(const char *base_path, const char *dname)
{
	char nbuf[strlen(base_path) + strlen(dname) + 16];
	struct xscom_chip *chip;
	int fd;
	char err_msg[512];

	snprintf(nbuf, sizeof(nbuf), "%s/%s/access", base_path, dname);
	fd = open(nbuf, O_RDWR);
	if (fd < 0) {
		sprintf(err_msg, "%s: Error opening scom access file: %s, %s\n", __FUNCTION__, nbuf, strerror(errno));
        hxfmsg(misc_htx_data, 0, HTX_HE_INFO, err_msg);
		return(fd);
	}

	chip = malloc(sizeof(*chip));
	assert(chip);
	memset(chip, 0, sizeof(*chip));
	chip->fd = fd;
	chip->chip_id = strtoul(dname, NULL, 16);
	chip->next = xscom_chips;
	xscom_chips = chip;

	return chip->chip_id;
}

static bool xscom_check_dirname(const char *n)
{
	while(*n) {
		char c = toupper(*(n++));

		if ((c < 'A' || c > 'Z') &&
		    (c < '0' || c > '9'))
			return false;
	}
	return true;
}

static uint32_t xscom_scan_chips(const char *base_path)
{
	int i, nfiles;
	struct dirent **filelist;
	uint32_t lower = 0xffffffff;
	char err_msg[512];

	nfiles = scandir(base_path, &filelist, NULL, alphasort);
	if (nfiles < 0) {
		sprintf(err_msg, "%s: Error opening sysfs scom directory: %s, %s\n", __FUNCTION__, base_path, strerror(errno));
        hxfmsg(misc_htx_data, 0, HTX_HE_INFO, err_msg);
		return(errno);
	}
	if (nfiles == 0) {
		sprintf(err_msg, "%s: No SCOM dir found in sysfs: %s, %s\n", __FUNCTION__, base_path, strerror(errno));
        hxfmsg(misc_htx_data, 0, HTX_HE_INFO, err_msg);
		return(errno);
	}

	for (i = 0; i < nfiles; i++) {
		struct dirent *d = filelist[i];
		uint32_t id;

		if (d->d_type != DT_DIR)
			continue;
		if (!xscom_check_dirname(d->d_name))
			continue;
		id = xscom_add_chip(base_path, d->d_name);
		if (id < lower)
			lower = id;
		free(d);
	}

	free(filelist);
	return lower;
}

static struct xscom_chip *xscom_find_chip(uint32_t chip_id)
{
	struct xscom_chip *c;

	for (c = xscom_chips; c; c = c->next)
		if (c->chip_id == chip_id)
			return c;
	return NULL;
}

static uint64_t xscom_mangle_addr(uint64_t addr)
{
	if (addr & (1ull << 63))
		addr |= (1ull << 59);
	return addr << 3;
}

int xscom_read(uint32_t chip_id, uint64_t addr, uint64_t *val)
{
	int rc;
	char err_msg[512];
	struct xscom_chip *c = xscom_find_chip(chip_id);
	off64_t seek_offset = 0;

	if (!c) {
		rc = -ENODEV;
		sprintf(err_msg, "%s: failed to find chipId: %X\n", __FUNCTION__, chip_id);
        hxfmsg(misc_htx_data, 0, HTX_HE_INFO, err_msg);
		return -ENODEV;
	}
	addr = xscom_mangle_addr(addr);
	seek_offset = lseek64(c->fd, addr, SEEK_SET);
	if (seek_offset < 0) {
		sprintf(err_msg, "%s: seek fail for addr: %lX, %s\n", __FUNCTION__, addr, strerror(errno));
        hxfmsg(misc_htx_data, 0, HTX_HE_INFO, err_msg);
		return -errno;
	}
	
	rc = read(c->fd, val, 8);
	if (rc < 0) {
		sprintf(err_msg, "%s: file read fail for seek offset: %lX, %s\n", __FUNCTION__, seek_offset, strerror(errno));
        hxfmsg(misc_htx_data, 0, HTX_HE_INFO, err_msg);
		return -errno;
	}
	if (rc != 8) {
		sprintf(err_msg, "%s: file read fail for seek offset: %lX, %s\n", __FUNCTION__, seek_offset, strerror(errno));
        hxfmsg(misc_htx_data, 0, HTX_HE_INFO, err_msg);
		return -EIO;
	}

	return 0;
}

int xscom_write(uint32_t chip_id, uint64_t addr, uint64_t val)
{
	struct xscom_chip *c = xscom_find_chip(chip_id);
	int rc;
	off64_t seek_offset = 0;
	char err_msg[512];

	if (!c) {
		rc = -ENODEV;
		sprintf(err_msg, "%s: failed to find chipId: %X\n", __FUNCTION__, chip_id);
        hxfmsg(misc_htx_data, 0, HTX_HE_INFO, err_msg);
		return -ENODEV;
	}
	addr = xscom_mangle_addr(addr);
	seek_offset = lseek64(c->fd, addr, SEEK_SET);
	rc = write(c->fd, &val, 8);
	if (rc < 0) {
		sprintf(err_msg, "%s: file write fail for seek offset: %lX, %s\n", __FUNCTION__, seek_offset, strerror(errno));
        hxfmsg(misc_htx_data, 0, HTX_HE_INFO, err_msg);
		return -errno;
	}
	if (rc != 8) {
		sprintf(err_msg, "%s: file read fail for seek offset: %lX, %s\n", __FUNCTION__, seek_offset, strerror(errno));
        hxfmsg(misc_htx_data, 0, HTX_HE_INFO, err_msg);
		return -EIO;
	}
	return 0;
}

int xscom_read_ex(uint32_t ex_target_id, uint64_t addr, uint64_t *val)
{
	uint32_t chip_id = ex_target_id >> 4;;

	addr |= (ex_target_id & 0xf) << 24;

	/* XXX TODO: Special wakeup ? */

	return xscom_read(chip_id, addr, val);
}

int xscom_write_ex(uint32_t ex_target_id, uint64_t addr, uint64_t val)
{
	uint32_t chip_id = ex_target_id >> 4;;

	addr |= (ex_target_id & 0xf) << 24;

	/* XXX TODO: Special wakeup ? */

	return xscom_write(chip_id, addr, val);
}

uint32_t xscom_init(void)
{
	return xscom_scan_chips(XSCOM_BASE_PATH);
}
