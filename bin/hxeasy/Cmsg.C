// @(#)78   1.5  src/htx/usr/lpp/htx/bin/hxeasy/Cmsg.C, exer_asy, htxubuntu 6/13/02 23:35:30
//
//   COMPONENT_NAME: exer_asy
//
//   FUNCTIONS: Cmsg
//		GetMsgAddr
//		GetTempStatsPtr
//		SendMsg
//		Update
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
#include "headers.h"
#else
#include <headers.h>
#endif

#define MISC 0xBEEFDEAD

#ifndef __HTX_LINUX__
#pragma mc_func trap { "7c810808" }
#pragma reg_killed_by trap
#endif


Cmsg::Cmsg(ARGS_T *ArgDataPtr)
{
    ArgData=ArgDataPtr;
    TempStatsPtr=new struct htx_data;
    assert(TempStatsPtr);
    msg_text=new char[MAX_TEXT_MSG];
    assert(msg_text);
    // now copy the base stats struct to the temp
    memcpy(TempStatsPtr,ArgDataPtr->BaseMsgObjectPtr->StatsBase,
        sizeof(struct htx_data));
}
Cmsg::~Cmsg()
{
    delete TempStatsPtr;
    delete msg_text;
}

void Cmsg::Update(struct htx_data *stats) 
{
	 hxfupdate(UPDATE,stats);
} 

struct htx_data *Cmsg::GetTempStatsPtr() 
{ 
	return TempStatsPtr; 
}

char *Cmsg::GetMsgAddr() 
{ 
	return msg_text; 
}
void Cmsg::SendMsg(int err, enum sev_code sev) 
{                                             
	if(ArgData->Flags&ABORT) return;

	if(ArgData->Rules.crash==2) { 
		if(sev==HTX_SYS_HARD_ERROR ||
			sev==HTX_HE_HARD_ERROR ||
			 sev==HTX_HE_SOFT_ERROR) {
			/*
			 * libmisc in linux will provide do_trap_htx for this purpose
			 * once its ready, it will be implemented..
			 */
#ifndef __HTX_LINUX__
			trap((int) MISC,(int) 2,(int) 2,(int) ArgData->my_ttynum);
#endif
		}
	}
	if(sev!= HTX_HE_INFO) {

		// add pass info to msg_text
		char add_info[512];
		sprintf(add_info,"\nbase num_oper=%d,"
				" ixon=%c, ixoff=%c,"
				" rts=%d, dtr=%d"
				" num_chars=%d,"
				" bufsize=%d,\n"
				"direction=%d,"
				" baudrate=%d,"
				" chsize=%d,"
				" cstop=%d,"
				" parity=%c"
				" wrap_type=%d,"
				" Flags=%02X,\n"
				"rule_id=%s,"
				" pass_count=%d",
				ArgData->Rules.num_oper,
				ArgData->Rules.ixon,
				ArgData->Rules.ixoff,
				ArgData->Rules.rts,
				ArgData->Rules.dtr,
				ArgData->Rules.num_chars,
				ArgData->Rules.bufsize,
				ArgData->Rules.default_dir,
				ArgData->TestParms.BaudRate,
				ArgData->TestParms.ChSize,
				ArgData->TestParms.CStopB,
				ArgData->TestParms.Parity,
				ArgData->TestParms.wrap_type,
				ArgData->Flags,
				ArgData->Rules.rule_id,
				(int)ArgData->pass_count);
		strcat(msg_text,add_info);
	}

	 hxfmsg(TempStatsPtr,err,sev,msg_text );
	return;
} 
