/* @(#)88	1.2  src/htx/usr/lpp/htx/bin/hxeasy/Cmsgbase.h, exer_asy, htxubuntu 1/15/02 10:14:54  */
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
