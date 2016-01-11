// @(#)81   1.9.1.1  src/htx/usr/lpp/htx/bin/hxeasy/classlib.C, exer_asy, htx53E 6/21/05 06:14:56
//
//   COMPONENT_NAME: exer_asy
//
//   FUNCTIONS: Reader
//		WriteComm
//		Writer
//		bytes
//		if
//
//   ORIGINS: 27
//
//   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
//   combined with the aggregated modules for this product)
//   OBJECT CODE ONLY SOURCE MATERIALS
//
//   (C) COPYRIGHT International Business Machines Corp. 88,93
//   All Rights Reserved
//   US Government Users Restricted Rights - Use, duplication or
//   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
//
#ifndef __HTX_LINUX__
#include"headers.h"
#pragma mc_func trap { "7c810808" }
#pragma reg_killed_by trap
#else
#include <headers.h>
#endif

#define MISC 0xBEEFDEAD

int Reader(ARGS_T *ArgDataPtr)
{

	// declare msg object for htx communication
	Cmsg ToHtx(ArgDataPtr);

	// get pointer to the stats structure. 
	struct htx_data *stats=ToHtx.GetTempStatsPtr();

	// typecast a pointer to the comport object so I can read. 
	Ccom *Port= (Ccom *)ArgDataPtr->Port; // type cast Port var.

	// Get pointers to the events the reader needs.          
	// These events are declared in hxeasy.C
	CcreateEvent *WstartReader=(CcreateEvent *)ArgDataPtr->WstartReader;
	CcreateEvent *WstopReader=(CcreateEvent *)ArgDataPtr->WstopReader;
	CcreateEvent *WstopedReader=(CcreateEvent *)ArgDataPtr->WstopedReader;

	// declare vars used in this routine. 
	DWORD   read0;              // counts number of times 0 bytes are read
								// this means the read timed out or the select
								// messed up.

	DWORD   num_oper_todo;      // counts down the num_oper loops
	DWORD   num_oper_done;      // counts up the num_oper loops

	int   	num_chars_left;     // counts the number of chars to be received per
								// num_oper loop

	DWORD	bufsize_bytes;		// counts the bytes to be read per bufsize
								// write. Since there is no guarentee
								// that all bufsize bytes are read in
								// a single read.  I keep reading till    
								// bufsize bytes are read. 

	char 	*cw_ptr;			// where I compare write data from, the current data pointer. 
	char 	*cr_ptr;			// where I compare read data from, the current data pointer. 
	char 	*r_ptr;		 		// where I read data to, the current data pointer. 

	DWORD	tmpread;			// holds the number of bytes the read says
								// it read each time its called. 

	DWORD	i;
								// a miscompare is found in one num_oper loop.
								// I don't want to print out 100 miscompares 
								// for each loop, so I just get the first one. 

	int try_reset=0;	// set this when miscompare happens on out of sync read.
	int retry_cnt;
	char cbuf_msg[MAX_TEXT_MSG];
	int rc=0;


	//**************************************************************
	//    DECIDE WHEATHER TO START THE WRITER.
	//**************************************************************
	// I need to see if the read needs to run and start this writer
	// If I start my processes writer then I don't start my read
	// becuase the other read will start to read my writes data.
	if( ArgDataPtr->Flags&HALF_DUPLEX) {
#ifdef BUGIT
		sprintf(ToHtx.GetMsgAddr(),"Read: RUN half_duplex,"
		"dir %d, IM_THE_WRITER %d, pass %ld\n",ArgDataPtr->Rules.default_dir,
		IM_THE_WRITER,ArgDataPtr->pass_count );
		ToHtx.SendMsg(0,HTX_HE_INFO);
#endif
		if(IM_THE_WRITER) {
			if(ArgDataPtr->Rules.default_dir) {
			
				// if I the writer then this side writes but       
				// does not read so let the other side start
				// my writer
#ifdef BUGIT
				sprintf(ToHtx.GetMsgAddr(),"Read: Don't read or start write.\n");
				ToHtx.SendMsg(0,HTX_HE_INFO);
#endif
				return(0);
			} else {
				// The other side writes and I read.
				WstartReader->SignalEvent();      
				if(ArgDataPtr->Flags&ABORT) goto readexit_error;
			}
		} else {
			// I am not the writer unless default_dir is 0.
			if(!(ArgDataPtr->Rules.default_dir)) {
				// if I the writer then this side writes but       
				// does not read so let the other side start
				// my writer
#ifdef BUGIT
				sprintf(ToHtx.GetMsgAddr(),"Read: Don't read or start write.\n");
				ToHtx.SendMsg(0,HTX_HE_INFO);
#endif
				return(0);
			} else {
				// The other side writes and I read.
				WstartReader->SignalEvent();      
				if(ArgDataPtr->Flags&ABORT) goto readexit_error;
			}
		}

	} else {
#ifdef BUGIT
		sprintf(ToHtx.GetMsgAddr(),"Read: Signal write to start......\n");
		ToHtx.SendMsg(0,HTX_HE_INFO);
#endif
		WstartReader->SignalEvent();      
	} 

	//*************************************************************
	// INIT VARIABLES FOR THIS RUN THRU THE TEST
	//*************************************************************
	num_oper_todo=(DWORD)ArgDataPtr->Rules.num_oper;
	num_oper_done=0;
	read0=0L;
	
	if(!(ArgDataPtr->Flags&WRITE_ONLY)) {

		while(num_oper_todo--) {   		// Loop for num_oper times
			++num_oper_done;
			num_chars_left=(DWORD)ArgDataPtr->Rules.num_chars;

			// reset the data pointers to the start of the num_chars buffers. 
			r_ptr=ArgDataPtr->TestParms.ReadDataPtr;
			cr_ptr=ArgDataPtr->TestParms.ReadDataPtr;
			cw_ptr=ArgDataPtr->TestParms.WriteDataPtr;

			// clear out the read buffer for the next num_oper use periods.
			memset(r_ptr,0x00,ArgDataPtr->Rules.num_chars);

			while(num_chars_left) {  	// read num_chars per num_oper
				// set the number of bytes to try and read for
				// this buffer. 
				if(num_chars_left > ArgDataPtr->Rules.bufsize)
					bufsize_bytes=ArgDataPtr->Rules.bufsize;
				else
					bufsize_bytes=num_chars_left;

				while(bufsize_bytes) {     // read one bufsize of bytes
					// check abort flag each loop. 
					if(ArgDataPtr->Flags&ABORT) goto readexit_error;

					tmpread=Port->ReadComm(bufsize_bytes,(char *)r_ptr,READ_TIMEOUT,ArgDataPtr);

					if(tmpread == BADBOY) {
						sprintf(ToHtx.GetMsgAddr(),"Read: BADBOY.\n");
						ToHtx.SendMsg(0,HTX_HE_INFO);
						goto readexit_error;
					}

					if(tmpread > bufsize_bytes) {
							sprintf(ToHtx.GetMsgAddr(),"bytes_to_read %d. BytesRead %d\n",
											(int)bufsize_bytes,(int)tmpread);
							ToHtx.SendMsg(0,HTX_HE_SOFT_ERROR);
							goto readexit_error;
					}

					if(tmpread==0) {

						if(ArgDataPtr->device_type == evcon) {
							retry_cnt=10;
						} else {
							retry_cnt=60;
						}
						// reset at 360  seconds since reads time out in 6 sec	
						if(++read0 > retry_cnt)   
						{

							if(ArgDataPtr->device_type == evcon) {
								sprintf(ToHtx.GetMsgAddr(), "Read 0 bytes hit %d retries. You may need wraplug on the serial port.\n"
								"num_oper count is %d,index into read buffer is %d\n",
								retry_cnt,(int)num_oper_done,(r_ptr-ArgDataPtr->TestParms.ReadDataPtr));

								ToHtx.SendMsg(0,HTX_HE_HARD_ERROR);
								goto readexit_error;
							} 
							else {
								sprintf(ToHtx.GetMsgAddr(), "Read 0 bytes hit %d retries try reset\n"
								"num_oper count is %d,index into read buffer is %d\n",
								retry_cnt,(int)num_oper_done,(r_ptr-ArgDataPtr->TestParms.ReadDataPtr));

								ToHtx.SendMsg(0,HTX_HE_INFO);

	//TWM change to try to restart the test...treat it as miscompare so the
	// writer will be notified to restart too.
								try_reset = 1;
								break;
							}
						}
						continue;
					}
					else { 
						read0=0L;
					}

//twm
//	    sprintf(ToHtx.GetMsgAddr(),"Reader: Read DONE. bytesread=%d, %x%x%x%x%x%x%x%x%x%x\n",tmpread,r_ptr[0],r_ptr[0],r_ptr[1],r_ptr[2],r_ptr[3],r_ptr[4],r_ptr[5],r_ptr[6],r_ptr[7],r_ptr[8],r_ptr[9]);
//	    ToHtx.SendMsg(0,HTX_HE_INFO);
					if(ArgDataPtr->device_type != evcon) {

						// Now compare the bytes you just read.......
						for(i=0; i<tmpread; ++i) {
							if( *cr_ptr != *cw_ptr ) {
								if(ArgDataPtr->Rules.crash==1) {
#ifndef __HTX_LINUX__
									trap((int) MISC,(long) cw_ptr,(long) cr_ptr,(int) ArgDataPtr->my_ttynum);
#endif
								}
								if(num_oper_done == 1 && i == 0) {
									sprintf(ToHtx.GetMsgAddr(), "Out of sync try reseting numoper= %d\n",(int)num_oper_done);		
									ToHtx.SendMsg(0, HTX_HE_INFO);
									try_reset = 1;
									break;     
								} else {
									sprintf(ToHtx.GetMsgAddr(), "MISCOMPARE  numoper %d\n",(int)num_oper_done);		
									ToHtx.SendMsg(0, HTX_HE_INFO);
									memset(cbuf_msg, 0, MAX_TEXT_MSG);
									rc = hxfcbuf(stats, ArgDataPtr->TestParms.WriteDataPtr, ArgDataPtr->TestParms.ReadDataPtr,
													ArgDataPtr->Rules.num_chars, cbuf_msg);
									strcpy(ToHtx.GetMsgAddr(), cbuf_msg);
									ToHtx.SendMsg(0, HTX_HE_SOFT_ERROR);
									stats->bad_reads++;
									break;	
								}

							}
							++cr_ptr;
							++cw_ptr;
						}
					}
					r_ptr += tmpread;
					bufsize_bytes-=tmpread;
					num_chars_left-=tmpread;
					if(ArgDataPtr->Flags&MISCOMPARE || try_reset) break;
				}				// end of read one bufsize of bytes


				// set the READ_DONE flag so the writer will write the next
				// buffer.  Or see the stop due to miscompare.
				if(ArgDataPtr->TestParms.wrap_type==ePlug &&
					!(ArgDataPtr->Flags&WRITE_ONLY)) {
						ArgDataPtr->Flags|=READ_DONE;
				}

				if(ArgDataPtr->Flags&MISCOMPARE || try_reset) break;
			}				// end of read num_chars per num_oper

			stats->bytes_read+=ArgDataPtr->Rules.num_chars;
			stats->good_reads++;

#ifdef BUGIT
			sprintf(ToHtx.GetMsgAddr(), "hxeasy: good_reads= %d, bytes_read=%d\n", stats->good_reads, stats->bytes_read);
			ToHtx.SendMsg(0,HTX_HE_INFO);
#endif
			hxfupdate(UPDATE,stats);
			if(ArgDataPtr->Flags&MISCOMPARE || try_reset) break;
		}					// end of loop for num_oper times

		// comes here too if try_reset is set or miscompare.. or end of numoper tests
		// signal the write to stop. 
		WstopReader->SignalEvent();
		sleep(5); 
		// Now clean the read data incase the write is blocked, we want any current
		// write to complete so the write process will see the stop signal. 
		Port->Clean_Read_Q(ArgDataPtr);

		if(ArgDataPtr->Flags&MISCOMPARE) {
			// reset miscompare flag ....
			ArgDataPtr->Flags&=~MISCOMPARE;
			// don't stop on miscompare, let it run.
			// goto readexit_error;
		}
	}
	
	// Read must wait here for write to signal its stopped.....
	if(WstopedReader->WaitEvent(COMM_TIMEOUT)==WAIT_TIMEOUT) {
		sprintf(ToHtx.GetMsgAddr(),"Read: Time out write not stopped\n");
		ToHtx.SendMsg(0,HTX_HE_SOFT_ERROR);
		goto readexit_error;
	}

return(0);

readexit_error:

	sprintf(ToHtx.GetMsgAddr(),"READexit: readexit_error entered\n");
	ToHtx.SendMsg(0,HTX_HE_INFO);

     if(!(ArgDataPtr->Flags&ABORT)) {
		// The error was in the reader..........
		// set the abort flag.....
		sprintf(ToHtx.GetMsgAddr(),"READexit: Set ABORT flag\n");
		ToHtx.SendMsg(0,HTX_HE_INFO);
		ArgDataPtr->Flags|=ABORT;
		return(BADBOY);
	}

	// return 0 if some other routine or thread set abort...
    return(0);
}


void Writer(LPDWORD arg)
{

	// type cast the pointer to the argdata struct. 
	ARGS_T *ArgDataPtr = (ARGS_T *)arg;

	// get an object for communication with htx
	Cmsg ToHtx(ArgDataPtr);
	struct htx_data *stats=ToHtx.GetTempStatsPtr();

	// typecast the pointer to the comport object.
	Ccom *Port= (Ccom *)ArgDataPtr->Port; // type cast Port var.

	// Get pointers to the events the reader needs.          
	// These events are declared in hxeasy.C
	CcreateEvent *WstartWriter=(CcreateEvent *)ArgDataPtr->WstartWriter;
	CcreateEvent *WstopWriter=(CcreateEvent *)ArgDataPtr->WstopWriter;
	CcreateEvent *WstopedWriter=(CcreateEvent *)ArgDataPtr->WstopedWriter;

	// declare vars used in this routine. 
	DWORD	rc;					// return code from wait event.

	DWORD   num_oper_todo;      // counts down the num_oper loops
	DWORD   num_oper_done;      // counts up the num_oper loops

	int   	num_chars_left;     // counts the number of chars to be xmited per
								// num_oper loop

	DWORD	bufsize_bytes;		// counts the bytes written per bufsize
								// write. Since there is no guarentee
								// that all bufsize bytes are written in
								// a single write.  I keep writting till    
								// bufsize bytes are written. 
	char 	*w_ptr;				// where I write from, the data pointer. 

	DWORD	tmpwritten;		// holds the number of bytes the write says
								// it writes each time its called. 

	// while I am using sigalrm to control read timing.....I need
	// to block the SIGALRM signal to this thread. 
	sigset_t BlockSigSet; 	// signal mask
	sigemptyset(&BlockSigSet); 	// empty the mask
	sigaddset(&BlockSigSet, SIGALRM);    // add SIGALRM to mask
	sigaddset(&BlockSigSet, SIGTERM);
	sigaddset(&BlockSigSet, SIGINT); 
	sigaddset(&BlockSigSet, SIGQUIT);
#ifndef __HTX_LINUX__
	sigaddset(&BlockSigSet, SIGDANGER);
#endif

	sigaddset(&BlockSigSet, SIGHUP);
	sigaddset(&BlockSigSet, SIGABRT);
	// set the mask. 
#ifndef __HTX_LINUX__
	sigthreadmask(SIG_SETMASK, &BlockSigSet, NULL);
#else
	/*
	 * In Linux, signals are handled at process level as there is one-to-one mapping 
	 * b/w process and thread.
	 */ 
	sigprocmask(SIG_SETMASK, &BlockSigSet, NULL);
#endif

    // Now go into a loop that runs thru out the htx run.. it only gets
    // exited when theres and error or control C from htx.

    while(1) {          // while alive loop 

#ifdef BUGIT
	    sprintf(ToHtx.GetMsgAddr(),"Writer: Wait for signal to start from Read.\n");
	    ToHtx.SendMsg(0,HTX_HE_INFO);
#endif

		if(ArgDataPtr->Flags & ABORT) goto writeexit_error;
		// wait for write start event from read.
		if(WstartWriter->WaitEvent(COMM_TIMEOUT)==WAIT_TIMEOUT) {
			sprintf(ToHtx.GetMsgAddr(),"Write: Time out waiting for teststart\n");
			ToHtx.SendMsg(0,HTX_HE_SOFT_ERROR);
			goto writeexit_error;
		}

#ifdef BUGIT
	    sprintf(ToHtx.GetMsgAddr(),"Writer: GOT start signal from Read.\n");
	    ToHtx.SendMsg(0,HTX_HE_INFO);
#endif

		// init al variables for this num oper loop.

		// I want to write the num_char buffer num_oper times
		num_oper_todo=((DWORD)ArgDataPtr->Rules.num_oper);
		num_oper_done=0;


		while(num_oper_todo--) {       // loop for num_oper times

			++num_oper_done;           // increment num_oper counter

			// Set the starting write data pointer for the next num_char set
			w_ptr=ArgDataPtr->TestParms.WriteDataPtr;

			// Set the number of chars to write per num_oper to the rule num_char
			// value. 
			num_chars_left=(DWORD) ArgDataPtr->Rules.num_chars;

			while(num_chars_left) {        // xmit num_chars per num_oper

				// set the number of bytes to try and write for
				// this buffer. 
				if(num_chars_left > ArgDataPtr->Rules.bufsize)
					bufsize_bytes=ArgDataPtr->Rules.bufsize;
				else
					bufsize_bytes=num_chars_left;

				while(bufsize_bytes) {     // write one bufsize of bytes
					// check abort flag each loop. 
					if(ArgDataPtr->Flags&ABORT) goto writeexit_error;


					// check the stop signal. The 0 time out says, check it
					// and retrun immediately. 
					if((rc=WstopWriter->WaitEvent(0))==WAIT_OBJECT_0) {
						// if event is signaled set flag an stopwrite
						ArgDataPtr->Flags|=STOP_WRITE; 
						break;
					} else {
						if(rc != WAIT_TIMEOUT) goto writeexit_error;
					}

					// Make sure the READ_DONE flag is reset so the reader
					// must set READ_DONE or writer waits.
					if(ArgDataPtr->TestParms.wrap_type==ePlug &&
						!(ArgDataPtr->Flags&WRITE_ONLY)) {
							ArgDataPtr->Flags&=~READ_DONE;
					}

					// Now do the writes.
					tmpwritten=Port->WriteComm(bufsize_bytes,
							  (char *)w_ptr,
							  ArgDataPtr);
//twm
//	    sprintf(ToHtx.GetMsgAddr(),"Writer: DO WRITE. byteswritten=%d, %x%x%x%x%x%x%x%x%x%x\n",tmpwritten,w_ptr[0],w_ptr[0],w_ptr[1],w_ptr[2],w_ptr[3],w_ptr[4],w_ptr[5],w_ptr[6],w_ptr[7],w_ptr[8],w_ptr[9]);
//	    ToHtx.SendMsg(0,HTX_HE_INFO);

					// add this so user can pace the test......
					if(ArgDataPtr->Rules.write_sleep) 
						usleep(ArgDataPtr->Rules.write_sleep);

					if(tmpwritten == BADBOY) {
                            sprintf(ToHtx.GetMsgAddr(), "num_oper count is %d,"
								"current index into write buffer is %d\n",
							 (int)num_oper_done,(w_ptr-ArgDataPtr->TestParms.WriteDataPtr));

                            ToHtx.SendMsg(0,HTX_HE_HARD_ERROR);
                            goto writeexit_error;
					}

					// See if the write system write wrote more bytes than it 
					// was told to write. 
					if(tmpwritten > bufsize_bytes) {
						// this is a system error.....
						sprintf(ToHtx.GetMsgAddr(),"The write call wrote %d"
						" bytes when I told it to write %d, num_oper was %d\n"
						,(int)tmpwritten,(int)bufsize_bytes,(int)num_oper_done);
						ToHtx.SendMsg(0,HTX_HE_SOFT_ERROR);
						goto writeexit_error;
					}
					//NOTE: the writer will write till the driver blocks the 
					//write. in port to port mode. 

					// now check to see if 0 bytes were written. 
					// this is not a valid condition in AIX. 
					// unless I am doing WRITE_ONLY
					// If all buffers are full
					// the write will block.  In NT it may return. 
					if(tmpwritten==0) {
						if(ArgDataPtr->Flags&WRITE_ONLY) {
							usleep(5000);
							continue;
						}
						sprintf(ToHtx.GetMsgAddr(),"Write 0 bytes, ERROR \n"
						"num_oper count is %d,current index into write buffer is %d\n\n",
						(int)num_oper_done,	(w_ptr-ArgDataPtr->TestParms.WriteDataPtr));

						ToHtx.SendMsg(0,HTX_HE_HARD_ERROR);
						goto writeexit_error;
					}
					else {
						w_ptr+=tmpwritten;
						stats->bytes_writ+=tmpwritten;
						bufsize_bytes-=tmpwritten;
						num_chars_left-=tmpwritten;
					}

				}   // end of write one bufsize of bytes.  

				if(ArgDataPtr->Flags&STOP_WRITE) break;

				// Now make sure on wrap plugs that I sync with the reader. 
				// The wrap plugs wrap rts to cts so there is no flow
				// control. So I can't write more that 16 bytes (the size of
				// the uart buffer) before reading the port. I will limit 
				// the bufsize to 16 bytes on all wrap plug connections. 
				if(ArgDataPtr->TestParms.wrap_type==ePlug &&
					!(ArgDataPtr->Flags&WRITE_ONLY)) {
					while(1) {
						// This will lock the read and write together
						// making the operations synchronous. 
						if(ArgDataPtr->Flags&READ_DONE) break;
						else {
							/* check to see if the reader signaled stop or the
								abort flag is set */
							// check the stop signal. The 0 time out says
							// check it and retrun immediately. 
							if((rc=WstopWriter->WaitEvent(0))==WAIT_OBJECT_0) {
								// if event is signaled set flag an stopwrite
								ArgDataPtr->Flags|=STOP_WRITE; 
								break;
							} else {
								if(rc != WAIT_TIMEOUT) goto writeexit_error;
							}
							// check abort flag each loop. 
							if(ArgDataPtr->Flags&ABORT) goto writeexit_error;
							sleep(1); 
						}
					}
				}
				if(ArgDataPtr->Flags&STOP_WRITE) break;
			}  // end of xmit num_chars per num_oper.. a num oper complete here
		
			stats->good_writes++;

#ifdef BUGIT
			sprintf(ToHtx.GetMsgAddr(), "hxeasy: good_writes= %d, bytes_writ=%d\n", stats->good_writes, stats->bytes_writ);
			ToHtx.SendMsg(0,HTX_HE_INFO);
#endif
			hxfupdate(UPDATE,stats);

			if(ArgDataPtr->Flags&STOP_WRITE) break;
		}           // end of loop for num_oper times

		// the write may get done before
		// the read can signal it to stop. So if I haven't received
		// stop here I have to wait for it.
		if(!(ArgDataPtr->Flags&WRITE_ONLY)) {
			if(!(ArgDataPtr->Flags&STOP_WRITE)) {
				// wait for write stop event from read.
				if(WstopWriter->WaitEvent(COMM_TIMEOUT)==WAIT_TIMEOUT) {
					sprintf(ToHtx.GetMsgAddr(),"Write: Time out "
								"waiting for stop from read\n");
					ToHtx.SendMsg(0,HTX_HE_SOFT_ERROR);
					goto writeexit_error;
				}
			} 
			// reset stop write flag. 
			ArgDataPtr->Flags&=~STOP_WRITE;
		}
 
		// signal the read that I am stopped..
		WstopedWriter->SignalEvent();

   }    // end of while alive loop.


writeexit_error:

#ifdef BUGIT
	sprintf(ToHtx.GetMsgAddr(),"WRITEexit: Enter writeexit_error\n");
	ToHtx.SendMsg(0,HTX_HE_INFO);
#endif


    if(!(ArgDataPtr->Flags&ABORT)) {
		sprintf(ToHtx.GetMsgAddr(),"writeexit: Set ABORT flag\n");
		ToHtx.SendMsg(0,HTX_HE_INFO);
		// let everyone else know to abort.
		ArgDataPtr->Flags|=ABORT;
	}

	// signal the read that I am stopped..
	WstopedWriter->SignalEvent();

    return;

}

int ReadPattern(ARGS_T *ArgDataPtr)
{

	/****************************************************/
	/* Call the separately compiled hxfpat function to  */
	/* open and read the pattern file specified in the  */
	/* ArgDataPtr->Rules file, and fill the async write data buffer */
	/* with data from that pattern file.                */
	/*                                                  */
	/* The passed arguments are:                        */
	/*                                                  */
	/* pattern_path - path to the pattern file.         */
	/*                                                  */
	/* pattern - the buffer into which the pattern file */
	/* is to be copied.                                 */
	/*                                                  */
	/* r.num_chars - the number of characters to be     */
	/* placed in the buffer.                            */
	/*                                                  */
	/* returned values:                                 */
	/* 1 = error opening the pattern file               */
	/* 2 = error reading the pattern file               */
	/* 0 = buffer initialized correctly                 */
	/****************************************************/
	 int k = hxfpat(        PATTERNPATH,
						PATTERNDATA,
						(ArgDataPtr->Rules.num_chars+1));
	 if (k == 1) {
		Cmsg ToHtx(ArgDataPtr);
	    (void) sprintf(ToHtx.GetMsgAddr(), "open error %s \n",
			   PATTERNPATH);
		ToHtx.SendMsg(0, HTX_HE_SOFT_ERROR);
	 } else if (k == 2) {
/********************************************/
/* Format error message and call hxfmsg()   */
/* to process error.                        */
/********************************************/
		Cmsg ToHtx(ArgDataPtr);
	    (void) sprintf(ToHtx.GetMsgAddr(), "read error %s \n",
			   PATTERNPATH);
		ToHtx.SendMsg(0, HTX_HE_SOFT_ERROR);

	 }                      /* endif */

	return(0);                      // success.....
}
