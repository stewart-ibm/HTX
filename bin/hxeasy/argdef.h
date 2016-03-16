/* IBM_PROLOG_BEGIN_TAG */
/* 
 * Copyright 2003,2016 IBM International Business Machines Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 		 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* IBM_PROLOG_END_TAG */

#ifndef USECLASS_H  
#define USECLASS_H

typedef struct infodata {
	char	startmark[8];   
	char	HostName[16];
    char	Pid[16];
	char	DeviceName[16];
	int		writer;
} INFO_T;

typedef struct parms {
	// This section has test setup values for the next read write test.
	int 	BaudRate;	
	int 	ChSize;	
	int 	CStopB;
	char	Parity;
	char 	*WriteDataPtr;	// data to be written during test. 
	char 	*ReadDataPtr;	// where data is read to. 
	enum wtype	wrap_type;
} PARMS_T;


typedef struct ArgData {
	BYTE	Flags;
#define					WRITE_ONLY		0x01
#define					CLIENT			0x02
#define					SERVER			0x04
#define					HALF_DUPLEX		0x08
#define					ABORT	   		0x10
#define					STOP_WRITE 		0x20
#define					MISCOMPARE 		0x40
#define					READ_DONE 		0x80

	DWORD   pass_count;
	int		my_ttynum;
	int		his_ttynum;
	int		sig_cnt;

	char *PatternDataPtr; 	// This has the data from pattern file
								// thread uses this too.

	char 		PatternPath[FNAME_MAX];  // has path name for pattern.
	char 		RulesPath[FNAME_MAX];  // has path name for rules file.
	char            lastpattern_id[32];
	enum atype	device_type;

	RULE_T 		Rules;
	void		*global_data;
	CmsgBase 	*BaseMsgObjectPtr;
    void        *MainPtr;       // This will be type cast later as Cmsg
	void		*Port;			// This will be type cast later as class Ccom
    void	   *Space_Class;
    void    *OpenEventS;        // this will be type cast later to CcreateEvent
    void    *OpenEventC;		// this too will go to CcreateEvent type. 
    void    *WstartReader;		// this too will go to CcreateEvent type. 
    void    *WstartWriter;		// this too will go to CcreateEvent type. 
    void    *WstopReader;		// this too will go to CcreateEvent type. 
    void    *WstopWriter;		// this too will go to CcreateEvent type. 
    void    *WstopedReader;		// this too will go to CcreateEvent type. 
    void    *WstopedWriter;		// this too will go to CcreateEvent type. 
	void	 *Write_Class;
	INFO_T		MyInfo;
	INFO_T		HisInfo;
	PARMS_T		TestParms;
	int			BaudValid;

    //      The variables that hold these names follow. They are initialized
    // in the ConnectPorts routine in classlib.cpp
    char WstartS[16];           // reads tells write to go.
    char WstartC[16];           // read tells write to go.
    char WstopS[16];           // read tells write to stop.
    char WstopC[16];           // read tells write to stop. 
    char WstopedS[16];           // write tells read its done.
    char WstopedC[16];           // write tells read its done.
    char OpenS[16];      // syncs opens on port to port.
    char OpenC[16];     // syncs opens on port to port
    char InitS[16];     // syncs first open on port to port before connect data

} ARGS_T;

#endif 






