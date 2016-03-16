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
