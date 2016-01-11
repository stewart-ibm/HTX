/* @(#)89	1.2  src/htx/usr/lpp/htx/bin/hxeasy/Cthread.h, exer_asy, htxubuntu 1/15/02 10:14:58  */
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
#ifndef _Cthread_h
#define _Cthread_h

class CThread {

// member data
private:
	void (*fptr)(LPDWORD);              
	pthread_attr_t  attr;
	pthread_t thread;

// member functions

public:
	// constructor and destructor. NOTE: passing ptr to function. 
	// LPDWORD is defined as.... ulong *  32 bits.
	CThread(void (*f_ptr)(LPDWORD)) : fptr(f_ptr) { } 
	void kill_thread();
	 
	~CThread();
	int	Start(LPDWORD arg);
};
#endif
