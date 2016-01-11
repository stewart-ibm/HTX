/* @(#)84	1.3  src/htx/usr/lpp/htx/bin/hxeasy/Ccom.h, exer_asy, htxubuntu 1/15/02 10:14:37  */
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
#ifndef _Ccom_h
#define _Ccom_h

class Ccom	{		// This class should be used to create one instance
					// of the class. One object that has all the information
					// about the com port. It opens the port gets info about
					// its state, purges any data left in read or write 
					// buffers. CREATE THE ONE INSTANCE IN THE MAIN PROGRAM THREAD.
					// You don't want this copy to go away. The other friend class
					// Ccomm will need the info in this base class.
					// This destructor does not close the device but it
					// does provide a function to do so.

// member data
private:
	int 			async_des;
	struct termios  terms;
	char			PortNamePtr[32];        // comman to both nt and aix.
	ARGS_T			*ArgPtr;

// member functions

public:
	// constructor and destructor.
	Ccom(char *device,ARGS_T *ArgDataPtr);
	~Ccom();

	// access functions: Note the destructor does close
	// this port too, but the class is meant to stay in
	// scope and alive till the main program exits.      
	int Fcntl(int command);
	int SetHW(ARGS_T *ArgDataPtr,char flag);
	enum atype GetDevType();
	void set_vmin(int chars_to_read,ARGS_T *ArgDataPtr);
	int SetPortDefaults(ARGS_T *ArgDataPtr,char clocal_flag);
	int Ltcsetattr(int method, int id, ARGS_T *ArgDataPtr);
	int sethibaud(int rate,ARGS_T *ArgDataPtr);
	int dev_ioctl(int cmd,void *ptr,unsigned size,char *msg,ARGS_T *ArgDataPtr);
	int Set731p(ARGS_T *ArgDataPtr); 
	int Set128p(ARGS_T *ArgDataPtr,char flag);
#define SET_FOR_TEST	0x02
#define SET_FOR_CONNECT	0x01

	void char_size(ARGS_T *ArgDataPtr,int i);
	int baud_rate(ARGS_T *ArgDataPtr,int i);
	void stop_bits(ARGS_T *ArgDataPtr,int i);
	void set_parity(ARGS_T *ArgDataPtr,char i);
	int set_flow(ARGS_T *ArgDataPtr);
	int ready_port(ARGS_T *ArgDataPtr);

	int ConnectPorts(ARGS_T *ArgDataPtr); 	// opens port,
											// makes connection
	int OpenPort(DWORD Oflag, ARGS_T *ArgDataPtr);
	int ClosePort(ARGS_T *ArgDataPtr); 
	int FlushBuffers(); // flush input/output buffers
	void Clean_Read_Q(ARGS_T *ArgDataPtr);
	int Quick_Sync(ARGS_T *ArgDataPtr);
    DWORD ReadComm(DWORD BytesToRead, char *Buffer, int time_out,ARGS_T *ArgDataPtr);
    DWORD WriteComm(DWORD BytesToWrite, char * Buffer, ARGS_T *ArgDataPtr);
};
#endif
