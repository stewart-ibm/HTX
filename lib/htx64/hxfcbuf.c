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

#include <htx_local.h>
#include <hxihtx.h>

#include <fcntl.h>

#ifdef	__HTX_LINUX__
#include <sys/stat.h>
#include <unistd.h>
#else
#include <sys/mode.h>
#endif

#ifdef	__HTX_LINUX__
extern	int	hxfmsg(struct htx_data *p, int err, enum sev_code, char *text);
#endif

#define DUMP_PATH "/tmp/"

#define MAX_MISCOMPARES 10
#define MAX_MSG_DUMP 20


/*
 * NAME: hxfcbuf()
 *
 * FUNCTION: Compares two buffers against each other.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function is called by any Hardware Exerciser (HE) program and
 *      is included as part of the libhtx.a library.
 *
 * NOTES:
 *
 *      This routine compares two buffers and if the compare is good returns
 *      a NULL pointer.
 *
 *      If the buffers do not compare a pointer to a character string in the
 *      following format is returned:
 *
 *          miscompare at displacement (decimal) = xxxxxxxx
 *          wbuf = xxxxxxxxxxxxxxxxxxxxxxx.... (20 bytes in hex)
 *          rbuf = xxxxxxxxxxxxxxxxxxxxxxx....
 *                 |
 *                 _ Byte that did not compare
 *
 *      And the two buffers are written out to the HTX dump directory.
 *
 *
 *      operation:
 *      ---------
 *
 *      set good return code (NULL error message pointer)
 *
 *      compare buffers
 *
 *      if buffers do not compare OK
 *          set pointer to error message
 *          build error message
 *
 *          save buffers to disk
 *
 *      return(message pointer)
 *
 *
 * RETURNS:
 *
 *      Return Codes:
 *      ----------------------------------------------------------------------
 *                   0 -- Normal exit; buffers compare OK.
 *       exit_code > 0 -- Problem on compare; pointer to error msg returned.
 *
 *
 */

int hxfcbuf(struct htx_data *ps, char *wbuf, char *rbuf, size_t len, char *misc_data)
     /*
      * ps -- pointer to the HE's htx_data structure
      * wbuf -- pointer to the write buffer
      * rbuf -- pointer to the read buffer
      * len -- the length in bytes of the buffers
      */
{
  /*
   ***  Data and Functions Definitions/Declarations  **************************
   */
	register long i;           /* compare loop counter                         */
	register long j;           /* error message loop counter                   */

	char *msg_ptr;             /* pointer to error message (NULL if good comp) */
	char path[128];            /* dump files path                              */
	char s[3];                 /* string segment used when building error msg  */
	char work_str[512];        /* work string                                  */

	int mis_flag, rc = 0;              /* miscompare flag: boolean                     */

	/*static ushort miscompare_count = 0;*//* miscompare count         */

	/*
	***  Beginning of Executable Code  *****************************************
	*/
	msg_ptr = 0;               /* set good return code (NULL pointer)          */

	mis_flag = FALSE;          /* set miscompare flag to FALSE                 */
	i = 0;

	while ((mis_flag == FALSE) && (i < len))
	{
		if (wbuf[i] != rbuf[i])
		{
			mis_flag = TRUE;
 		}
		else
  		{
			i++;
		}
	} /* endwhile */

 	if (mis_flag == TRUE)      /* problem with the compare?                    */
  	{
   		rc = -1;
   		msg_ptr = misc_data;             /* show bad compare                         */
		(void) sprintf(msg_ptr, "Miscompare at displacement (decimal) = %d ",(int)i);

		(void) htx_strcat(msg_ptr, "  wbuf = ");

      		for (j = i; ((j - i) < MAX_MSG_DUMP) && (j < len); j++)
        	{

#ifdef	__HTX_LINUX__
			(void) sprintf(s, "%x", wbuf[j]);
#else
			(void) sprintf(s, "%0.2x", wbuf[j]);
#endif
       			(void) htx_strcat(msg_ptr, s);
       		} /* endfor */

      		(void) htx_strcat(msg_ptr, "  rbuf = ");

      		for (j = i; ((j - i) < MAX_MSG_DUMP) && (j < len); j++)
        	{

#ifdef	__HTX_LINUX__
			(void) sprintf(s, "%x", rbuf[j]);
#else
			(void) sprintf(s, "%0.2x", rbuf[j]);
#endif

          		(void) htx_strcat(msg_ptr, s);
        	} /* endfor */

      		/* (void) htx_strcat(msg, "\n"); */


      		if (ps->miscompare_count < MAX_MISCOMPARES)
		{
	  		/*
	   		* Copy write and read buffers to dump file.
	   		*/
	  		ps->miscompare_count++;
	  		(void) htx_strcpy(path, DUMP_PATH);
	  		(void) htx_strcat(path, "htx");
	  		(void) htx_strcat(path, &(ps->sdev_id[5]));
	  		(void) htx_strcat(path, ".wbuf");
	  		(void) sprintf(work_str, "_%d_%-d",getpid(), ps->miscompare_count);
	  		(void) htx_strcat(path, work_str);

            (void) htx_strcat(msg_ptr, ". The miscompare buffer dump files are ");
            (void) htx_strcat(msg_ptr, path);

	  		(void) hxfsbuf(wbuf, len, path, ps);

	  		(void) htx_strcpy(path, DUMP_PATH);
	  		(void) htx_strcat(path, "htx");
	  		(void) htx_strcat(path, &(ps->sdev_id[5]));
	  		(void) htx_strcat(path, ".rbuf");
	  		(void) htx_strcat(path, work_str);

            (void) htx_strcat(msg_ptr, " and ");
            (void) htx_strcat(msg_ptr, path);

	  		(void) hxfsbuf(rbuf, len, path, ps);
		}
      		else
		{
	  		(void) sprintf(work_str, "The maximum number of saved miscompare \
			buffers (%d) have already\nbeen saved.  The read and write buffers for this \
			miscompare will\nnot be saved to disk.\n", MAX_MISCOMPARES);
	  		(void) htx_strcat(msg_ptr, work_str);
		} /* endif */

    	} /* endif */

  	return(rc);

} /* hxfcbuf() */



/*
 * NAME: hxfsbuf()
 *
 * FUNCTION: Saves a buffer to a specified file on disk.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function is called by the hxfcbuf() function on the libhtx.a
 *      library.  Typically it will be called under an HTX Hardware
 *      Exerciser (HE) process.
 *
 * NOTES:
 *
 *      operation:
 *      ---------
 *
 *      set good return code
 *
 *      if file opens OK
 *
 *          if write() goes OK
 *
 *              fsync() the file to disk
 *
 *          close file
 *
 *      return
 *
 *
 * RETURNS:
 *
 *      Return Codes:
 *      ----------------------------------------------------------------------
 *                   0 -- Normal exit; buffers compare OK.
 *       exit_code > 0 -- The errno value at the time of the error.
 *
 *
 */

int hxfsbuf(char *buf, size_t len, char *fname, struct htx_data *ps)
     /*
      * buf -- pointer to the buffer to be copied to disk
      * int -- the length of the buffer
      * fname -- the name of the file the buffer is to be saved to
      * ps -- pointer to the HE's htx_data structure
      */
{
  /*
   ***  Data and Functions Definitions/Declarations  **************************
   */
	char  err_msg[MAX_TEXT_MSG];  /* error message string                      */

  	int   exit_code;           /* exit code                                    */
  	int   fileid;              /* file id                                      */
  	int   mode;                /* file mode                                    */
  	int   num_bytes;           /* write() return code                          */
  	int   oflag;               /* open() flag                                  */


  /*
   ***  Beginning of Executable Code  *****************************************
   */
  	exit_code = GOOD;          /* set good exit code                           */
  	errno = 0;                 /* clear errno                                  */

  	oflag = O_CREAT | O_WRONLY | O_TRUNC;
  	mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
                             /* set permission to -rw-r--r--                 */

  	if ((fileid = open(fname, oflag, mode)) == -1)
    	{
      		exit_code = errno;
      		(void) sprintf(err_msg, "Error opening %s.", fname);
      		(void) hxfmsg(ps, exit_code, HTX_SYS_HARD_ERROR, err_msg);
    	}
  	else                       /* open() OK                                    */
    	{
      		if ((num_bytes = write(fileid, buf, (int) len)) == -1)
        	{
          		exit_code = errno;
          		(void) sprintf(err_msg, "Error writing to %s.", fname);
          		(void) hxfmsg(ps, exit_code, HTX_SYS_HARD_ERROR, err_msg);
        	}
      		else                       /* write() OK                               */
        	{
          		if (num_bytes != (int) len)
            		{
              			(void) sprintf(err_msg, "Error writing to %s.\nOnly %d of %d \
				bytes successfully transferred on write() system call.",
                             	fname, num_bytes, len);
			        (void) hxfmsg(ps, exit_code, HTX_SYS_HARD_ERROR, err_msg);
            		} /* endif */

          		if (fsync(fileid) != GOOD)
            		{
              			exit_code = errno;
              			(void) sprintf(err_msg, "Error on fsync() to %s.", fname);
              			(void) hxfmsg(ps, exit_code, HTX_SYS_HARD_ERROR, err_msg);
         	   	} /* endif */

	        } /* endif */

 	     	if (close(fileid) != GOOD)
  	 	{
   	 	    	exit_code = errno;
      		    	(void) sprintf(err_msg, "Error on close() of %s.", fname);
        	  	(void) hxfmsg(ps, exit_code, HTX_SYS_HARD_ERROR, err_msg);
        	} /* endif */

    	} /* endif */

    	return(exit_code);

} /* hxfsbuf() */
