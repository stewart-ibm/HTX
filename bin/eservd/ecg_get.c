
/* @(#)00	1.20.3.1  src/htx/usr/lpp/htx/bin/eservd/ecg_get.c, eserv_daemon, htxubuntu 3/14/12 04:16:56 */

#include "eservd.h"
#include "global.h"
#ifndef __OS400__    /* 400 */
#include <libgen.h>
#endif
#ifdef __OS400__    /* 400 */
#include "popen.h"
#include "libos400.h"
/* extern char * basename(char *); */
extern char * dirname(char *);
#endif
time_t ecg_mtime,ecg_ctime;
extern int num_ecgs;

int ecg_get(void)
{
  FILE *pdes;
  int num,i,rc_stat,tmp,ii;
  char buf[80],ls_cmd[80];
  struct stat ecg_stat;
  char *tmp1,*tmp2,cmd[200];
  char tmp_ecgname[64];

  DBTRACE(DBENTRY,("enter ecg_get.c ecg_get\n"));
  /*Taking file name in tmp2 var and path name in tmp1 var for feature select ecg as well as mdt*/
  strcpy(tmp_ecgname, path_for_ecgmdt);
  tmp1 = dirname(tmp_ecgname);
  strcpy(tmp_ecgname, path_for_ecgmdt);
  tmp2 = basename(tmp_ecgname);

  print_log(LOGMSG, "path_for_ecgmdt:%s, tmp1:%s, tmp2:%s", path_for_ecgmdt, tmp1, tmp2);

  /** To check weather it's a default ecg if yes then procced below code(means only default ecg) */
  if(strcmp(tmp1,".") == 0 ||(strcmp(tmp1,"/") == 0)){

    rc_stat = stat("/usr/lpp/htx/ecg",&ecg_stat);  // Check status of ecg directory
    if (rc_stat == -1) {
    print_log(LOGERR,"No ecg directory available\n"); fflush(stdout);
    DBTRACE(DBEXIT,("return -1 ecg_get.c ecg_get\n"));
    return -1;
    }
    if ( (num_ecgs > 0) &&
       (ecg_mtime == ecg_stat.st_mtime) &&
       (ecg_ctime == ecg_stat.st_ctime) ) { // if no modification or change
    print_log(LOGMSG,"No changes to ecgs.\n"); fflush(stdout);
    DBTRACE(DBEXIT,("return -2 ecg_get.c ecg_get\n"));
    return -2;
   }

   print_log(LOGMSG,"going to do popen \n"); fflush(stdout);
   pdes =  popen("ls /usr/lpp/htx/ecg|wc -l", "r");
   if (pdes == NULL) {
    print_log(LOGERR,"Error opening pipe.\n"); fflush(stdout);
    DBTRACE(DBEXIT,("return -3 ecg_get.c ecg_get\n"));
    return -3;
   }
   fgets(buf,80,pdes);
   //print_log(LOGMSG,"buf = %s\n",buf); fflush(stdout);
   pclose(pdes);

   num = atoi(buf);	// number of ecg files from ls|wc command

   ecg_mtime = ecg_stat.st_mtime;
   ecg_ctime = ecg_stat.st_ctime;

   print_log(LOGMSG,"building the ecg file list\n"); fflush(stdout);

   if ( num_ecgs == 0 ) {	// if the ecg file list is empty
    ecg_info[num_ecgs].ecg_pos = num_ecgs;	// mark the position of ecg name in the structure
    strcpy(ecg_info[num_ecgs].ecg_name,"ecg.all");	// Prepare the structure with the initial information
    strcpy(ecg_info[num_ecgs].ecg_path,"");
    strcpy(ecg_info[num_ecgs].ecg_status,"UNLOADED");	// Mark the initial status as inactive
    strcpy(ecg_info[num_ecgs].ecg_desc,"universal set of ecgs");	// No Ecg description is available
    ecg_info[num_ecgs].ecg_shmkey = 0;	// No Ecg description is available
    ecg_info[num_ecgs].ecg_semkey = 0;	// No Ecg description is available
    num_ecgs++;
   }

   tmp = 0;
   pdes = popen("ls /usr/lpp/htx/ecg","r"); /* p+ */
   for (i=0; i<num; i++) { /* p+ */
    if ( NULL == fgets(buf,80,pdes) ) { /* P+- */	// read the ecg name
      num = i;	// ran out of input lines too soon
      break;	// exit the for loop
    }
	//fscanf(pdes,"%s",buf);
	//print_log(LOGMSG,"buf[%d] = %s\n",i, buf); fflush(stdout);
    buf[strlen(buf)-1] = '\0';
    if(strcmp(buf,"/ecg.all") == 0) {	// cannot use a file by this name, skip it
      print_log(LOGMSG,"this is the logical file, filename can't be used\n"); fflush(stdout);
      continue;
     }

	 //print_log(LOGMSG,"buf[%d] = %s\n",i, buf); fflush(stdout);
     for (ii=0; ii<num_ecgs; ii++) {	 // search list for same ecg file name
    	 if (strcmp(ecg_info[ii].ecg_name, buf) == 0)
	     break;
     }
     if (ii >= num_ecgs) {		// new ecg file name, add it to the list
	    //if (strcmp(buf,"ecg") == 0) continue;
	    //print_log(LOGMSG,"here: num=%d ecg = %s. key = %d\n",num_ecgs, buf,10601+num_ecgs);
	    //fflush(stdout);
       ecg_info[num_ecgs].ecg_pos = num_ecgs;	// mark the position of ecg name in the structure
       strcpy(ecg_info[num_ecgs].ecg_name,buf);	// Prepare the structure with the initial information
       strcpy(ecg_info[num_ecgs].ecg_path,"/usr/lpp/htx/ecg");	// Prepare the structure with the initial information
       strcpy(ecg_info[num_ecgs].ecg_status,"UNLOADED");		// Mark the initial status as inactive
       strcpy(ecg_info[num_ecgs].ecg_desc," ");			// No Ecg description is available
       ecg_info[num_ecgs].ecg_shmkey = 10601+num_ecgs;		// No Ecg description is available
       ecg_info[num_ecgs].ecg_semkey = 10201+num_ecgs;		// No Ecg description is available
       num_ecgs++;
     }

  } /*p+ */
  /* Populating mdt also in shared memory structure(ecg_info) */
  /* Start Code changes for the issue defect 673733   */
   print_log(LOGMSG,"going to do popen \n"); fflush(stdout);
   pdes =  popen("ls /usr/lpp/htx/mdt|wc -l", "r");
   if (pdes == NULL) {
     print_log(LOGERR,"Error opening pipe.\n"); fflush(stdout);
     DBTRACE(DBEXIT,("return -3 ecg_get.c ecg_get\n"));
     return -3;
   }
   fgets(buf,80,pdes);
   print_log(LOGMSG,"buf = %s\n",buf); fflush(stdout);
   pclose(pdes);

   num = atoi(buf);     // number of ecg files from ls|wc command
   print_log(LOGMSG,"Number of mdts(only while reading mdt) = %d",num);

   print_log(LOGMSG,"building the ecg file list\n"); fflush(stdout);

   if ( num_ecgs == 0 ) {       // if the ecg file list is empty
      ecg_info[num_ecgs].ecg_pos = num_ecgs;      // mark the position of ecg name in the structure
      strcpy(ecg_info[num_ecgs].ecg_name,"ecg.all");      // Prepare the structure with the initial in formation
      strcpy(ecg_info[num_ecgs].ecg_path,"");
      strcpy(ecg_info[num_ecgs].ecg_status,"UNLOADED");   // Mark the initial status as inactive
      strcpy(ecg_info[num_ecgs].ecg_desc,"universal set of ecgs");        // No Ecg description is available
      ecg_info[num_ecgs].ecg_shmkey = 0;  // No Ecg description is available
      ecg_info[num_ecgs].ecg_semkey = 0;  // No Ecg description is available
      num_ecgs++;
   }

   /* End Code changes for the issue defect 673733  */
   tmp = 0;
   pdes = popen("ls /usr/lpp/htx/mdt","r"); /* p+ */
   for (i=0; i<num; i++) { /* p+ */
    if ( NULL == fgets(buf,80,pdes) ) { /* P+- */       // read the ecg name
      num = i;  // ran out of input lines too soon
      break;    // exit the for loop
    }
        //fscanf(pdes,"%s",buf);
        //print_log(LOGMSG,"buf[%d] = %s\n",i, buf); fflush(stdout);
     buf[strlen(buf)-1] = '\0';
     if(strcmp(buf,"/ecg.all") == 0) {   // cannot use a file by this name, skip it
      print_log(LOGMSG,"this is the logical file, filename can't be used\n"); fflush(stdout);
      continue;
     }

        //print_log(LOGMSG,"buf[%d] = %s\n",i, buf); fflush(stdout);
     for (ii=0; ii<num_ecgs; ii++) {     // search list for same ecg file name
       if (strcmp(ecg_info[ii].ecg_name, buf) == 0)
         break;
     }
      /* Populating mdt also in shared memory structure(ecg_info) */
     if (ii >= num_ecgs) {       // new mdt file name, add it to the list
        //if (strcmp(buf,"ecg") == 0) continue;
        //print_log(LOGMSG,"here: num=%d ecg = %s. key = %d\n",num_ecgs, buf,10601+num_ecgs);
        //fflush(stdout);
       ecg_info[num_ecgs].ecg_pos = num_ecgs;    // mark the position of ecg name in the structure
       strcpy(ecg_info[num_ecgs].ecg_name,buf);  // Prepare the structure with the initial information
       strcpy(ecg_info[num_ecgs].ecg_path,"/usr/lpp/htx/mdt");   // Prepare the structure with the initial information
       strcpy(ecg_info[num_ecgs].ecg_status,"UNLOADED");         // Mark the initial status as inactive
       strcpy(ecg_info[num_ecgs].ecg_desc," ");                  // No Ecg description is available
       ecg_info[num_ecgs].ecg_shmkey = 10601+num_ecgs;           // No Ecg description is available
       ecg_info[num_ecgs].ecg_semkey = 10201+num_ecgs;           // No Ecg description is available
       num_ecgs++;
     }
	 //print_log(LOGMSG,"Ending i = %d \n", i); fflush(stdout);
   } /* p+ */

  pclose(pdes);

  print_log(LOGMSG,"NUMECGS = %d\n", num_ecgs); fflush(stdout);
  DBTRACE(DBEXIT,("return ecg_get.c ecg_get\n"));
  return num_ecgs;
 }

  else
  {
    rc_stat = stat(tmp1,&ecg_stat);  // Check status of ecg directory

   if (rc_stat == -1) {
    print_log(LOGERR,"No ecg directory available\n"); fflush(stdout);
    DBTRACE(DBEXIT,("return -1 ecg_get.c ecg_get\n"));
    return -1;
    }
   //num_ecgs=0;
  /* if ( (num_ecgs > 0) &&
       (ecg_mtime == ecg_stat.st_mtime) &&
       (ecg_ctime == ecg_stat.st_ctime) ) { // if no modification or change
     print_log(LOGMSG,"No changes to ecgs.\n"); fflush(stdout);
     DBTRACE(DBEXIT,("return -2 ecg_get.c ecg_get\n"));
     return -2;
    }*/

   print_log(LOGMSG,"going to do popen \n"); fflush(stdout);
   sprintf(cmd,"ls %s|wc -l",tmp1);

   pdes=popen(cmd,"r");
   if (pdes == NULL) {
     print_log(LOGERR,"Error opening pipe.\n"); fflush(stdout);
     DBTRACE(DBEXIT,("return -3 ecg_get.c ecg_get\n"));
     return -3;
   }
   fgets(buf,80,pdes);
   //print_log(LOGMSG,"buf = %s\n",buf); fflush(stdout);
   pclose(pdes);

   num = atoi(buf);     // number of ecg files from ls|wc command

   ecg_mtime = ecg_stat.st_mtime;
   ecg_ctime = ecg_stat.st_ctime;

   print_log(LOGMSG,"building the ecg file list\n"); fflush(stdout);

   if ( num_ecgs == 0 ) {       // if the ecg file list is empty
     ecg_info[num_ecgs].ecg_pos = num_ecgs;     // mark the position of ecg name in the structure
     strcpy(ecg_info[num_ecgs].ecg_name,"ecg.all");     // Prepare the structure with the initial information
     strcpy(ecg_info[num_ecgs].ecg_path,"");
     strcpy(ecg_info[num_ecgs].ecg_status,"UNLOADED");  // Mark the initial status as inactive
     strcpy(ecg_info[num_ecgs].ecg_desc,"universal set of ecgs");       // No Ecg description is available
     ecg_info[num_ecgs].ecg_shmkey = 0; // No Ecg description is available
     ecg_info[num_ecgs].ecg_semkey = 0; // No Ecg description is available
     num_ecgs++;
   }

   tmp = 0;
   sprintf(cmd,"ls %s",tmp1);
   pdes=popen(cmd,"r");
   for (i=0; i<num; i++) { /* p+ */
     if ( NULL == fgets(buf,80,pdes) ) { /* P+- */      // read the ecg name
       num = i; // ran out of input lines too soon
       break;   // exit the for loop
     }
         //fscanf(pdes,"%s",buf);
         //print_log(LOGMSG,"buf[%d] = %s\n",i, buf); fflush(stdout);
     buf[strlen(buf)-1] = '\0';

     strcpy(tmp_ecgname, buf);
     tmp1  = basename(tmp_ecgname);
     strcpy(buf, tmp1);
     print_log(LOGMSG, "buf:%s", buf);

     if(strcmp(buf,"/ecg.all") == 0) {  // cannot use a file by this name, skip it
       print_log(LOGMSG,"this is the logical file, filename can't be used\n"); fflush(stdout);
       continue;
     }

        //print_log(LOGMSG,"buf[%d] = %s\n",i, buf); fflush(stdout);
     for (ii=0; ii<num_ecgs; ii++) {    // search list for same ecg file name
       if (strcmp(ecg_info[ii].ecg_name, buf) == 0)
         break;
     }
     if (ii >= num_ecgs) {      // new ecg file name, add it to the list
         //if (strcmp(buf,"ecg") == 0) continue;
         //print_log(LOGMSG,"here: num=%d ecg = %s. key = %d\n",num_ecgs, buf,10601+num_ecgs);
         //fflush(stdout);
       ecg_info[num_ecgs].ecg_pos = num_ecgs;   // mark the position of ecg name in the structure
       strcpy(ecg_info[num_ecgs].ecg_name,buf); // Prepare the structure with the initial information
       strcpy(ecg_info[num_ecgs].ecg_path,tmp1);        // Prepare the structure with the initial information
       strcpy(ecg_info[num_ecgs].ecg_status,"UNLOADED");                // Mark the initial status as inactive
       strcpy(ecg_info[num_ecgs].ecg_desc," ");                 // No Ecg description is available
       ecg_info[num_ecgs].ecg_shmkey = 10601+num_ecgs;          // No Ecg description is available
       ecg_info[num_ecgs].ecg_semkey = 10201+num_ecgs;          // No Ecg description is available
       num_ecgs++;
     }
         //print_log(LOGMSG,"Ending i = %d \n", i); fflush(stdout);
   } /* p+ */
   pclose(pdes);

   print_log(LOGMSG,"NUMECGS = %d\n", num_ecgs); fflush(stdout);
   DBTRACE(DBEXIT,("return ecg_get.c ecg_get\n"));
   return num_ecgs;
   }
}

int user_ecg_get(char * full_ecgname, char * result_msg)
{
  char tmp_ecgname[56];
  /*char full_name[56];*/
  char workstr[128];
  char ecg_names[128][56];
  char ecg_name[30];
  char ecg_path[40];
  char line[128];
  char prev_workdir[128];

  char *tmp1, *tmp2;
  FILE *fp;
  int i, ind=0, ecg_req, ii,rc_stat;
  struct stat ecg_stat;
  DBTRACE(DBENTRY,("enter ecg_get.c user_ecg_get\n"));

  strcpy(tmp_ecgname,full_ecgname);
  tmp1 = dirname(tmp_ecgname);
  if ( strcmp(tmp1, ".") == 0 ) {
    print_log(LOGMSG,"no path was given along with the ecg name\n");
    strcpy(ecg_path, "/usr/lpp/htx/ecg");
    sprintf(full_name,"/usr/lpp/htx/ecg/");
  } else {
    strcpy(ecg_path,tmp1);
    sprintf(full_name,"%s/",tmp1);
  }


  strcpy(tmp_ecgname,full_ecgname);
  tmp2 = basename(tmp_ecgname);

  strcpy(ecg_name, tmp2);
  strcat(full_name,tmp2);

  print_log(LOGMSG,"ecg_path: %s ecg_name: %s full_name: %s\n",
	 ecg_path, ecg_name, full_name);
  if (strchr(full_name,'*')) {
    print_log(LOGMSG,"wild character found, let me analyze it\n");
    fp = popen("pwd","r");
    if ( fp == NULL ) {
      print_log(LOGERR,"error opening pipe\n");
      DBTRACE(DBEXIT,("return/a -1 ecg_get.c user_ecg_get\n"));
      return(-1);
    }
    if (fgets(prev_workdir,128,fp) == NULL) {
      print_log(LOGERR,"directory doesn't exist\n");
      pclose(fp);
      DBTRACE(DBEXIT,("return/b -1 ecg_get.c user_ecg_get\n"));
      return(-1);
    }
    print_log(LOGMSG,"curr dir: %s\n",prev_workdir);
    pclose(fp);
    chdir(ecg_path);
    sprintf(workstr,"ls %s",ecg_name);
    fp = popen(workstr,"r");
    if ( fp == NULL ) {
      print_log(LOGERR,"error opening pipe\n");
      DBTRACE(DBEXIT,("return/c -1 ecg_get.c user_ecg_get\n"));
      return(-1);
    }
    while( fgets(line,128,fp) != NULL ) {
      tmp1 = strchr(line, '\n');
      *tmp1 = '\0';
      tmp1 = strtok(line,"");
      if ( tmp1 == NULL ) {
	print_log(LOGERR,"file doesn't exist\n");
	pclose(fp);
	DBTRACE(DBEXIT,("return/d -1 ecg_get.c user_ecg_get\n"));
	return(-1);
      }
      strcpy(ecg_names[ind++],tmp1);
      print_log(LOGMSG,"name:%s..\n",ecg_names[ind-1]);
      while( (tmp1 = strtok( NULL, "")) != NULL ) {
	strcpy(ecg_names[ind++],tmp1);
	print_log(LOGMSG,"name:%s..\n",ecg_names[ind-1]);
      }
    }
    pclose(fp);
    chdir(prev_workdir);
    ecg_req = ind;
  } else {
    print_log(LOGMSG,"single file to add\n");
    rc_stat = stat(full_name, &ecg_stat);
    if (rc_stat == -1) {
      print_log(LOGERR," the requested ecg doesn't exist\n"); fflush(stdout);
      sprintf(result_msg," the requested ecg doesn't exist");
      DBTRACE(DBEXIT,("return/e -1 ecg_get.c user_ecg_get\n"));
      return -1;
    }
#ifdef __OS400__    /* 400 */
    if ( S_ISDIR(ecg_stat.st_mode))
#else
      if ( (ecg_stat.st_mode & S_IFMT) == S_IFDIR )
#endif
      {
	print_log(LOGERR,"the requested ecgname is directory\n");
	DBTRACE(DBEXIT,("return/f -1 ecg_get.c user_ecg_get\n"));
	return -1;
      }
    ecg_req = 1;
    strcpy(ecg_names[0],ecg_name);
  }
  print_log(LOGMSG,"ecg_req:%d,\n",ecg_req);

  for ( ii = 0; ii < ecg_req ; ii++ ) {
    sprintf(full_name,"%s/%s",ecg_path,ecg_names[ii]);
    rc_stat = stat(full_name, &ecg_stat);
    if (rc_stat == -1) {
      print_log(LOGERR," the requested ecg doesn't exist\n"); fflush(stdout);
      sprintf(result_msg," the requested ecg doesn't exist");
      continue;
    }
#ifdef __OS400__    /* 400 */
    if ( S_ISDIR(ecg_stat.st_mode))
#else
      if ( (ecg_stat.st_mode & S_IFMT) == S_IFDIR )
#endif
      {
	print_log(LOGERR,"the requested ecgname is directory\n");
	continue;
      }
    for (i=0; i<num_ecgs; i++) {
      if ((strcmp(ecg_info[i].ecg_name, ecg_names[ii]) == 0) && (strcmp(ecg_info[i].ecg_path, ecg_path ) == 0 ))
	break;
    }
    if (i < num_ecgs) { // found in upper loop
      print_log(LOGERR," ecg already present in the list\n");
    } else {
      ecg_info[num_ecgs].ecg_pos = num_ecgs;                            // mark the position of ecg name in the structure
      strcpy(ecg_info[num_ecgs].ecg_name,ecg_names[ii]);               // Prepare the structure with the initial information
      strcpy(ecg_info[num_ecgs].ecg_path,ecg_path);
      strcpy(ecg_info[num_ecgs].ecg_status,"UNLOADED");                         // Mark the initial status as inactive
      strcpy(ecg_info[num_ecgs].ecg_desc," ");                                  // No Ecg description is available
      ecg_info[num_ecgs].ecg_shmkey = 10601+num_ecgs;                                // No Ecg description is available
      ecg_info[num_ecgs].ecg_semkey = 10201+num_ecgs;                                // No Ecg description is available
      num_ecgs++;
      print_log(LOGMSG,"added the ecg %s to list, num_ecgs:%d\n", ecg_names[ii],num_ecgs);
    }
  }
  sprintf(result_msg," added the ecgs to the list");
  DBTRACE(DBEXIT,("return ecg_get.c user_ecg_get\n"));
  return (num_ecgs);
}


/*
 * Argument
 *   ret_str    : (char *) to store return string
 * Return       : (char *) running ecg/mdt name
 * Note         : active_ecg_name is a global variable declared in global.h
 */
char *get_running_ecg_name(char *ret_str) 
{
        if (ret_str != NULL) {
                if ( strlen (active_ecg_name) > 0) {
                        sprintf (ret_str, "%s%s\n%s\n",
					"Currently running ECG/MDT : ",
					active_ecg_name,
					"===========================");
                }
                else {
                        strcpy (ret_str, "Currently running ECG/MDT : No ECG/MDT is running currently\n===========================\n");
                }

        	return ret_str;
        } else {

		return active_ecg_name;
	}
}



char *attach_ecg_name_front(char *ret_str)
{
	char tmp_str[1024];
	char *tmp_ret_str;

	if (ret_str != NULL)
	{
		get_running_ecg_name (tmp_str);
		tmp_ret_str = malloc (strlen (tmp_str) + strlen (ret_str) + 10);
		if (tmp_ret_str == NULL) {
			return ret_str;
		}
		sprintf (tmp_ret_str, "%s\n%s", tmp_str, ret_str);
		strcpy ( ret_str, tmp_ret_str);
		free (tmp_ret_str);
	}
	
	return ret_str;
}


/***************************     END OF ecg_get.c   ***************************************/ 

