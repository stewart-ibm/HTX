
/* @(#)45	1.2  src/htx/usr/lpp/htx/bin/hxssup/get_dispid.c, htx_sup, htxubuntu 5/24/04 13:42:49 */

/*
 *   FUNCTIONS: cleanup_odm
 *		do_odm_free_list
 *		get_CuAt
 *		get_CuDv
 *		get_PdAt
 *		get_dispid
 *		get_odm_error_msg
 *		initialize_cfgodm
 *
 *   ORIGINS: 27
 */


#include "hxssup.h"

/*
 * ODM not implemented for Linux
 */

#ifndef	__HTX_LINUX__
/*
 * get_dispid() error code definitions
 */
#define BAD_ODM_INIT	(-1)
#define BAD_CUAT_GET_ATTR	(-2)
#define BAD_GET_CUDV	(-3)
#define BAD_PDAT_GET_ATTR	(-4)
#define NO_ATTR		(-5)
#define BAD_ATTR_VALUE	(-6)


/*
 * Global variables (must be global since cleanup_odm() can be called
 *                   from anywhere an unrecoverable error is detected)
 */
struct CuAt *cat;
struct CuDv *cudv;
struct PdAt *pat;

struct listinfo cat_info;
struct listinfo cu_info;
struct listinfo pat_info;

#endif		/* __HTX_LINUX__ */


/*
 * NAME: get_dispid()
 *                                                                    
 * FUNCTION: Gets the display id of the passed display ODM entry name.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function is called by the AH_system() function (found in the
 *      AH_system.c module) of the HTX supervisor program, "hxssup".
 *                                                                   
 * NOTES: 
 *
 *      operation:
 *      ---------
 *
 * 
 *               
 * RETURNS: 
 *
 *      Return Codes:
 *      ------------------------------------------------------------------
 *       0 -- Normal exit.
 *
 */

int get_dispid(char *entry_name)
{

#ifdef	__HTX_LINUX__
	return	1;
#else

	char error_msg[1024];
	char uniquetype[1024];
	char *value;

	int display_id;
	int errno_save;

	/*
	 * ODM is not implemented for Linux ... so pass a success return value
	 */

	cat = NULL;		/* clear customized attribute object pointer */
	cudv = NULL;		/* clear customized device object pointer */
	pat = NULL;		/* clear predefined attribute object pointer */

	if (initialize_cfgodm() != 0)  {
		return (BAD_ODM_INIT);
	}

	if (get_CuAt(entry_name, "display_id") < 0) {
		cleanup_odm();
		return (BAD_CUAT_GET_ATTR);
	}

	if (cat != NULL)  {	/* Find it in Customized Attributes objects? */
		value = cat->value;
	}

	else {			/* Go get it in Predefined Attributes objects */
		if (get_CuDv(entry_name, uniquetype) < 0) {
			 cleanup_odm();
			return (BAD_GET_CUDV);
		}

		if (get_PdAt(uniquetype, "display_id") < 0) {
			 cleanup_odm();
			return (BAD_PDAT_GET_ATTR);
		}

		if (pat != NULL)  {
			value = pat->deflt;
		}

		else {
			 sprintf(error_msg,"Error: Unable to find \"display_id\"attribute for %s.\nuniquetype = %s.", entry_name, uniquetype);
			 send_message(error_msg, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
			 return (NO_ATTR);
		}
	}

	errno = 0;
	display_id = (int) strtol(value, NULL, 16);
	if ((display_id == 0) && (errno != 0)) {
		errno_save = errno;
		sprintf(error_msg, "Error on strtol() conversion for display_id attribute \nvalue (%s) for %s.\nerrno: %d (%s).\n", cat->value, entry_name, errno_save, strerror(errno_save));
		send_message(error_msg, errno_save, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
		display_id = BAD_ATTR_VALUE;
	}

	cleanup_odm();

	return (display_id);

#endif		/* __HTX_LINUX__ */
}				/* get_dispid */



/*
 *   NAME:     cleanup_odm
 *   FUNCTION: frees resources allocated by odm subroutines and
 *             terminates current odm session.
 *
 *   RETURNS:	0 = good return
 *           	0x00000001 = bad do_odm_free_list() return
 *           	0x00000002 = bad odm_terminate() return
 *
 *              Note: bad return codes can be OR'ed together.
 *
 */

/*
 * ODM is not implemented for LINUX
 */

#ifndef	__HTX_LINUX__

#define BAD_DO_ODM_FREE_LIST 0x00000001
#define BAD_ODM_TERMINATE 0x00000002

int	cleanup_odm(void)
{
	char *odm_err_msg_ptr;
	char workstr[1024];
	int rc = 0;

	if (cudv != NULL)  {
		if (do_odm_free_list((void *) cudv, &cu_info) != 0)  {
			rc |= BAD_DO_ODM_FREE_LIST;
		}
	}

	if (pat != NULL)  {
		if (do_odm_free_list(pat, &pat_info) != 0)  {
			rc |= BAD_DO_ODM_FREE_LIST;
		}
	}

	if (cat != NULL)  {
		if (do_odm_free_list(cat, &cat_info) != 0)  {
			rc |= BAD_DO_ODM_FREE_LIST;
		}
	}

	odmerrno = 0;
	if (odm_terminate() != 0) {
		get_odm_error_msg(odmerrno, &odm_err_msg_ptr);
		sprintf(workstr, "odm_terminate() failed!  odmerrno = %d\n(%s)", odmerrno, odm_err_msg_ptr);
		send_message(workstr, odmerrno, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
		rc |= BAD_ODM_TERMINATE;
	}

	return (rc);
}				/* cleanup_odm() */



/*
 *   NAME:     do_odm_free_list
 *   FUNCTION: frees resources allocated by odm subroutines
 *
 *   RETURNS:	0 = good return
 *             -1 = bad odm_free_list() return
 *
 */

#define BAD_ODM_FREE_LIST	(-1)

int	do_odm_free_list(void *resource_ptr, struct listinfo *listinfo_ptr)
{
	char *odm_err_msg_ptr;
	char workstr[1024];
	int rc = 0;

	if (resource_ptr != NULL) {
		odmerrno = 0;
		if (odm_free_list(resource_ptr, listinfo_ptr) != 0) {
			 get_odm_error_msg(odmerrno, &odm_err_msg_ptr);
			 sprintf(workstr, "odm_free_list() failed!  odmerrno = %d\n(%s)", odmerrno, odm_err_msg_ptr);
			 send_message(workstr, odmerrno, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
			 rc = BAD_ODM_FREE_LIST;
		}
	}

	return (rc);
}				/* do_odm_free_list() */



/*
 *   NAME:     get_CuAt
 *   FUNCTION: get the attribute specified by the attribute string for the
 *             named object.
 *
 *   RETURNS:   0 = good return
 *              -1 = bad return from odm_get_list()
 *              -2 = non-unique return from odm_get_list()
 *
 */

#define BAD_ODM_GET_LIST	(-1)
#define NON_UNIQUE_RETURN	(-2)

int	get_CuAt(char *entry_name, char *attribute)
{
	char criteria[1024];
	char *odm_err_msg_ptr;
	char workstr[512];
	int rc = 0;

	NLsprintf(criteria, "name = '%s' AND attribute = '%s'", entry_name, attribute);
	odmerrno = 0;
	cat = (struct CuAt *) odm_get_list(CuAt_CLASS, criteria, &cat_info, 1, 1);

	if ((int) cat == -1) {
		get_odm_error_msg(odmerrno, &odm_err_msg_ptr);
		sprintf(workstr, "odm_get_list() for CuAt failed!  odmerrno = %d\n(%s)\n Search criteria = %s", odmerrno, odm_err_msg_ptr, criteria);
		send_message(workstr, odmerrno, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
		rc = BAD_ODM_GET_LIST;
	}
	
	else if (cat_info.num > 1) {
		sprintf(workstr, "ERROR: %d items returned from odm_get_list for CuAt!\n" "Search criteria (%s) did not specify a unique item!", cat_info.num, criteria);
		send_message(workstr, 0, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
		rc = NON_UNIQUE_RETURN;
	}

	return (rc);
}				/* get_CuAt() */



/*
 *   NAME:     get_CuDv
 *   FUNCTION: get set of customized devices and linked predefined for the
 *             named object.
 *
 *   RETURNS:   0 = good return
 *              -1 = bad return from odm_get_list()
 *              -2 = non-unique return from odm_get_list()
 *
 */
#define BAD_ODM_GET_LIST	(-1)
#define NON_UNIQUE_RETURN	(-2)
#define NO_ODM_GET_LIST_RETURN	(-3)

int	get_CuDv(char *entry_name, char *uniquetype)
{
	char criteria[1024];
	char *odm_err_msg_ptr;
	char workstr[512];
	int rc = 0;

	NLsprintf(criteria, "name = '%s' AND status = %d", entry_name, AVAILABLE);
	odmerrno = 0;
	cudv = (struct CuDv *) odm_get_list(CuDv_CLASS, criteria, &cu_info, 64, 2);
	if ((int) cudv == -1) {
		get_odm_error_msg(odmerrno, &odm_err_msg_ptr);
		sprintf(workstr, "odm_get_list() failed!  odmerrno = %d\n(%s)\nSearch criteria = %s", odmerrno, odm_err_msg_ptr, criteria);
		send_message(workstr, odmerrno, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
		rc = BAD_ODM_GET_LIST;
	}
	
	else if (cu_info.num > 1) {
		sprintf(workstr, "ERROR: %d items returned from odm_get_list!\nSearch criteria (%s) did not specify a unique item!", cu_info.num, criteria);
		send_message(workstr, 0, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
		rc = NON_UNIQUE_RETURN;
	}
	
	else if (cudv == (struct CuDv *) 0) {
		sprintf(workstr, "Error: Unable to find CuDv entry from odm_get_list().\nSearch criteria = %s.", criteria);
		send_message(workstr, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
		rc = NO_ODM_GET_LIST_RETURN;
	}
	
	else  {
		 NLstrcpy(uniquetype, cudv->PdDvLn_Lvalue);
	}

	return (rc);
}				/* get_CuDv() */

/*
 *   NAME:     get_odm_error_msg
 *   FUNCTION: get the odm error message associated with the passed
 *             error number.
 *
 *   RETURNS:	 0 = good return
 *           	-1 = bad odm_err_msg() return
 *
 */

#define BAD_ODM_ERR_MSG	(-1)

int	get_odm_error_msg(long odm_error_number, char **message_ptr)
{
	int rc = 0;
	static char *bad_odm_err_msg_ptr = "Unable to retrieve ODM error message";

	if (odm_err_msg(odm_error_number, message_ptr) < 0) {
		*message_ptr = bad_odm_err_msg_ptr;
		rc = BAD_ODM_ERR_MSG;
	}

	return (rc);
}				/* get_odm_error_msg() */



/*
 *   NAME:     get_PdAt
 *   FUNCTION: get the attribute specified by the attribute string for the
 *             named object.
 *
 *   RETURNS:   0 = good return
 *              -1 = bad return from odm_get_list()
 *              -2 = non-unique return from odm_get_list()
 *
 */
#define BAD_ODM_GET_LIST	(-1)
#define NON_UNIQUE_RETURN	(-2)

int	get_PdAt(char *uniquetype, char *attribute)
{
	char criteria[1024];
	char *odm_err_msg_ptr;
	char workstr[512];

	int rc = 0;
	NLsprintf(criteria, "uniquetype = '%s' AND attribute = '%s'", uniquetype, attribute);
	odmerrno = 0;
	pat = (struct PdAt *) odm_get_list(PdAt_CLASS, criteria, &pat_info, 1, 1);

	if ((int) pat == -1) {
		get_odm_error_msg(odmerrno, &odm_err_msg_ptr);
		sprintf(workstr, "odm_get_list() for PdAt failed!  odmerrno = %d\n(%s)\n Search criteria = %s", odmerrno, odm_err_msg_ptr, criteria);
		send_message(workstr, odmerrno, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
		rc = BAD_ODM_GET_LIST;
	}
	
	else if (pat_info.num > 1) {
		 sprintf(workstr, "ERROR: %d items returned from odm_get_list for PdAt!\nSearch criteria (%s) did not specify a unique item!", pat_info.num, criteria);
		 send_message(workstr, 0, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
		 rc = NON_UNIQUE_RETURN;
	}

	return (rc);
}				/* get_PdAt() */



/*
 *   NAME:     initialize_cfgodm
 *   FUNCTION: Gets the config lock.
 *             Runs odm_initialize, but if the ODMDIR env variable
 *             is not set it uses /etc/objrepos instead of current dir
 *   RETURNS:  what was returned by odm_initialize or odm_set_path
 */
int	initialize_cfgodm(void)
{
	char *odm_err_msg_ptr;
	char workstr[1024];
	int rc;

	odmerrno = 0;
	if ((rc = odm_initialize()) == -1) {
		get_odm_error_msg(odmerrno, &odm_err_msg_ptr);
		sprintf(workstr, "odm_initialize() failed!  odmerrno = %d\n(%s)", odmerrno, odm_err_msg_ptr);
		send_message(workstr, odmerrno, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
	}
	
	else {
		if (!getenv("ODMDIR")) {
			odmerrno = 0;
			if ((rc = (int) odm_set_path("/etc/objrepos")) ==  -1) {
				get_odm_error_msg(odmerrno, &odm_err_msg_ptr);
				sprintf(workstr, "odm_set_path() failed!  odmerrno = %d\n(%s)", odmerrno, odm_err_msg_ptr);
				send_message(workstr, odmerrno, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
			}
		}
	}

	return (rc);
}				/* initialize_cfgodm() */

#endif		/* __HTX_LINUX__ */

