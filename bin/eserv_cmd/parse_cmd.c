/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* htxltsbml src/htx/usr/lpp/htx/bin/eserv_cmd/parse_cmd.c 1.12.5.3       */
/*                                                                        */
/* Licensed Materials - Property of IBM                                   */
/*                                                                        */
/* Restricted Materials of IBM                                            */
/*                                                                        */
/* COPYRIGHT International Business Machines Corp. 2003,2012              */
/* All Rights Reserved                                                    */
/*                                                                        */
/* US Government Users Restricted Rights - Use, duplication or            */
/* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.      */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

/* @(#)91	1.12.5.4  src/htx/usr/lpp/htx/bin/eserv_cmd/parse_cmd.c, eserv_cmd, htxubuntu 12/11/12 06:02:22 */

extern char file_sut[40];
extern int cmd;
#include <stdio.h>

int parse_inp(char *inp, char *sut_ip)
{
int i,j,num,found,rc=0,ip_found = 0, num_found=0, num_list_ecg=0;
int got_ecg=0, got_name =0, got_path=0, add_ecg =1, got_list=0, exer_setup_info=0;
char *tmp_ptr1, ecgpath[40], ecgname[16];
char *commands[512], *ip, tmp_inp[1024];
char *ecg_list[80] ;
char new_inp[10*1024];
int runFound = FALSE, nonblockFound = FALSE;
char *valid_parms[] = {"-sut", "-ecg ", "-run ", "-stop ", "-activate ", "-suspend ", "-coe ", "-soe ", "-shutdown ", "-status ", "-getstats ", "-geterrlog ", "-geterrsum ", "-addexer ", "-terminate ", "-restart ", "-getvpd", "-getsysdata", "-getmediachk", "-getcablechk", "-getactecg", "-query ", "-f ", "-cmd ", "-clrerrlog ", "-clrerrsum ", "-stoptest ", "-getecglist ", "-gettestsum ", "-getecgsum ", "-start_halted ", "-start_coe ", "-force ", "-dup_device ", "-addecg", "-exersetupinfo", "-nonblock", "-get_run_time", "-get_dev_cycles", "-get_last_update_time", "-get_fail_status"};

char *tmp_inp_str = NULL;
char *tmp_inp_ptr = NULL;

/****  initialize the environment variable *******/

   if ( (tmp_ptr1 = (char *)getenv("ECGPATH")) == (char *)0) {
     /*printf("ECGPATH is not set");*/
     sprintf(ecgpath,"");
   } else {
     if ( tmp_ptr1[strlen(tmp_ptr1)-1] == '/') 
        strcpy(ecgpath,tmp_ptr1);
     else
        sprintf(ecgpath,"%s/",tmp_ptr1);
     /*printf("ECGPATH is set to %s\n",ecgpath);*/
     got_path = 1;
   }   

   if ( (tmp_ptr1 = (char *)getenv("ECGNAME")) == (char *)0) {
      sprintf(ecgname,"");
   } else {
     /*printf("ECGNAME is set to %s\n",tmp_ptr1);*/
     strcpy(ecgname,tmp_ptr1);
     got_name = 1;
   }   

   if ( (tmp_ptr1 = (char *)getenv("ECGLIST")) != (char *)0) {
     /*printf("ECGLIST is set to %s\n",tmp_ptr1);*/
     got_list = 1;
     num_list_ecg = 0;
     ecg_list[num_list_ecg++] = strtok( tmp_ptr1, " "); 
     /*printf("ecg_list[%d]:%s\n",num_list_ecg-1,ecg_list[num_list_ecg-1]);*/
     while( (ecg_list[num_list_ecg++] = strtok( NULL, " ")) != NULL);
     num_list_ecg--;    /* it is increased twice in last loop */
   }
/*** split the string into commands ********/

   i=0;
   /*printf("inp:%s:\n",inp);*/
   tmp_inp_str = malloc (strlen (inp) + 1);
   if (tmp_inp_str == NULL) {
      	printf("ERROR: malloc failed, exiting...\n");
	exit(0);
   }
   strcpy (tmp_inp_str, inp);
   commands[i] = (char *)strtok(inp,"-");

   while (commands[i]) {
     i++;
     commands[i] = (char *)strtok(NULL,"-");
     if (!commands[i]) {
        break;
     }
   };

   num = i;

   if (num <= 0) {
      memset(inp,0,1024);
      sprintf(inp,"");
      if (tmp_inp_str != NULL)
      {
	free (tmp_inp_str);
	tmp_inp_str = NULL;
      }
      return -1;
   }


/*** search for the host ip address in the command ******/
   
   for (j=0; j<num; j++) {
       if (strncmp(commands[j],"sut",3) ==0) {
          ip_found = 1;
          ip = (char *)strtok(commands[j]," ");
          ip = (char *)strtok(NULL," ");
          if ( ip == NULL) {
             memset(inp,0,1024);
             sprintf(inp,"Specify IP address of the system you want to connect to \n\n");
      	     if (tmp_inp_str != NULL)
     	     {
		free (tmp_inp_str);
		tmp_inp_str = NULL;
      	     }
             return -1;
             break;
          }
               strncpy(sut_ip,ip,strlen(ip));
               sut_ip[strlen(ip)] = '\0';
       }
   }

/****** ip wasn't found in the command ******/
   if (ip_found == 0) {
      if ( (tmp_ptr1 = (char *)getenv("SUT")) != (char *)0) {  /*if SUT is set, use it*/
          /*printf("connecting to the server: %s\n",tmp_ptr1);*/
          strcpy(sut_ip,tmp_ptr1);
          ip_found = 1;
      } else {        /* SUT wasn't set, wrong usage */
          memset(inp,0,1024);
          sprintf(inp, "Specify -sut parameter.\n");
     	  if (tmp_inp_str != NULL)
      	  {
	    free (tmp_inp_str);
	    tmp_inp_str = NULL;
     	  }
          return -1;
      }
   }
   else if (num <1) {
      memset(inp,0,1024);
      sprintf(inp,"Incorrect use of esrv command. Use correct options");
      if (tmp_inp_str != NULL)
      {
	free (tmp_inp_str);
	tmp_inp_str = NULL;
      }
      return -1;
   }

   for (j=0; j<num; j++) {
	if (strncmp(commands[j],"run",3) ==0) {
	  runFound = TRUE;
	}
	if (strncmp(commands[j],"nonblock",8) ==0) {
	  nonblockFound = TRUE;
	}
   }
   if ( (runFound == FALSE) && (nonblockFound == TRUE) ) {
      memset(inp,0,1024);
      sprintf(inp,"Incorrect use of esrv command, 'nonblock' option should be with 'run' only.");
      if (tmp_inp_str != NULL)
      {
	free (tmp_inp_str);
	tmp_inp_str = NULL;
      }
      return -1;
   }
   memset(new_inp,0,1024);

/**** parse each command and rearrange them in desired order ******/

   for (j=0; j<num; j++) {
     found = 0;
     for (i=0; i<(sizeof(valid_parms)/sizeof(char *)); i++) {
         if (strncmp(commands[j],(valid_parms[i]+1),(strlen(valid_parms[i])-1))==0) {
            found++; 
            num_found++; 
            //printf("cmd = %s\n", commands[j]);
            if (strncmp(commands[j],"cmd",3) ==0) {
	       tmp_inp_ptr = strstr (tmp_inp_str, "-cmd");
	       strcat (new_inp, tmp_inp_ptr + 4);

		  add_ecg = 0;
                  cmd = 9022;
            }
            else if (strncmp(commands[j],"sut",3) !=0) {
               if ( (strncmp(commands[j],"run",3)== 0) 
                  || (strncmp(commands[j],"stop",4)== 0) 
                  || (strncmp(commands[j],"getecgsum",9)== 0)
				  || (strncmp(commands[j],"getvpd",6)==0)) {
	            /*printf("adding the env path\n");*/
                    add_ecg = 0; /* don't add -ecg option, as they don't use it with these commands*/
                    strcat(new_inp,"-");
		    tmp_ptr1 = (char *)strtok(commands[j]," ");
		    strcat(new_inp,tmp_ptr1);
                    strcat(new_inp," ");
                    tmp_ptr1 = (char *)strtok(NULL," ");
                    if ( tmp_ptr1 == NULL ) {
                       if ( got_name) {
                          strcat(new_inp,ecgpath);
                          strcat(new_inp,ecgname);
                          strcat(new_inp," ");
                       }
                       if ( got_list) {
                          for ( j = 0; j < num_list_ecg; j++) {  
                             strcat(new_inp,ecg_list[j]);
                             strcat(new_inp," ");
                          }
                       }
                       strcat(new_inp," ");
                    } else {
                      do {
                         if ( tmp_ptr1[0] != '/' )
                            strcat(new_inp,ecgpath);
                         strcat(new_inp,tmp_ptr1);
                         strcat(new_inp," ");
                      }while ( (tmp_ptr1 = (char *)strtok(NULL," ")) != NULL ); 
                    } 
	       }else if(strncmp(commands[j],"ecg",3)== 0) {
                    got_ecg = 1;
                    strcat(new_inp,"-");
                    tmp_ptr1 = (char *)strtok(commands[j]," ");
                    strcat(new_inp,tmp_ptr1);
                    strcat(new_inp," ");
                    while ( (tmp_ptr1 = (char *)strtok(NULL," ")) != NULL ) {
                         if ( tmp_ptr1[0] != '/' )
                            strcat(new_inp,ecgpath);
                         strcat(new_inp,tmp_ptr1);
                         strcat(new_inp," ");
                    } 
               }else if (strncmp(commands[j],"addecg",6) == 0) {
                    /*printf("adding the env path\n");*/
                    add_ecg = 0; /* don't add -ecg option, as they don't use it with these commands*/
                    strcat(new_inp,"-");
                    tmp_ptr1 = (char *)strtok(commands[j]," ");
                    strcat(new_inp,tmp_ptr1);
                    strcat(new_inp," ");
                    while ( (tmp_ptr1 = (char *)strtok(NULL," ")) != NULL ) {
                         if ( tmp_ptr1[0] != '/' )
                            strcat(new_inp,ecgpath);
                         strcat(new_inp,tmp_ptr1);
                         strcat(new_inp," ");
                    }
               }else if (strncmp(commands[j],"getactecg",9) == 0) {    /* simply skipping option -ecg from the command, as it is not being used with this command */
			strcat(new_inp,"-");
			strcat(new_inp,commands[j]);
			strcat(new_inp," ");
			add_ecg = 0;
               } else  {

                    if (strncmp(commands[j],"exersetupinfo", 13) == 0) {
						exer_setup_info = 1;
					}
					strcat(new_inp,"-");
                    strcat(new_inp,commands[j]);
                    strcat(new_inp," ");
                    if (strncmp(commands[j],"f ",2) ==0) {
                        cmd = 9023;
                        strcpy(file_sut, commands[j]+2);
                    }
               }
            }
            break;
         }
     }
     if (cmd == 9022) {
	break;
     }

     if (found == 0) {
        memset(tmp_inp,0,1024);
        sprintf(tmp_inp, "esrv: Unrecognized option: %s",commands[j]);
        memset(inp,0,1024);
        sprintf(inp,"%s",tmp_inp);
        if (tmp_inp_str != NULL)
        {
	  free (tmp_inp_str);
	  tmp_inp_str = NULL;
        }
        return -1;
        break;
     }
   } /* for */

/****** if -ecg wasn't specified, add the default *********/
   if ( (got_ecg == 0 ) && add_ecg ) {
      if (exer_setup_info == 1) {
		  sprintf(inp, "Specify -ecg <ecg_name> along with exersetupinfo option\n");
      		  if (tmp_inp_str != NULL)
      		  {
			  free (tmp_inp_str);
		  	tmp_inp_str = NULL;
                  }
		  return -1;
	  }

	  strcat(new_inp,"-ecg ");
      if ( got_name) {
           strcat(new_inp, ecgpath);
           strcat(new_inp, ecgname);
           strcat(new_inp," ");
      } 
      if ( got_list ) {
           for ( j = 0; j < num_list_ecg; j++) {
                strcat(new_inp,ecg_list[j]);
                strcat(new_inp," ");
           }
      }

            
   }

   memset(inp,0,1024);
   strcpy(inp,new_inp);
   /*printf("inp:%s:\n",inp);*/
   if (tmp_inp_str != NULL)
   {
	free (tmp_inp_str);
	tmp_inp_str = NULL;
   }
   return rc;
}
