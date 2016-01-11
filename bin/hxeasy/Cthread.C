// @(#)80	1.2  src/htx/usr/lpp/htx/bin/hxeasy/Cthread.C, exer_asy, htxubuntu 1/15/02 10:14:21
//
//   COMPONENT_NAME: exer_asy
//
//   FUNCTIONS: none
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

