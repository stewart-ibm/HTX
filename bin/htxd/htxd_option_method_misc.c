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
/* @(#)56	1.6  src/htx/usr/lpp/htx/bin/htxd/htxd_option_method_misc.c, htxd, htxubuntu 9/15/15 20:28:32 */



#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "htxd.h"
#include "htxd_instance.h"
#include "htxd_ecg.h"
#include "htxd_ipc.h"
#include "htxd_util.h"
#include "htxd_option_methods.h"
#include "htxd_define.h"
#include "htxd_signal.h"
#include "htxd_trace.h"
#include "htxd_define.h"



extern int htxd_run_HE_script(char *, char *, int *);
extern int htxd_get_ecg_list_length(htxd_ecg_manager *);
extern int htxd_load_exerciser(struct htxshm_HE *);
extern int htxd_get_common_command_result(int, htxd_ecg_info *, char *, char *);


int htxd_expand_device_name_list(htxd_ecg_info *p_ecg_info_list, char *device_name_list)
{
	char temp_device_name_list[MAX_OPTION_LIST_LENGTH];
	char *p_device_name;
	int search_length;
	int i;
	struct htxshm_HE *p_HE;


	temp_device_name_list[0] = '\0';
	p_device_name = strtok(device_name_list, " ");
	
	while(p_device_name != NULL) {
		search_length = strlen(p_device_name) - 1;
		if(p_device_name[search_length] == '*') {
			p_HE = (struct htxshm_HE *)(p_ecg_info_list->ecg_shm_addr.hdr_addr + 1);
			for(i = 0; i < p_ecg_info_list->ecg_shm_exerciser_entries ; i++) {
				if(strncmp(p_HE->sdev_id, p_device_name, search_length) == 0) {
					strcat(temp_device_name_list, p_HE->sdev_id);
					strcat(temp_device_name_list, " ");
				}
				p_HE++;
			}
		} else {
			strcat(temp_device_name_list, p_device_name);
			strcat(temp_device_name_list, " ");
		}
		p_device_name = strtok(NULL, " ");
	}	
	strcpy(device_name_list, temp_device_name_list);

	return 0;
}

int htxd_is_list_regular_expression(char *device_name_list)
{
	char *temp_prt = NULL;

	temp_prt = strchr(device_name_list, '*');
	if(temp_prt != NULL) {
		return TRUE;
	} else {
		return FALSE;
	}
}

int htxd_option_method_getactecg(char **result)
{
	htxd *htxd_instance;
	int active_ecg_count;


	htxd_instance = htxd_get_instance();

	*result = malloc(EXTRA_BUFFER_LENGTH);
	if(*result == NULL) {
		return 1;	
	}
	
	if( htxd_is_daemon_idle() == TRUE) {
		strcpy(*result, "No ECG/MDT is currently running, daemon is idle");
		return 0;
		
	}

	if(htxd_instance->p_ecg_manager->ecg_info_list == NULL) {
		strcpy(*result, "No ECG/MDT is currently running");
		return 0;
	}

	*result[0] = '\0';
		
	active_ecg_count = htxd_get_ecg_list_length(htxd_instance->p_ecg_manager);
	if(active_ecg_count > 0) {
		htxd_get_active_ecg_name_list(htxd_instance->p_ecg_manager, *result);
	}

	return 0;
}



int htxd_get_device_run_status(struct htxshm_HE *p_HE, htxd_ecg_info *p_ecg_info, int device_position, char *status_result)
{

	time_t epoch_time_now;
	int device_sem_status;
	int device_sem_id;


	if(status_result == NULL) {
		return -1;
	}

	device_sem_id = p_ecg_info->ecg_sem_id;
	epoch_time_now =  time( (time_t *) 0);
	
	if( (p_ecg_info->ecg_shm_addr.hdr_addr->started == 0)&& (p_HE->is_child) ) {
			strcpy(status_result, "  ");
	} else if(p_HE->PID == 0) {
		if(p_HE->DR_term == 1) {
			strcpy(status_result, "DT");
		} else if(p_HE->user_term == 0) {
			strcpy(status_result, "DD");
		} else {
			strcpy(status_result, "TM");
		}
	} else if(p_HE->no_of_errs > 0) {
		device_sem_status = htxd_get_device_error_sem_status(device_sem_id, device_position);
		if(device_sem_status == -1) {
			return -1;	
		} else if(device_sem_status == 0) {
			strcpy(status_result, "PR");
		} else {
			strcpy(status_result, "ER");
		}	
	} else if( (p_HE->equaliser_halt == 1) || ( (device_sem_status = htxd_get_device_run_sem_status(device_sem_id, device_position) ) != 0) ) {
		strcpy(status_result, "ST");
	} else if( (p_HE->max_cycles != 0) && (p_HE->cycles >= p_HE->max_cycles) ) {
		strcpy(status_result, "CP");
	} else if( (epoch_time_now - p_HE->tm_last_upd) > ( (long)(p_HE->max_run_tm + p_HE->idle_time) ) ) {  ////?????
		strcpy(status_result, "HG");
	} else if( (p_HE->max_cycles != 0 ) && (p_HE->cycles >= p_HE->max_cycles) ) {
		strcpy(status_result, "PR");
	} else {
		strcpy(status_result, "RN");	
	}

	return 0;
}


int htxd_get_device_coe_soe_status(struct htxshm_HE *p_HE, char *status_result)
{

	if(p_HE->cont_on_err == HTX_SOE) {
		strcpy(status_result, "SOE");	
	} else {
		strcpy(status_result, "COE");	
	}

	return 0;
}


int htxd_get_device_last_update_day_time(void)
{

	return 0;
}



int htxd_get_device_active_suspend_status(int device_sem_id, int device_position, char *result_status)
{

	int sem_status;

	sem_status = htxd_get_device_run_sem_status(device_sem_id, device_position);
	if(sem_status == 0) {
		strcpy(result_status, "ACTIVE");
	} else {
		strcpy(result_status, "SUSPEND");
	}

	return 0;
}



int htxd_get_query_row_entry(struct htxshm_HE *p_HE, htxd_ecg_info * p_ecg_info_to_query, int device_position, char *p_query_row_entry)
{
	char device_run_status[5];
	char device_coe_soe_status[5];
	char device_active_suspend_status[15];
	char last_update_day_of_year[10];
	char last_update_time[80];
	char last_error_day_of_year[10];
	char last_error_time[80];

	htxd_get_device_run_status(p_HE, p_ecg_info_to_query, device_position, device_run_status);
	htxd_get_device_active_suspend_status(p_ecg_info_to_query->ecg_sem_id, device_position, device_active_suspend_status);
	htxd_get_device_coe_soe_status(p_HE, device_coe_soe_status);
	htxd_get_time_details(p_HE->tm_last_upd, NULL, last_update_day_of_year, last_update_time);
	if(p_HE->tm_last_err != 0) {
		htxd_get_time_details(p_HE->tm_last_err, NULL, last_error_day_of_year, last_error_time);
	} else {
		strcpy(last_error_day_of_year, "NA");
		strcpy(last_error_time, "NA");
	}

	//sprintf (p_query_row_entry, "%c%-8s%c%3s%c%7s%c%3s%c%s%c%s%c%-8d%c%-3u%c%-4s%c%-8s%c%s%c%d", 
	//sprintf (p_query_row_entry, "%c%-8s%c%3s%c%7s%c%3s%c%s%c%s%c%-8llu%c%-3u%c%-4s%c%-8s%c%s%c%d", 
	sprintf (p_query_row_entry, "%c%-8s%c%3s%c%7s%c%3s%c%s%c%s%c%-8llu%c%-3llu%c%-4s%c%-8s%c%s%c%llu", 
		'\n',
		p_HE->sdev_id, ' ',
		device_run_status, ' ',
		device_active_suspend_status, ' ',
		device_coe_soe_status, ' ',
		last_update_day_of_year, ' ',
		last_update_time, ' ',
		(unsigned long long)p_HE->cycles, ' ',
		(unsigned long long)p_HE->test_id, ' ',
		last_error_day_of_year, ' ',
		last_error_time, ' ',
		p_HE->slot_port, ' ',
		(unsigned long long)p_HE->no_of_errs);


	return 0;
}


int htxd_query_device(htxd_ecg_info *p_ecg_info_to_query, char *p_command_option_list, char *command_result)
{
	struct htxshm_HE *p_HE;
	int return_code = 0;
	int i;
	char *p_device_name = NULL;
	char query_row_entry[256];

	if(htxd_is_list_regular_expression(p_command_option_list) == TRUE) {
		htxd_expand_device_name_list(p_ecg_info_to_query, p_command_option_list);
	}

	p_device_name = strtok(p_command_option_list, " ");
	while(p_device_name != NULL) {

		p_HE = (struct htxshm_HE *)(p_ecg_info_to_query->ecg_shm_addr.hdr_addr + 1);
		for(i = 0; i < p_ecg_info_to_query->ecg_shm_exerciser_entries ; i++) {
			if(strcmp(p_HE->sdev_id, p_device_name) == 0) {
				return_code = htxd_get_query_row_entry(p_HE, p_ecg_info_to_query, i, query_row_entry);
				strcat(command_result, query_row_entry);
				break;
			}
			p_HE++;
		}
		p_device_name = strtok(NULL, " ");
	}


	return return_code;
}

int htxd_query_all_device(htxd_ecg_info * p_ecg_info_to_query, char *command_result)
{
	struct htxshm_HE *p_HE;
	int i;
	char query_row_entry[256];
	int return_code = 0;


	p_HE = (struct htxshm_HE *)(p_ecg_info_to_query->ecg_shm_addr.hdr_addr + 1);

	for(i = 0; i < p_ecg_info_to_query->ecg_shm_exerciser_entries ; i++) {
		return_code = htxd_get_query_row_entry(p_HE, p_ecg_info_to_query, i, query_row_entry);
		strcat(command_result, query_row_entry);

		p_HE++;
	}
	
	return return_code;
}


int htxd_option_method_query(char **command_result)
{

	htxd_ecg_info * p_ecg_info_list;
	htxd_ecg_info * p_ecg_info_to_query = NULL;
	htxd_option_method_object query_method;
	int device_entries_present;


	htxd_init_option_method(&query_method);

	query_method.return_code = htxd_validate_command_requirements(query_method.htxd_instance, query_method.error_string);
	if(query_method.return_code != 0) {
		*command_result = malloc(EXTRA_BUFFER_LENGTH);
		strcpy(*command_result, query_method.error_string);
		return query_method.return_code;
	}

	device_entries_present = htxd_get_total_device_count();
	*command_result = malloc(EXTRA_BUFFER_LENGTH + device_entries_present * LINE_ENTRY_LENGTH);
	if(*command_result == NULL) {
		return 1;
	}

	strcpy(*command_result, "--------------------------------------------------------------------------------\n");
	strcat(*command_result, " Device   ST  ACTIVE COE Last Update  Count Stanza Last Error   Slot/  Num_errs\n");
	strcat(*command_result, "             SUSPEND SOE day Time                  Day  Time    Port\n");
	strcat(*command_result, "--------------------------------------------------------------------------------");

	p_ecg_info_list = htxd_get_ecg_info_list(query_method.htxd_instance->p_ecg_manager);
	if(query_method.command_ecg_name[0] != '\0') {
		while(p_ecg_info_list != NULL) {
			if( strcmp(query_method.command_ecg_name, p_ecg_info_list->ecg_name) == 0) {
				p_ecg_info_to_query = p_ecg_info_list;
				break;
			}
			 p_ecg_info_list = p_ecg_info_list->ecg_info_next;
		}

		if(p_ecg_info_to_query == NULL) {
			sprintf(*command_result, "Specified ECG/MDT<%s> is not currently running", query_method.command_ecg_name);
			return 0;
		}

		if(strlen(query_method.command_option_list) > 0) {
			query_method.return_code = htxd_query_device(p_ecg_info_to_query, query_method.command_option_list, *command_result);
		} else {
			query_method.return_code = htxd_query_all_device(p_ecg_info_to_query, *command_result);
		}
	} else {
		if(strlen(query_method.command_option_list) > 0) {
			query_method.return_code = htxd_process_all_active_ecg_device(htxd_query_device, query_method.command_option_list, *command_result);
		} else {
			query_method.return_code = htxd_process_all_active_ecg(htxd_query_all_device, *command_result);
		}
	}

	return query_method.return_code;
}


int htxd_get_status_row_entry(struct htxshm_HE *p_HE, htxd_ecg_info * p_ecg_info_to_status, int device_position, char *p_status_row_entry)
{
	int return_code = 0;
	char device_run_status[5];
	char last_update_day_of_year[10];
	char last_update_time[80];
	char last_error_day_of_year[10];
	char last_error_time[80];


	htxd_get_device_run_status(p_HE, p_ecg_info_to_status, device_position, device_run_status);
	htxd_get_time_details(p_HE->tm_last_upd, NULL, last_update_day_of_year, last_update_time);
	if(p_HE->tm_last_err != 0) {
		htxd_get_time_details(p_HE->tm_last_err, NULL, last_error_day_of_year, last_error_time);
	} else {
		strcpy(last_error_day_of_year, "NA");
		strcpy(last_error_time, "NA");
	}

/*	sprintf (p_status_row_entry, "%c%-3s%c%-9s%c%-5s%c%-9s%c%-6u%c%-7u%c%-5s%c%s", 
		'\n',
		device_run_status, ' ',
		p_HE->sdev_id, ' ',
		last_update_day_of_year, ' ',
		last_update_time, ' ',
		p_HE->cycles, ' ',
		p_HE->test_id, ' ',
		last_error_day_of_year, ' ',
		last_error_time); */
	sprintf (p_status_row_entry, "%c%-3s%c%-9s%c%-5s%c%-9s%c%-6d%c%-7d%c%-5s%c%s", 
		'\n',
		device_run_status, ' ',
		p_HE->sdev_id, ' ',
		last_update_day_of_year, ' ',
		last_update_time, ' ',
		(int)p_HE->cycles, ' ',
		(int)p_HE->test_id, ' ',
		last_error_day_of_year, ' ',
		last_error_time); 
/*	sprintf (p_status_row_entry, "%c%-3s%c%-9s%c%-5s%c%-9s%c%-5s%c%s", 
		'\n',
		device_run_status, ' ',
		p_HE->sdev_id, ' ',
		last_update_day_of_year, ' ',
		last_update_time, ' ',
		last_error_day_of_year, ' ',
		last_error_time); 
*/
	return return_code;

}

int htxd_status_device(htxd_ecg_info * p_ecg_info_to_status, char *p_command_option_list,  char *command_result)
{
	struct htxshm_HE *p_HE;
	int return_code = 0;
	int i;
	char *p_device_name = NULL;
	char status_row_entry[512];


	p_device_name = strtok(p_command_option_list, " ");
	while(p_device_name != NULL) {

		p_HE = (struct htxshm_HE *)(p_ecg_info_to_status->ecg_shm_addr.hdr_addr + 1);
		for(i = 0; i < p_ecg_info_to_status->ecg_shm_exerciser_entries ; i++) {
			if(strcmp(p_HE->sdev_id, p_device_name) == 0) {
				htxd_get_status_row_entry(p_HE, p_ecg_info_to_status, i, status_row_entry);
				strcat(command_result, status_row_entry);
				break;
			}
			p_HE++;
		}
		p_device_name = strtok(NULL, " ");
	}

	return return_code;
}


int htxd_status_all_device(htxd_ecg_info * p_ecg_info_to_status, char *command_result)
{
	struct htxshm_HE *p_HE;
	int i;
	char status_row_entry[256];


	p_HE = (struct htxshm_HE *)(p_ecg_info_to_status->ecg_shm_addr.hdr_addr + 1);

	for(i = 0; i < p_ecg_info_to_status->ecg_shm_exerciser_entries ; i++) {
		htxd_get_status_row_entry(p_HE, p_ecg_info_to_status, i, status_row_entry);
		strcat(command_result, status_row_entry);

		p_HE++;
	}
	
	return 0;
}


int htxd_option_method_status(char **command_result)
{

	htxd_ecg_info * p_ecg_info_list;
	htxd_option_method_object status_method;
	int device_entries_present;

	htxd_init_option_method(&status_method);

	status_method.return_code = htxd_validate_command_requirements(status_method.htxd_instance, status_method.error_string);
	if(status_method.return_code != 0) {
		*command_result = malloc(EXTRA_BUFFER_LENGTH);
		strcpy(*command_result, status_method.error_string);
		return status_method.return_code;
	}

	device_entries_present = htxd_get_total_device_count();	
	*command_result = malloc(EXTRA_BUFFER_LENGTH + device_entries_present * LINE_ENTRY_LENGTH);
	if(*command_result == NULL) {
		return 1; 
	}


	strcpy(*command_result, "---------------------------------------------------------\n");
	strcat(*command_result, "              Last  Update    Cycle  Curr    Last  Error\n");
	strcat(*command_result, "ST  Device    Day   Time      Count  Stanza  Day   Time \n");
	strcat(*command_result, "---------------------------------------------------------");

	if(status_method.command_ecg_name[0] != '\0') {
		p_ecg_info_list = htxd_get_ecg_info_node(status_method.htxd_instance->p_ecg_manager, status_method.command_ecg_name);	
		if(p_ecg_info_list == NULL) {
			sprintf(*command_result, "Specified ECG/MDT<%s> is not currently running", status_method.command_ecg_name);
			return -1;
		}

		if(strlen(status_method.command_option_list) > 0) {
			status_method.return_code = htxd_status_device(p_ecg_info_list, status_method.command_option_list, *command_result);	
		} else {
			status_method.return_code = htxd_status_all_device(p_ecg_info_list, *command_result);	
		}
	} else {
		if(strlen(status_method.command_option_list) > 0) {
			status_method.return_code = htxd_process_all_active_ecg_device(htxd_status_device, status_method.command_option_list, *command_result);
		} else {
			status_method.return_code = htxd_process_all_active_ecg(htxd_status_all_device, *command_result);
		}
	}

	return status_method.return_code;
}



int htxd_option_method_refresh(char **command_result)
{

	htxd *htxd_instance;
	int current_ecg_list_length;
	int result = 0;

	htxd_instance = htxd_get_instance();	

	*command_result = malloc(EXTRA_BUFFER_LENGTH);
	if(*command_result == NULL) {
		return 1;
	}
	current_ecg_list_length = htxd_get_ecg_list_length(htxd_instance->p_ecg_manager);
	if(current_ecg_list_length > 0) {
		sprintf(*command_result, "command failed: <%d> MDT(s) are running currently, stop all MDTs before system refresh", current_ecg_list_length);
		return 0;
	}

	result = htxd_reload_htx_profile(&(htxd_instance->p_profile) );


	if(result == 0) {	
		strcpy(*command_result, "system refresh is completed successfully");
	} else {
		 strcpy(*command_result, "refresh command failed");
	}

	return result;
}


int htxd_activate_device(htxd_ecg_info *p_ecg_info_to_activate, char *p_command_option_list, char *command_result)
{
	struct htxshm_HE *p_HE;
	int return_code = 0;
	int i;
	char *p_device_name = NULL;


	p_device_name = strtok(p_command_option_list, " ");
	while(p_device_name != NULL) {

		p_HE = (struct htxshm_HE *)(p_ecg_info_to_activate->ecg_shm_addr.hdr_addr + 1);
		for(i = 0; i < p_ecg_info_to_activate->ecg_shm_exerciser_entries ; i++) {
			if(strcmp(p_HE->sdev_id, p_device_name) == 0) {
				htxd_set_device_run_sem_status(p_ecg_info_to_activate->ecg_sem_id, i, 0);
				break;
			}
			p_HE++;
		}
		p_device_name = strtok(NULL, " ");
	}

	return return_code;
}



int htxd_activate_all_device(htxd_ecg_info * p_ecg_info_to_activate, char *command_result)
{
	struct htxshm_HE *p_HE;
	int return_code = 0;
	int i;


	p_HE = (struct htxshm_HE *)(p_ecg_info_to_activate->ecg_shm_addr.hdr_addr + 1);

	for(i = 0; i < p_ecg_info_to_activate->ecg_shm_exerciser_entries ; i++) {
		htxd_set_device_run_sem_status(p_ecg_info_to_activate->ecg_sem_id, i, 0);
	}

	return return_code;
}



int htxd_option_method_activate(char **command_result)
{
	htxd_ecg_info *p_ecg_info_list = NULL;
	htxd_option_method_object activate_method;
	char *temp_option_list = NULL;	
	int option_list_length;
	int device_entries_present;


	htxd_init_option_method(&activate_method);

	activate_method.return_code = htxd_validate_command_requirements(activate_method.htxd_instance, activate_method.error_string);
	if(activate_method.return_code != 0) {
		*command_result = malloc(EXTRA_BUFFER_LENGTH);
		strcpy(*command_result, activate_method.error_string);
		return activate_method.return_code;
	}

	device_entries_present = htxd_get_total_device_count();
	*command_result = malloc(EXTRA_BUFFER_LENGTH + device_entries_present * LINE_ENTRY_LENGTH);
	if(*command_result == NULL) {
		return 1;
	}

	option_list_length = strlen(activate_method.command_option_list);
	if(option_list_length > 0) {
		temp_option_list = malloc(option_list_length + STRING_EXTRA_SPACE);
		if(temp_option_list == NULL) {
			return 1;
		}
		strcpy(temp_option_list, activate_method.command_option_list);
	}

	if(activate_method.command_ecg_name[0] != '\0') {
		p_ecg_info_list = htxd_get_ecg_info_node(activate_method.htxd_instance->p_ecg_manager, activate_method.command_ecg_name);	
		if(p_ecg_info_list == NULL) {
			sprintf(*command_result, "Specified ECG/MDT<%s> is not currently running", activate_method.command_ecg_name);
			return -1;
		}

		if(option_list_length > 0) {
			activate_method.return_code = htxd_activate_device(p_ecg_info_list, activate_method.command_option_list, *command_result);	
			strcpy(temp_option_list, activate_method.command_option_list);
		} else {
			activate_method.return_code = htxd_activate_all_device(p_ecg_info_list, *command_result);	
		}
	} else {
		if(option_list_length > 0) {
			activate_method.return_code = htxd_process_all_active_ecg_device(htxd_activate_device, activate_method.command_option_list, *command_result);
			strcpy(temp_option_list, activate_method.command_option_list);
		} else {
			activate_method.return_code = htxd_process_all_active_ecg(htxd_activate_all_device, *command_result);
		}
	}

	htxd_get_common_command_result(ACTIVE_SUSPEND_STATE, p_ecg_info_list, temp_option_list, *command_result);

	if(temp_option_list != NULL) {
		free(temp_option_list);
	}

	return activate_method.return_code;

}




int htxd_suspend_device(htxd_ecg_info *p_ecg_info_to_suspend, char *p_command_option_list, char *command_result)
{
	struct htxshm_HE *p_HE;
	int return_code = 0;
	int i;
	char *p_device_name = NULL;


	p_device_name = strtok(p_command_option_list, " ");
	while(p_device_name != NULL) {

		p_HE = (struct htxshm_HE *)(p_ecg_info_to_suspend->ecg_shm_addr.hdr_addr + 1);
		for(i = 0; i < p_ecg_info_to_suspend->ecg_shm_exerciser_entries ; i++) {
			if(strcmp(p_HE->sdev_id, p_device_name) == 0) {
				htxd_set_device_run_sem_status(p_ecg_info_to_suspend->ecg_sem_id, i, 1);
				break;
			}
			p_HE++;
		}
		p_device_name = strtok(NULL, " ");
	}

	return return_code;
}

int htxd_suspend_all_device(htxd_ecg_info * p_ecg_info_to_suspend, char *command_result)
{
	struct htxshm_HE *p_HE;
	int return_code = 0;
	int i;


	p_HE = (struct htxshm_HE *)(p_ecg_info_to_suspend->ecg_shm_addr.hdr_addr + 1);

	for(i = 0; i < p_ecg_info_to_suspend->ecg_shm_exerciser_entries ; i++) {
		htxd_set_device_run_sem_status(p_ecg_info_to_suspend->ecg_sem_id, i, 1);
	}

	return return_code;
}



int htxd_option_method_suspend(char **command_result)
{

	htxd_ecg_info *p_ecg_info_list = NULL;
	htxd_option_method_object suspend_method;
	char *temp_option_list = NULL;
	int option_list_length;
	int device_entries_present;


	htxd_init_option_method(&suspend_method);

	suspend_method.return_code = htxd_validate_command_requirements(suspend_method.htxd_instance, suspend_method.error_string);
	if(suspend_method.return_code != 0) {
		*command_result = malloc(EXTRA_BUFFER_LENGTH);
		strcpy(*command_result, suspend_method.error_string);
		return suspend_method.return_code;
	}

	device_entries_present = htxd_get_total_device_count();
	*command_result = malloc(EXTRA_BUFFER_LENGTH + device_entries_present * LINE_ENTRY_LENGTH);
	if(*command_result == NULL) {
		return 1;
	}

	option_list_length = strlen(suspend_method.command_option_list);
	if(option_list_length > 0) {
		temp_option_list = malloc(option_list_length + STRING_EXTRA_SPACE);
		if(temp_option_list == NULL) {
			return 1;
		}
		strcpy(temp_option_list, suspend_method.command_option_list);
	}

	if(suspend_method.command_ecg_name[0] != '\0') {
		p_ecg_info_list = htxd_get_ecg_info_node(suspend_method.htxd_instance->p_ecg_manager, suspend_method.command_ecg_name);	
		if(p_ecg_info_list == NULL) {
			sprintf(*command_result, "Specified ECG/MDT<%s> is not currently running", suspend_method.command_ecg_name);
			return -1;
		}

		if( option_list_length > 0) {
			suspend_method.return_code = htxd_suspend_device(p_ecg_info_list, suspend_method.command_option_list, *command_result);	
			strcpy(temp_option_list, suspend_method.command_option_list);
		} else {
			suspend_method.return_code = htxd_suspend_all_device(p_ecg_info_list, *command_result);	
		}
	} else {
		if( option_list_length > 0) {
			suspend_method.return_code = htxd_process_all_active_ecg_device(htxd_suspend_device, suspend_method.command_option_list, *command_result);
			strcpy(temp_option_list, suspend_method.command_option_list);
		} else {
			suspend_method.return_code = htxd_process_all_active_ecg(htxd_suspend_all_device, *command_result);
		}
	}

	htxd_get_common_command_result(ACTIVE_SUSPEND_STATE, p_ecg_info_list, temp_option_list, *command_result);

	if(temp_option_list != NULL) {
		free(temp_option_list);
	}

	return suspend_method.return_code;

}


int htxd_set_device_terminate_status(htxd_ecg_info *p_ecg_info_to_terminate, int device_position)
{

	struct htxshm_HE *p_HE;	
	int save_run_status;
	int is_pause = 0;
	int return_code = 0;
	int kill_errno;
	pid_t HE_pid;
	char regular_expression[256];
	int dummy;

	
	p_HE = (struct htxshm_HE *)(p_ecg_info_to_terminate->ecg_shm_addr.hdr_addr + 1);
	p_HE += device_position;


	HE_pid = p_HE->PID;
	if( (HE_pid != 0) && (p_HE->tm_last_upd != 0) ) {
		save_run_status = htxd_get_device_run_sem_status(p_ecg_info_to_terminate->ecg_sem_id, device_position);
		htxd_set_device_run_sem_status(p_ecg_info_to_terminate->ecg_sem_id, device_position, 0);
		htxd_set_device_error_sem_status(p_ecg_info_to_terminate->ecg_sem_id, device_position, 0);
		
		p_HE->user_term = 1;
		p_HE->max_cycles = 0;

		alarm(htxd_get_slow_shutdown_wait());
		

		if( getpid() == getpgid(HE_pid) ) {
			is_pause = 1;	
		}
	
		return_code = htxd_send_SIGTERM(HE_pid); 
		kill_errno = errno;
		if(return_code == 0) {
			if(is_pause == 1) {
				pause();
			} else {
				/* external process death */
				HTXD_TRACE(LOG_OFF, "NOT calling pause()");
			}
			if(!alarm (0)) {
				htxd_send_SIGKILL(HE_pid);
			}
			sleep(1);
			strcpy (regular_expression, "^");
			strcat (regular_expression, p_HE->HE_name);
			strcat (regular_expression, ".*cleanup[\t ]*$");
			return_code = htxd_run_HE_script(regular_expression, p_HE->sdev_id, &dummy);
		} else {
			alarm ((unsigned) 0);
			p_HE->user_term = 0;
			p_HE->DR_term = 0;

			if (kill_errno == ESRCH) {
				printf("No such process.  Perhaps it is already deceased?\n");
			} else {
				printf("kill return error <%d>\n", kill_errno);
			}
			return_code = -1;
		}
		
		htxd_set_device_run_sem_status(p_ecg_info_to_terminate->ecg_sem_id, device_position, save_run_status);
	}

	
	return return_code;
}



int htxd_terminate_device(htxd_ecg_info *p_ecg_info_to_terminate, char *p_command_option_list, char *command_result)
{
	struct htxshm_HE *p_HE;
	int return_code = 0;
	int i;
	char *p_device_name = NULL;


	p_device_name = strtok(p_command_option_list, " ");
	while(p_device_name != NULL) {

		p_HE = (struct htxshm_HE *)(p_ecg_info_to_terminate->ecg_shm_addr.hdr_addr + 1);
		for(i = 0; i < p_ecg_info_to_terminate->ecg_shm_exerciser_entries ; i++) {
			if(strcmp(p_HE->sdev_id, p_device_name) == 0) {
				return_code = htxd_set_device_terminate_status(p_ecg_info_to_terminate, i);
				break;
			}
			p_HE++;
		}
		p_device_name = strtok(NULL, " ");
	}

	return return_code;
}



int htxd_terminate_all_device(htxd_ecg_info *p_ecg_info_to_terminate, char *command_result)
{

	int return_code = 0;
	int i;

	for(i = 0; i < p_ecg_info_to_terminate->ecg_shm_exerciser_entries ; i++) {
		htxd_set_device_terminate_status(p_ecg_info_to_terminate, i);
	}

	htxd_display_exer_table();
	htxd_display_ecg_info_list();

	return return_code;

}


int htxd_option_method_terminate(char **command_result)
{

	htxd_ecg_info *p_ecg_info_list = NULL;
	htxd_option_method_object terminate_method;
	char *temp_option_list = NULL;
	int option_list_length;
	int device_entries_present;


	htxd_init_option_method(&terminate_method);

	terminate_method.return_code = htxd_validate_command_requirements(terminate_method.htxd_instance, terminate_method.error_string);
	if(terminate_method.return_code != 0) {
		*command_result = malloc(EXTRA_BUFFER_LENGTH);
		strcpy(*command_result, terminate_method.error_string);
		return terminate_method.return_code;
	}

	device_entries_present = htxd_get_total_device_count();
	*command_result = malloc(EXTRA_BUFFER_LENGTH + device_entries_present * LINE_ENTRY_LENGTH);
	if(*command_result == NULL) {
		return 1;
	}

	option_list_length = strlen(terminate_method.command_option_list);
	if(option_list_length > 0) {
		temp_option_list = malloc(option_list_length + STRING_EXTRA_SPACE);
		if(temp_option_list == NULL) {
			return 1;
		}
		strcpy(temp_option_list, terminate_method.command_option_list);
	}

	if(terminate_method.command_ecg_name[0] != '\0') {
		p_ecg_info_list = htxd_get_ecg_info_node(terminate_method.htxd_instance->p_ecg_manager, terminate_method.command_ecg_name);	
		if(p_ecg_info_list == NULL) {
			sprintf(*command_result, "Specified ECG/MDT<%s> is not currently running", terminate_method.command_ecg_name);
			return -1;
		}

		if(option_list_length > 0) {
			terminate_method.return_code = htxd_terminate_device(p_ecg_info_list, terminate_method.command_option_list, *command_result);	
			strcpy(temp_option_list, terminate_method.command_option_list);

		} else {
			terminate_method.return_code = htxd_terminate_all_device(p_ecg_info_list, *command_result);	
		}
	} else {
		if(option_list_length > 0) {
			terminate_method.return_code = htxd_process_all_active_ecg_device(htxd_terminate_device, terminate_method.command_option_list, *command_result);
			strcpy(temp_option_list, terminate_method.command_option_list);
		} else {
			terminate_method.return_code = htxd_process_all_active_ecg(htxd_terminate_all_device, *command_result);
		}
	}

	htxd_get_common_command_result(RUNNING_STATE, p_ecg_info_list, temp_option_list, *command_result);

	if(temp_option_list != NULL) {
		free(temp_option_list);
	}

	return terminate_method.return_code;
}

int htxd_set_device_restart_status(htxd_ecg_info *p_ecg_info_to_restart, int device_position)
{

	struct htxshm_HE *p_HE;
	char regular_expression[256];
	int dummy;


	p_HE = (struct htxshm_HE *)(p_ecg_info_to_restart->ecg_shm_addr.hdr_addr + 1);
	p_HE += device_position;	

	htxd_set_FD_close_on_exec_flag();		

	strcpy(regular_expression, "^");
	strcpy(regular_expression, p_HE->HE_name);
	strcpy(regular_expression, ".*setup[\t ]*$");
	htxd_run_HE_script(regular_expression,  p_HE->sdev_id, &dummy);
	
	htxd_load_exerciser(p_HE);	


	htxd_reset_FD_close_on_exec_flag();		

	return 0;
}




int htxd_restart_device(htxd_ecg_info *p_ecg_info_to_restart, char *p_command_option_list, char *command_result)
{
	struct htxshm_HE *p_HE;
	int return_code = 0;
	int i;
	char *p_device_name = NULL;


	p_device_name = strtok(p_command_option_list, " ");
	while(p_device_name != NULL) {

		p_HE = (struct htxshm_HE *)(p_ecg_info_to_restart->ecg_shm_addr.hdr_addr + 1);
		for(i = 0; i < p_ecg_info_to_restart->ecg_shm_exerciser_entries ; i++) {
			if(strcmp(p_HE->sdev_id, p_device_name) == 0) {
				return_code = htxd_set_device_restart_status(p_ecg_info_to_restart, i);
				break;
			}
			p_HE++;
		}
		p_device_name = strtok(NULL, " ");
	}

	return return_code;
}




int htxd_restart_all_device(htxd_ecg_info * p_ecg_info_to_restart, char *command_result)
{

	int return_code = 0;
	int i;

	for(i = 0; i < p_ecg_info_to_restart->ecg_shm_exerciser_entries ; i++) {
		htxd_set_device_restart_status(p_ecg_info_to_restart, i);
	}

	return return_code;

}

int htxd_option_method_restart(char **command_result)
{

	htxd_ecg_info *p_ecg_info_list = NULL;
	htxd_option_method_object restart_method;
	char *temp_option_list = NULL;
	int option_list_length;
	int device_entries_present;


	htxd_init_option_method(&restart_method);

	restart_method.return_code = htxd_validate_command_requirements(restart_method.htxd_instance, restart_method.error_string);
	if(restart_method.return_code != 0) {
		*command_result = malloc(EXTRA_BUFFER_LENGTH);
		strcpy(*command_result, restart_method.error_string);
		return restart_method.return_code;
	}

	device_entries_present = htxd_get_total_device_count();
	*command_result = malloc(EXTRA_BUFFER_LENGTH + device_entries_present * LINE_ENTRY_LENGTH);
	if(*command_result == NULL) {
		return 1;
	}

	option_list_length = strlen(restart_method.command_option_list);
	if(option_list_length > 0) {
		temp_option_list = malloc(option_list_length + STRING_EXTRA_SPACE);
		if(temp_option_list == NULL) {
			return 1;
		}
		strcpy(temp_option_list, restart_method.command_option_list);
	}

	if(restart_method.command_ecg_name[0] != '\0') {
		p_ecg_info_list = htxd_get_ecg_info_node(restart_method.htxd_instance->p_ecg_manager, restart_method.command_ecg_name);	
		if(p_ecg_info_list == NULL) {
			sprintf(*command_result, "Specified ECG/MDT<%s> is not currently running", restart_method.command_ecg_name);
			return -1;
		}

		if(option_list_length > 0) {
			restart_method.return_code = htxd_restart_device(p_ecg_info_list, restart_method.command_option_list, *command_result);	
			strcpy(temp_option_list, restart_method.command_option_list);
		} else {
			restart_method.return_code = htxd_restart_all_device(p_ecg_info_list, *command_result);	
		}
	} else {
		if(option_list_length > 0) {
			htxd_process_all_active_ecg_device(htxd_restart_device, restart_method.command_option_list, *command_result);
			strcpy(temp_option_list, restart_method.command_option_list);
		} else {
			restart_method.return_code = htxd_process_all_active_ecg(htxd_restart_all_device, *command_result);
		}
	}

	htxd_get_common_command_result(RUNNING_STATE, p_ecg_info_list, temp_option_list, *command_result);

	if(temp_option_list != NULL) {
		free(temp_option_list);
	}


	return restart_method.return_code;
}



int htxd_coe_device(htxd_ecg_info *p_ecg_info_to_coe, char *p_command_option_list, char *command_result)
{
	struct htxshm_HE *p_HE;
	int return_code = 0;
	int i;
	char *p_device_name = NULL;


	p_device_name = strtok(p_command_option_list, " ");
	while(p_device_name != NULL) {

		p_HE = (struct htxshm_HE *)(p_ecg_info_to_coe->ecg_shm_addr.hdr_addr + 1);
		for(i = 0; i < p_ecg_info_to_coe->ecg_shm_exerciser_entries ; i++) {
			if(strcmp(p_HE->sdev_id, p_device_name) == 0) {
				p_HE->cont_on_err = HTX_COE;
				break;
			}
			p_HE++;
		}
		p_device_name = strtok(NULL, " ");
	}

	return return_code;
}




int htxd_coe_all_device(htxd_ecg_info * p_ecg_info_to_coe, char *command_result)
{

	int return_code = 0;
	int i;
	struct htxshm_HE *p_HE;

	p_HE = (struct htxshm_HE *)(p_ecg_info_to_coe->ecg_shm_addr.hdr_addr + 1);

	for(i = 0; i < p_ecg_info_to_coe->ecg_shm_exerciser_entries ; i++) {
		p_HE->cont_on_err = HTX_COE;
		p_HE++;
	}

	return return_code;

}



int htxd_option_method_coe(char **command_result)
{

	htxd_ecg_info * p_ecg_info_list = NULL;
	htxd_option_method_object coe_method;
	char *temp_option_list = NULL;
	int option_list_length;
	int device_entries_present;


	htxd_init_option_method(&coe_method);

	coe_method.return_code = htxd_validate_command_requirements(coe_method.htxd_instance, coe_method.error_string);
	if(coe_method.return_code != 0) {
		*command_result = malloc(EXTRA_BUFFER_LENGTH);
		strcpy(*command_result, coe_method.error_string);
		return coe_method.return_code;
	}

	device_entries_present = htxd_get_total_device_count();
	*command_result = malloc(EXTRA_BUFFER_LENGTH + device_entries_present * LINE_ENTRY_LENGTH);
	if(*command_result == NULL) {
		return 1;
	}

	option_list_length = strlen(coe_method.command_option_list);

	if(option_list_length > 0) {
		temp_option_list = malloc(option_list_length + STRING_EXTRA_SPACE);	
		if(temp_option_list == NULL) {
			return 1;
		}
		strcpy(temp_option_list, coe_method.command_option_list);
	}

	if(coe_method.command_ecg_name[0] != '\0') {
		p_ecg_info_list = htxd_get_ecg_info_node(coe_method.htxd_instance->p_ecg_manager, coe_method.command_ecg_name);	
		if(p_ecg_info_list == NULL) {
			sprintf(*command_result, "Specified ECG/MDT<%s> is not currently running", coe_method.command_ecg_name);
			return -1;
		}

		if( option_list_length > 0) {
			coe_method.return_code = htxd_coe_device(p_ecg_info_list, temp_option_list, *command_result);	
			strcpy(temp_option_list, coe_method.command_option_list);
		} else {
			coe_method.return_code = htxd_coe_all_device(p_ecg_info_list, *command_result);	
		}
	} else {
		if(option_list_length > 0) {
			strcpy(temp_option_list, coe_method.command_option_list);
			coe_method.return_code = htxd_process_all_active_ecg_device(htxd_coe_device, temp_option_list, *command_result);
		} else {
			coe_method.return_code = htxd_process_all_active_ecg(htxd_coe_all_device, *command_result);
		}
	}

	htxd_get_common_command_result(COE_SOE_STATE, p_ecg_info_list, temp_option_list, *command_result); 

	if(temp_option_list != NULL) {
		free(temp_option_list);
	}		

	return coe_method.return_code;
}



int htxd_soe_device(htxd_ecg_info *p_ecg_info_to_soe, char *p_command_option_list, char *command_result)
{
	struct htxshm_HE *p_HE;
	int return_code = 0;
	int i;
	char *p_device_name = NULL;


	p_device_name = strtok(p_command_option_list, " ");
	while(p_device_name != NULL) {

		p_HE = (struct htxshm_HE *)(p_ecg_info_to_soe->ecg_shm_addr.hdr_addr + 1);
		for(i = 0; i < p_ecg_info_to_soe->ecg_shm_exerciser_entries ; i++) {

			if(strcmp(p_HE->sdev_id, p_device_name) == 0) {
				p_HE->cont_on_err = HTX_SOE;
				break;
			}
			p_HE++;
		}
		p_device_name = strtok(NULL, " ");
	}

	return return_code;
}



int htxd_soe_all_device(htxd_ecg_info * p_ecg_info_to_soe, char *command_result)
{

	int return_code = 0;
	int i;
	struct htxshm_HE *p_HE;

	p_HE = (struct htxshm_HE *)(p_ecg_info_to_soe->ecg_shm_addr.hdr_addr + 1);

	for(i = 0; i < p_ecg_info_to_soe->ecg_shm_exerciser_entries ; i++) {
		p_HE->cont_on_err = HTX_SOE;
		p_HE++;
	}

	return return_code;

}




int htxd_option_method_soe(char **command_result)
{

	htxd_ecg_info * p_ecg_info_list = NULL;
	htxd_option_method_object soe_method;
	char *temp_option_list = NULL;
	int option_list_length;
	int device_entries_present;


	htxd_init_option_method(&soe_method);

	soe_method.return_code = htxd_validate_command_requirements(soe_method.htxd_instance, soe_method.error_string);
	if(soe_method.return_code != 0) {
		*command_result = malloc(EXTRA_BUFFER_LENGTH);
		strcpy(*command_result, soe_method.error_string);
		return soe_method.return_code;
	}

	device_entries_present = htxd_get_total_device_count();
	*command_result = malloc(EXTRA_BUFFER_LENGTH + device_entries_present * LINE_ENTRY_LENGTH);
	if(*command_result == NULL) {
		return 1;
	}

	option_list_length = strlen(soe_method.command_option_list);
	if(option_list_length > 0) {
		temp_option_list = malloc(option_list_length + STRING_EXTRA_SPACE);
		if(temp_option_list == NULL) {
			return 1;
		}
		strcpy(temp_option_list, soe_method.command_option_list);
	}

	if(soe_method.command_ecg_name[0] != '\0') {
		p_ecg_info_list = htxd_get_ecg_info_node(soe_method.htxd_instance->p_ecg_manager, soe_method.command_ecg_name);	
		if(p_ecg_info_list == NULL) {
			sprintf(*command_result, "Specified ECG/MDT<%s> is not currently running", soe_method.command_ecg_name);
			return -1;
		}

		if(option_list_length > 0) {
			soe_method.return_code = htxd_soe_device(p_ecg_info_list, temp_option_list, *command_result);	
			strcpy(temp_option_list, soe_method.command_option_list);
		} else {
			soe_method.return_code = htxd_soe_all_device(p_ecg_info_list, *command_result);	
		}
	} else {
		if(option_list_length > 0) {
			soe_method.return_code = htxd_process_all_active_ecg_device(htxd_soe_device, temp_option_list, *command_result);
			strcpy(temp_option_list, soe_method.command_option_list);
		} else {
			soe_method.return_code = htxd_process_all_active_ecg(htxd_soe_all_device, *command_result);
		}
	}

	htxd_get_common_command_result(COE_SOE_STATE, p_ecg_info_list, temp_option_list, *command_result);
	if(temp_option_list != NULL) {
		free(temp_option_list);
	}

	return soe_method.return_code;
}


void htxd_bootme_error_string(int error_code, char ** result_string)
{
	switch(error_code) {
	case 2:
		sprintf(*result_string, "Error: Not sufficient space, failed to start bootme");
		break;
	case 11:
		sprintf(*result_string, "Error: Please check the value of REBOOT in /usr/lpp/htx/rules/reg/bootme/default, failed to start bootme");
		break;
	case 12:
		sprintf(*result_string, "Error: Please check the value of BOOT_CMD in /usr/lpp/htx/rules/reg/bootme/default, failed to start bootme");
		break;
	case 13:
		sprintf(*result_string, "Error: Please check the value of BOOT_WAIT in /usr/lpp/htx/rules/reg/bootme/default, failed to start bootme");
		break;
	case 21:
		sprintf(*result_string, "bootme is already on");
		break;
	case 31:
		sprintf(*result_string, "bootme flag file </usr/lpp/htx/.htxd_autostart> was missing");
		break;
	case 32:
		sprintf(*result_string, "bootme is already off");
		break;
	case 41:
		sprintf(*result_string, "bootme status : off");
		break;
	case 42:
		sprintf(*result_string, "bootme status : inconsistent state, bootme cron entry is present, bootme flag file </usr/lpp/htx/.htxd_autostart> was missing");
		break;
	case 0:
		break;
	default: 
		sprintf(*result_string, "Error: unknown error from bootme, error code = <%d>", error_code);
		break;	
	}
}


int htxd_option_method_bootme(char **command_result)
{

	htxd_option_method_object bootme_method;
	char *temp_option_list = NULL;
	int option_list_length;
	char trace_string[256];
	FILE *p_boot_flag;
	int bootme_status;
	char *running_mdt_name;
	int bootme_return_code;


	htxd_init_option_method(&bootme_method);

	bootme_method.return_code = htxd_validate_command_requirements(bootme_method.htxd_instance, bootme_method.error_string);
	if(bootme_method.return_code != 0) {
		*command_result = malloc(EXTRA_BUFFER_LENGTH);
		strcpy(*command_result, bootme_method.error_string);
		return bootme_method.return_code;
	}

	*command_result = malloc( 2 * 1024);
	if(*command_result == NULL) {
		sprintf(trace_string, "command_result: malloc failed with errno = <%d>", errno);
		HTXD_TRACE(LOG_ON, trace_string);
		return 1;
	}

	option_list_length = strlen(bootme_method.command_option_list);
	if(option_list_length > 0) {
		temp_option_list = malloc(option_list_length + STRING_EXTRA_SPACE);
		if(temp_option_list == NULL) {
			sprintf(trace_string, "temp_option_list: malloc failed with errno = <%d>", errno);
			HTXD_TRACE(LOG_ON, trace_string);
			return 1;
		}
		strcpy(temp_option_list, bootme_method.command_option_list);
		
		if( strcmp(temp_option_list, "on") == 0 ) {
			sprintf(trace_string, "%s on", HTXD_BOOTME_SCRIPT);
			bootme_status = system(trace_string);
			bootme_return_code = WEXITSTATUS(bootme_status);
			if(bootme_return_code == 0) {
				running_mdt_name = htxd_get_running_ecg_name();
				p_boot_flag = fopen(HTXD_AUTOSTART_FILE, "w");
				if(p_boot_flag == NULL) {
					sprintf(trace_string, "fopen failed with errno = <%d>", errno);
					HTXD_TRACE(LOG_ON, trace_string);
					sprintf(*command_result, "bootme on failed, could not set bootme flag file <%s>", HTXD_AUTOSTART_FILE);
					sprintf(trace_string, "%s off", HTXD_BOOTME_SCRIPT);
					system(trace_string);
				} else {
					fprintf(p_boot_flag, "%s", running_mdt_name);
					fclose(p_boot_flag);
					strcpy(*command_result, "bootme on is completed successfully");
				}
			} else {
				htxd_bootme_error_string(bootme_return_code, command_result);
			}

		} else if(  strcmp(temp_option_list, "off") == 0 ) {
			sprintf(trace_string, "%s off", HTXD_BOOTME_SCRIPT);
			bootme_status = system(trace_string);
			bootme_return_code = WEXITSTATUS(bootme_status);
			if(bootme_return_code == 0) {
				strcpy(*command_result, "bootme off is completed successfully");
			} else {
				htxd_bootme_error_string(bootme_return_code, command_result);
			}
		} else if( strcmp(temp_option_list, "status") == 0 ) {
			sprintf(trace_string, "%s status", HTXD_BOOTME_SCRIPT);
			bootme_status = system(trace_string);
			bootme_return_code = WEXITSTATUS(bootme_status);
			if(bootme_return_code == 0) {
				strcpy(*command_result, "bootme status: on");
			} else {
				htxd_bootme_error_string(bootme_return_code, command_result);	
			}
		} else {
			strcpy(*command_result, "Error: bootme unknow option\nUsage: bootme (on|off|status)");
		}
	} else {
		strcpy(*command_result, "Error: invalid bootme command\nUsage: bootme (on|off|status)");
	}
 

	if(temp_option_list != NULL) {
		free(temp_option_list);
	}

	return 0;
}



int htxd_option_method_exersetupinfo(char **command_result)
{

	htxd_ecg_info * p_ecg_info_list = NULL;
	htxd_option_method_object exersetupinfo_method;
	struct htxshm_HE *p_HE;
	int exer_setup_info_flag = 1;
	int i;


	htxd_init_option_method(&exersetupinfo_method);
	*command_result = malloc(EXTRA_BUFFER_LENGTH);

	exersetupinfo_method.return_code = htxd_validate_command_requirements(exersetupinfo_method.htxd_instance, exersetupinfo_method.error_string);
	if(exersetupinfo_method.return_code != 0) {
		strcpy(*command_result, exersetupinfo_method.error_string);
		return exersetupinfo_method.return_code;
	}

	if(exersetupinfo_method.command_ecg_name[0] != '\0') {
		p_ecg_info_list = htxd_get_ecg_info_node(exersetupinfo_method.htxd_instance->p_ecg_manager, exersetupinfo_method.command_ecg_name);	
		if(p_ecg_info_list == NULL) {
			sprintf(*command_result, "Specified ECG/MDT<%s> is not currently running", exersetupinfo_method.command_ecg_name);
			return -1;
		}
	} else {
		p_ecg_info_list = htxd_get_ecg_info_node(exersetupinfo_method.htxd_instance->p_ecg_manager, exersetupinfo_method.htxd_instance->p_ecg_manager->running_ecg_name);
		if(p_ecg_info_list == NULL) {
			sprintf(*command_result, "Internal Error: Failed to get a running ecg<%s>", exersetupinfo_method.command_ecg_name);
			return -1;
		}
	}

	p_HE = (struct htxshm_HE *)(p_ecg_info_list->ecg_shm_addr.hdr_addr + 1);
	for(i = 0; i < p_ecg_info_list->ecg_shm_exerciser_entries ; i++) {
		if(p_HE->upd_test_id == 0 ) {
			exer_setup_info_flag = 0;
			break;
		}
		p_HE++;
	}

	sprintf(*command_result, "setup_of_all_exers_done = %d", exer_setup_info_flag);


	return 0;
}



int htxd_getstats_ecg(htxd_ecg_info * p_ecg_info_to_getstats, char **command_result)
{

	int return_code = 0;
	int  htx_stats_pid;

	/* get HTX stats program PID */
	htx_stats_pid = htxd_get_htx_stats_pid();

	/* truncate the existing stats file */
	if(truncate(HTX_STATS_SEND_FILE, 0) == -1) {
		if(errno == ENOENT ) {
			if(creat(HTX_STATS_SEND_FILE, 0) >= 0) {
			} else {
			}
				
		} else {
		}
	}

	/* have to set shm key to system header shared memory so that htx stats is able to connect */	
	htxd_set_system_header_info_shm_with_current_shm_key(p_ecg_info_to_getstats->ecg_shm_key);	

	/* sending signal SIGUSR1 to htx stats program to trigger the statistics collection*/
	htxd_send_SIGUSR1(htx_stats_pid);

	/* wait till stats file is updated */
	sleep(5);

	/* read the HTX stats file to string buffer */
	return_code = htxd_read_file(HTX_STATS_FILE, command_result); 
	
	return return_code;
}


int htxd_option_method_getstats(char **command_result)
{

	htxd *htxd_instance;
	char error_string[512];
	int return_code = 0;
	htxd_ecg_info * p_ecg_info_list;
	char command_ecg_name[MAX_ECG_NAME_LENGTH];

	htxd_instance = htxd_get_instance();
	strcpy(command_ecg_name, htxd_get_command_ecg_name() );

	*command_result = NULL;

	if(command_ecg_name[0] == '\0') {
		strcpy(command_ecg_name, DEFAULT_ECG_NAME );
	}

	return_code = htxd_validate_command_requirements(htxd_instance, error_string);


	if(return_code != 0) {
		*command_result = malloc(512);
		strcpy(*command_result, error_string);
		return return_code;
	}

	p_ecg_info_list = htxd_get_ecg_info_node(htxd_instance->p_ecg_manager, command_ecg_name);	
	if(p_ecg_info_list == NULL) {
		*command_result = malloc(512);
		HTXD_TRACE(LOG_OFF, *command_result);
		sprintf(*command_result, "Specified ECG/MDT<%s> is not currently running", command_ecg_name);
		return -1;
	}

	return_code = htxd_getstats_ecg(p_ecg_info_list, command_result);	
	if(return_code != 0) {
		if(*command_result != 0) {
			free(*command_result);
		}
		*command_result = malloc(512);
		sprintf(*command_result, "Error : htxd_option_method_getstats() htxd_getstats_ecg() return <%d>", return_code);
		HTXD_TRACE(LOG_ON, *command_result);
		sprintf(*command_result, "Error : while getting test statistics details, error code = <%d>", return_code);
		return -1;
	}

	return 0;

}



int htxd_option_method_geterrlog(char **command_result)
{
	int return_code;

	return_code = htxd_read_file(HTX_ERR_LOG_FILE, command_result); 
	if(return_code != 0) {
		if(*command_result != 0) {
			free(*command_result);
		}
		*command_result = malloc(512);
		sprintf(*command_result, "Error : htxd_option_method_getstats() htxd_getstats_ecg() return <%d>", return_code);
		HTXD_TRACE(LOG_ON, *command_result);
		sprintf(*command_result, "Error : while getting error log, error code = <%d>", return_code);
	}

	return return_code;
}



int htxd_option_method_clrerrlog(char **command_result)
{
	int return_code;

	*command_result = malloc(512);
	if(*command_result == NULL) {
		return -1;
	}

	return_code = truncate(HTX_ERR_LOG_FILE, 0);
	if(return_code == -1) {
		sprintf(*command_result, "Error : failed to truncate htx stats file to send <%s>, errno = <%d>", HTX_ERR_LOG_FILE, errno);
	
	} else {
		strcpy(*command_result, "HTX error log file cleared successfully");
	}

	return return_code;
}




int htxd_option_method_cmd(char **command_result)
{

	int return_code;
	char command_string[512];
	htxd *htxd_instance;

	htxd_instance = htxd_get_instance();

	sprintf(command_string, "echo \" Error: failed to execute command <%s> \" >%s; sync",  htxd_instance->p_command->option_list, HTX_CMD_RESULT_FILE);
	system(command_string);

	sprintf(command_string, " (%s) > %s 2>&1 ; sync", htxd_instance->p_command->option_list, HTX_CMD_RESULT_FILE);
	system(command_string);

	return_code = htxd_read_file(HTX_CMD_RESULT_FILE, command_result);
	if(return_code != 0) {
		if(*command_result != 0) {
			free(*command_result);
		}
		*command_result = malloc(HTX_ERR_MESSAGE_LENGTH);
		if(*command_result == NULL) {
			return 1;
		}
		sprintf(*command_result,"Error: failed to get command result");
		
	}

	return return_code;
}


int htxd_option_method_set_eeh(char **command_result)
{

	int return_code = 0;
	htxd *htxd_instance;

	htxd_instance = htxd_get_instance();

	*command_result = malloc(512);
	

	if( (strlen(htxd_instance->p_command->option_list) == 0) || (strcmp(htxd_instance->p_command->option_list, "1") == 0) ) {
		unsetenv("HTX_EEH_OFF");
		strcpy(*command_result, "set eeh flag successfully");
	} else if(strcmp(htxd_instance->p_command->option_list, "0") == 0) {
		setenv("HTX_EEH_OFF", "1", 1);
		strcpy(*command_result, "unset eeh flag successfully");
	} else {
		strcpy(*command_result, "Error : failed while setting eeh flag because of invalid argument, valid argument is 1 or 0");
	}

	return return_code;
}


int htxd_option_method_set_kdblevel(char **command_result)
{

	int return_code = 0;
	htxd *htxd_instance;

	htxd_instance = htxd_get_instance();

	*command_result = malloc(512);

	if( (strlen(htxd_instance->p_command->option_list) == 0) || (strcmp(htxd_instance->p_command->option_list, "1") == 0) ) {
		setenv("HTXKDBLEVEL", "0", 1);
		strcpy(*command_result, "set kdb level flag successfully");
	} else if(strcmp(htxd_instance->p_command->option_list, "0") == 0) {
		setenv("HTXKDBLEVEL", "0", 1);
		strcpy(*command_result, "unset kdb level  flag successfully");
	} else {
		strcpy(*command_result, "Error : failed while setting kdb level flag because of invalid argument, valid argument is 1 or 0");
	}

	return return_code;

	return 0;
}




int htxd_option_method_set_hxecom(char **command_result)
{

	*command_result = malloc(512);

	if( htxd_is_file_exist("/build_net") == FALSE) {
		strcpy(*command_result,"Error : hxecom needs the build_net script, network setup for hxecom cannot be done");
		return -1;
	}

	system("/usr/lpp/htx/ecg/ecg_net");
	strcpy(*command_result,"hxecom setup completed successfully (ecg_net is appeneded to ecg.bu");
	
	return 0;
}




int htxd_option_method_getvpd(char **command_result)
{
	int return_code;
	char vpd_command_string[256];

	sprintf(vpd_command_string, "%s > %s", HTX_VPD_SCRIPT, HTX_VPD_FILE);

	system(vpd_command_string);

	return_code = htxd_read_file(HTX_VPD_FILE, command_result);
	if(return_code != 0) {
		if(*command_result != 0) {
			free(*command_result);
		}
		*command_result = malloc(512);
		sprintf(*command_result, "Error : while getting VPD information");
	}
	return return_code;
}


void init_ecg_summary(htxd_ecg_summary *p_summary_details)
{
	p_summary_details->total_device			= 0;
	p_summary_details->running_device		= 0;
	p_summary_details->suspended_device		= 0;
	p_summary_details->error_device			= 0;
	p_summary_details->partially_running_device	= 0;
	p_summary_details->terminated_device		= 0;
	p_summary_details->died_device			= 0;
	p_summary_details->hung_device			= 0;
	p_summary_details->completed_device		= 0;
	p_summary_details->inactive_device		= 0;
}



int htxd_getecgsum_all_device(htxd_ecg_info * p_ecg_info_to_getecgsum, char *command_result)
{
	struct htxshm_HE *p_HE;
	int i;
	char device_run_status[5];
	int return_code = 0;
	htxd_ecg_summary summary_details;

	init_ecg_summary(&summary_details);


	p_HE = (struct htxshm_HE *)(p_ecg_info_to_getecgsum->ecg_shm_addr.hdr_addr + 1);

	for(i = 0; i < p_ecg_info_to_getecgsum->ecg_shm_exerciser_entries ; i++) {
		htxd_get_device_run_status(p_HE, p_ecg_info_to_getecgsum, i, device_run_status);
		if( strcmp(device_run_status, "RN") == 0) {
			summary_details.running_device++;
		} else if( strcmp(device_run_status, "DD") == 0) {
			summary_details.died_device++;
		} else if( strcmp(device_run_status, "DT") == 0) {
			summary_details.dr_terminated_device++;
		} else if( strcmp(device_run_status, "TM") == 0) {
			summary_details.terminated_device++;
		} else if( strcmp(device_run_status, "ER") == 0) {
			summary_details.error_device++;
		} else if( strcmp(device_run_status, "ST") == 0) {
			summary_details.suspended_device++;
		} else if( strcmp(device_run_status, "CP") == 0) {
			summary_details.completed_device++;
		} else if( strcmp(device_run_status, "HG") == 0) {
			summary_details.hung_device++;
		} else if( strcmp(device_run_status, "PR") == 0) {
			summary_details.partially_running_device++;
		} else if( strcmp(device_run_status, "  ") == 0) {
			summary_details.inactive_device = p_ecg_info_to_getecgsum->ecg_shm_exerciser_entries;
			break;
		} 
		p_HE++;
	}

	summary_details.total_device = p_ecg_info_to_getecgsum->ecg_shm_exerciser_entries;

	sprintf (command_result,
		"%s is in %s state\n"
		"Total Devices       = %d\n"
		" Running            = %d\n"
		" Suspended          = %d\n"
		" Error              = %d\n"
		" Partially Running   = %d\n"
		" Terminated         = %d\n"
		" Died               = %d\n"
		" Hung               = %d\n"
		" Completed          = %d\n"
		" Inactive           = %d\n\n", p_ecg_info_to_getecgsum->ecg_name,  "ACTIVE", 
						summary_details.total_device,
						summary_details.running_device,
						summary_details.suspended_device,
						summary_details.error_device,
						summary_details.partially_running_device,
						summary_details.terminated_device,
						summary_details.died_device,
						summary_details.hung_device,
						summary_details.completed_device,
						summary_details.inactive_device  );
			
	
	return return_code;
}



int htxd_option_method_getecgsum(char **command_result)
{
	htxd *htxd_instance;
	char error_string[512];
	int return_code = 0;
	htxd_ecg_info * p_ecg_info_list;
	char command_ecg_name[MAX_ECG_NAME_LENGTH];

	htxd_instance = htxd_get_instance();
	strcpy(command_ecg_name, htxd_get_command_ecg_name() );

	*command_result = malloc(EXTRA_BUFFER_LENGTH *4);

	return_code = htxd_validate_command_requirements(htxd_instance, error_string);
	if(return_code != 0) {
		strcpy(*command_result, error_string);
		return return_code;
	}

	if(command_ecg_name[0] != '\0') {
		p_ecg_info_list = htxd_get_ecg_info_node(htxd_instance->p_ecg_manager, command_ecg_name);	
		if(p_ecg_info_list == NULL) {
			sprintf(*command_result, "Specified ECG/MDT<%s> is not currently running", command_ecg_name);
			return -1;
		}

		return_code = htxd_getecgsum_all_device(p_ecg_info_list, *command_result);	
	} else {
		return_code = htxd_process_all_active_ecg(htxd_getecgsum_all_device, *command_result);
	}



	return return_code;
}



int htxd_option_method_getecglist(char **command_result)
{
	int mdt_count = 0;
	int return_code;
	char trace_string[256];


	return_code = htxd_get_regular_file_count(MDT_DIR);
	if(return_code == -1) {
		*command_result = malloc(256);
		strcpy(*command_result, "Error while accessing MDT directory");
		HTXD_TRACE(LOG_ON, *command_result);
		return -1;
	}
	mdt_count = return_code;

	if(mdt_count > 0) {

		return_code = htxd_wrtie_mdt_list(MDT_DIR, MDT_LIST_FILE, mdt_count, "w", " ");
		if(return_code == -1) {
			HTXD_TRACE(LOG_ON, "htxd_wrtie_mdt_list returns qith -1");
			return -1;
		}
		return_code = htxd_read_file(MDT_LIST_FILE, command_result);
		if(return_code != 0) {
			sprintf(trace_string, "htxd_read_file() returned with %d", return_code);
			HTXD_TRACE(LOG_ON, trace_string);
			return -1;
		}
	} else {
		*command_result = malloc(256);
		strcpy(*command_result, "No files present in MDT directory");
 
	}

	return 0;
}



int htxd_get_common_command_result_row_entry(int mode, struct htxshm_HE *p_HE, htxd_ecg_info * p_ecg_info_to_query, int device_position, char *p_query_row_entry)
{


	char device_coe_soe_status[5];
	char device_active_suspend_status[15];

	if(mode == COE_SOE_STATE){
		htxd_get_device_coe_soe_status(p_HE, device_coe_soe_status);

		sprintf (p_query_row_entry, "%c%-7s%c%-7s%c%-12s%c%-19s%c%s", 
			'\n',
			device_coe_soe_status, ' ',
			p_HE->sdev_id, ' ',
			p_HE->adapt_desc, ' ',
			p_HE->device_desc, ' ',
			p_HE->slot_port);
	} else {
		htxd_get_device_active_suspend_status(p_ecg_info_to_query->ecg_sem_id, device_position, device_active_suspend_status);

		sprintf (p_query_row_entry, "%c%-7s%c%-7s%c%-12s%c%-19s%c%s", 
			'\n',
			device_active_suspend_status, ' ',
			p_HE->sdev_id, ' ',
			p_HE->adapt_desc, ' ',
			p_HE->device_desc, ' ',
			p_HE->slot_port);
	}

	return 0;	
}



int htxd_get_common_command_result_ecg(int mode, htxd_ecg_info *p_ecg_info, char *option_list, char *command_result)
{

	struct htxshm_HE *p_HE;
	int i;
	char result_row_entry[256];
	char *p_device_name;


	p_HE = (struct htxshm_HE *)(p_ecg_info->ecg_shm_addr.hdr_addr + 1);

	if(option_list == NULL)  {
		for(i = 0; i < p_ecg_info->ecg_shm_exerciser_entries ; i++) {
			htxd_get_common_command_result_row_entry(mode, p_HE, p_ecg_info, i, result_row_entry);
			strcat(command_result, result_row_entry);

			p_HE++;
		}
	} else {
		p_device_name = strtok(option_list, " ");
		while(p_device_name != NULL) {
			p_HE = (struct htxshm_HE *)(p_ecg_info->ecg_shm_addr.hdr_addr + 1);
			for(i = 0; i < p_ecg_info->ecg_shm_exerciser_entries ; i++) {
				if(strcmp(p_HE->sdev_id, p_device_name) == 0) {
					htxd_get_common_command_result_row_entry(mode, p_HE, p_ecg_info, i, result_row_entry);
					strcat(command_result, result_row_entry);
					break;
				}
				p_HE++;
			}
			p_device_name = strtok(NULL, " ");
		}
	}

	return 0;
}





int htxd_get_common_command_result_all_active_ecg(int mode, char *device_list, char *command_result)
{
	htxd *htxd_instance;
	htxd_ecg_info * p_ecg_info_list;
	int return_code = 0;


	htxd_instance = htxd_get_instance();

	p_ecg_info_list = htxd_get_ecg_info_list(htxd_instance->p_ecg_manager);

	while(p_ecg_info_list != NULL) {
		return_code = htxd_get_common_command_result_ecg(mode, p_ecg_info_list, device_list, command_result);
		p_ecg_info_list = p_ecg_info_list->ecg_info_next;
	}

	return return_code;
}




int htxd_get_common_command_result(int mode, htxd_ecg_info *p_ecg_info, char *option_list, char *command_result)
{
	int return_code = 0;


	strcpy(command_result, "State   Dev     Adapt Desc   Device Desc         Slot Port\n");
	strcat(command_result, "-----------------------------------------------------------");

	if(p_ecg_info == NULL) {
		htxd_get_common_command_result_all_active_ecg(mode, option_list, command_result);
	} else {
		htxd_get_common_command_result_ecg(mode, p_ecg_info, option_list, command_result);	
	}
	return return_code;
}

