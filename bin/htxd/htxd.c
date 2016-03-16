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
/* @(#)34	1.5  src/htx/usr/lpp/htx/bin/htxd/htxd.c, htxd, htxubuntu 8/4/15 03:37:17 */



#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>


#include "htxd.h"
#include "htxd_instance.h"
#include "htxd_trace.h"


htxd *htxd_global_instance = NULL;
volatile int htxd_shutdown_flag = FALSE;
volatile int htxd_ecg_shutdown_flag = FALSE;

int htxd_trace_level = 2;
FILE *htxd_trace_fp;
FILE *htxd_log_fp;



int htxd_validate_arguments(int argc, char *argv[], htxd *p_htxd_instance)
{
	int arg_option;
	int ret = 0;;


	while ((arg_option = getopt(argc, argv, "ast:")) != -1) 
	{
    	switch(arg_option)
		{
		case 's':
			p_htxd_instance->run_level = HTXD_SHUTDOWN;
			break;
		case 't':
			p_htxd_instance->trace_level = atoi(optarg);
			if(p_htxd_instance->trace_level < HTXD_TRACE_NO || p_htxd_instance->trace_level > HTXD_TRACE_HIGH)
			{
				return -1;
			}
			break;
		case 'p':
			p_htxd_instance->port_number = atoi(optarg);
		default :
			ret = -1;
		}
	}

	return ret;
}


int main(int argc, char *argv[])
{
	int ret = 0;
	char trace_str[512];


	system("date +\"HTX Daemon (htxd) was started on %x at %X %Z\">>/tmp/htxd.start.stop.time");
	htxd_trace_fp = fopen("/tmp/htxd_trace", "w");
	htxd_log_fp = fopen("/tmp/htxd_log", "w");

	HTXD_FUNCTION_TRACE(FUN_ENTRY, "main");

	htxd_global_instance = htxd_create_instance();
	init_htxd_instance(htxd_global_instance); 

	HTXD_TRACE(LOG_ON, "validate arguments to daemon launcher");
	ret = htxd_validate_arguments(argc, argv, htxd_global_instance);
	if (ret == -1)
	{
		HTXD_TRACE(LOG_ON, "htxd_validate_arguments() failed, exiting");
		exit(1);
	}

	if(htxd_global_instance->run_level == HTXD_SHUTDOWN)
	{
/* shutdown all currently running ECGc */
		exit(0);
	}

#if !defined(__HTX_LINUX__) && !defined(__OS400__)
	system("errlogger --- HTXD Started ---");
#endif

	HTXD_TRACE(LOG_ON, "forking daemon process");	
	htxd_global_instance->daemon_pid = fork();
	switch (htxd_global_instance->daemon_pid)
	{
	case -1:
		sprintf (trace_str, "ERROR: fork failed with errno = %d", errno);
		HTXD_TRACE(LOG_ON, trace_str);
		exit (-1);
	case 0:
		setpgrp();
#ifndef __HTX_LINUX__
		setpriority(PRIO_PROCESS, 0, -1);
#endif
		htxd_set_program_name(argv[0]);
		htxd_set_htx_path("/usr/lpp/htx");
		HTXD_TRACE(LOG_ON, "starting daemon function");
		htxd_start_daemon(htxd_global_instance);
		break;

	default:
		HTXD_TRACE(LOG_ON, "daemon launcher process is about to exit");
		return 0;
	}

	system("date +\"HTX Daemon (htxd) was stopped on %x at %X %Z\">>/tmp/htxd.start.stop.time");
	HTXD_FUNCTION_TRACE(FUN_EXIT, "main");
	return 0;
}




