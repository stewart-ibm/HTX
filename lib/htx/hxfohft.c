
/* @(#)99	1.9  src/htx/usr/lpp/htx/lib/htx/hxfohft.c, htx_libhtx, htxubuntu 5/24/04 18:08:30 */

/*****************************************************************************
 *****  I B M   I n t e r n a l   U s e   O n l y  ***************************
 *****************************************************************************

   MODULE NAME    =    hxfohft.c
   COMPONENT NAME =    libhtx (HTX function library)
   LPP NAME       =    HTX

   DESCRIPTIVE NAME =  HTX Open HFT device Function.

   STATUS =            Release 1 Version 0

   FUNCTION = If program is running under the HTX Supervisor, check to see
      if there is a valid value for hft_fd[0] in shared memory.  If the val
      is OK (not = 0), return that value; otherwise, try to open the hft
      device.

   COMPILER OPTIONS =  -I/src/master/htx/common -g -Nn3000 -Nd4000 -Np1200
                       -Nt2000

   CHANGE ACTIVITY =
      DATE    :LEVEL:PROGRAMMER:DESCRIPTION
      MMMMMMMMNMMMMMNMMMMMMMMMMNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
      10/06/89:     :J BURGESS :Original release.
              :     :          :

 *****************************************************************************/

#include <htx_local.h>
#include <hxiipc.h>
#include <hxihtx.h>

/****************************************************************************/
/*  Global Variable Declarations  *******************************************/
/****************************************************************************/

/*****************************************************************************/
/*****  h x f o h f t ( )  ***************************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     hxfohft()                                             */
/*                                                                           */
/* DESCRIPTIVE NAME =  Opens the /dev/hft device.                            */
/*                                                                           */
/* FUNCTION =          If the HTX supervisor has already opened the hft      */
/*                     device, the associated file descriptor is returned.   */
/*                     If the hft device has not already been opened, then   */
/*                     this function will open it and returned the file      */
/*                     descriptor.                                           */
/*                                                                           */
/* INPUT =             oflag - the open flag (not used if hft has already    */
/*                            been opened.                                   */
/*                                                                           */
/* OUTPUT =            The returned hft file descriptor.                     */
/*                                                                           */
/* NORMAL RETURN =     fd > 0 to indicate successful completion.             */
/*                                                                           */
/* ERROR RETURNS =     -1 = error on open.                                   */
/*                     Note:  If the hft has not been configured into the    */
/*                            system, errno will be set to ENODEV (no such   */
/*                            device).                                       */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    NONE.                                                                  */
/*                                                                           */
/*****************************************************************************/

int hxfohft (int oflag)
{
#ifdef HTX_REL_tu320
	extern  struct htxshm_hdr  *hft_p_shm_hdr;/*pointer to shm header.   */
        extern  struct htxshm_HE   *hft_p_shm_HE;/* pointer to shm HE entry. */

        int     hft;                            /* hft number */
        int     vt;                             /* virtual terminal number */

        /********************************************************************/
        /***  Beginning of executable code  *********************************/
        /********************************************************************/

        if (hft_p_shm_hdr != 0)  /* shared memory attached? */
        {
        	hft = hft_p_shm_HE->hft_num;
          	vt = hft_p_shm_HE->VT_num;
          	if (hft_p_shm_hdr->hft_devices[hft].fileid[vt] != 0)   /* hft open? */
          	{
           	 	return(hft_p_shm_hdr->hft_devices[hft].fileid[vt]);
          	}
          	else   /* hft NOT opened. */
          	{
          		errno = ENODEV;
            		return(-1);
	        } /* endif */
        }
        else  /* shm NOT attached */
        {
        	return(open("/dev/hft", oflag));
        } /* endif */
#else
        return (-1);
#endif

} /* hxfohft() */
