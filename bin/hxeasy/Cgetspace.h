/* @(#)86	1.2  src/htx/usr/lpp/htx/bin/hxeasy/Cgetspace.h, exer_asy, htxubuntu 1/15/02 10:14:45  */
/*
 *   COMPONENT_NAME: exer_asy
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *   OBJECT CODE ONLY SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 88,93
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _Cgetspace_h
#define _Cgetspace_h

class CGetSpace {
private:
	char *pbuf;
	char *wbuf;
	char *rbuf;

public:

	CGetSpace(int size);
	~CGetSpace();
	char *GetPbuf();
	char *GetWbuf();
	char *GetRbuf();
};
#endif
