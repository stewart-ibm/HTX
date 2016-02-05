
/* @(#)66	1.2  src/htx/usr/lpp/htx/lib/misc64/config_rw.c, libmisc, htxubuntu 5/24/04 14:51:52 */

#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <asm/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/misclib.h>
/*#include<sys/mdio.h>*/

int ioctl_linux(char *str,int command, struct device_struct *dev_struct)
{
int rc;
FILE *fd;
		fd = open("/dev/miscchar", O_RDWR);
		if (fd < 0)
		{
		printf("\nerror opening file in user fn ioctl_linux\n");
		return -1;
		}
		/*printf("calling dev_struct bus=%d\n",dev_struct->bus_num);*/
		if(command == 0)
		rc = ioctl(fd, MIOPCFGET,dev_struct);
		/*printf("rc= %d\n",rc);*/
		close(fd);
		return rc;

}
