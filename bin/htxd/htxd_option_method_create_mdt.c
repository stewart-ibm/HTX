/* @(#)54	1.5  src/htx/usr/lpp/htx/bin/htxd/htxd_option_method_create_mdt.c, htxd, htxubuntu 11/9/15 10:26:37 */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "htxd_util.h"
#include "htxd_trace.h"


int htxd_option_method_create_mdt(char **result_string)
{
	int return_code;
	char trace_string[256];
	int stanza_count;
	FILE *fp;

	*result_string = malloc(256);
	if(*result_string == NULL) {
		sprintf(trace_string, "Error: malloc failed with errno <%d>", errno);	
		HTXD_TRACE(LOG_ON, trace_string);
		return -1;
	}

	return_code = htxd_execute_shell_profile();
	fp = popen("grep HE_name /usr/lpp/htx/mdt/mdt.bu 2>/dev/null | wc -l", "r");
	if(fp == NULL) {
		sprintf(trace_string, "Error: popen failed with errno <%d>", errno);
		strcpy(*result_string, trace_string);
		HTXD_TRACE(LOG_ON, trace_string);
		return -1;
	}

	fscanf(fp, "%d", &stanza_count);
	return_code = pclose(fp);
	if(return_code == -1 && errno != 10) {
		sprintf(trace_string, "Error: pclose failed with errno <%d>", errno);
		strcpy(*result_string, trace_string);
		HTXD_TRACE(LOG_ON, trace_string);
		return -1;
	}
	
	if(stanza_count > 2) {	
		sprintf(*result_string, "mdts are created successfully.");
		strcpy(trace_string, *result_string);
		HTXD_TRACE(LOG_ON, trace_string);
	} else {
		sprintf(*result_string, "Error: mdt creation is failed, please check error at log file <%s>.", "/tmp/htxd_bash_profile_output");
		strcpy(trace_string, *result_string);
		HTXD_TRACE(LOG_ON, trace_string);
		return -1;
	}

	return 0;
}
