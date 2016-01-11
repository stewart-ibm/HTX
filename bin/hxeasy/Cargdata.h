/* @(#)83	1.2  src/htx/usr/lpp/htx/bin/hxeasy/Cargdata.h, exer_asy, htxubuntu 1/15/02 10:14:33  */
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
#ifndef _Cargdata_h
#define _Cargdata_h

class CArgData {
// member data
private:
	ARGS_T *dataptr;

// member functions

public:
	// constructor and destructor.
	CArgData();
	~CArgData();

	// Note this function is public but it should only be used for the
	// initialization of the htx_data struct. Do not use this for sending
	// htx messages use the Cmsg class...!!!!
	ARGS_T *GetArgDataPtr();


};
#endif
