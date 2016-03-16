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


#include <string.h>
#include "eservd.h"
#include "global.h"
#include "cfgclibdef.h"

#ifndef __HTX_LINUX__
#define SEMCTL(x,y,z,a) (void)semctl(x,y,z,a)
#else
#define SEMCTL(x,y,z,a) semctl_arg.val=a, (void)semctl(x,y,z,semctl_arg)
#endif

extern int cmprnset_dup_name (struct htxshm_HE *my_exer);
extern int add_to_ecg_all (void);
/* extern thtx_message msg_rcv;   use definition from global.h */

int H_device(struct htxshm_HE add_HE, char *msg_text)
{
    char	msg[81];				/* message line message text buffer */
    char	*p_msg;

    int	i;
    int	dup_num=0;
    int	semval;	
    int	prev_cur_ecg_pos;

    struct htxshm_HE  tmp_HE_entry, *p_added_shm_HE ;
    struct sigaction sigvector;          /* structure for signal specs */

    unsigned int	max_entries, prev_max_entries;	/* local copy */

  /**  static storage  *******************************************************/
  /* menu items */
    static char	add_name[26];	/* device to add or MDT file name */
    char   save_devid[40];
    int    prev_ecg_pos;
    DBTRACE(DBENTRY,("enter H_device.c H_device\n"));

  /***************************************************************************/
  /**  beginning of executable code  *****************************************/


    /******  Set signals to SIG_DFL  ****************************/
    /******  for the newly restarted HE's  **********************/
    sigvector.sa_handler = SIG_DFL; /* set default action                  */
    sigemptyset(&(sigvector.sa_mask));  /* do not block other signals      */
    sigvector.sa_flags = 0;         /* No special flags                    */

	/****** set file descriptor to close on exec ******/
    (void) fcntl(fileno(stdin), F_SETFD, 1);  /* stdin to close on exec  */
    (void) fcntl(fileno(stdout), F_SETFD, 1); /* stdout to close on exec */
    (void) fcntl(fileno(stderr), F_SETFD, 1); /* stderr to close on exec */

    p_msg = NULL;		/* init message pointer */

    prev_max_entries = max_entries = ECGSHMADDR_HDR->max_entries;
    prev_cur_ecg_pos = cur_ecg_pos;
    cur_ecg_pos = 0; //first we need to check in all the ecgs if the exerciser is already present
	//prev_max_entries = max_entries = ECG_MAX_ENTRIES;

    bzero(&tmp_HE_entry, sizeof(tmp_HE_entry));

    strcpy(tmp_HE_entry.HE_name, add_HE.HE_name);
    strcpy(tmp_HE_entry.sdev_id, add_HE.sdev_id);
    strcpy(tmp_HE_entry.adapt_desc, add_HE.adapt_desc);
    strcpy(tmp_HE_entry.device_desc, add_HE.device_desc);
    strcpy(tmp_HE_entry.reg_rules, add_HE.reg_rules);

    tmp_HE_entry.max_run_tm =  add_HE.max_run_tm;
    tmp_HE_entry.port =  add_HE.port;
    tmp_HE_entry.slot =  add_HE.slot;
    tmp_HE_entry.PID =  add_HE.PID;
    tmp_HE_entry.max_cycles =  add_HE.max_cycles;
    tmp_HE_entry.idle_time =  add_HE.idle_time;
    tmp_HE_entry.halt_sev_level =  add_HE.halt_sev_level;
    tmp_HE_entry.cont_on_err =  add_HE.cont_on_err;
    tmp_HE_entry.err_ack =  add_HE.cont_on_err;
    semval = tmp_HE_entry.start_halted =  add_HE.start_halted;
    tmp_HE_entry.dup_device =  add_HE.dup_device;
    tmp_HE_entry.log_vpd =  add_HE.log_vpd;

    print_log(LOGMSG,"HENAME =%s DEVID = %s RULES = %s PID  = %d\n",tmp_HE_entry.HE_name, tmp_HE_entry.sdev_id, tmp_HE_entry.reg_rules, tmp_HE_entry.PID);

    strcpy(save_devid, tmp_HE_entry.sdev_id);
    if (tmp_HE_entry.dup_device) {
	strcpy(tmp_HE_entry.sdev_id, save_devid);
	cur_ecg_pos = prev_ecg_pos;
	if (ECG_MAX_ENTRIES >= ECGSHMADDR_HDR->pseudo_entry_0) {
	    (void) sprintf(msg_text, "No room for new devices, see /tmp/htxmsg");
	    p_msg = msg_text;
	    DBTRACE(DBEXIT,("return/a -4 H_device.c H_device\n"));
	    return -4;
	}
	p_added_shm_HE = ((struct htxshm_HE *)  ((struct htxshm_hdr  *) ECGSHMADDR_HDR + 1)) + ECG_MAX_ENTRIES;
	print_log(LOGMSG,"H_device. doing cmprnset\n"); fflush(stdout);
	dup_num = cmprnset_dup_name(&tmp_HE_entry);
	if ( dup_num < 0 )
	{
	    DBTRACE(DBEXIT,("return/b -1 H_device.c H_device\n"));
	    return -1;
	}
	max_entries++;
	bcopy(&tmp_HE_entry, (char *) p_added_shm_HE, sizeof(tmp_HE_entry));
	print_log(LOGMSG,"cur_ecg_pos = %d ECG_MAX_ENT = %d\n",cur_ecg_pos, ECG_MAX_ENTRIES);
	print_log(LOGMSG,"TMP HENAME =%s DEVID = %s RULES = %s\n",tmp_HE_entry.HE_name, tmp_HE_entry.sdev_id, tmp_HE_entry.reg_rules);
	print_log(LOGMSG,"ADDED HENAME =%s DEVID = %s RULES = %s PID = %d\n",p_added_shm_HE->HE_name, p_added_shm_HE->sdev_id, p_added_shm_HE->reg_rules, p_added_shm_HE->PID);
	ECGEXER_ADDR(ECG_MAX_ENTRIES) = p_added_shm_HE;
	ECGEXER_ADDR(ECG_MAX_ENTRIES)->is_child = 0;
	ECGEXER_SEMID(ECG_MAX_ENTRIES) = ECGSEMHEID;
	ECGEXER_POS(ECG_MAX_ENTRIES) =  ECG_MAX_ENTRIES;
	ECGEXER_ECGPOS(ECG_MAX_ENTRIES) =  cur_ecg_pos;
	ECGEXER_HDR(ECG_MAX_ENTRIES) = ECGSHMADDR_HDR;
	ECGEXER_SHMKEY(ECG_MAX_ENTRIES) = ECGSHMKEY;
	ECGEXER_SEMKEY(ECG_MAX_ENTRIES) = ECGSEMKEY;
	EXER_PID(ECG_MAX_ENTRIES) = p_added_shm_HE->PID;
	ecg_info[0].exer_list[NUM_EXERS].exer_pid = p_added_shm_HE->PID;
	strcpy(EXER_NAME(ECG_MAX_ENTRIES), p_added_shm_HE->sdev_id);
	i = add_to_ecg_all();
    }
    else {
	for (i = 0; i < ECG_MAX_ENTRIES; i++) {
	    if (!ECGEXER_ADDR(i))
		continue;
	    if (0 == (strcmp(tmp_HE_entry.sdev_id, (ECGEXER_ADDR(i))->sdev_id))) 
		break;	
	} /* for */

	print_log(LOGERR,"Device name not found...we can plugin the new device\n");

	if (i >= ECG_MAX_ENTRIES) { 	/* Device name not found? */
	    cur_ecg_pos = prev_cur_ecg_pos; //Now change the ecg to the requested one.
	      if ( ECG_MAX_ENTRIES < ECGSHMADDR_HDR->pseudo_entry_0) {  /* room left? */
		  print_log(LOGMSG,"MAX_ENTRIES before adding to shm is %d\n", ECG_MAX_ENTRIES); fflush(stdout);
		  p_added_shm_HE = ((struct htxshm_HE *)  ((struct htxshm_hdr  *) ECGSHMADDR_HDR + 1)) + ECG_MAX_ENTRIES;
		  max_entries++;
		  bcopy(&tmp_HE_entry, (char *) p_added_shm_HE, sizeof(tmp_HE_entry));

		  print_log(LOGMSG,"cur_ecg_pos = %d ECG_MAX_ENT = %d\n",cur_ecg_pos, ECG_MAX_ENTRIES); fflush(stdout);
		  print_log(LOGMSG,"TMP HENAME =%s DEVID = %s RULES = %s\n",tmp_HE_entry.HE_name, tmp_HE_entry.sdev_id, tmp_HE_entry.reg_rules); fflush(stdout);
		  print_log(LOGMSG,"ADDED HENAME =%s DEVID = %s RULES = %s PID = %d\n",p_added_shm_HE->HE_name, p_added_shm_HE->sdev_id, p_added_shm_HE->reg_rules, p_added_shm_HE->PID); fflush(stdout);
		  ECGEXER_ADDR(ECG_MAX_ENTRIES) = p_added_shm_HE;
		  ECGEXER_ADDR(ECG_MAX_ENTRIES)->is_child = 0;
		  ECGEXER_SEMID(ECG_MAX_ENTRIES) = ECGSEMHEID;
		  ECGEXER_POS(ECG_MAX_ENTRIES) =  ECG_MAX_ENTRIES;
		  ECGEXER_ECGPOS(ECG_MAX_ENTRIES) =  cur_ecg_pos;
		  ECGEXER_HDR(ECG_MAX_ENTRIES) = ECGSHMADDR_HDR;
		  ECGEXER_SHMKEY(ECG_MAX_ENTRIES) = ECGSHMKEY;
		  ECGEXER_SEMKEY(ECG_MAX_ENTRIES) = ECGSEMKEY;
		  EXER_PID(ECG_MAX_ENTRIES) = p_added_shm_HE->PID;
		  ecg_info[0].exer_list[NUM_EXERS].exer_pid = p_added_shm_HE->PID;
		  strcpy(EXER_NAME(ECG_MAX_ENTRIES), p_added_shm_HE->sdev_id);
		  i = add_to_ecg_all();
	      }
	      else {  /* Can't add any more new ones */
		  print_log(LOGMSG,"All %d entries in shared memory allocated for new devices are used.\nIncrease \"max_added_devices\" in /usr/lpp/htx/.htx_profile and restart HTX", MAX_ADDED_DEVICES);
		  (void) sprintf(msg_text, "All %d entries in shared memory allocated for new devices are used.\nIncrease \"max_added_devices\" in /usr/lpp/htx/.htx_profile and restart HTX", MAX_ADDED_DEVICES);
		  (void) send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);
		  (void) sprintf(msg_text, "No room for new devices, see /tmp/htxmsg");
		  print_log(LOGERR,"No room for new devices, see /tmp/htxmsg");
		  fflush(stdout);
		  (void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
		  p_msg = strcpy(msg, msg_text);
		  strncpy(add_name, "", DIM(add_name)); /* empty name field */
		  DBTRACE(DBEXIT,("return/c -1 H_device.c H_device\n"));
		  return -1;
	      } /* else */
	} /* if i >= max_entries */
	else {
	    print_log(LOGERR,"Device %s already defined\n", tmp_HE_entry.sdev_id);
	    fflush(stdout);
	    (void) sprintf(msg_text, "Device %s already defined.", tmp_HE_entry.sdev_id);
	    p_msg = msg_text;
	    DBTRACE(DBEXIT,("return/d -3 H_device.c H_device\n"));
	    return -3;
	}
    }

    SEMCTL(ECGEXER_SEMID(ECG_MAX_ENTRIES), ((ECGEXER_POS(ECG_MAX_ENTRIES) * SEM_PER_EXER) + SEM_GLOBAL), SETVAL, semval);
    SEMCTL(ECGEXER_SEMID(ECG_MAX_ENTRIES), ((ECGEXER_POS(ECG_MAX_ENTRIES) * SEM_PER_EXER) + SEM_GLOBAL), SETVAL, semval);

    ECGSHMADDR_HDR->num_entries++;
    ECGSHMADDR_HDR->max_entries = max_entries;
    ECG_MAX_ENTRIES = max_entries;

    print_log(LOGMSG,"Number of entries now are ECG_MAX_ENTRIES = %d ECGSHMADDR_HDR->max_entries= %d\n",ECG_MAX_ENTRIES, ECGSHMADDR_HDR->max_entries);
    fflush(stdout);

    p_added_shm_HE->tm_last_upd = -1;

    p_msg = msg;
    (void) sprintf(msg, "Started /dev/%s",p_added_shm_HE->sdev_id);
    (void) sprintf(msg_text, "Addition or replacement of exerciser for %s completed sucessfully.", tmp_HE_entry.sdev_id);
    print_log(LOGMSG,"Addition or replacement of exerciser for %s completed sucessfully.", tmp_HE_entry.sdev_id);

    (void) send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);

	 /* clear device/file name for next entry */
    strncpy(add_name, "", DIM(add_name)); /* empty name field */

    (void) fcntl(fileno(stdin), F_SETFD, 0); /* stdin NOT to close on exec */
    (void) fcntl(fileno(stdout), F_SETFD, 0); /*stdout NOT to clse on exec */
    (void) fcntl(fileno(stderr), F_SETFD, 0); /*stderr NOT to clse on exec */

    DBTRACE(DBEXIT,("return H_device.c H_device\n"));
    return 0;
} /* H_device */
