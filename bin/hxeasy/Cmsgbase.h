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

// this is the header file for the base msg class. It handles communication
// with htx. 
#ifndef _Cmsgbase_h
#define _Cmsgbase_h


class CmsgBase {	 // This class should be used to create one instance
					// of the class. One object that has the stats struct
					// to be used for all threads. The object created will
					// be passed to all threads and functions that need it
					// and the friend class Cmsg will be used to create an
					// object that will be a copy of the base struct.

friend class Cmsg;

	// member data
private:
	struct htx_data *StatsBase;

	// member functions
public:
	// constructor and destructor.
	CmsgBase();
	~CmsgBase();

	// Note this function is public but it should only be used for the
	// initialization of the htx_data struct. Do not use this for sending
	// htx messages use the Cmsg class...!!!!

	struct htx_data *GetStatsPtr();

};
#endif
