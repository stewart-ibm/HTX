/* @(#)54	1.3  src/htx/usr/lpp/htx/bin/stxclient/nfdef.h, eserv_gui, htxubuntu 7/10/03 12:41:15 */
#define DISPKEY 10111
#define HE_STATUS 20
struct disp_msg
{
	long mtype;
	char msg[1000];
};

void * start_display(void);
