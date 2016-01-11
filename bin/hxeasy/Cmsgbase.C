// @(#)79	1.2  src/htx/usr/lpp/htx/bin/hxeasy/Cmsgbase.C, exer_asy, htxubuntu 1/15/02 10:14:17
//
//   COMPONENT_NAME: exer_asy
//
//   FUNCTIONS: CmsgBase
//		GetStatsPtr
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

CmsgBase::CmsgBase()
{
    StatsBase = new struct htx_data;
    // if theres no memory left the user must do
    // something to cut usage or add more mem. SO
    // I will just assert if out out memory.
    assert(StatsBase);
	
	// now 0 the memory allocated.
	memset(StatsBase,0x00,sizeof(struct htx_data));

}

CmsgBase::~CmsgBase()
{
    delete StatsBase;
}
struct htx_data *CmsgBase::GetStatsPtr()
{
    return StatsBase;
}
