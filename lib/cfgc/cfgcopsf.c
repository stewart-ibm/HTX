/* IBM_PROLOG_BEGIN_TAG */
/* 
 * Copyright 2003,2016 IBM International Business Machines Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 		 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* IBM_PROLOG_END_TAG */

/* @(#)90	1.5  src/htx/usr/lpp/htx/lib/cfgc/cfgcopsf.c, htx_libcfgc, htxubuntu 5/24/04 18:00:28 */

/********************************************************************/
/*                                                                  */
/* Module Name:  cfgcopsf                                           */
/* Component Name:  CFG - System Configuration                      */
/* LPP Name:  BOS                                                   */
/*                                                                  */
/* Descriptive Name:  Attribute File Open Routine                   */
/*                                                                  */
/* Function:  This routine will open an attribute file for update.  */
/*      It allocates a structure which contains the file pointer    */
/*      and space for a pointer to a default buffer which will be   */
/*      allocated by cfgcread if it is required.                    */
/*                                                                  */
/* Compiler Options:                                                */
/*      -DBUG turns on debug print statements                       */
/*                                                                  */
/* Input:                                                           */
/*      fname  - pointer to name of the attribute file              */
/*                                                                  */
/* Output:                                                          */
/*                                                                  */
/* Normal Return:                                                   */
/*      Pointer to the stanza file table                            */
/*                                                                  */
/* Error Return:                                                    */
/*      NULL pointer                                                */
/*                                                                  */
/* External References:                                             */
/*                                                                  */
/*   Other Routines:                                                */
/*      fopen(3)                                                    */
/*      fclose(3)                                                   */
/*      malloc(3)                                                   */
/*      free(3)                                                     */
/*                                                                  */
/*   Data Areas:                                                    */
/*                                                                  */
/* Change Activity:                                                 */
/*                                                                  */
/********************************************************************/

/*********************************************************************
* Begin (Open Attribute File)
*
*   Set return value = NULL
*   Open the file
*   If open successful
*     Allocate memory for pointer table
*     If allocated
*       Store file pointer
*       Set return value = pointer to pointer table
*     Else
*       Close the file
*     Endif
*   Endif
*
* End (Open Attribute File)
*********************************************************************/

#include <cfgclib.h>
#include <cfgcom.h>
#include <cfgpest.h>

CFG__SFT	*cfgcopsf(char *fname)
{
    	FILE	 *fptr = NULL;
    	CFG__SFT *sfptr = NULL;
    	CFG__SFT *retval = NULL;

    	buglpr(BUGNTF,("Open file: %s\n",fname));
    	fptr = fopen(fname,"r+");
    	if (fptr != NULL) {
		sfptr = (CFG__SFT *)malloc(sizeof(CFG__SFT));
		if (sfptr != NULL) {
	    		sfptr->sfile = fptr;
	    		htx_strcpy(sfptr->spath,fname);
	    		sfptr->defbuf = NULL;
	    		retval = sfptr;
	    		buglpr(BUGNFO,("File %s opened\n",sfptr->spath));
		}

		else {
	    		fclose(fptr);
	    		buglpr(BUGNFO,("malloc failed\n"));
		}
    	}

    	buglx(BUGNFO,retval != NULL,bugpr(("File %s could not be opened\n",fname)));
    	buglrt(BUGNTF,"exit",*retval,X);

    	if (retval == NULL)  {		/* if open failed set name variable */
		htx_strcpy(msgc2,fname);
	}

    	return(retval);
}


