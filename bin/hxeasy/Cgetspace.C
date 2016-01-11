// @(#)77	1.2  src/htx/usr/lpp/htx/bin/hxeasy/Cgetspace.C, exer_asy, htxubuntu 1/15/02 10:14:08
//
//   COMPONENT_NAME: exer_asy
//
//   FUNCTIONS: CGetSpace
//		GetPbuf
//		GetRbuf
//		GetWbuf
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

CGetSpace::CGetSpace(int size)
{
		pbuf = new char[ size ];
		assert(pbuf);
		wbuf = new char[ size ];
		assert(wbuf);
		rbuf = new char[ size ];
		assert(rbuf);
}

CGetSpace::~CGetSpace()
{
		delete pbuf;
		delete wbuf;
		delete rbuf;
}
char *CGetSpace::GetPbuf()
{
	return(pbuf);
}

char *CGetSpace::GetWbuf()
{
	return(wbuf);
}

char *CGetSpace::GetRbuf()
{
	return(rbuf);
}
