
/* @(#)18	1.2  src/htx/usr/lpp/htx/bin/hxsmsg/start_display.c, htx_msg, htxubuntu 5/24/04 13:39:43 */

#include <unistd.h>
#include "hxsmsg.h"

int start_display_device(void)
{
	pid_t pid;
	if((pid = fork()) == 0)
	{
		execl("/usr/bin/htx/bin/nf",(char *)0);
		exit(1);
	}
	return(GOOD);
}
