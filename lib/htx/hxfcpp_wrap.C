
// @(#)72	1.2  src/htx/usr/lpp/htx/lib/htx/hxfcpp_wrap.C, htx_libhtx, htxubuntu 5/24/04 18:12:02

#include <string.h>
#include <hxfcpp_wrap.H>


/*
 * Fill the htx_data with exerciser details
 */
cHtxLib::cHtxLib (char *pExerName, char *pDevName, char *pRunType)
{
	strcpy (htx_data.HE_name, pExerName);
	strcpy (htx_data.sdev_id, pDevName);
	setRunType (pRunType);
}

/*
 * Indicate start of exerciser to Supervisor
 */
void 
cHtxLib::start ()
{
	hxfupdate (START, &htx_data);
}

/*
 * Indicate exit of exerciser to Supervisor
 */
void 
cHtxLib::finish ()
{
	hxfupdate (FINISH, &htx_data);
}

/*
 * Send messages/errors to Supervisor
 */
void 
cHtxLib::sendMsg (int error_code, int sev_code, char *msg_text)
{
	hxfmsg (&htx_data, error_code, (enum sev_code) sev_code, msg_text);
}

/*
 * Open /dev/hft device
 */

int 
cHtxLib::hxfohft (int oflags)
{
	return (hxfohft (oflags));
}

/*
 * Copy pattern from filename to the buffer
 */
int 
cHtxLib::hxfpat (char *filename, char *pattern_buf, size_t num_chars)
{
	return (hxfpat(filename, pattern_buf, num_chars));
}

void 
cHtxLib::setRunType (char *pRunType)
{
	strcpy (htx_data.run_type, ((*pRunType | 0x20) == 'e')? "EMC":((*pRunType | 0x20) == 'r')? "REG": "OTH");
}

cHtxLib::~cHtxLib()
{
}
