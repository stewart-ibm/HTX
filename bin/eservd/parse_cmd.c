
/* @(#)35	1.26.5.5  src/htx/usr/lpp/htx/bin/eservd/parse_cmd.c, eserv_daemon, htxubuntu 9/6/15 20:11:41 */

#include <stdio.h>
#include "eservd.h"
#include "global.h"

struct parm_table
{
   char name[30];
   void* addr;
   int len;
   int* nums;
   int cmds;
   char def_val[60];
   int def_num;
};

#define PARMNUMS(i) (*((int *)(parm[i].nums)))
#define PARMNAME(i) (parm[i].name)
#define PARMCMDS(i) (parm[i].cmds)
#define PARMADDR(i) ((char ***)(parm[i].addr))

extern void make_list(int argc,char argv[][80], int *num_found);
char inp[512];

int
parse_cmd ( char *commands[512],int *num)
{
    int i, j, ind = 0, k = 0;
    char *str_ar;
    int num_parms, num_found, found;
    char dev_list[80][80];

    //name, addr, len, nums, cmds, def_val, def_num

    struct parm_table parm[] = {
	{"ecg", &ecg, 1, &num_ecg, -1, "/usr/lpp/htx/ecg/ecg.bu", 0},
	{"run", &ecg, 1, &num_ecg, 2011, "/ecg.all", 0},
	{"stop", &ecg, 1, &num_ecg, 2021, "", 0},
	{"activate", &actv, 1, &num_actv, 2022, "all", -2},
	{"suspend", &suspend, 1, &num_suspend, 2032, "all", -2},
	{"coe", &coe, 1, &num_coe, 2023, "all", -2},
	{"soe", &soe, 1, &num_soe, 2033, "all", -2},
	{"shutdown", &ecg, 1, &num_ecg, 2004, "", 0},
	{"status", &stts, 1, &num_stts, 2005, "all", -2},
	{"getstat", &ecg, 1, &num_ecg, 2006, "/ecg.all", 0},
	{"getstats", &ecg, 1, &num_ecg, 2006, "/ecg.all", 0},
	{"geterrlog", &f_err, 0, &num_f_err, 2007, "", 0},
	{"clrerrlog", &f_err, 0, &num_f_err, 2017, "", 0},
	{"geterrsum", &f_sum, 0, &num_f_sum, 2008, "", 0},
	{"clrerrsum", &f_sum, 0, &num_f_sum, 2018, "", 0},
	{"get_run_time", &ecg, 1, &num_runtime, 2042, "", 0},
	{"get_last_update_time", &ecg, 1, &num_last_update_time, 2044, "", 0},
	{"get_fail_status", &ecg, 1, &num_failstatus, 2045, "", 0},
	{"get_dev_cycles", &devcycles, 1, &num_devcycles, 2046, "", 0},
	{"addexer", &add_exer, 1, &num_addexer, 2029, "", 0},
	{"restart", &rstrt, 1, &num_rstrt, 2049, "null", -2},
	{"terminate", &term, 1, &num_term, 2069, "null", -2},
	{"getsysdata", &f_sysdata, 0, &num_sysdata, 3000, "", 0},
	{"getvpd", &f_vpd, 0, &num_vpd, 3001, "", 0},
	{"stoptest", &f_vpd, 0, &num_vpd, 1009, "", 0},
	{"query", &query, 1, &num_query, 9999, "all", -2},
	{"getecglist", &f_sum, 0, &num_f_sum, 9998, "all", -2},
	{"getecgsum", &ecg, 1, &num_ecg, 9997, "/ecg.all", 0},
	{"gettestsum", &f_sum, 0, &num_f_sum, 9996, "all", -2},
	{"getactecg", &get_actecg, 0, &num_get_actecg, 9995, "", 0},
	{"start_halted", &add_ah, 0, &num_add_ah, -1,"n", 0},
	{"start_coe", &add_coe, 0, &num_add_coe, -1, "n", 0},
	{"force", &add_force, 0, &num_add_force, -1, "n", 0},
	{"dup_device", &add_dup, 0, &num_add_dup, -1, "n", 0},
	{"nonblock", &nonblk, 0, &num_nonblk, -1, "n", 0},
	{"addecg", &ecg, 1, &num_ecg, 2010, "/usr/lpp/htx/ecg/ecg.bu", 0},
	{"getecginfo", &f_sum, 0, &num_f_sum, 2020, "/ecg.all", -2},
	{"debuglevel", &dblevel, 0, &num_dblevel, 5000, "", 0},
	{"exersetupinfo", &ecg, 1, &num_stts, 2009, "", 0}
    }; /*sampan */
    DBTRACE(DBENTRY,("enter parse_cmd.c parse_cmd\n"));

    ecg[0]        = 0;
    stop          = 0;
    actv[0]       = 0;
    suspend[0]    = 0;
    coe[0]        = 0;
    soe[0]        = 0;
    stts[0]       = 0;
    f_err         = 0;
    f_sum         = 0;
    add_exer[0]   = 0;
    rstrt[0]      = 0;
    term[0]       = 0;
    f_vpd         = 0;
    query[0]      = 0;
    devcycles[0]  = 0;
    add_ah        = 0;
    add_coe       = 0;
    add_force     = 0;
    add_dup       = 0;
    dblevel	  = 0; /*sampan */
    get_actecg	  = 0;
    nonblk	  = 0;

    num_dblevel   = 0; /*sampan */
    num_ecg       = 0;
    num_stop      = 0;
    num_actv      = 0;
    num_suspend   = 0;
    num_coe       = 0;
    num_soe       = 0;
    num_stts      = 0;
    num_f_err     = 0;
    num_f_sum     = 0;
    num_addexer   = 0;
    num_rstrt     = 0;
    num_term      = 0;
    num_vpd       = 0;
    num_query     = 0;
    num_add_ah    = 0;
    num_add_coe   = 0;
    num_add_force = 0;
    num_add_dup   = 0;
    num_get_actecg   = 0;
    num_nonblk    = 0;
    num_runtime   = 0;
    num_failstatus = 0;
    num_devcycles = 0;
    num_last_update_time = 0;

    num_found = 0;
    //num_ecg = 0;
    strcpy (dev_list[0], "none");
    for (i = 0; i < 80; i++)
	lst[i][0] = '\0';

    i = 0;
    num_parms = sizeof (parm) / sizeof (struct parm_table);
    print_log(LOGMSG,"input = %si size = %d\n", (char *) &msg_rcv, num_parms);
    fflush (stdout);
    memset (inp, 0, 512);
    sprintf (inp, "%s", (char *) &msg_rcv);
    commands[i] = strtok (inp, "-");

    while (commands[i]) {
	print_log(LOGMSG,"in while: commands = %s\n", commands[i]);
	fflush (stdout);
	i++;
	commands[i] = strtok (NULL, "-");
	if (!commands[i]) {
	    break;                 //exit(0);
	}
    };
    *num = i;

    for (i = 0; i < *num; i++)
	print_log(LOGMSG,"Commands[%d] = %s\n", i, commands[i]);

    ind = 0;
    num_found = 0;

    for (j = 0; j < *num; j++) {
	for (i = 0; i < num_parms; i++) {
	    //print_log(LOGMSG,"name[%d] =%s inp = %s\n",i,PARMNAME(i),commands[j]);
	    //fflush(stdout);

	    if (strcmp (commands[j], "stoptest") == 0) {
		msg_rcv.cmd = 1009;
		num_shtd = 0;
		break;
	    }
	    else if (strncmp (commands[j], PARMNAME (i), strlen (PARMNAME (i)))
		     == 0) {
		ind = 0;
		num_found = 0;
		//if (strcmp(PARMNAME(i),"ecg")!=0)
		    if (PARMCMDS (i) != -1)
			msg_rcv.cmd = (ushort) parm[i].cmds;
		str_ar = strtok (commands[j], " ");
		// PARMNUMS (i) = 0;

		while (str_ar) {
		    str_ar = strtok (NULL, " ");
		    if (!str_ar) {
			print_log(LOGMSG,"Found NULL: command = %s i = %d j = %d\n",
				commands[j], i, j);
			fflush (stdout);
			if (PARMNUMS(i) == 0) {
			    if (parm[i].len != 0) {
				*PARMADDR (i) = (char **) stx_malloc (80);
				print_log(LOGMSG,"parmname = %s\n", parm[i].name);
				fflush (stdout);
				strcpy ((char *)(*(PARMADDR (i) + 0)), parm[i].def_val);
				print_log(LOGMSG,"parmname = %s\n", parm[i].name);
				fflush (stdout);
				PARMNUMS (i) = parm[i].def_num;
			    }
			    else {
				print_log(LOGMSG,"mallocing\n");
				fflush (stdout);
				*((char **) (parm[i].addr)) =
				  (char *) stx_malloc (80);
				print_log(LOGMSG,"malloced\n");
				fflush (stdout);
				strcpy ((*((char **) (parm[i].addr))),
					parm[i].def_val);
				PARMNUMS (i) = parm[i].def_num;
			    }
			}
			break;
		    }
		    print_log(LOGMSG,"str_ar = %s\n", str_ar);
		    if (parm[i].len == 0) {
			ind = 0;
			*((char **) (parm[i].addr)) = str_ar;
			((char **) (parm[i].addr))[strlen (str_ar)] = '\0';
			PARMNUMS (i)++;
			print_log
			  (LOGMSG,"len 0: ind = %d name = %s val = %s nums = %d add_ah = %d num_add_coe = %d num_dup = %d\n",
			   ind, PARMNAME (i), (char *)(*(PARMADDR (i) + ind)), PARMNUMS (i),
			   num_add_ah, num_add_coe, num_add_dup);
			ind++;
		    }
		    else {

			*(PARMADDR (i) + ind) = (char **)str_ar;
			(PARMADDR (i) + ind)[strlen (str_ar)] = '\0';
			PARMNUMS (i)++;
			strcpy (dev_list[ind + 1], (char*)(*(PARMADDR (i) + ind)));
			print_log(LOGMSG,"ind+1 = %d devl = %s\n", ind + 1,
				dev_list[ind + 1]);
			print_log(LOGMSG,"len: ind = %d name = %s val = %s nums = %d\n", ind,
				(char *)PARMNAME (i), *(PARMADDR (i) + ind), PARMNUMS (i));
			fflush (stdout);
			if (strcmp (str_ar, "all") == 0) {
			    *(PARMADDR (i) + 0) = (char **)str_ar;
			    //(*((int *)(parm[i].nums))) = -2;
			    PARMNUMS (i) = -2;
			    break;
			}
			ind++;
			//fflush(stdout);

		    }
		}
	    }
	}
    }

    ind = 0;
    for (i=0; i<num_ecg; i++) {
	if (strcmp(ecg[i],"/ecg.all") == 0) {
	    strcpy(ecg[0],"/ecg.all");
	    num_ecg=0;
	    break;
	}
    }
    for (i = 0; i < num_parms; i++) {
	//if ( ( PARMNUMS(i) == -2) || (strncmp(PARMNAME(i),"ecg",3)==0) || (strncmp(PARMNAME(i),"addexer",7)==0) || (strncmp(PARMNAME(i),"start_halted",12)==0) || (strncmp(PARMNAME(i),"start_coe",3)==0) || (strncmp(PARMNAME(i),"force",5)==0) || (strncmp(PARMNAME(i),"dup_device",3)==0))
      /*if ((PARMNUMS (i) == -2) || (PARMCMDS (i) == -1)
	  || (PARMCMDS (i) == 2029) || (PARMCMDS (i) == 2010)
	  || (PARMCMDS (i) == 1009) || (PARMCMDS (i) == 2011)
	  || (PARMCMDS (i) == 2021) || (PARMCMDS (i) == 2004)
	  || (PARMCMDS (i) == 2006) || (PARMCMDS (i) == 2007)
	  || (PARMCMDS (i) == 2017) || (PARMCMDS (i) == 2008)
	  || (PARMCMDS (i) == 2018)
	 continue;*/
	    if ((PARMNUMS (i) == -2) ||
		((PARMCMDS (i) != 2022) && (PARMCMDS (i) != 2032)
		 && (PARMCMDS (i) != 2023) && (PARMCMDS (i) != 2033)
		 && (PARMCMDS (i) != 2005) && (PARMCMDS (i) != 2049)
		 && (PARMCMDS (i) != 2069) && (PARMCMDS (i) != 9999)))
		continue;
	for (j = 0; j < *num; j++) {
	    if (strncmp (commands[j], PARMNAME (i), strlen (PARMNAME (i))) == 0) {
		print_log(LOGMSG,"Command = %s:%s:\n", PARMNAME (i), commands[j]);
		print_log(LOGMSG,"num = %d\n", PARMNUMS (i));        //get the full list, as dev_list could contain wild cards
		if (PARMNUMS (i) == 0) {
		    *PARMADDR (i) = (char **) stx_malloc (10);
		    strcpy ((char*)(*(PARMADDR (i) + 0)), parm[i].def_val);
		    break;
		}
		for (k = 0; k < PARMNUMS (i); k++)
		    strcpy (dev_list[k + 1], (char*)(*(PARMADDR (i) + k)));
		make_list (k + 1, dev_list, &num_found);    //get the full list, as dev_list could contain wild cards
		for (found = 0; found < num_found; found++)
		    *(PARMADDR (i) + found) = (char**)lst[found];
		PARMNUMS (i) = num_found;
	    }
	}
    }

    print_log(LOGMSG,"returning \n");
    fflush (stdout);
    DBTRACE(DBEXIT,("return 0 parse_cmd.c parse_cmd\n"));
    return 0;
}
