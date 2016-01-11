
/* @(#)54	1.2  src/htx/usr/lpp/htx/bin/hxssup/nfdef.h, htx_sup, htxubuntu 5/24/04 13:45:19 */

#define DISPKEY 10111
#define HE_STATUS 20
struct disp_msg
{
	long mtype;
	char msg[1000];
};

void * start_display(void);
