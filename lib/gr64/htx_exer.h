/* @(#)74	1.3  src/htx/usr/lpp/htx/lib/gr64/htx_exer.h, htx_libgr, htxubuntu 6/7/04 14:35:54 */

/*****************************************************************************

Header HTX Monterey Token Ring Exerciser
Header file for the program invoked by HTX to test the
Monterey Token Ring Adapter.

*****************************************************************************/

#define OK              0	/* 'good' and 'bad' return codes */

#define RULE_FILE_LEN 128	/* strlen of rules file name */

#define HTX_HE_LEN     14	/* strlen of HE_name in struct htx_data  */
#define HTX_SDEV_LEN   14	/* strlen of sdev_id in struct htx_data  */
#define HTX_RUN_LEN     3	/* strlen of run_type in struct htx_data */
#define HTX_MSG_LEN   219	/* strlen of msg_text in struct htx_data */

extern void strlencpy(char *,char *,int);
