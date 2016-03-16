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


#include "eservd.h"
#include "global.h"

#if defined(__OS400__)   /* 400 */
#include <spawn.h>
#include "libos400.h"
#include <qshell.h>
#define system QzshSystem
/*char * tempnam(char *, char *); */
extern  char  *regcmp(char *, char *);
extern  char  *regex(char *, char *);
#endif

#if defined(__HTX_LINUX__) || defined(__OS400__)   /* 400 */
#include <sys/wait.h>
#endif

extern int cur_ecg_pos;

/* extern thtx_message msg_rcv;   use definition from global.h */
extern char reg_exp[10];

#define	HE_SCRIPT_FILE	"hxsscripts"

static char	*script_name;	/* path name of script use by SIGARLM_handler */

/*
 ***  Externals global to this module **************************************
 */
  extern  union shm_pointers shm_addr; /* shared memory union pointers      */
  extern  int   semhe_id;              /* semaphore id                      */
  extern  char  HTXPATH[];             /* HTX file system path         */
  extern int      system_call;            /* set to TRUE before system() */
  extern int		editor_PID;
  extern int 		shell_PID;

int add_pid_to_shm( int , struct htxshm_HE * );
#ifdef __HTX_LINUX__
extern char * regcmp (char *, char *);
extern char * regex (char *, char *);
#endif

/*****************************************************************************/
/*****  r u n _ c m d  *******************************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     run_cmd                                               */
/*                                                                           */
/* DESCRIPTIVE NAME =  Run a system command and return the output            */
/*                                                                           */
/* FUNCTION =          Runs a shell command string and returns the exit      */
/*                     code and optionally contents of output file.          */
/*                     To return output from the file, it must be redirected */
/*                     into a file by the command string, and the name of    */
/*                     the file must be passed in.                           */
/*                                                                           */
/* INPUT =             cmd_string - command string to execute.               */
/*                     ct_output  - pointer to size of cmd_output.           */
/*                     cmdout_filename - name of file to read cmd output     */
/*                                from.                                      */
/*                                                                           */
/* OUTPUT =            ct_output  - pointer to length of output, can be      */
/*                                  larger than cmd_output, see notes.       */
/*                     cmd_output - points to where to put the contents of   */
/*                                the command output file.                   */
/*                                                                           */
/* NORMAL RETURN =     0 - command ran and returned 0 as an exit code.       */
/*                                                                           */
/* ERROR RETURNS =     < 0 - problem with running the system command.  No    */
/*                         output from command is available.   ct_output     */
/*                         and cmd_output are not affected.                  */
/*                                                                           */
/*                     > 0 - system command returned non-zero exit code.     */
/*                     -128 - system() call failed.                          */
/*                     -129 - failed to open command output file.            */
/*                                                                           */
/*    NOTES =          ct_output is an input and output parameter.  If it    */
/*                     returns larger than it was on input, this indicates   */
/*                     that the output was truncated.                        */
/*                                                                           */
/*                     If cmd_output is NULL or cmdout_filename is null      */
/*                     then no output is returned.                           */
/*                                                                           */
/*    EXTERNAL REFERENCES = system_call                                      */
/*                                                                           */
/*    DATA AREAS =                                                           */
/*                                                                           */
/*    MACROS =                                                               */
/*                                                                           */
/*****************************************************************************/
int run_cmd(char *cmd_string, char *cmd_output, int *ct_output,
			char *cmdout_filename)
{
    FILE    *cmdstdio;	/* pipe fd */
    char 	msg_text[MAX_TEXT_MSG];
    char    *p_cmdout, cmdout[256], *p_cmdOutput;
    int 	ct_cmdOutput = 0; /* counter for global output chars */
    int 	size_of_outbuf;
    int 	rc;
    struct stat out_stat;
    DBTRACE(DBENTRY,("enter R_device.c run_cmd\n"));

/*
 ********  Begin executable code ********
 */

    size_of_outbuf = *ct_output;
    if (cmd_output != NULL) bzero(cmd_output, size_of_outbuf);
    p_cmdOutput = cmd_output;

	/* run the command with system() */
    system_call = TRUE;
    rc = system(cmd_string);
    system_call = FALSE;
	/* check rc for system here */
    if (rc == -1 || rc == 127 ) { /* some problem system() */
	DBTRACE(DBEXIT,("return/a -128 R_device.c run_cmd\n"));
	return -128;
    } /* if */

    if (WIFSIGNALED(rc)) {  /* script killed by a signal */
	(void) sprintf(msg_text,
		       "Command killed by signal %d.",
		       WTERMSIG(rc));
	(void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
	rc = -WTERMSIG(rc);
    } /* if */
    else
	rc = WEXITSTATUS(rc);

    if (cmd_output != NULL && cmdout_filename != NULL) {

		/* open output file */
	cmdstdio = fopen(cmdout_filename, "r");
	if (cmdstdio == NULL) {
	    (void) sprintf(msg_text,
			   "Failed to open command output file: %s.   errno = %d\n",
			   cmdout_filename, errno);
	    (void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
	    DBTRACE(DBEXIT,("return/b -129 R_device.c run_cmd\n"));
	    return -129;
	}


	if (stat(cmdout_filename, &out_stat) == -1) {
	    (void) sprintf(msg_text, "Error getting the stat data for %s. errno=%d (%s)\n",cmdout_filename, errno, strerror(errno));
	    print_log(LOGMSG,"%s\n", msg_text);
	    (void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
	    fclose(cmdstdio);
	    DBTRACE(DBEXIT,("return/c -140 R_device.c run_cmd\n"));
	    return -140;
	}
	else if (out_stat.st_size<=0) {
	    (void) sprintf(msg_text, "Output file size found = %d. \n",(int)out_stat.st_size);
	    print_log(LOGMSG,"%s", msg_text);
	    (void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
	    fclose(cmdstdio);
	    DBTRACE(DBEXIT,("return/d -141 R_device.c run_cmd\n"));
	    return -141;
	}

		/* read the output from the command until EOF or error */
	while (  NULL != (p_cmdout = fgets(cmdout,255, cmdstdio)) ) {
	    ct_cmdOutput = ct_cmdOutput + strlen(p_cmdout);
	    ct_cmdOutput = (ct_cmdOutput >= size_of_outbuf) ? size_of_outbuf : ct_cmdOutput;
	    p_cmdOutput = strncat(p_cmdOutput, p_cmdout, size_of_outbuf - ct_cmdOutput);
	    if (ct_cmdOutput >= size_of_outbuf) { /* buffer full */
		cmd_output[size_of_outbuf-1]='\0'; /* make sure it is a string */
		cmd_output[size_of_outbuf-2]='\n'; /* and ends with a newline  */
		break;	/* bail out */
	    } /* if */
	} /* while */
	*ct_output = ct_cmdOutput;
	fclose(cmdstdio);
    } /* if */

    DBTRACE(DBEXIT,("return R_device.c run_cmd\n"));
    return(rc);
} /* run_cmd */

/*****************************************************************************/
/*****  S I G A L R M _ h a n d l e r  ***************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     SIGALRM_handler                                       */
/*                                                                           */
/* DESCRIPTIVE NAME =  alarm signal handler for script time out              */
/*                                                                           */
/* FUNCTION =          Upon reciept of an alarm signal, kills the script     */
/*                     that is name in script_name.                          */
/*                                                                           */
/* INPUT =             script_name - global pointer to script path name      */
/*                                                                           */
/* OUTPUT =            None.                                                 */
/*                                                                           */
/* NORMAL RETURN =     None - void function.                                 */
/*                                                                           */
/* ERROR RETURNS =     None - void function.                                 */
/*                                                                           */
/* EXTERNAL REFERENCES: None.                                                */
/*                                                                           */
/* GLOBAL REFERENCES: script_name - path name of startup/cleanup script      */
/*                     which is running.                                     */
/*                                                                           */
/*****************************************************************************/

void SIGALRM_handler(int sig, int code, struct sigcontext *scp)
{
    char *cmd_string;
    DBTRACE(DBENTRY,("enter R_device.c SIGALRM_handler\n"));

    cmd_string = malloc(PATH_MAX);
    sprintf(cmd_string, "ps -eo \"pid args\" | awk '/%s */ { if ($2 != \"awk\") {print \"kill -9 \" $1 }}' | ksh", script_name);
    system_call = TRUE;
    (void) system(cmd_string);
    system_call = FALSE;
    free(cmd_string);
    cmd_string = NULL;
    DBTRACE(DBEXIT,("return R_device.c SIGALRM_handler\n"));
    return;                              /* exit signal handler               */
} /* SIGALRM_handler() */

/*****************************************************************************/
/*****  e x e c _ H E _ s c r i p t  *****************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     exec_HE_script                                        */
/*                                                                           */
/* DESCRIPTIVE NAME =  Execute Hardware Exerciser setup/cleanup Script       */
/*                                                                           */
/* FUNCTION =          Runs a script taken from the list of HE scripts       */
/*                                                                           */
/* INPUT =             pattern - a regular expression                        */
/*                     device_name - HTX logical device name (like in MDT).  */
/*                                                                           */
/* OUTPUT =            Runs the selected setup or cleanup script or for      */
/*                     that matter any script executable from the home dir.  */
/*                                                                           */
/*                     conf_emsg - hints that inportant error message was    */
/*                                 posted for operator on console message    */
/*                                 line.                                     */
/*                                                                           */
/* NORMAL RETURN =     0 - no script found for exerciser or it was found     */
/*                         and it returned 0.                                */
/*                                                                           */
/* ERROR RETURNS =     < 0 - problem with finding a script for the exerciser */
/*                           see the htxmsg log.                             */
/*                     > 0 - script returned non-zero exit code, see htxmsg. */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*                                                                           */
/*    DATA AREAS =     HTXPATH - pointer to a string which contains the path */
/*                         to the top level of the HTX file hierarchy        */
/*                         (usually "/usr/lpp/htx").                         */
/*                                                                           */
/*    MACROS =         PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                                                                           */
/* NOTES = Please note that this function contains those hated constructs,   */
/*         the goto.  The idea is to contain as much clean up at the end     */
/*         of the function as possible without getting the indent level      */
/*         very great.  So I check first for failures, issue message and     */
/*         go to cleanup.                                                    */
/*                                                                           */
/*****************************************************************************/
int exec_HE_script(char pattern[], char device_name[], boolean_t *conf_emsg)
{
    struct sigaction sigvector;          /* structure for signal specs */
    struct sigaction old_SIGALRM_vector;  /* structure for signal specs */
    int	rc;
    char	*script_list_file;	/* path name of HE_SCRIPT_FILE */
    char	*msg_text;		/* HTX log message text */
    char	*cmdOutput;		/* output from AIX commands */
    char	*cmd_string;	/* command to pass to shell */
    char	*script_cmd;	/* formatted script command */
    char	*p_script_cmd;
    char	*p_temp_filename;	/* name of temporary file to hold awk output */

    int		ct_cmdOutput;	/* byte count of output in cmdOutput */
    int		ct_script_cmd;
    unsigned int	rc_alarm;
    DBTRACE(DBENTRY,("enter R_device.c exec_HE_script\n"));

/*
 *	Get some memory to work with.
 */
    print_log(LOGMSG,"HTXPATH = %s\n", HTXPATH);
    script_list_file = malloc(strlen(HTXPATH) + strlen(HE_SCRIPT_FILE) + 3);
    msg_text = malloc(MSG_TEXT_SIZE);
    cmdOutput = malloc(MSG_TEXT_SIZE);
    cmd_string = malloc(PATH_MAX);
    script_cmd = malloc(MSG_TEXT_SIZE);

    *conf_emsg = FALSE;

/*
 *	check access to HE_SCRIPT_FILE file
 */
    script_list_file = strcpy(script_list_file, HTXPATH);
    script_list_file = strcat(script_list_file, "/");
    script_list_file = strcat(script_list_file, HE_SCRIPT_FILE);
    if (0 != (rc = access(script_list_file, R_OK))) {
	sprintf(msg_text,
		"Cannot access %s. Errno = %d", script_list_file, errno);
	print_log(LOGERR,"Cannot access %s. Errno = %d", script_list_file, errno);
	(void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
	if (!editor_PID && !shell_PID) print_log(LOGERR,"Cannot access %s for %s.  See message log.\n",
					      script_list_file, device_name );
	rc = -1;
	*conf_emsg = TRUE;
	goto getout;
    } /* if */

/*
 *	Create a temporary file name for the script command
 */
    p_temp_filename = tempnam(NULL, "hxs");
    if (p_temp_filename == NULL) {
	(void) sprintf(msg_text,
		       "Problem getting the temporary file name for awk output.  errno = %d",
		       errno);
	(void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
	if (!editor_PID && !shell_PID) print_log(LOGMSG,"%s\n", msg_text);
	rc = -1;
	*conf_emsg = TRUE;
	goto getout;
    } /* if */

/*
 *	Use awk to get find the script and build the command line.
 *
 *  This is a long command, but it should functionally look something like
 *  this, using "^hxecrick.*\.runsetup$" and "tok0" as the pattern and
 *  device name, respectively:
 *
 *  awk 'BEGIN {f=1}  \
 *		/^hxecrick.*\.runsetup$/ { \
 *			f=0	\
 *			print "/usr/lpp/htx/" $2 "tok0 >/tmp" substr($2,index($2,"/")) \
 *					".tok0.out 2>&1" } \
 *		END { exit f }' \
 *		/usr/lpp/htx/hxsscripts > /tmp/hxssupj234j23k 2>&1
 *
 *	And this should result in a line in /tmp/hxssupj234j23k like:
 *
 * /usr/lpp/htx/runsetup/crick.runsetup tok0 >/tmp/crick.runsetup.tok0.out 2>&1
 *
 *	Clear?  Ok, so the idea is to let awk do all that horrible string stuff
 *  and just read that from /tmp/hxssupj234j23k and execute it with the
 *  output going to a file in /tmp which is unique to the run.
 */
    (void) sprintf(cmd_string,
		   "awk 'BEGIN {f=1} /%s/ { f=0; print \"%s\" $2 \" %s >/tmp\" substr($2,index($2,\"/\")) \".%s.out 2>&1\" } END {exit f}' %s > %s 2>&1",
		   pattern, HTXPATH, device_name, device_name, script_list_file, p_temp_filename);

    ct_script_cmd = MSG_TEXT_SIZE;
    rc = run_cmd(cmd_string, script_cmd, &ct_script_cmd, p_temp_filename);

    if (rc != 0 && rc != 1) { /* some problem with running awk  */
	(void) sprintf(msg_text,
		       "Problem getting the script name from %s.\nPattern =\"%s\"\nrun_cmd() returns %d.  errno=%d   See %s for output.",
		       script_list_file, pattern, rc, errno, p_temp_filename);
	(void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
	if (!editor_PID && !shell_PID) print_log(LOGERR,"Failed to get script name for %s.  See message log.\n", device_name);
	rc = -1;
	*conf_emsg = TRUE;
	goto getout;
    } /* if */

    (void) remove(p_temp_filename);  /* cleanup temp file */

    if (rc == 1) { /* nothing for this pattern, leave quietly */
	rc = 0;
	goto getout;
    } /* if */

    /********************************************************************/
	/* If we get here, then there is a script command line to run.      */
	/* awk has built the command line(s), so just run the command(s).   */
    /********************************************************************/

    if ( script_name != (char*)NULL ) {
	free(script_name);
	script_name = NULL;
    }
    script_name = malloc(strlen(script_cmd)); /* SIGALRM_handler uses this */

	/* setup SIGALRM handler */
    sigemptyset(&(sigvector.sa_mask));     /* do not block signals       */
    sigvector.sa_flags = 0;       /* do not restart system calls on sigs */
    sigvector.sa_handler = (void (*)(int)) SIGALRM_handler;
    (void) sigaction(SIGALRM, &sigvector, &old_SIGALRM_vector);

	/* get first line from the command buffer */
    p_script_cmd = strtok( script_cmd, "\n");

    while (p_script_cmd != NULL) {

		/* get the pathname of the script */
	(void) sscanf(p_script_cmd, "%[^ ]", script_name);

		/* Check to make sure that access is OK */
	if (0 != (rc = access(script_name, (F_OK | X_OK | R_OK)))) {

	    switch(errno) {
		case ENOENT:
		    sprintf(msg_text, "Can't find %s\n", script_name);
		    break;
		case EACCES:
		    sprintf(msg_text, "Can't run %s: permission denied.\n",
			    script_name);
		    break;
		default:
		    sprintf(msg_text, "Can't run %s\n: errno = %d",
			    script_name, errno);

	    } /* end switch */

	    (void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
	    if (!editor_PID && !shell_PID) print_log(LOGERR,"Cannot run %s for %s.  See message log.\n",
						  script_name, device_name);
	    *conf_emsg = TRUE;
	} /* if */

	else {

			/* get the base name of the script for SIGALRM_handler */
	    do {
		rc = sscanf(script_name, "/%s", script_name);
		rc = sscanf(script_name, "%*[^/]%s", script_name);
	    } while ( rc != 0 && rc != EOF );

	    (void) sprintf(msg_text,
			   "Running %s for %s, command line:\n%s",
			   script_name, device_name, p_script_cmd);
	    (void) send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);

	    (void) alarm(60);	/* time to wait for script to complete */
	    ct_cmdOutput = 0;
	    rc = run_cmd(p_script_cmd, NULL, &ct_cmdOutput, NULL);
	    rc_alarm = alarm(0);

	    if ( rc_alarm == 0 ) { /* timed out while waiting for script exec */
		(void) sprintf(msg_text,
			       "Killed %s after waiting for %d seconds",
			       script_name, 60);
		(void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
	    } /* if */

	    if ( rc < 0 ) { /* problem with running it */
		(void) sprintf(msg_text,
			       "Problem running %s for %s.\nrun_cmd() returns %d.  errno=%d",
			       script_name, device_name, rc, errno);
		(void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
		if (!editor_PID && !shell_PID) print_log(LOGERR,"Failed to run %s for %s.  See message log.\n",
						      script_name, device_name );
		*conf_emsg = TRUE;
	    } /* if */

	    if ( rc >= 0 ) {	/* returned non-negative */
		(void) sprintf(msg_text,
			       "%s for %s exit code = %d.",  script_name, device_name, rc);
	    } /* if */

	    if ( rc > 0 ) {	/* script returned non-zero exit code */
		(void) send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
		if (!editor_PID && !shell_PID) print_log(LOGMSG,"%s for %s returned %d.  See message log.\n",
						      script_name, device_name, rc );
	    } /* if */

	    if ( rc == 0 ) {	/* script returned zero exit code */
		(void) send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	    } /* if */
	} /* end else */

		/* get subsequent lines one at a time from the command buffer */
	p_script_cmd = strtok( NULL, "\n");
    } /* while */

	/* restore SIGALRM handler to default */
    (void) sigaction(SIGALRM, &old_SIGALRM_vector, (struct sigaction *) NULL);

/*
 *	Free the memory
 */
    getout:

      (void) remove(p_temp_filename);  /* cleanup temp file */
    free(p_temp_filename);
    free(cmdOutput);
    free(msg_text);
    free(script_cmd);
    free(script_list_file);
    free(cmd_string);

    p_temp_filename = NULL;
    cmdOutput = NULL;
    msg_text = NULL;
    script_cmd = NULL;
    script_list_file = NULL;
    cmd_string = NULL;

    DBTRACE(DBEXIT,("return R_device.c exec_HE_script\n"));
    return rc;
} /* exec_HE_script */



/*****************************************************************************/
/*****  l o a d _ e x e r s i c e r  *****************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     load_exerciser                                        */
/*                                                                           */
/* DESCRIPTIVE NAME =  Load HTX Exerciser                                    */
/*                                                                           */
/* FUNCTION =          Spawns the hardware exercisers.                       */
/*                                                                           */
/* INPUT =             p_HE - pointer to htxshm_HE structure.                */
/*                     sigvec - pointer to sigvector structure               */
/*                                                                           */
/* OUTPUT =            Loads hardware exerciser program and sets the PID     */
/*                     in the htxshm_HE structure.                           */
/*                                                                           */
/* NORMAL RETURN =     0 - OK                                                */
/*                                                                           */
/* ERROR RETURNS =     1 - problem with fork()                               */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = N/A                                                   */
/*                                                                           */
/*    DATA AREAS =     HTXPATH - pointer to a string which contains the path */
/*                         to the top level of the HTX file hierarchy        */
/*                         (usually "/usr/lpp/htx").                         */
/*                     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                                                                           */
/*    MACROS =         PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                                                                           */
/*****************************************************************************/

int load_exerciser(struct htxshm_HE *p_HE, struct sigaction *sigvec)
{
    char	workstr[512];          /* work string                       */
    char	workstr2[64];          /* work string                       */
    char	workstr3[64];          /* work string                       */
    char	workstr4[64];          /* work string                       */
    char	workstr5[64];          /* work string                       */
    char	tmpstr3[64];           /* temp string to store sdev_id      */
    int		workint;               /* work integer                      */
    int		frkpid;                /* fork PID                          */
    int		j;
    sigset_t 	blk_mask;
    int		null_fd;

/*
 ***  Begin executable code ***
 */
#if defined(__OS400__)   /* 400 */
    char * spw_path;
    char * spw_argv[6];
    char * spw_env[1];
    struct inheritance spw_inherit;
    DBTRACE(DBENTRY,("enter R_device.c load_exerciser/OS400\n"));


    htx_strcpy(workstr, HTXPATH);
    htx_strcat(workstr, "/bin/");

    htx_strcpy(workstr2, p_HE->HE_name);

    htx_strcat(workstr, workstr2);

    htx_strcpy(workstr3, "/dev/");
    htx_strcat(workstr3, p_HE->sdev_id);

    if (shm_addr.hdr_addr->emc == 1)
      htx_strcpy(workstr4, "EMC");
    else
      htx_strcpy(workstr4, "REG");
    htx_strcpy(workstr5, HTXPATH);
    htx_strcat(workstr5, "/rules/");
    if (shm_addr.hdr_addr->emc == 1) {
      htx_strcat(workstr5, "emc/");
      htx_strcat(workstr5, p_HE->emc_rules);
    }
    else {
      htx_strcat(workstr5, "reg/");
      htx_strcat(workstr5, p_HE->reg_rules);
    }    /* endif */

    spw_path = workstr;
    spw_argv[0] = workstr2;
    spw_argv[1] = workstr3;
    spw_argv[2] = workstr4;
    spw_argv[3] = workstr5;
    spw_argv[4] = NULL;

    spw_env[0] = NULL;

    memset(&spw_inherit,0x00, sizeof(spw_inherit));
    spw_inherit.flags = SPAWN_SETSIGMASK | SPAWN_SETSIGDEF;
    spw_inherit.pgroup = SPAWN_NEWPGROUP;
    sigemptyset(&(spw_inherit.sigmask));
    sigfillset(&(spw_inherit.sigdefault));
     /* assign nice value of p_HE->priority to process */
    frkpid = spawn(spw_path, 0, NULL, &spw_inherit, spw_argv, spw_env);
    switch (frkpid) {
	case -1:    /* problem with fork() call       */
	    htx_strcpy(workstr2, p_HE->HE_name);

	    print_log(LOGERR,"Unable to fork for %s hardware "
		   "exerciser: %s\nerrno = %d: %s\n",
		   spw_path,
		   workstr2, errno,
		   strerror(errno));
	    fflush(stdout);
	    sprintf(workstr, "Unable to fork for %s hardware "
		    "exerciser: %s  errno = %d: %s",
		    spw_path,
		    workstr2, errno,
		    strerror(errno));
	    //PRTMSG(MSGLINE, 0, ("%s", htx_strcat(workstr, "\n")));
	    send_message(workstr, errno, HTX_SYS_HARD_ERROR,
			 HTX_SYS_MSG);
	    DBTRACE(DBEXIT,("return/a 1 R_device.c load_exerciser\n"));
	    return 1;
	    break;

	default:    /* parent process               */
            add_pid_to_shm(frkpid, p_HE);
	    //p_HE->PID = frkpid;
	    //print_log(LOGMSG,"Parent: chld= %d\n",frkpid);
	    //fflush(stdout);
	    break;
    }      /* endswitch */
#else
    DBTRACE(DBENTRY,("enter R_device.c load_exerciser\n"));
    frkpid = fork();

    switch (frkpid)
    {
	case 0:                        /* child process                 */
	    if ( listener != 0 ) {
		close(listener);
	    }
	    if ( fd_num != 0 ) {
		close(fd_num);
	    }

	    /* do not close std fds since the closing causes fd polution for exerciser std fds operations */
	    fcntl(fileno(stdin), F_SETFD, 0);
	    fcntl(fileno(stdout), F_SETFD, 0);
	    fcntl(fileno(stderr), F_SETFD, 0);

	    /* enabling stdin,stdout and stderr for dummy use of printf in exercisors */
	    null_fd = open("/dev/null", O_RDWR);
	    dup2(null_fd, fileno(stdin) );
	    dup2(null_fd, fileno(stdout) );
	    dup2(null_fd, fileno(stderr));

	    for (j = 1; j <= SIGMAX; j++) /* set default signal processing */
		(void) sigaction(j, sigvec, (struct sigaction *) NULL);

#if !defined(__HTX_LINUX__) && !defined(__OS400__)  /* 400 */
	    blk_mask.losigs = 0xffffffff;
	    blk_mask.hisigs = 0xffffffff;

	    sigprocmask(SIG_UNBLOCK, &blk_mask, NULL);
#endif

	    (void) setsid();  /* change the process group so HE's don't get
				   signals meant for the supervisor program */

	    workint = p_HE->priority;
	    (void) nice(workint);

	    sleep(1);		/* let the supervisor update shared memory */

	    putenv("EXTSHM=OFF");

	    (void) strcpy(workstr, HTXPATH);
	    (void) strcat(workstr,"/bin/");

	    (void) strcpy(workstr2, p_HE->HE_name);

	    (void) strcat(workstr, workstr2);

	    (void) strcpy(workstr3, "/dev/");
	    (void) strcpy(tmpstr3,p_HE->sdev_id);
	    (void) strcat(workstr3, strtok(tmpstr3,"("));

	    if (shm_addr.hdr_addr->emc == 1)
		(void) strcpy(workstr4,"EMC");
	    else (void) strcpy(workstr4,"REG");

		if (shm_addr.hdr_addr->emc == 1)
		{
			if (p_HE->emc_rules[0] != '/')
			{  /* in case of relative path */
				strcpy(workstr5, HTXPATH);
				strcat(workstr5, "/rules/");
				strcat(workstr5,"emc/");
				strcat(workstr5, p_HE->emc_rules);
			}
			else
			{   /* in case of absolute path no change */
				strcpy(workstr5, p_HE->emc_rules);
			}
		}
		else
		{
			if (p_HE->reg_rules[0] != '/')
			{  /* in case of relative path */
				strcpy(workstr5, HTXPATH);
				strcat(workstr5, "/rules/");
				strcat(workstr5,"reg/");
				strcat(workstr5, p_HE->reg_rules);
			}
			else
			{  /* in case of absolute path no change */
				strcpy(workstr5, p_HE->reg_rules);
			}
		}

	    if ( (execl(workstr, workstr2, workstr3, workstr4, workstr5,
			(char *) 0) ) == -1)
	    {
		(void) sprintf(workstr, "Unable to load %s hardware "
			       "exerciser.  errno = %d", workstr2, errno);
		if (!editor_PID && !shell_PID) print_log(LOGMSG,"%s", strcat(workstr, "\n"));
		(void) send_message(workstr, errno, HTX_SYS_HARD_ERROR,
				    HTX_SYS_MSG);
		exit(-1);
	    } /* endif */
	    break;

	case -1:                       /* problem with fork() call       */
	    (void) strcpy(workstr2, p_HE->HE_name);

	    (void) sprintf(workstr, "Unable to fork for %s hardware "
			   "exerciser.  errno = %d", workstr2, errno);
	    if (!editor_PID && !shell_PID) print_log(LOGMSG,"%s", strcat(workstr, "\n"));
	    (void) send_message(workstr, errno, HTX_SYS_HARD_ERROR,
				HTX_SYS_MSG);
	    DBTRACE(DBEXIT,("return/b 1 R_device.c load_exerciser\n"));
	    return 1;
	    break;

	default :                      /* parent process               */
	    add_pid_to_shm(frkpid, p_HE);
	    //p_HE->PID  = frkpid;
	    //print_log(LOGMSG,"loaded with id: %d\n",frkpid)
	    break;
    } /* endswitch */
#endif

    DBTRACE(DBEXIT,("return R_device.c load_exerciser\n"));
    return 0;
} /* load_exerciser */


/*****************************************************************************/
/*****  s i g _ r e s t a r t _ H E  *****************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     sig_restart_HE                                        */
/*                                                                           */
/* DESCRIPTIVE NAME =  Restart hardware exerciser by signal                  */
/*                                                                           */
/* FUNCTION =          Spawns a hardware exerciser which has died or was     */
/*                     terminated.  SIGUSR2 handler.                         */
/*                                                                           */
/* INPUT =             p_shm_hdr - points to header of shared memory         */
/*                                                                           */
/* OUTPUT =            Loads hardware exerciser programs                     */
/*                                                                           */
/*                     Updated system semaphore structure to reflect the     */
/*                     state of the HE's which are restarted.                */
/*                                                                           */
/* NORMAL RETURN =     N/A - void function                                   */
/*                                                                           */
/* ERROR RETURNS =     N/A - void function                                   */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*                                                                           */
/*    OTHER ROUTINES = load_exerciser () - loads the exerciser.              */
/*                     send_message() - outputs a message to the screen and  */
/*                                     the message log file                  */
/*                                                                           */
/*    DATA AREAS =                                                           */
/*                     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                                                                           */
/*    MACROS =                                                               */
/*                                                                           */
/*****************************************************************************/
void sig_restart_HE(int sig, int code, struct sigcontext *scp)
{
    boolean_t dummy;
    struct htxshm_HE  *p_shm_addr;
    struct htxshm_hdr *p_shm_hdr;
    struct sigaction sigvector;          /* structure for signal specs */
    char	*msg_text;		/* HTX log message text */
    char reg_expr[256];
    int i,rc;
    DBTRACE(DBENTRY,("enter R_device.c sig_restart_HE\n"));

    msg_text = malloc(MSG_TEXT_SIZE);
    p_shm_hdr = shm_addr.hdr_addr;
    p_shm_addr = ((struct htxshm_HE *)
		  ((struct htxshm_hdr  *) shm_addr.hdr_addr + 1));    /* skip over hea
der */

  /* set file descriptor to close on exec */

    (void) fcntl(fileno(stdin), F_SETFD, 1);  /* stdin to close on exec  */
    (void) fcntl(fileno(stdout), F_SETFD, 1); /* stdout to close on exec */
    (void) fcntl(fileno(stderr), F_SETFD, 1); /* stderr to close on exec */

  /* setup signal vector structure for HE */
    /************************************************************/
    /******  Set signals to SIG_DFL  ****************************/
    /******  for the newly restarted HE's  **********************/
    /************************************************************/
    sigvector.sa_handler = SIG_DFL; /* set default action                  */
    sigemptyset(&(sigvector.sa_mask));  /* do not block other signals      */
    sigvector.sa_flags = 0;         /* No special flags                    */

    for (i = 0; i < (p_shm_hdr)->max_entries; i++) {
	if ( (((p_shm_addr + i)->PID) == 0)
	     && (((p_shm_addr + i)->restart_HE) == 1)) {
	    (void) sprintf(msg_text,"Restart of %s signaled by remote.",
			   (p_shm_addr + i)->sdev_id);
	    (void) send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	    strcpy(reg_expr,"^");
	    strcat(reg_expr,(p_shm_addr+i)->HE_name);
	    strcat(reg_expr, ".*setup[\t ]*$");
	    rc = exec_HE_script(reg_expr, (p_shm_addr+i)->sdev_id, &dummy);
	    if ((p_shm_addr+i)->start_halted)
		(void) semctl(semhe_id, ((i * SEM_PER_EXER) + SEM_GLOBAL), SETVAL, 1);
	    (p_shm_addr+i)->user_term = 0;	/* reset */
	    (p_shm_addr+i)->DR_term = 0;		/* initialize DR termination flag */
	    (p_shm_addr+i)->restart_HE = 0;	/* reset */
	    rc = load_exerciser(p_shm_addr+i, &sigvector); /* fork & exec */
	    if (rc ==0) {
		((p_shm_hdr)->num_entries)++;
		(p_shm_addr+i)->tm_last_upd = -1;
		sleep(2);
	    } /* endif */
	} /* endif */
    } /* endfor */

    (void) fcntl(fileno(stdin), F_SETFD, 0); /* stdin NOT to close on exec */
    (void) fcntl(fileno(stdout), F_SETFD, 0); /*stdout NOT to clse on exec */
    (void) fcntl(fileno(stderr), F_SETFD, 0); /*stderr NOT to clse on exec */
    (void) free(msg_text);
    msg_text = NULL;
    DBTRACE(DBEXIT,("leave R_device.c sig_restart_HE\n"));
} /* sig_restart_HE */

/*****************************************************************************/
/*****  r e s t a r t _ e x e r c i s e r  ***********************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     restart_exerciser                                     */
/*                                                                           */
/* DESCRIPTIVE NAME =  Restart hardware exerciser                            */
/*                                                                           */
/* FUNCTION =          Spawns a hardware exerciser which has died or was     */
/*                     terminated.                                           */
/*                                                                           */
/* INPUT =             p_shm_hdr - points to header of shared memory         */
/*                     HE_pos - position of HE in shared memory, used also   */
/*                              for the semaphore in the HE semaphore set.   */
/*                                                                           */
/* OUTPUT =            Loads hardware exerciser programs                     */
/*                                                                           */
/*                     Updated system semaphore structure to reflect the     */
/*                     state of the HE's which are restarted.                */
/*                                                                           */
/* NORMAL RETURN =     N/A - void function                                   */
/*                                                                           */
/* ERROR RETURNS =     N/A - void function                                   */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*                                                                           */
/*    OTHER ROUTINES = load_exerciser () - loads the exerciser.              */
/*                     send_message() - outputs a message to the screen and  */
/*                                     the message log file                  */
/*                                                                           */
/*    DATA AREAS =                                                           */
/*                     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                                                                           */
/*    MACROS =         PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                                                                           */
/*****************************************************************************/
int restart_exerciser( struct htxshm_hdr *p_shm_hdr, int HE_pos, char *result_msg )
{
    boolean_t dummy;
    //struct htxshm_HE  *p_shm_addr;
    struct sigaction sigvector;          /* structure for signal specs */
    struct sigaction old_SIGINT_vector;  /* structure for signal specs */
    struct sigaction old_SIGQUIT_vector; /* structure for signal specs */
    char	*msg_text;		/* HTX log message text */
    char reg_expr[256];
    int i,rc;
    DBTRACE(DBENTRY,("enter R_device.c restart_exerciser\n"));

	/***************************************************************************/
	/**  beginning of executable code  *****************************************/
	/***************************************************************************/

	/*if ( ! p_shm_hdr->started ) {
		sprintf(result_msg, "%s", "System must be running to restart exercisers.");
		print_log(LOGMSG,"%s\n", "System must be running to restart exercisers.");
		return -1;
	}  if
	p_shm_addr = ((struct htxshm_HE *)
			((struct htxshm_hdr  *) p_shm_hdr + 1));	 skip over header */

    msg_text = malloc(MSG_TEXT_SIZE);

	/* set file descriptor to close on exec */

    (void) fcntl(fileno(stdin), F_SETFD, 1);  /* stdin to close on exec  */
    (void) fcntl(fileno(stdout), F_SETFD, 1); /* stdout to close on exec */
    (void) fcntl(fileno(stderr), F_SETFD, 1); /* stderr to close on exec */

	/* setup signal vector structure for HE */

	/************************************************************/
	/***  Ignore SIGINT and SIGQUIT signals  ********************/
	/************************************************************/
    sigvector.sa_handler = SIG_IGN; /* set default action                  */
    sigemptyset(&(sigvector.sa_mask));  /* do not block other signals      */
    sigvector.sa_flags = 0;         /* No special flags                    */

    (void) sigaction(SIGINT, &sigvector, &old_SIGINT_vector);
    (void) sigaction(SIGQUIT, &sigvector, &old_SIGQUIT_vector);

	/************************************************************/
	/******  Set signals to SIG_DFL  ****************************/
	/******  for the newly restarted HE's  **********************/
	/************************************************************/
    sigvector.sa_handler = SIG_DFL; /* set default action                  */
    sigemptyset(&(sigvector.sa_mask));  /* do not block other signals      */
    sigvector.sa_flags = 0;         /* No special flags                    */

    i = HE_pos;
    if ( ((ECGEXER_ADDR(i))->PID) == 0) {
	(void) sprintf(msg_text,"Restart of %s requested by operator.",
		       (ECGEXER_ADDR(i))->sdev_id);
	(void) send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	strcpy(reg_expr,"^");
	strcat(reg_expr,(ECGEXER_ADDR(i))->HE_name);
	strcat(reg_expr, ".*setup[\t ]*$");
	rc = exec_HE_script(reg_expr, (ECGEXER_ADDR(i))->sdev_id, &dummy);
	if ((ECGEXER_ADDR(i))->start_halted)
	    (void) semctl(semhe_id, ((i * SEM_PER_EXER) + SEM_GLOBAL), SETVAL, 1);
	(ECGEXER_ADDR(i))->user_term = 0;  /* reset */
	(ECGEXER_ADDR(i))->DR_term = 0;
	rc = load_exerciser(ECGEXER_ADDR(i), &sigvector); /* fork & exec */
	if (rc ==0) {
	    ((ECGEXER_HDR(i))->num_entries)++;
	    (ECGEXER_ADDR(i))->tm_last_upd = -1;
	} /* endif */
    } /* endif */

    (void) fcntl(fileno(stdin), F_SETFD, 0); /* stdin NOT to close on exec */
    (void) fcntl(fileno(stdout), F_SETFD, 0); /*stdout NOT to clse on exec */
    (void) fcntl(fileno(stderr), F_SETFD, 0); /*stderr NOT to clse on exec */

    (void) sigaction(SIGINT, &old_SIGINT_vector, (struct sigaction *) 0);
    (void) sigaction(SIGQUIT, &old_SIGQUIT_vector, (struct sigaction *) 0);

    (void) free(msg_text);
    msg_text = NULL;

    DBTRACE(DBEXIT,("return R_device.c restart_exerciser\n"));
    return 0;
} /* restart_exerciser */

/*****************************************************************************/
/*****  R _ d e v i c e  *****************************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     R_device                                              */
/*                                                                           */
/* DESCRIPTIVE NAME =  Restart hardware exerciser                            */
/*                                                                           */
/* FUNCTION =          Builds and displays the Restart exerciser menu and    */
/*                     handles operators requests.                           */
/*                                                                           */
/* INPUT =             No parameters                                         */
/*                                                                           */
/* OUTPUT =            Restarts HE's and tracks progress.                    */
/*                                                                           */
/*                     Updated screen to reflect the                         */
/*                     state of the HE's which are restarted.                */
/*                                                                           */
/* NORMAL RETURN =     0 - OK                                                */
/*                                                                           */
/* ERROR RETURNS =     non zero - some fatal programming flaw.               */
/*                     Operational errors, such as failures to exec hardware */
/*                     exercisers are handled in lower functions and result  */
/*                     in error messages.                                    */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*                                                                           */
/*    OTHER ROUTINES = display_scn() - displays the restart screen template  */
/*                     get_dst() - returns the device status.                */
/*                     get_string() - reads operator input.                  */
/*                     regcmp - compile regular expression                   */
/*                     regex - compare regular expression                    */
/*                     send_message() - outputs a message to the screen and  */
/*                                     the message log file                  */
/*                     restart_exerciser () - restarts the exerciser.        */
/*                                                                           */
/*    DATA AREAS =     standard curses library externals, LINES, COLS.       */
/*                     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*    MACROS =         PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                     CLRLIN - Clears the message line.                     */
/*                                                                           */
/*****************************************************************************/
int R_device(char *result_msg, char **dev_list, int num_list, int *num_disp)
{
  /***************************************************************************/
  /**  variable definitions  *************************************************/
  /***************************************************************************/
  char                *cmp_regexp;     /* ptr to compiled reg expression    */
  char                ret[3];          /* return array for regex()          */
  char                *work;           /* work int */
  char                workstr[128];    /* work string                       */

  enum  t_dev_status  dev_status;      /* return from get_dst               */
  extern char         *__loc1;         /* beginning of reg exp match        */

  int                 i,ii;            /* loop counter                      */
  int                 max_strt_ent;    /* max starting entry                */
  int                 num_entries = 0; /* local number of shm HE entries    */
  int                 strt_ent = 1;    /* first entry on screen             */
  int                 workint;

  struct ahd_entry    *p_td_table = NULL;  /* points to begin td seq tbl    */
  struct ahd_entry    *p_into_table;   /* points into td sequence table     */

  struct htxshm_HE    *p_htxshm_HE;    /* pointer to a htxshm_HE struct     */


  int num_found, found, tmp, rc=0;
  int loop, max_loops, max_found=0, ecg_cnt, found_ecg;



  /***************************************************************************/
  /**  beginning of executable code  *****************************************/
  /***************************************************************************/

  print_log(LOGMSG,"In R_device. num_list = %d disp = %d\n", num_list, *num_disp); fflush(stdout);

  memset(result_msg,0,80);
  sprintf(result_msg,"R_device:No Valid device or ecg name provided");
  strt_ent = (msg_rcv.indx > 0)?msg_rcv.indx:1;

  *num_disp = 0;
  max_found = 0;
  loop = 0;

  if (num_list ==0) {
     sprintf(result_msg,"R_device:No valid exerciser list  or ecg name provided. Kindly make sure that the ECG is not in Unloaded/Inactive.");
     return -1;
  }

  if (num_list == -1)
     max_loops = 1;
  else {
     if (num_ecg == 0) {
        max_loops = 1;
        strcpy(ecg[0], "/ecg.all");
     }
     else
        max_loops = num_ecg;
  }

  print_log(LOGMSG,"max_loop = %d\n",max_loops);

  for (loop=0; loop<max_loops; loop++) {
      /********** for command line client *********/
      if (num_list != -1) {                           //If selected ecgs have to be searched
         for(ecg_cnt=0; ecg_cnt<num_ecgs; ecg_cnt++) {
         found_ecg=0;
         cur_ecg_pos = ecg_cnt;
            //print_log(LOGMSG,"R_device: num_list = %d ECGNAME = :%s: ecgname = :%s:\n",num_list, ECGNAME,ecg[loop]);
			PUT_FULL_ECG;
            if (strcmp(full_name, ecg[loop])==0) {
            print_log(LOGMSG,"R_device: found ECGNAME = :%s/%s: ecgname = :%s:\n",ECGPATH,ECGNAME,ecg[loop]);
               cur_ecg_pos = ecg_cnt;
               found_ecg=1;
               break;
            }
         }
      }
      /********** Done: for command line client *********/
      else if (num_list == -1) {
         found_ecg = 1;
         print_log(LOGMSG,"name = %s/%s status  %s\n", ECGPATH,ECGNAME, ECGSTATUS);
       }

      if (!found_ecg)
         continue;
      found_ecg = 0;


      REM_CUR->max_entries = ECG_MAX_ENTRIES ;

      if ( ECG_MAX_ENTRIES == 0) {  /* no HE programs?        */
         sprintf(result_msg, "There are no Hardware Exerciser programs currently defined in %s/%s.", ECGPATH,ECGNAME);
         print_log(LOGMSG,"There are no Hardware Exerciser programs currently defined.");
         continue; //return(-1);
      }
      /*else if (strcmp(ECGSTATUS, "INACTIVE")==0) {
         sprintf(result_msg, "The ECG(%s/%s) is in Inactive state. Activate the ECG to restart an exerciser", ECGPATH,ECGNAME);
         print_log(LOGMSG,"%s\n", result_msg);
         continue; //return(-1);

      }*/
      else {  /* some HE's defined.                */
         num_entries = init_ahd_tbl(&p_td_table);
         REM_CUR->num_entries = ECG_MAX_ENTRIES ;
         if (num_entries <= 0) {
            sprintf(result_msg, "Number of Entries in %s/%s is %d. \n", ECGPATH,ECGNAME, num_entries);
            print_log(LOGMSG,"Number of Entries in %s/%s is %d. \n", ECGPATH,ECGNAME, num_entries);
            if (p_td_table != NULL) {
               free((char *) p_td_table);   /* release memory for td order tbl*/
               p_td_table = NULL;
            }
            continue;  // return(num_entries);
         }
			/* problem in init fcn - bye!      */
         else { /* init fcn ran ok.                */
            if (num_entries <= 13) { /* 13 or fewer entries?          */
               max_strt_ent = 1;
            }
            else { /* more than 13 entries          */
               max_strt_ent = num_entries;
            } /* endif */
         } /* endif */
      } /* endif */

      /***********************************************************************/
      /**  build screen data for the current strt_ent  ***********************/
      /***********************************************************************/
      p_into_table = p_td_table + strt_ent - 1;
      *num_disp = num_entries - strt_ent + 1;
      if (*num_disp > 13) *num_disp = 13;

     /*if (num_list >0) {
       *num_disp = num_list;
     }
     else if (num_list == -2) {
       *num_disp = num_entries;
     }*/

     *num_disp = (num_list>0)?num_list:((num_list==-2)?num_entries:*num_disp);

      num_found = -1;

      for (i = 0; i <num_entries; i++) {

          if (num_list >= 0) {
             found = 0;
             for (tmp=0; tmp<*num_disp; tmp++) {
                 //print_log(LOGMSG,"loop: dev = :%s: list = :%s:\n",p_htxshm_HE->sdev_id,dev_list[tmp]);
                 fflush(stdout);
                 p_htxshm_HE = ECGEXER_ADDR(p_into_table->shm_pos);
                 if (strcmp(p_htxshm_HE->sdev_id,dev_list[tmp]) == 0) {
                    print_log(LOGMSG,"found: dev = %s list = %s\n",p_htxshm_HE->sdev_id,dev_list[tmp]);
                    fflush(stdout);
                    found = 1;
                    num_found++;
                    break;
                 }
             }

             if (found != 1) {
                p_into_table++;
                continue;
             }
          }
          else {
             num_found++;
             if (i >= *num_disp)
                break;
			 p_htxshm_HE = ECGEXER_ADDR(p_into_table->shm_pos);
          }

          work =  get_dst(p_htxshm_HE, p_into_table->shm_pos, &dev_status);
          workint = htx_strlen(work);  /* length of status string */
          htx_strncpy(workstr, "        ", 8 - workint);  /* spaces */
          workstr[8 - workint] = '\0';

          sprintf(INFO_SEND_2(max_found+num_found).status,work);

          /* set display attributes based on status */

          /* set color for device id based on status */
          htx_strcpy(INFO_SEND_2(max_found+num_found).slot_port, p_htxshm_HE->slot_port);
          htx_strcpy(INFO_SEND_2(max_found+num_found).sdev_id, p_htxshm_HE->sdev_id);
          htx_strcpy(INFO_SEND_2(max_found+num_found).adapt_desc, p_htxshm_HE->adapt_desc);
          htx_strcpy(INFO_SEND_2(max_found+num_found).device_desc, p_htxshm_HE->device_desc);
          //print_log(LOGMSG,"id = %s\n",INFO_SEND_2(max_found+num_found).sdev_id);
          fflush(stdout);

        p_into_table++;
      }      /* endfor */
    if (num_found == -1) {
       sprintf(result_msg,"%s","No valid exerciser list provided");
       if (p_td_table != NULL) {
          free((char *) p_td_table);   /* release memory for td order tbl*/
          p_td_table = NULL;
       }
       return -1;
    }

      num_found = -1;
      switch (msg_rcv.cmd) {
         case SCREEN_9_R_D:
          p_into_table = p_td_table;
          for (i=0; i<num_entries; i++) {
              p_htxshm_HE = ECGEXER_ADDR(p_into_table->shm_pos);
              found = 0;
              if (num_list>0) {
                 //print_log(LOGMSG,"cmp: dev = %s list = %s\n",p_htxshm_HE->sdev_id,dev_list[tmp]);
                 for (tmp=0; tmp<*num_disp; tmp++) {
                     if (strcmp(dev_list[tmp],p_htxshm_HE->sdev_id) == 0) {
                        print_log(LOGMSG,"found to restart: dev = %s list = %s\n",p_htxshm_HE->sdev_id,dev_list[tmp]);
                        found = 1;
                        num_found++;
                        break;
                     }
                 }

                 if (found != 1) {
                    p_into_table++;
                    continue;
                 }
                 else {
                    if (p_htxshm_HE->PID == 0) {
                       rc = restart_exerciser(shm_addr.hdr_addr, p_into_table->shm_pos,result_msg);
                       sleep(2);
                       work =  get_dst(p_htxshm_HE, p_into_table->shm_pos, &dev_status);
                       sprintf(INFO_SEND_2(max_found+num_found).status,work);
                    } /* endif */
                    else {
                       print_log(LOGMSG,"The exerciser for %s, %s is already running.", p_htxshm_HE->sdev_id, p_htxshm_HE->HE_name);
                       sprintf(result_msg,"The exerciser for %s, %s is already running.", p_htxshm_HE->sdev_id, p_htxshm_HE->HE_name);
                       rc = -1;
                    }  /* else */
                 }
              }
              else if (num_list == -2) {
                 num_found++;
                 if (p_htxshm_HE->PID == 0) {
                    rc = restart_exerciser(shm_addr.hdr_addr, p_into_table->shm_pos,result_msg);
                    sleep(2);
                    work =  get_dst(p_htxshm_HE, p_into_table->shm_pos, &dev_status);
                    sprintf(INFO_SEND_2(num_found).status,work);
                 } /* endif */
                 else {
                    print_log(LOGMSG,"The exerciser for %s, %s is already running.", p_htxshm_HE->sdev_id, p_htxshm_HE->HE_name);
                    sprintf(result_msg,"The exerciser for %s, %s is already running.", p_htxshm_HE->sdev_id, p_htxshm_HE->HE_name);
                    rc = -1;
                 }  /* else */
              }
              else if (msg_rcv.subcmd == SUBCMD_w){

                 cmp_regexp = regcmp((char *)reg_exp, (char *) 0);  /* compile exp */
                 if (cmp_regexp != NULL) { /* regular expression OK? */
                    for (ii=1; ii<=*num_disp; ii++) {
                       (void) sprintf(workstr, "%-d", ii);
                       work = regex(cmp_regexp, workstr);
                       if (work != NULL) {
                          (void) strncpy(ret, " ", (size_t) sizeof(ret));
                          /* clear return (ret) array */
                          (void) strncpy(ret, __loc1, (size_t) (work - __loc1));
                          if (strcmp(workstr, ret) == 0) {
                             p_into_table = p_td_table + (strt_ent - 1) + (ii - 1);
                             p_htxshm_HE = ECGEXER_ADDR(p_into_table->shm_pos);
                             //num_found = ii - 1;
                             num_found++;
                             if (p_htxshm_HE->PID == 0) {
                                rc = restart_exerciser(shm_addr.hdr_addr, p_into_table->shm_pos,result_msg);
                                sleep(2);
                                work =  get_dst(p_htxshm_HE, p_into_table->shm_pos, &dev_status);
                                sprintf(INFO_SEND_2(ii-1).status,work);
                             } /* endif */
                             else {
                                print_log(LOGMSG,"The exerciser for %s, %s is already running.", p_htxshm_HE->sdev_id, p_htxshm_HE->HE_name);
                                sprintf(result_msg,"The exerciser for %s, %s is already running.", p_htxshm_HE->sdev_id, p_htxshm_HE->HE_name);
                                rc = -1;
                             }  /* else */
                          }
                       }
                    }
                 }
              }
           if (num_list == -1)
              break;
           p_into_table++;
           if ((num_list >0) && ((max_found+num_found+1) >= num_list))
           break;
          }

           //if (num_list != -1)
           //   *num_disp = num_found +1;
          if (num_list != -1) {
                 max_found = max_found+num_found+1;
                 *num_disp = max_found;
              }

           break;
        }      /* endswitch */
        if (p_td_table != NULL) {
           free(p_td_table);   /* release memory for td order tbl*/
           p_td_table = NULL;
        }
      }
      return rc;

}        /* R_device */


int add_pid_to_shm( int pid, struct htxshm_HE * p_he )
{

  int i, rc = -1;
  int temp_ecg_pos;

  p_he->PID  = pid;
  //print_log(LOGMSG,"added the pid to shared mem:%d, %s\n",p_he->PID, p_he->sdev_id);
  for (i = 0; i < ECG_MAX_ENTRIES; i++) {
     if( strcmp(p_he->sdev_id, EXER_NAME(i) ) == 0 ) {
	   rc = 0;
	   EXER_PID(i) = pid;
	   //print_log(LOGMSG,"added the pid:%d for %s in its ecg\n",EXER_PID(i), EXER_NAME(i));
	 break;
	 }
  }
  temp_ecg_pos = cur_ecg_pos;
  cur_ecg_pos = 0;
  for (i = 0; i < NUM_EXERS; i++) {
     if( strcmp(p_he->sdev_id, EXER_NAME(i) ) == 0 ) {
	   rc = 0;
	   EXER_PID(i) = pid;
	   //print_log(LOGMSG,"added the pid:%d for %s in ecg.all\n",EXER_PID(i), EXER_NAME(i));
	 break;
	 }
  }
  cur_ecg_pos = temp_ecg_pos;
  return(rc);
}
