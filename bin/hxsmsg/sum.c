
/* @(#)27	1.5  src/htx/usr/lpp/htx/bin/hxsmsg/sum.c, htx_msg, htxubuntu 5/24/04 18:19:55 */

/*
 *   FUNCTIONS: sum
 */


#include <htx_local.h>

/*
 * NAME: sum()
 *                                                                    
 * FUNCTION: Calculates a checksum.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function is called by the check_ckpt() and ... functions of
 *      the "hxsmsg"
 *      program which is a child process of the HTX supervisor program,
 *      "hxssup". 
 *
 * NOTES: 
 *
 *      operation:
 *      ---------
 *
 *      while (characters left to be summed)
 *        {
 *          right circular shift sum
 *          add next character to sum
 *        }
 *
 *      return(sum)
 * 
 *               
 * RETURNS: 
 *
 *      Return Codes:
 *      -------------
 *        -1 -- Error.
 *      >= 0 -- the sum value.
 *
 *
 */  

long sum(register char *array_ptr, off_t num_bytes)
     /*
      * array_ptr -- pointer to the array of bytes to be summed
      * num_bytes -- the number of bytes to be summed
      */

{
  /*
   ***  Data and Functions Definitions/Declarations  **************************
   */
  	register long checksum;    /* checksum variable                       */
  	register long i;           /* loop counter                            */


  /*
   ***  Beginning of Executable Code  *****************************************
   */
  	checksum = 0;

  	for (i = 0; i < num_bytes; i++)
    	{
      /*
       * right circular shift
       */
      		if ((checksum & 0x0001) != 0)
		{
			checksum = (checksum >> 1) + 0x8000;
		}
      		else
		{
			checksum >>= 1;
		}

      		checksum += *array_ptr;
      		checksum &= 0xFFFF;

      		array_ptr++;
    	} /* endfor */

  	return(checksum);

} /* sum() */
