// @(#)74	1.2  src/htx/usr/lpp/htx/bin/hxeasy/Cargdata.C, exer_asy, htxubuntu 1/15/02 10:13:53
//
//   COMPONENT_NAME: exer_asy
//
//   FUNCTIONS: CArgData
//		GetArgDataPtr
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

CArgData::CArgData()
{
		dataptr = new ARGS_T;
		assert(dataptr);
}

CArgData::~CArgData()
{
		delete dataptr;
}

ARGS_T *CArgData::GetArgDataPtr()
{
	return(dataptr);
}
