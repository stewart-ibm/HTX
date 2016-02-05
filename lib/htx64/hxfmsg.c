/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* htxfedora src/htx/usr/lpp/htx/lib/htx64/hxfmsg.c 1.3.4.2               */
/*                                                                        */
/* Licensed Materials - Property of IBM                                   */
/*                                                                        */
/* Restricted Materials of IBM                                            */
/*                                                                        */
/* COPYRIGHT International Business Machines Corp. 1997,2014              */
/* All Rights Reserved                                                    */
/*                                                                        */
/* US Government Users Restricted Rights - Use, duplication or            */
/* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.      */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

/* %Z%%M%	%I%  %W% %G% %U% */

#include <htx_local.h>
#include <hxihtx64.h>
#include <stdlib.h>


/*
 * NAME: hxfmsg()
 *
 * FUNCTION: Sends a message to the HTX system via the hxfupdate() ERROR call.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function is called by any Hardware Exerciser (HE) program and
 *      is included as part of the libhtx.a library.
 *
 * NOTES:
 *
 *      operation:
 *      ---------
 *
 *      set htx_data error number and severity fields
 *
 *      copy message string to htx_data text field
 *
 *      call hxfupdate() ERROR to send message to HTX system
 *
 *      return(hxfupdate return code)
 *
 *
 * RETURNS:
 *
 *      Return Codes:
 *      ----------------------------------------------------------------------
 *                   0 -- Normal exit; buffers compare OK.
 *       exit_code > 0 -- Problem in hxfupdate() ERROR call.
 *
 *
 */

int hxfmsg(struct htx_data *p, int err, enum sev_code  sev, char *text)
     /*
      * p -- pointer to the Hardware Exerciser's htx_data data structure
      * err -- error code
      * sev -- severity code
      * text -- pointer to message text
      */
{
  /*
   ***  Beginning of Executable Code  *****************************************
   */
	if(p == NULL){ /* to handle cases when the htx_data pointer is not updated in hxfupdate*/
		printf("%s",text);
		return 0;
	}
	else{
		p->error_code = err;
		p->severity_code = sev;

		(void) strncpy(p->msg_text, text, MAX_TEXT_MSG);
		if (p->msg_text[MAX_TEXT_MSG - 1] != '\0')
		p->msg_text[MAX_TEXT_MSG -1] = '\0';  /* string MUST end with null char  */

		return(hxfupdate(ERROR, p));
	}

} /* hxfmsg() */
