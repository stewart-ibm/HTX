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

#include"headers.h"

CThread::~CThread()
{
	return;
}

int CThread::Start(LPDWORD arg)
{
        int rc;
        ARGS_T *ArgDataPtr = (ARGS_T *)arg;
        if(!(rc=pthread_attr_init(&attr))) {
            pthread_attr_setdetachstate(
                &attr,
                PTHREAD_CREATE_DETACHED);

            rc=pthread_create(
                &thread,
                &attr,
                 (void *(*)(void *))fptr,
                arg);
            if(rc) {
        		Cmsg ToHtx(ArgDataPtr);
                sprintf(ToHtx.GetMsgAddr(),"Thread failed to start\n");
                ToHtx.SendMsg(1,HTX_HE_HARD_ERROR);
                return(BADBOY);
            }
            pthread_attr_destroy(&attr);
        } else {
			Cmsg ToHtx(ArgDataPtr);
            sprintf(ToHtx.GetMsgAddr(),"pthread_attr_init failed errno %d\n",rc);
            ToHtx.SendMsg(1,HTX_HE_HARD_ERROR);
            return(BADBOY);
        }

        return(0);
}				// end CThread::Start(LPDWORD arg) routine. 

