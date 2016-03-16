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

extern int run_cmd(char *, char *, int *, char *);
extern int load_exerciser(struct htxshm_HE *, struct sigaction *);
/*extern int process_mdt_to_shm (char *, int , union shm_pointers shm_point, char **, CFG__SFT * );*/
extern int cmprnset_dup_name (struct htxshm_HE *);
extern int add_to_ecg_all(void);
extern char reg_exp[10];


/*
 ***  Externals global to this module **************************************
 */

  extern  char   *default_mdt_snz;    /* ptr to default mdt stanza    */

int A_device(char *msg_text)
{
    boolean_t	confirm_errmsg;
    CFG__SFT *mdt_fd = NULL;           /* pointer to MDT attribute file*/

    char	AIX_logical_device_name[32];
    char	HTX_logical_device_name[32];
    char	*MDT_filename = NULL;
    char	cmd_string[PATH_MAX];	/* command to pass to shell */
    char	*htx_mkmdt_str;			/* command to pass to shell from environment */
    char	msg[81];				/* message line message text buffer */
    char	*p_default_snz;
    char	*p_msg;
    char	*p_temp_filename = NULL; /* pointer to temp_filename if file is open */
    char	reg_expr[256];
    char	stanza[4096];               /* attribute file stanza        */
    char	tag[40];
    char	temp_filename[L_tmpnam]; /* name of temporary file to hold awk output */
    char   save_devid[32];

    int	cfg_rc;		/* return from process_mdt_to_shm */
    int	ct_HE_started;	/* number of HE's actually spawned     */
    int	ct_stanza;	/* number of stanzas found in MDT file */
    int	i;
    int	prev_max_wait_tm;
    int	rc;
    int	semval;	
    int	workint, prev_ecg_pos, dup_num;
    int tmp_cnt, num_devs_to_add;

    size_t	AIX_ldn_length;		/* length of AIX logical device name */

    union shm_pointers shm_point;
    struct htxshm_HE  *p_shm_HE_base, tmp_HE_entry, *p_added_shm_HE ;
    struct sigaction sigvector;          /* structure for signal specs */

    unsigned int	max_entries, prev_max_entries;	/* local copy */

  /**  static storage  *******************************************************/
  /* menu items */
    static char	add_name[26];	/* device to add or MDT file name */
    //static enum t_add_method	addMethod = AD_Method_Default;
    static enum t_start_now 	startNow = AD_Start_Default;
    static boolean_t			forceRedef = FALSE;
    DBTRACE(DBENTRY,("enter A_device.c A_device\n"));

  /***************************************************************************/
  /**  beginning of executable code  *****************************************/
  /***************************************************************************/
  /**  Do things that only get done once  ************************************/
  /***************************************************************************/
    shm_point.hdr_addr = (struct htxshm_hdr *)&tmp_HE_entry;

    p_shm_HE_base = ((struct htxshm_HE *)
		     ((struct htxshm_hdr  *) ECGSHMADDR_HDR + 1)); /* skip over header */

   /******  Set signals to SIG_DFL  ****************************/
   /******  for the newly restarted HE's  **********************/
    sigvector.sa_handler = SIG_DFL; /* set default action                  */
    sigemptyset(&(sigvector.sa_mask));  /* do not block other signals      */
    sigvector.sa_flags = 0;         /* No special flags                    */

	/****** set file descriptor to close on exec ******/
    (void) fcntl(fileno(stdin), F_SETFD, 1);  /* stdin to close on exec  */
    (void) fcntl(fileno(stdout), F_SETFD, 1); /* stdout to close on exec */
    (void) fcntl(fileno(stderr), F_SETFD, 1); /* stderr to close on exec */

	/*** init menu stuff ***/
    strncpy(add_name, "", DIM(add_name));	 /* empty name field */
    p_msg = NULL;		/* no message */
    confirm_errmsg = FALSE;
    prev_max_entries = max_entries = ECGSHMADDR_HDR->max_entries; /* init to current value */
    prev_ecg_pos = cur_ecg_pos;
    cur_ecg_pos = 0;

    if (mdt_fd != (CFG__SFT *) NULL) {	/* close MDT cfg file */
	if (cfgcclsf(mdt_fd) != CFG_SUCC) {
	   print_log(LOGERR,"ERROR: Unable to close device MDT file."); fflush(stdout);
	    (void) sprintf(msg_text, "ERROR: Unable to close device MDT file.");
	    (void) send_message(msg_text, 0, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
	    DBTRACE(DBEXIT,("return/a -1 A_device.c A_device\n"));
	    return -1;	/* fatal programming error, bail out */
	} /* endif */
	else mdt_fd = NULL;
    } /* end if */

    if (MDT_filename != NULL) {
	free(MDT_filename);
	MDT_filename = NULL;
    } /* if */

    if (p_temp_filename != NULL) { /* remove temporary file */
	(void) remove(temp_filename);  /* cleanup temp file */
	p_temp_filename = NULL;
    } /* if */

	/*************************************************************************/
	/**  Display menu and get operator request  ******************************/
	/*************************************************************************/

    strcpy(add_name,(char *)reg_exp);
   print_log(LOGMSG,"add_name = %s\n",add_name);
    fflush(stdout);

    p_msg = NULL;		/* init message pointer */

    AIX_logical_device_name[0]=' ';

		/* Generate the MDT stanza if add method is by device name. */

    (void) strcpy(AIX_logical_device_name, add_name);
      /* Get the temp file name.  The pointer to it is used to clean up later.  Save the name in malloc'd memory. */
    p_temp_filename = tmpnam(temp_filename);	
    MDT_filename = malloc(strlen(temp_filename) + 1);
    (void) strcpy(MDT_filename, temp_filename);

		/*	Special case device name for Tigershark PCI adapter name. Transforms ddriciopNN to ddricioNN */
    AIX_ldn_length = strlen(AIX_logical_device_name);
    (void) strcpy(HTX_logical_device_name, AIX_logical_device_name);

    if (AIX_ldn_length > 8) {
	if ( 0 == strncmp(AIX_logical_device_name, "ddriciop", 8) )
	    strcpy(&HTX_logical_device_name[7], strpbrk(&AIX_logical_device_name[8],"0123456789"));
    }
      /* Format the command to generate the MDT file.
		   If HTX_MKMDT is set, use its value to generate the MDT stanza.  */

    htx_mkmdt_str = getenv("HTX_MKMDT");

    if (htx_mkmdt_str != NULL) {
	(void) sprintf(cmd_string, "%s %s", htx_mkmdt_str, AIX_logical_device_name);
	(void) sprintf(cmd_string, "%s > %s 2>&1" , cmd_string, MDT_filename);
    } else {	/* use standard command */ 
	(void) sprintf(cmd_string, 
		       "lsdev -C -Sa -l %s -F 'status name type location class subclass parent' | /usr/lpp/htx/etc/scripts/htxconf.awk 1>/tmp/htxconf.op 2>/dev/null; /usr/lpp/htx/etc/scripts/htxselect.pl %s > %s 2>&1" ,
		       AIX_logical_device_name, HTX_logical_device_name,MDT_filename);
    }


      /* Run the command, dont care about getting output.  */
    rc =  run_cmd(cmd_string, (char *) NULL, &workint, MDT_filename);

      /* if grep returns 1, assume that it did not find the device stanza */
    if (rc == 1) {
	sprintf(msg, "Device %s not found.", AIX_logical_device_name);
	sprintf(msg_text, "Device %s not found.", AIX_logical_device_name);
	p_msg = msg;
	if (MDT_filename != NULL) {
	    free(MDT_filename);
	    MDT_filename = NULL;
	} /* if */
	DBTRACE(DBEXIT,("return/b -1 A_device.c A_device\n"));
	return -1;
    } /* endif */

    if (rc != 0) {
	(void) sprintf(msg, "Problem generating the MDT for this device.  Not added.");
	p_msg = msg;
	if (MDT_filename != NULL) {
	    free(MDT_filename);
	    MDT_filename = NULL;
	} /* if */
	DBTRACE(DBEXIT,("return/c -1 A_device.c A_device\n"));
	return -1;
    } /* endif */

     /* Open the MDT attribute file.  */
    mdt_fd = cfgcopsf(MDT_filename);
    if (mdt_fd == (CFG__SFT *) NULL) {
	print_log(LOGERR,"Unable to open MDT file %s", MDT_filename);
	fflush(stdout);
	(void) sprintf(msg_text, "Unable to open MDT file %s", MDT_filename);
	p_msg = msg;
	if (MDT_filename != NULL) {
	    free(MDT_filename);
	    MDT_filename = NULL;
	} /* if */
	DBTRACE(DBEXIT,("return/d -1 A_device.c A_device\n"));
	return -1;
    } /* endif */
   print_log(LOGMSG,"opened MDT file %s", MDT_filename);
    fflush(stdout);


		/* Fill in the temporary HE entry */
    ct_stanza = ct_HE_started = 0;

		/* setup for first call to process_mdt_to_shm	*/
    p_default_snz = (char *) &default_mdt_snz;	

     /*	EOF is normal exit when processing user specified MDT and
				at least one stanza was found in it.  */

    do {
	print_log(LOGMSG,"In do loop\n");
	bzero(&tmp_HE_entry, sizeof(tmp_HE_entry));

	cfg_rc = process_mdt_to_shm(stanza, sizeof(stanza), shm_point, &p_default_snz, mdt_fd);
	print_log(LOGMSG,"done process_mdt\n");

	if (cfg_rc != CFG_SUCC) { /* not OK ?   */

	   print_log(LOGERR,"ERROR: MDT file read failure CFG_");
	    (void) sprintf(msg_text, "ERROR: MDT file read failure CFG_");
	    switch (cfg_rc) {
		case CFG_EOF:             /* empty file             */
		    (void) sprintf(tag, "EOF");
		    break;
		case CFG_SZNF:            /* req stanza not found   */
		    (void) sprintf(tag, "SZNF");
		    break;
		case CFG_SZBF:            /* stanza too long        */
		    (void) sprintf(tag, "SZBF");
		    break;
		case CFG_UNIO:            /*  I/O error             */
		    (void) sprintf(tag, "UNIO");
		    break;
		default:                  /* unexpected error       */
		    (void) sprintf(tag, "????");
		    break;
	    } /* endswitch */
	    (void) strcat(msg_text, tag);
	    (void) sprintf(tag, ", rc = %d.", cfg_rc);
	   print_log(LOGMSG,"rc = %d.", cfg_rc);
	    p_msg = strcat(msg_text, tag);
	    (void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
	    if (MDT_filename != NULL) {
		free(MDT_filename);
		MDT_filename = NULL;
	    } /* if */
	    DBTRACE(DBEXIT,("return/e -1 A_device.c A_device\n"));
	    return -1;
	} /* endif */
	else ct_stanza++;

	if (!is_cmdline) {
	    forceRedef = (msg_rcv.indx)&0xFF;
	    startNow = (msg_rcv.indx>>8)&0xFF;
	    num_devs_to_add = msg_rcv.subcmd;
	    if (startNow == 2)
		startNow = 4;
	}
	else {
	   print_log(LOGMSG,"Parameters are. num_add_force = %d add_force = %s\n", num_add_force,add_force);
	   print_log(LOGMSG,"Parameters are. num_add_ah = %d add_ah = %s\n", num_add_ah,add_ah);
	   print_log(LOGMSG,"Parameters are. num_add_coe = %d add_coe = %s\n", num_add_coe,add_coe);
	   print_log(LOGMSG,"Parameters are. num_add_dup = %d add_dup = %s\n", num_add_dup,add_dup);

	    if ((num_add_force>0) && (strcmp(add_force,"y") == 0))
		forceRedef = 1;
	    else
		forceRedef = 0;

	   print_log(LOGMSG,"add_ah = :%s: num_add_ah = %d\n", add_ah, num_add_ah);
	    if ((num_add_ah>0) && (strcmp(add_ah,"y") == 0))
		startNow = 0;
	    else
		startNow = 1;

	    if ((num_add_coe>0) && (strcmp(add_coe,"y") == 0))
		tmp_HE_entry.cont_on_err = 1;
	    else
		tmp_HE_entry.cont_on_err = 0;

	    if (num_add_dup>0) 
		num_devs_to_add = atoi(add_dup);
	}
	strcpy(save_devid,tmp_HE_entry.sdev_id);
	tmp_HE_entry.dup_device = ((tmp_HE_entry.dup_device)||(num_devs_to_add>0))?1:0;

	if (num_devs_to_add <= 0)
	    num_devs_to_add = 1;
	for(tmp_cnt=0; tmp_cnt<num_devs_to_add; tmp_cnt++) {
	    if ( tmp_HE_entry.dup_device ) { // If dup_device is set, just add the device.
	    strcpy(tmp_HE_entry.sdev_id, save_devid);
	    cur_ecg_pos = prev_ecg_pos;
	    if ( ECG_MAX_ENTRIES >= ECGSHMADDR_HDR->pseudo_entry_0) {  /* room left? */
		(void) sprintf(msg_text, "No room left in %s/%s.", ECGPATH,ECGNAME);
		p_msg = msg_text;
		if (MDT_filename != NULL) {
		    free(MDT_filename);
		    MDT_filename = NULL;
		} /* if */
		DBTRACE(DBEXIT,("return/f -4 A_device.c A_device\n"));
		return -4;
	    }
	    p_added_shm_HE = ((struct htxshm_HE *) ((struct htxshm_hdr *) ECGSHMADDR_HDR +1)) + ECG_MAX_ENTRIES;
	   print_log(LOGMSG,"Doing cmprnset\n"); fflush(stdout);
	    dup_num = cmprnset_dup_name(&tmp_HE_entry);
	   print_log(LOGMSG,"Done from cmprnset\n"); fflush(stdout);
	    if ( dup_num < 0 ) {
		if (MDT_filename != NULL) {
		    free(MDT_filename);
		    MDT_filename = NULL;
		} /* if */
		DBTRACE(DBEXIT,("return/g -1 A_device.c A_device\n"));
		return -1;
	    }
	    max_entries++;
	    bcopy(&tmp_HE_entry, (char *) p_added_shm_HE, sizeof(tmp_HE_entry));
	   print_log(LOGMSG,"done bcopy  \n"); fflush(stdout);
	    ECGEXER_ADDR(ECG_MAX_ENTRIES) = p_added_shm_HE;
	    ECGEXER_SEMID(ECG_MAX_ENTRIES) = ECGSEMHEID;
	    ECGEXER_POS(ECG_MAX_ENTRIES) =  ECG_MAX_ENTRIES;
	    ECGEXER_ECGPOS(ECG_MAX_ENTRIES) =  cur_ecg_pos;
	    ECGEXER_HDR(ECG_MAX_ENTRIES) = ECGSHMADDR_HDR;
	    ECGEXER_SHMKEY(ECG_MAX_ENTRIES) = ECGSHMKEY;
	    ECGEXER_SEMKEY(ECG_MAX_ENTRIES) = ECGSEMKEY;
	    strcpy(EXER_NAME(ECG_MAX_ENTRIES), (p_added_shm_HE)->sdev_id);
	   print_log(LOGMSG,"Adding to ecgall\n"); fflush(stdout);
	    i = add_to_ecg_all();
	   print_log(LOGMSG,"added to ecg all\n"); fflush(stdout);
	    } 
	    else {   // dup_device is not set, take care while adding it
		for (i = 0; i < ECG_MAX_ENTRIES; i++) {  // look for the matching device name
		    if (!ECGEXER_ADDR(i))
			continue;
		if (0 == (strcmp(tmp_HE_entry.sdev_id, 
				 (ECGEXER_ADDR(i))->sdev_id))) 
		    break;	
		} /* for ECG_MAX_ENTRIES */

	    if (i >= ECG_MAX_ENTRIES) { 	/* Device name not found? */
		cur_ecg_pos = prev_ecg_pos; //Now change the ecg to the requested one.
		  if ( ECG_MAX_ENTRIES < ECGSHMADDR_HDR->pseudo_entry_0) {  /* room left? */
		      cur_ecg_pos = prev_ecg_pos; //Now change the ecg to the requested one.
			p_added_shm_HE = ((struct htxshm_HE *) ((struct htxshm_hdr *) ECGSHMADDR_HDR +1)) + ECG_MAX_ENTRIES;
		      max_entries++;
		      bcopy(&tmp_HE_entry, (char *) p_added_shm_HE, sizeof(tmp_HE_entry));
		     print_log(LOGMSG,"done bcopy  \n"); fflush(stdout);
		      ECGEXER_ADDR(ECG_MAX_ENTRIES) = p_added_shm_HE;
		      ECGEXER_SEMID(ECG_MAX_ENTRIES) = ECGSEMHEID;
		      ECGEXER_POS(ECG_MAX_ENTRIES) =  ECG_MAX_ENTRIES;
		      ECGEXER_ECGPOS(ECG_MAX_ENTRIES) =  cur_ecg_pos;
		      ECGEXER_HDR(ECG_MAX_ENTRIES) = ECGSHMADDR_HDR;
		      ECGEXER_SHMKEY(ECG_MAX_ENTRIES) = ECGSHMKEY;
		      ECGEXER_SEMKEY(ECG_MAX_ENTRIES) = ECGSEMKEY;
		      strcpy(EXER_NAME(ECG_MAX_ENTRIES), (p_added_shm_HE)->sdev_id);
		     print_log(LOGMSG,"Adding to ecgall\n"); fflush(stdout);
		      i = add_to_ecg_all();
		     print_log(LOGMSG,"added to ecg all\n"); fflush(stdout);
		  } 
		  else {  /* Can't add any more new ones */
		     print_log(LOGMSG,"All %d entries in shared memory allocated for new devices are used."
			     "\nIncrease \"max_added_devices\" in /usr/lpp/htx/.htx_profile and restart HTX", MAX_ADDED_DEVICES);
		      (void) sprintf(msg_text, "All %d entries in shared memory allocated for new devices are used."
				     "\nIncrease \"max_added_devices\" in /usr/lpp/htx/.htx_profile and restart HTX", MAX_ADDED_DEVICES);
		      (void) send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);
		      (void) sprintf(msg_text, "No room for new devices, see /tmp/htxmsg");
		     print_log(LOGMSG,"No room for new devices, see /tmp/htxmsg");
		      fflush(stdout);
		      (void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
		      p_msg = strcpy(msg, msg_text);
		      strncpy(add_name, "", DIM(add_name)); /* empty name field */
		      if (MDT_filename != NULL) {
			  free(MDT_filename);
			  MDT_filename = NULL;
		      } /* if */
		      DBTRACE(DBEXIT,("return/h -1 A_device.c A_device\n"));
		      return -1;
		  } /* else */
	    } /* if i >= max_entries */
	    else {	/* over writing existing device */
		p_added_shm_HE = ECGEXER_ADDR(i);
		cur_ecg_pos = ecg_info[0].exer_list[i].parent_ecg_pos;
		print_log(LOGMSG,"ECGNAME = %s/%s\n", ECGPATH,ECGNAME); fflush(stdout);
		bcopy(&tmp_HE_entry, (char *) p_added_shm_HE, sizeof(tmp_HE_entry));
		if (forceRedef) {
		    if (p_added_shm_HE->PID != 0) {	/* old HE must be deceased */
			print_log(LOGMSG,"Device %s lives!  Terminate the exerciser before redefining it.", tmp_HE_entry.sdev_id);
			fflush(stdout);
			(void) sprintf(msg_text, "Device %s lives!  Terminate the exerciser before redefining it.", tmp_HE_entry.sdev_id);
			p_msg = msg_text;
			if (MDT_filename != NULL) {
			    free(MDT_filename);
			    MDT_filename = NULL;
			} /* if */
			DBTRACE(DBEXIT,("return/i -2 A_device.c A_device\n"));
			return -2;
		    } /* if */
		    if (prev_ecg_pos != cur_ecg_pos) {
			(void) sprintf(msg_text, "Device %s belongs to %s/%s, please specify %s in option 1, "
				       "or set dup_device as TRUE using option 5.", p_added_shm_HE->sdev_id, ECGPATH,ECGNAME, ECGNAME);
			p_msg = msg_text;
			if (MDT_filename != NULL) {
			    free(MDT_filename);
			    MDT_filename = NULL;
			} /* if */
			DBTRACE(DBEXIT,("return/j -2 A_device.c A_device\n"));
			return -2;
		    }
		} /* if */
		else {	/* dont force redefinition */
		   print_log(LOGMSG,"Device %s already defined, toggle option 4 to override.", tmp_HE_entry.sdev_id);
		    fflush(stdout);
		    (void) sprintf(msg_text, "Device %s already defined, toggle option 4 to override.", tmp_HE_entry.sdev_id);
		    p_msg = msg_text;
		    if (MDT_filename != NULL) {
			free(MDT_filename);
			MDT_filename = NULL;
		    } /* if */
		    DBTRACE(DBEXIT,("return/k -3 A_device.c A_device\n"));
		    return -3;
		} /* else */
	    } /* else */
	    }

			/*****************************************************************/
			/**  Ok, the entry is good, copy it to shared memory  ************/
			/**  and initialize  the semaphores.  ****************************/
			/*****************************************************************/

			/*
			 *	Initialize the new HE semaphores.
			 */
	    switch (startNow) {
		case AD_Start_Active: 	semval = 0; break;
		case AD_Start_Halted: 	semval = 1; break;
		case AD_Start_Default: 	semval = p_added_shm_HE->start_halted; 
		    break;
	    } /* switch */

	 /*prev_ecg_pos = cur_ecg_pos;
	 cur_ecg_pos = 0;
			SEMCTL(ECGEXER_SEMID(i), ((ECGEXER_POS(i) * SEM_PER_EXER) + SEM_GLOBAL), SETVAL, semval);
	 cur_ecg_pos = prev_ecg_pos;*/
	    SEMCTL(ECGEXER_SEMID(ECG_MAX_ENTRIES), ((ECGEXER_POS(ECG_MAX_ENTRIES) * SEM_PER_EXER) + SEM_GLOBAL), SETVAL, semval);

			/* Setup to run setup/runsetup scripts and spawn exercisers.  */
	   print_log(LOGMSG,"Adding %s by operator request.", tmp_HE_entry.sdev_id);
	    fflush(stdout);
	    (void) sprintf(msg_text,"Adding %s by operator request.",
			   tmp_HE_entry.sdev_id);
	    (void) send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);

			/* *	Run the scripts */
	    strcpy(reg_expr,"^");
	    strcat(reg_expr,p_added_shm_HE->HE_name);
	    strcat(reg_expr, ".*setup[\t ]*$");
	    rc = exec_HE_script(reg_expr, p_added_shm_HE->sdev_id, 
				&confirm_errmsg);
	    if (rc < 0) { 	/* could not find out what to run or could not run it */
		print_log(LOGERR,"WARNING: Failed running setup script(s) for %s",tmp_HE_entry.sdev_id);
		(void) sprintf(msg_text, "WARNING: Failed running setup script(s) for %s",tmp_HE_entry.sdev_id);
		//PRTMSG((MSGLINE-1), 0, (msg_text));
		p_msg = strcpy (msg, msg_text);
		(void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
	    } /* endif */
	   print_log(LOGMSG,"execed the script %s\n", p_added_shm_HE->sdev_id);
	    fflush(stdout);


			/*
			 *	Set counters to reflect presence of the added device.
			 * 	This commits the new shared memory entry.
			 */

			/* check/reset maximum time allowed on a semop system wait */
	    prev_max_wait_tm = max_wait_tm;
	    workint = (p_added_shm_HE->max_run_tm * 2) +
	      p_added_shm_HE->idle_time;
	    if (workint > max_wait_tm) { /* new maximum wait time?          */
		max_wait_tm = workint;
	    } /* endif */

			/* increment counters if device add was sucessfull	*/
	    ECGSHMADDR_HDR->num_entries++;
	    ECGSHMADDR_HDR->max_entries = max_entries;
	    ECG_MAX_ENTRIES = max_entries;

	    p_added_shm_HE->tm_last_upd = -1;

	   print_log(LOGMSG,"calling load_exerciser for id  %s\n",p_added_shm_HE->sdev_id);
	    rc = load_exerciser(p_added_shm_HE, &sigvector); /* fork & exec */
	    if (rc != 0) {
				/* restore counters */
		max_wait_tm = prev_max_wait_tm;
		ECGSHMADDR_HDR->num_entries--;
		ECGSHMADDR_HDR->max_entries = prev_max_entries;
		ECG_MAX_ENTRIES= prev_max_entries;

		print_log(LOGERR,"ERROR: Failed to start %s. ", 
		       p_added_shm_HE->HE_name);
		(void) sprintf(msg_text, "ERROR: Failed to start %s. ", 
			       p_added_shm_HE->HE_name);

				/* setup lingering message for user */
		p_msg = strcpy(msg, msg_text);
		(void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
		if (MDT_filename != NULL) {
		    free(MDT_filename);
		    MDT_filename = NULL;
		} /* if */
		DBTRACE(DBEXIT,("return/l -1 A_device.c A_device\n"));
		return -1;
	    } /* endif */

	    ct_HE_started++;

			/*
			 *	Set successful completion message and log in htxmsg.
			 */
	    p_msg = msg;
	    (void) sprintf(msg, "Started /dev/%s",p_added_shm_HE->sdev_id);
	    (void) sprintf(msg_text, "Addition or replacement of exerciser for %s completed sucessfully.", tmp_HE_entry.sdev_id);
	   print_log(LOGMSG,"Addition or replacement of exerciser for %s completed sucessfully.", tmp_HE_entry.sdev_id);

	    (void) send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);

			/* clear device/file name for next entry */
	    strncpy(add_name, "", DIM(add_name)); /* empty name field */
	}

	break;
    } while (1);

	/*
	 *	Cleanup 
	 */

    (void) fcntl(fileno(stdin), F_SETFD, 0); /* stdin NOT to close on exec */
    (void) fcntl(fileno(stdout), F_SETFD, 0); /*stdout NOT to clse on exec */
    (void) fcntl(fileno(stderr), F_SETFD, 0); /*stderr NOT to clse on exec */

    DBTRACE(DBEXIT,("return 0 A_device.c A_device\n"));
    return 0;
} /* A_device */
/* vi:set ts=4: */

/***************************************************/
/*		Function: reconfig_restart							*/
/***************************************************/

void reconfig_restart(char *mdtfilename)
{
    struct htxshm_HE  *p_shm_HE_base, tmp_HE_entry, *p_added_shm_HE ;
    struct sigaction sigvector;          /* structure for signal specs */

    unsigned int  max_entries, prev_max_entries;  /* local copy */

    char MDT_filename[128], msg_text[MSG_TEXT_SIZE], msg[81];
    char  *p_default_snz;
    char  reg_expr[256];
    char  stanza[4096];               /* attribute file stanza        */
    char  tag[40];

    int cfg_rc;		/* return from process_mdt_to_shm */
    int ct_HE_started;	/* number of HE's actually spawned     */
    int ct_stanza;	/* number of stanzas found in MDT file */
    int i;
    int prev_max_wait_tm;
    int rc;
    int semval;
    int workint;

    boolean_t confirm_errmsg;
    CFG__SFT *mdt_fd = NULL;
    DBTRACE(DBENTRY,("enter A_device.c reconfig_restart\n"));

	/****** set file descriptor to close on exec ******/
    (void) fcntl(fileno(stdin), F_SETFD, 1);  /* stdin to close on exec  */
    (void) fcntl(fileno(stdout), F_SETFD, 1); /* stdout to close on exec */
    (void) fcntl(fileno(stderr), F_SETFD, 1); /* stderr to close on exec */

    sprintf(msg_text, "reconfig restart called for MDT file %s %d\n",mdtfilename, strlen(mdtfilename));
    send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);

    p_shm_HE_base = ((struct htxshm_HE *)
		     ((struct htxshm_hdr  *) shm_addr.hdr_addr + 1)); /* skip over header */

    sigvector.sa_handler = SIG_DFL; /* set default action                  */
    sigemptyset(&(sigvector.sa_mask));  /* do not block other signals      */
    sigvector.sa_flags = 0;         /* No special flags                    */

    prev_max_entries = max_entries = shm_addr.hdr_addr->max_entries;

    (void) strcpy(MDT_filename, HTXPATH);
    (void) strcat(MDT_filename, "mdt/");
    (void) strcat(MDT_filename, mdtfilename);

    mdt_fd = cfgcopsf(MDT_filename);
    if (mdt_fd == (CFG__SFT *) NULL)
    {
	(void) sprintf(msg_text, "Unable to open MDT file %s",
		       MDT_filename);

	send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
	DBTRACE(DBEXIT,("return/a A_device.c reconfig_restart\n"));
	return;
    }

    ct_stanza = ct_HE_started = 0;
    p_default_snz = (char *) &default_mdt_snz;

    do {
	bzero(&tmp_HE_entry, sizeof(tmp_HE_entry));

	cfg_rc = process_mdt_to_shm(stanza, sizeof(stanza), &tmp_HE_entry,
				    &p_default_snz, mdt_fd);

	if (cfg_rc != CFG_SUCC) {
	    if (cfg_rc == CFG_EOF && ct_stanza > 0) {
		continue;
	    }
	    else {
		(void) sprintf(msg_text, "ERROR: MDT file read failure ");
		switch (cfg_rc)
		{
		    case CFG_EOF:             /* empty file             */
			(void) sprintf(tag, "CFG_EOF");
			break;
		    case CFG_SZNF:            /* req stanza not found   */
			(void) sprintf(tag, "CFG_SZNF");
			break;
		    case CFG_SZBF:            /* stanza too long        */
			(void) sprintf(tag, "CFG_SZBF");
			break;
		    case CFG_UNIO:            /*  I/O error             */
			(void) sprintf(tag, "CFG_UNIO");
			break;
		    default:                  /* unexpected error       */
			(void) sprintf(tag, "CFG_????");
			break;
		} /* endswitch */
		(void) strcat(msg_text, tag);
		(void) sprintf(tag, ", rc = %d.", cfg_rc);
		(void) strcat(msg_text, tag);
		(void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
		continue;
	    }
	} /* end if */
	else ct_stanza++;

	for (i = 0; i < max_entries; i++) {
	    if (0 == (strcmp(tmp_HE_entry.sdev_id, (p_shm_HE_base + i)->sdev_id)))
		break;
	}

	if ((i<max_entries) && (shm_addr.hdr_addr->started == 0))
	{
	    sprintf(msg_text,"System not started. Device %s already there(%d, %d)\n",
		    tmp_HE_entry.sdev_id, shm_addr.hdr_addr->num_entries, shm_addr.hdr_addr->max_entries);
	    send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	    continue;
	}

	p_added_shm_HE = (p_shm_HE_base + i); /* point to entry for new dev */

	if (i >= max_entries) {
	    if ( max_entries < shm_addr.hdr_addr->pseudo_entry_0) {
		prev_max_entries = max_entries;
		max_entries++;
	    }
	    else {
		(void) sprintf(msg_text,
			       "All %d entries in shared memory allocated for new devices are used.\nIncrease\"max_added_devices\" in /usr/lpp/htx/.htx_profile and restart HTX",
			       MAX_ADDED_DEVICES);
		(void) send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);
		(void) sprintf(msg_text,
			       "No room for new devices, see /tmp/htxmsg");
		(void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR,
				    HTX_SYS_MSG);
		continue;
	    }
	}
	else {
	    if (p_added_shm_HE->PID != 0) {
		(void) sprintf(msg_text, "Attempt to restart active device %s", tmp_HE_entry.sdev_id);
		(void) send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);
		continue;
	    }
	}

	bcopy(&tmp_HE_entry, (char *) p_added_shm_HE, sizeof(tmp_HE_entry));
	semval = 0;
	SEMCTL(semhe_id, ((i * SEM_PER_EXER) + SEM_GLOBAL), SETVAL, semval);

	(void) sprintf(msg_text,"Adding %s by operator request for DR.", tmp_HE_entry.sdev_id);
	(void) send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);

	if (shm_addr.hdr_addr->started != 0) {
	    strcpy(reg_expr,"^");
	    strcat(reg_expr,p_added_shm_HE->HE_name);
	    strcat(reg_expr, ".*setup[\t ]*$");
	    rc = exec_HE_script(reg_expr, p_added_shm_HE->sdev_id, &confirm_errmsg);
	    if (rc < 0) { 
		(void) sprintf(msg_text, "WARNING: Failed running setup script(s) for %s",tmp_HE_entry.sdev_id);
		(void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
	    }

	    prev_max_wait_tm = max_wait_tm;
	    workint = (p_added_shm_HE->max_run_tm * 2) + p_added_shm_HE->idle_time;
	    if (workint > max_wait_tm) max_wait_tm = workint;

	    shm_addr.hdr_addr->num_entries++;
	    shm_addr.hdr_addr->max_entries = max_entries;

	    p_added_shm_HE->tm_last_upd = -1;

	    rc = load_exerciser(p_added_shm_HE, &sigvector);
	    if (rc != 0) {
		max_wait_tm = prev_max_wait_tm;
		shm_addr.hdr_addr->num_entries--;
		shm_addr.hdr_addr->max_entries = max_entries = prev_max_entries;

		(void) sprintf(msg_text, "ERROR: Failed to start %s. ", p_added_shm_HE->HE_name);

		(void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
		continue;
	    }

/*		while (p_added_shm_HE->tm_last_upd < 0); */
	} /* for case where HTX no yet in RUNNING state */
	else
	{
	    prev_max_wait_tm = max_wait_tm;
	    workint = (p_added_shm_HE->max_run_tm * 2) + p_added_shm_HE->idle_time;
	    if (workint > max_wait_tm) max_wait_tm = workint;

	    shm_addr.hdr_addr->num_entries++;
	    shm_addr.hdr_addr->max_entries = max_entries;

	    p_added_shm_HE->tm_last_upd = -1;
	}

	ct_HE_started++;

	(void) sprintf(msg, "Started /dev/%s",p_added_shm_HE->sdev_id);
	(void) sprintf(msg_text, "Addition or replacement of exerciser for %s completed sucessfully.", tmp_HE_entry.sdev_id);
	(void) send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);

    } while (ct_stanza > 0 && cfg_rc == CFG_SUCC);/* end do-while */

  /* close MDT cfg file */
    if (cfgcclsf(mdt_fd) != CFG_SUCC)
    {
	//PRTMSG(0, 0, ("ERROR: Unable to close MDT file in DR."));
	print_log(LOGERR,"ERROR: Unable to close MDT file in DR.");
	sprintf(msg_text, "Unable to close MDT file in DR. ");
	(void) send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);
    } /* endif */

    (void) fcntl(fileno(stdin), F_SETFD, 0); /* stdin NOT to close on exec */
    (void) fcntl(fileno(stdout), F_SETFD, 0); /*stdout NOT to clse on exec */
    (void) fcntl(fileno(stderr), F_SETFD, 0); /*stderr NOT to clse on exec */

    DBTRACE(DBEXIT,("return/b A_device.c reconfig_restart\n"));
    return;
}
