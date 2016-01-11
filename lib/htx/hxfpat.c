
/* @(#)00	1.9  src/htx/usr/lpp/htx/lib/htx/hxfpat.c, htx_libhtx, htxubuntu 5/24/04 18:08:50 */

#include <htx_local.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <memory.h>
#include <unistd.h>
#include <stdio.h>

/*
 * NAME: hxfpat()
 *                                                                    
 * FUNCTION: Opens and reads the specified pattern file into the specified
 *           buffer.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function is called by any Hardware Exerciser (HE) program and
 *      is included as part of the libhtx.a library.
 *
 * NOTES: 
 *
 *      operation:
 *      ---------
 *      set good return code
 *
 *      if (unable to open pattern file)
 *
 *        set return code to 1
 *
 *      else
 *        
 *        clear pattern buffer
 *
 *        if (unable to read pattern file)
 *
 *          set return code to 2
 *
 *        else
 *
 *          copy read pattern into pattern buffer until pattern
 *          buffer is full
 *
 *        endif
 *
 *        if (error on close of pattern file)
 *       
 *          set return code to 3
 *
 *      endif
 *
 *      return(return code)
 *
 *               
 * RETURNS: 
 *
 *      Return Codes:
 *      ----------------------------------------------------------------------
 *      0 -- Normal return; no errors.
 *      1 -- Unable to open pattern file.
 *      2 -- Unable to read pattern file.
 *      3 -- Error closing pattern file.
 *
 *
 */  

int hxfpat(char *filename, char *pattern_buf, size_t num_chars)
     
{
  /*
   ***  Data and Functions Definitions/Declarations  **************************
   */
  	int chars_read;             /* number of characters from read() system call*/
 	#if 0  	
		int errno_save;     /* save area for errno      */
	#endif  	
	int fildes;                 /* file descriptor of pattern file             */
	int mode_flag;              /* mode flag for open() system call            */
  	int return_code;            /* return code for function                    */
 
  	size_t chars_copied;        /* the number of chars copied                  */
  	size_t chars_left_to_copy;  /* the number of chars left to copy            */
  	size_t chars_this_copy;     /* the number of chars to copy this time in    */
  /*                             the loop                                    */
  
  /*
   ***  Beginning of Executable Code  *****************************************
   */

  	return_code = 0;
  	mode_flag = S_IWUSR | S_IWGRP | S_IWOTH;
  	errno = 0;
   
   /* Open the pattern file and check for errors...
   */
	if ((fildes = open(filename, O_RDONLY, mode_flag)) == -1)
    	{
	#ifdef DEBUG
      		errno_save = errno;
      		(void) sprintf(error_message,
		"\nhxfpat() -- Error opening pattern file (%s).\n\
		errno = %d (%s).\n",
		filename,
		errno_save,
		strerror(errno_save));
      		(void) fprintf(stderr, "%s", error_message);
                (void) fflush(stderr);
	#endif /* DEBUG */
      		return_code = 1;
    	}
  	else                        /* If here, pattern file opened OK.            */
    	{
      
      		/*
       		* Clear the pattern buffer...
       		*/
      		(void) memset((void *) pattern_buf, 0, num_chars);
      
      		errno = 0;
      		/*
       		* Read the pattern file and check for errors...
       		*/
      		if ((chars_read = read(fildes, pattern_buf, num_chars)) == -1)
		{
		#ifdef DEBUG
	  		errno_save = errno;
	  		(void) sprintf(error_message,
			"\nhxfpat() -- Error reading pattern file (%s).\n\
			errno = %d (%s).\n",
			filename,
			errno_save,
			strerror(errno_save));
	  		(void) fprintf(stderr, "%s", error_message);
                        (void) fflush(stderr);
		#endif /* DEBUG */
	  		return_code = 2;
		}
      		else                        /* If here, read() was OK.                 */
		{
			/*
	  		* Copy the pattern until the buffer is full...
			*/
	  		chars_copied = chars_read;
	  		while (chars_copied < num_chars)
	    		{
	      			chars_left_to_copy = num_chars - chars_copied;
	      
	      			if (chars_left_to_copy < chars_copied)
				{
					chars_this_copy = chars_left_to_copy;
	   			}				
		   		else
				{
					chars_this_copy = chars_copied;
	    			}	  
	      				(void) memcpy((void *) (pattern_buf + chars_copied),
			    		(void *) pattern_buf,
			    		chars_this_copy);
	      
	      			chars_copied += chars_this_copy;
	    		} /* endwhile */
		} /* endif */
      		/*
       		* Close the pattern file.
       		*/
      		errno = 0;
      		if (close(fildes) != 0)
		{
		#ifdef DEBUG
	  		errno_save = errno;
	  		(void) sprintf(error_message,
			 "\nhxfpat() -- Error closing pattern file (%s).\n\
			 errno = %d (%s).\n",
			 filename,
			 errno_save,
			 strerror(errno_save));
	  		(void) fprintf(stderr, "%s", error_message);
			(void) fflush(stderr);
		#endif /* DEBUG */
	  		return_code = 3;
		} /* endif */
    	} /* endif */
  
  	return(return_code);
  
} /* hxfpat() */
