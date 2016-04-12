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
#ifndef _Cevent_H
#define _Cevent_H

#ifdef __HTX_LINUX__
#include <sys/sem.h>
#endif

class CcreateEvent {

// member data
private:
	char	eventname[32];
    ARGS_T *ArgDataP;

	int     id;
    key_t   key;                    // key for this sempahpore event.
	// The wait must do two operations on my one event.
	// first it waits for sem to be 0.. Then it resets to one
	// for the next access of the sem_wait call.
	struct sembuf op_wait[2]; 

	// the signal call to semop only has to do the one operation.. 
	// it sets the value of the semaphore to 0.. or decrements it. 
	struct sembuf op_signal[1]; 

	// This struct is used by semctl to set and get sem value and
	// remove the semaphores... 
#ifndef __HTX_LINUX__
    union semun {
                int                 val;
                struct semid_ds     *buf;
                ushort              *array;
    } semctl_arg;
#else
    	/*
	 * Linux defines it in sys/sem.h
	 */ 
	union semun semctl_arg;
#endif
	int init_event;

// member functions

public:
	// constructor and destructor. NOTE: passing ptr to function. 
	// LPDWORD is defined in windef.h its ulong *  32 bits.
	CcreateEvent(   int ManReset,   // TRUE if manual, FALSE auto reset
					int Signaled,   // TRUE for initial signaled.
									 // signaled means the wait is
					 				 // satisfied.
					char *name,
					ARGS_T *ArgDataPtr);

	~CcreateEvent();

	DWORD WaitEvent(DWORD timeout);
	void SignalEvent();      // in aix sets sem to 0. in nt signals event. 

    int sem_create(int initval);
    void sem_remove();
	void display_value();
	void SetEvent();         // sets sem to 1. 
	int  GetEvent();         // Gets the value of the semaphore. 

};

#endif
