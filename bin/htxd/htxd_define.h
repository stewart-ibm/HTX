/* @(#)39	1.4  src/htx/usr/lpp/htx/bin/htxd/htxd_define.h, htxd, htxubuntu 9/15/15 20:28:07 */



#ifndef HTXD__LIMITS__HEADER
#define HTXD__LIMITS__HEADER



#define MAX_DEVICE_INDEX_LENGTH 200
#define	HTX_ERR_MESSAGE_LENGTH 512

#define HTX_PATH "/usr/lpp/htx"
#define DEFAULT_ECG_NAME "/usr/lpp/htx/mdt/mdt.bu"
#define RUN_SETUP_SCRIPT "/usr/lpp/htx/etc/scripts/exer_setup"
#define RUN_SETUP_ERROR_OUTPUT "/tmp/res_setup"
#define HE_SCRIPT_FILE "hxsscripts"
#define HTX_STATS_FILE "/tmp/htxstats"
#define HTX_STATS_SEND_FILE "/tmp/htxstats.cmd_ln"
#define HTX_ERR_LOG_FILE "/tmp/htxerr"
#define HTX_CMD_RESULT_FILE "/tmp/command_result"
#define HTX_VPD_SCRIPT "/usr/lpp/htx/etc/scripts/gen_vpd"
#define HTX_VPD_FILE "/tmp/htx_vpd" 
#define MDT_DIR "/usr/lpp/htx/mdt"
#define MDT_LIST_FILE "/tmp/mdt_list"
#define DR_MDT_NAME "/usr/lpp/htx/mdt/mdt.dr"
#define HTXD_AUTOSTART_FILE "/usr/lpp/htx/.htxd_autostart"
#define HTXD_BOOTME_SCRIPT "/usr/lpp/htx/etc/scripts/htxd_bootme"

#define EXTRA_DEVICE_ENTRIES 0

#define HTXD_PATH_MAX 1024

#define KYE_OFF_SET_LIMIT	1000
#define EXER_TABLE_LENGTH_LIMIT	1600

#define	EXER_RUNNING_CHECK_COUNT 200
#define WAIT_TIME_TO_STOP_EXER	4

#define HTX_COE 1
#define HTX_SOE 0

#define	COE_SOE_STATE		1
#define	ACTIVE_SUSPEND_STATE	2
#define	RUNNING_STATE		3

#define STRING_EXTRA_SPACE 5

#define SIGALRM_EXP_TIME 900

#define MSG_TOO_LONG 0x0001
#define BAD_GETTIMER 0x0002
#define BAD_MSGSND 0x0004
#define NO_MSG_QUEUE 0x0008

#define EXTRA_BUFFER_LENGTH 1024
#define	LINE_ENTRY_LENGTH 200

#ifndef GOOD
# define GOOD 0
#endif

#ifdef __HTX_LINUX__
# define SIGMAX SIGRTMAX
#endif

#define DUP_DEVICE_NOT_FOUND	0
#define DUP_DEVICE_FOUND	1
#define DUP_DEVICE_TERMINATED	2

#define HTXD_MDT_SHUTDOWN_MSG	190

#endif
