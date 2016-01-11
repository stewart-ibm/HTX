/* @(#)87	1.2  src/htx/usr/lpp/htx/bin/hxeasy/Cmsg.h, exer_asy, htxubuntu 1/15/02 10:14:49  */
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
#ifndef _Cmsg_h
#define _Cmsg_h

class Cmsg {

// member data
private:
	struct htx_data *TempStatsPtr;
	char *msg_text;
	ARGS_T *ArgData;        // set in constructor.

// member functions

public:


	// constructor and destructor.
	Cmsg(ARGS_T *ArgDataPtr); 
	~Cmsg();

	// access functions
	void SendMsg(int err, enum sev_code sev);
	void Update(struct htx_data *stats);

	struct htx_data *GetTempStatsPtr();
	char *GetMsgAddr();

};
#endif
