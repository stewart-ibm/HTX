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

#ifndef __HTX_LINUX__
#include "headers.h"
#else
#include <headers.h>
#endif

// constructor for the event class both nt and aix. 

CcreateEvent::CcreateEvent(	int ManReset,
							int Signaled,
							char *name,
							ARGS_T *ArgDataPtr)
{

// save the event name
	strcpy(eventname,name);
	ArgDataP=ArgDataPtr; 

// don't create the event if its an open sync event and we are doing
// a wrap plug test. I only use the open sync events for port to port. 
	if((((strncmp(eventname,"EOpen",5)==0)||
		(strncmp(eventname,"Einit",5)==0)) && 
		(ArgDataPtr->TestParms.wrap_type==ePlug)) ||
		(strcmp(eventname,"INVALID_NAME")==0) ) return;



							// NOTE: 0 is signaled......
    op_wait[0].sem_num=0;		 // sem number is 0 , I only have one in set.
    op_wait[0].sem_op=0;         // set to 0 so it waits for semval to be 0
    op_wait[0].sem_flg=SEM_UNDO; // allow system to use adjustment value.   

    op_wait[1].sem_num=0;		 // sem number is 0 , I only have one in set.
    op_wait[1].sem_op=1;         // Allow increments semval after wait for 0. 
    op_wait[1].sem_flg=SEM_UNDO; // allow system to use adjustment value.   

    op_signal[0].sem_num=0;		 // sem number is 0 , I only have one in set.
    op_signal[0].sem_op=-1;      // decrements semval to 0 signaled.......
    op_signal[0].sem_flg=(IPC_NOWAIT|SEM_UNDO);
								// if already 0 return no wait.


	// pull the tty number off the end of the name.
	int num=0;
	for (int i=0; i<(int)strlen(name); i++) {
		if(isdigit(name[i])) {
			num=atoi(name+i);
			break;	
		} 
	}

	// set the base key for the semaphore
	if( (strncmp(name,"EOpenS",6)==0)  ) {
			init_event=0;
			key=0xeea10000|num;
	} else if( (strncmp(name,"EOpenC",6)==0)  ) {
			init_event=0;
			key=0xeea20000|num;
	} else if( (strncmp(name,"EstrtS",6)==0)  ) { 
			init_event=0;
			key=0xeea30000|num; 
	} else if( (strncmp(name,"EstrtC",6)==0)  ) { 
			init_event=0;
			key=0xeea40000|num;
	} else if( (strncmp(name,"EstopS",6)==0)  ) {
			init_event=0;
			key=0xeea50000|num;
	} else if( (strncmp(name,"EstopC",6)==0)  ) {
			init_event=0;
			key=0xeea60000|num;
	} else if( (strncmp(name,"EstpdS",6)==0)  ) {
			init_event=0;
			key=0xeea70000|num;
	} else if( (strncmp(name,"EstpdC",6)==0)  ) {
			init_event=0;
			key=0xeea80000|num;
	} else if( (strncmp(name,"EinitS",6)==0)  ) {
			init_event=1;
			key=0xeea90000|num;
	} else {
		Cmsg ToHtx(ArgDataPtr);
		sprintf(ToHtx.GetMsgAddr(),"Createevent: Unknown event name %s\n",name);
		ToHtx.SendMsg(1, HTX_HE_HARD_ERROR); 
		 ArgDataPtr->Flags |= ABORT;
		return;
	}
	
	
#ifdef BUGIT
	Cmsg ToHtx(ArgDataPtr);
	sprintf(ToHtx.GetMsgAddr(), "Event key %08X for  %s, number %d\n",key,eventname,num);
	ToHtx.SendMsg(0, HTX_HE_INFO); 
#endif


	// now create or open the semaphore..... If the create fails
	// I assume its already created so I open it, If that fails
	// then the stupid aix has screwed up again and I print error.
	
	if ( sem_create(!Signaled) ) {
		Cmsg ToHtx(ArgDataPtr);
		sprintf(ToHtx.GetMsgAddr(), "Sem_create failed!\n");
		ToHtx.SendMsg(0, HTX_HE_INFO); 
		ArgDataP->Flags |= ABORT;
	}
}

CcreateEvent::~CcreateEvent()
{

// don't remove the event if its an open sync event and we are doing
// a wrap plug test. I only use the open sync events for port to port.
    if( ((strncmp(eventname,"EOpen",5)==0) &&
        (ArgDataP->TestParms.wrap_type==ePlug)) ||
        (strcmp(eventname,"INVALID_NAME")==0) ) return;

	sem_remove();
}

DWORD CcreateEvent::WaitEvent(DWORD timeout)
{
	int semval=-1;
	int retry;
	// The only value of timeout that the aix code cares about is 0. 
	// any non 0 value and the code will wait forever for the 
	// event or sem to be signaled. 
	// Since alarm does not work so well with threads I will just let
	// if this process hang and use dbx to attach to find where the
	// hang is. 

	// The routine waits for sem to be 0 then resets it to 1. 
    // this returns WAIT_OBJECT_0 if event is signaled..
	// WAIT_TIMEOUT if times out waiting for event. 
	// If the timeout = 0 , I just check the state of the sem..
	// if its 0(signaled) I reset it and return WAIT_OBJECT_0
	// If its 1(not signaled I return WAIT_TIMEOUT.

	if(timeout) {		// handle case where timeout is not 0. 
		retry=0;
		while(1) {
		   	if(semop(id,&op_wait[0],2) < 0) {
				if(++retry > timeout) return((DWORD)WAIT_TIMEOUT);
				usleep(1000000);
			}
			else return((DWORD)WAIT_OBJECT_0);
		}

	} else {			// handle the 0 timeout case. 
#ifndef __HTX_LINUX__
		if((semval=semctl(id,0,GETVAL,0)) < 0) {
#else
		/*
		 * Linux will SEG FAULT if 4th argument is NULL 
		 */  	
		if((semval=semctl(id,0,GETVAL,semctl_arg)) < 0) {	
#endif
				Cmsg ToHtx(ArgDataP);
				sprintf(ToHtx.GetMsgAddr(),"wait_event: semcntl Getval for %s,"
						"errno=%s\n",eventname,strerror(errno));
				ToHtx.SendMsg(1,HTX_HE_HARD_ERROR);
				return(BADBOY);
		} else {
			// change this to !=1 instead of ==0, incase value gets
			// messed up somehow. 
			if(semval!=1) {
				// reset it to 1.
				semctl_arg.val=1;
				if(semctl(id,0,SETVAL,semctl_arg) < 0) {
						Cmsg ToHtx(ArgDataP);
						sprintf(ToHtx.GetMsgAddr(),"wait_evnet: semcntl "
						"SETVAL for %s,errno=%s\n",eventname,strerror(errno));
						ToHtx.SendMsg(1,HTX_HE_HARD_ERROR);
						return(BADBOY);
				}

				return((DWORD)WAIT_OBJECT_0);
			} else return((DWORD)WAIT_TIMEOUT);
		}

	}		// end of else 0 timeout case.. 
}


void CcreateEvent::SignalEvent()
{
	// this routine sets sem to 0......
	// instead of using semop lets use SETVAL......
	semctl_arg.val=0;
	if(semctl(id,0,SETVAL,semctl_arg) < 0) {
			Cmsg ToHtx(ArgDataP);
			sprintf(ToHtx.GetMsgAddr(),"signal_event: semcntl "
			"SETVAL for %s,errno=%s\n",eventname,strerror(errno));
			ToHtx.SendMsg(1,HTX_HE_INFO);
			return;
	}
}


void CcreateEvent::SetEvent()
{
	// set val to 1.............
	// instead of using semop lets use SETVAL......
	semctl_arg.val=1;
	if(semctl(id,0,SETVAL,semctl_arg) < 0) {
			Cmsg ToHtx(ArgDataP);
			sprintf(ToHtx.GetMsgAddr(),"set_event: semcntl "
			"SETVAL for %s,errno=%s\n",eventname,strerror(errno));
			ToHtx.SendMsg(1,HTX_HE_HARD_ERROR);
			return;
	}
}

int CcreateEvent::GetEvent()
{
	int semval;
	// get val of semaphore. ...
	// instead of using semop lets use SETVAL......
	if((semval=semctl(id,0,GETVAL,semctl_arg)) < 0) {
/*
			Cmsg ToHtx(ArgDataP);
			sprintf(ToHtx.GetMsgAddr(),"getevent: semcntl "
			"GETVAL for %s,errno=%s\n",eventname,strerror(errno));
			ToHtx.SendMsg(1,HTX_HE_HARD_ERROR);
*/
			return(BADBOY);
	}
	return(semval);
}

void CcreateEvent::display_value()
{
	int semval;
	Cmsg ToHtx(ArgDataP);
#ifndef __HTX_LINUX__
	if((semval=semctl(id,0,GETVAL,0)) < 0) {
#else
	if((semval=semctl(id,0,GETVAL,semctl_arg)) < 0) {
#endif
			sprintf(ToHtx.GetMsgAddr(),"wait_event: semcntl Getval for %s,"
					"errno=%s\n",eventname,strerror(errno));
			ToHtx.SendMsg(1,HTX_HE_HARD_ERROR);
			return;
	} else {
			sprintf(ToHtx.GetMsgAddr(),"event %s, key %08X, value is %d\n"
					,eventname,key,semval);
			ToHtx.SendMsg(0,HTX_HE_INFO);
	}
}


int CcreateEvent::sem_create(int initval)
{
	FILE * listFile;
	unsigned char cmFileName[64] = { "/tmp/hxeasy.ipcrm" };

	// This method of setting up the semaphores depends on the 
	// object being declared once in the main code and a pointer
	// to the object being passed to the threads that need to use
	// the semaphore. All these object pointers are in the argdata struct. 
	Cmsg ToHtx(ArgDataP);

	// Try to create the semaphore.... 
	if( (id=semget(key,1,IPC_CREAT | 0777)) < 0 ) {

		sprintf(ToHtx.GetMsgAddr(),"semget fail for %s,errno=%s\n",
					eventname,strerror(errno));
		ToHtx.SendMsg(0,HTX_HE_INFO);

		// This means its already created..
		// open it to get the id then see if it needs to be signaled. 
		if((id=semget(key,0,0)<0)) {
			sprintf(ToHtx.GetMsgAddr(),"open fail for %s,errno=%s\n",
				eventname,strerror(errno));
			ToHtx.SendMsg(0,HTX_HE_INFO);
			return(BADBOY);
		} 
	}
	//preserve this semget IDs here so that we can delete them later
	listFile = fopen((char*)cmFileName, "a");
	chmod((char*)cmFileName,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IROTH);
	if (listFile) {
        	fprintf(listFile, "ipcrm -s %d\n", id);
      		fclose(listFile);
	}
	
	// set the initial value to initval whether I created it or opened it. 
	// on port to port you get hangs if you let both client and server set
	// initial value, so just let the server do it. 	
	
	if(!(ArgDataP->Flags&CLIENT) && !init_event) {
		semctl_arg.val=initval;
		if(semctl(id,0,SETVAL,semctl_arg) < 0) {
				sprintf(ToHtx.GetMsgAddr(),"semcntl SETVAL for %s,errno=%s\n",eventname,strerror(errno));
				ToHtx.SendMsg(1,HTX_HE_HARD_ERROR);
				return(BADBOY);
		}
	}
	return(0);
}
void CcreateEvent::sem_remove()
{
#ifndef __HTX_LINUX__
	semctl(id,0,IPC_RMID,0);
#else
	semctl(id,0,IPC_RMID,semctl_arg);
#endif
}
