
/* @(#)01	1.5  src/htx/usr/lpp/htx/lib/htx/hxfpgen.c, htx_libhtx, htxubuntu 5/24/04 18:09:10 */

#include <htx_local.h>

#define NO_CAST_MIN_VAL_LIMIT 0

#define MAX_PATTERN_SIZE ((size_t) (64 * 1048576))
#define MAX_REPEAT_LIMIT ((size_t) 1048576)
#define MAX_VAL_LIMIT ((ushort) 0xff)

#define MIN_PATTERN_SIZE ((size_t) 1)
#define MIN_REPEAT_LIMIT ((size_t) 1)
#define MIN_VAL_LIMIT ((ushort) NO_CAST_MIN_VAL_LIMIT)
                                                             
/*
 * NAME: hxfpgen()
 *                                                                    
 * FUNCTION: Generates a random sequence of strings containing only one 
 *           character byte value.  
 *
 *           E.g., AAAAEE4444444444010101010101   (hex)
 *           
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function is called by HTX hardware exerciser programs as part
 *      the libhtx.a library.
 *                                                                   
 * NOTES: 
 *
 *
 * RETURNS:
 *
 *      Returned Values:
 *      ----------------
 *          0 -- Successful pattern generation
 *          1 -- Invalid "pattern" -- pointer to pattern buffer
 *
 */  

short hxfpgen(ushort max_val, ushort min_val, size_t max_repeat,
	      size_t min_repeat, char *pattern, size_t pattern_size)
     /*
      * max_val -- max byte value {0x00:0xff}
      * min_val -- min byte value {0x00:0xff} <= max_val
      * max_repeat -- max number of allowed repeats
      * min_repeat -- min number of repeats
      * pattern -- pointer to pattern buffer
      * pattern_size -- size of pattern
      */
{
  /*
   ***  Local Definitions...  *************************************************
   */

	short rc;                      /* return code                              */	

 	size_t bytes_generated;        /* number of bytes generated for pattern    */
  	size_t bytes_to_write;         /* number of bytes to write to pattern      */
	size_t i;                      /* loop counter                             */
  	size_t rand_length;            /* random string length                     */

  	ushort new_rand_byte;          /* new random byte value                    */
  	ushort rand_byte;              /* random byte value                        */

  	/*
   	***  Beginning of Executable Code...  **************************************
   	*/

  	/*
  	 *  check function input parameters...
   	*/
  	if (max_val > MAX_VAL_LIMIT)
    	{
		max_val = MAX_VAL_LIMIT;
	}
	#if NO_CAST_MIN_VAL_LIMIT > 0     /* If limit is zero, the next if statement */
  		if (min_val < MIN_VAL_LIMIT)    /* would be a degenerate unsigned compar.  */
  		{ 			
			min_val = MIN_VAL_LIMIT;
		}
	#endif

  	if (min_val > max_val)
    	{
		min_val = max_val;
	}	
 	if (max_repeat > MAX_REPEAT_LIMIT)
 	{   	
		max_repeat = MAX_REPEAT_LIMIT;
	}	
  	if (min_repeat < MIN_REPEAT_LIMIT)
  	{  	
		min_repeat = MIN_REPEAT_LIMIT;
	}	
  	if (min_repeat > max_repeat)
 	{  	
		min_repeat = max_repeat;
	}
  	if (pattern_size < MIN_PATTERN_SIZE)
	{   	
		pattern_size = MIN_PATTERN_SIZE;	
	}	
  	if (pattern_size > MAX_PATTERN_SIZE)
 	{   	
		pattern_size = MAX_PATTERN_SIZE;
	}
  	if (pattern == (char *) 0)
   	{
		rc = 1;
	}
  	else
    	{
	
		rc = 0;                        /* set good return code                 */
      		/*
       		*  generate pattern...
       		*/
      		rand_byte = 0;
      		bytes_generated = 0;
      		while (bytes_generated < pattern_size)
		{

	  		/* the following loop makes sure that random bytes are not
	     		repeated */
	  		do
	    		{
	      			new_rand_byte = rand()%((max_val - min_val) + 1) + min_val;
	    		} 
	  		while (rand_byte == new_rand_byte);

	  		rand_byte = new_rand_byte;

	  		rand_length = rand()%((max_repeat - min_repeat) + 1) + min_repeat;
	  		bytes_to_write = MIN((pattern_size - bytes_generated), rand_length);

			for (i = 0; i < bytes_to_write; i++)
	    		{
	      			*pattern = (char) rand_byte;
	      			pattern++;
	    		} /* end_for */

	  		bytes_generated += bytes_to_write;
		} /* end_while */

    	} /* endif */

  	return(rc);

} /* hxfpgen() */
