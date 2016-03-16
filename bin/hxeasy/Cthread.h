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

#ifndef _Cthread_h
#define _Cthread_h

class CThread {

// member data
private:
	void (*fptr)(LPDWORD);              
	pthread_attr_t  attr;
	pthread_t thread;

// member functions

public:
	// constructor and destructor. NOTE: passing ptr to function. 
	// LPDWORD is defined as.... ulong *  32 bits.
	CThread(void (*f_ptr)(LPDWORD)) : fptr(f_ptr) { } 
	void kill_thread();
	 
	~CThread();
	int	Start(LPDWORD arg);
};
#endif
