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
#include <string.h>

#define FALSE 0
#define TRUE 1

/*char list[80][80];*/ // = {"sarma","satya","rp","neeraj","vaidya","sreekanth","jk","rpm","daljeet","prasanjeet","anand"};
char (* list)[80];

void buildPatterns(int argc,char argv[][80],char***patterns);
char* squeeze(char* pattern);
int isMatched(char* str,char* pattern);
extern int init_ecg (char *);

extern char lst[80][80];

int orig_list(void)
{
    int i;
    union shm_pointers shm_addr_wk;
    extern union shm_pointers shm_addr;

    struct htxshm_HE *p_htx_HE;
    DBTRACE(DBENTRY,("enter list.c orig_list\n"));

    list = malloc((ECGSHMADDR_HDR)->max_entries * 80);

    //shm_addr_wk.hdr_addr = shm_addr.hdr_addr; /* copy addr to work space  */
    shm_addr_wk.hdr_addr = ECGSHMADDR_HDR; /* copy addr to work space  */
    (shm_addr_wk.hdr_addr)++;  /* skip over header                  */

    print_log(LOGMSG,"NUM_ENTRIES list = %d\n",(ECGSHMADDR_HDR)->num_entries); fflush(stdout);
    print_log(LOGMSG,"MAX_ENTRIES list = %d\n",(ECGSHMADDR_HDR)->max_entries); fflush(stdout);
/*  print_log(LOGMSG,"NUM_ENTRIES list = %d\n",(shm_addr.hdr_addr)->num_entries); fflush(stdout);
  print_log(LOGMSG,"MAX_ENTRIES list = %d\n",(shm_addr.hdr_addr)->max_entries); fflush(stdout);*/
    for (i=0; i<(shm_addr.hdr_addr)->max_entries; i++) {
	p_htx_HE = shm_addr_wk.HE_addr + i;
	//print_log(LOGMSG,"Dev = %s \n", p_htx_HE->sdev_id);
	strcpy(list[i], p_htx_HE->sdev_id);
    }

    DBTRACE(DBEXIT,("return list.c orig_list\n"));
    return (ECGSHMADDR_HDR)->max_entries;
}


void make_list(int argc,char argv[][80], int *num_found)
{
  int i,list_cnt;
  char* pattern=NULL;
  char* str, result_msg[150];
  char **patterns=NULL;
    //static int num_found;
  int num_entries;
  int ecg_cnt, tmp, found_ecg;
  DBTRACE(DBENTRY,("enter list.c make_list\n"));

  print_log(LOGMSG,"in make_list agc = %d argv[0]=%s \n", argc, argv[0]);
  fflush(stdout);
  if (argc < 2)
  {
    print_log(LOGERR,"\n wrong usage!!!\n\nUsage:\n\tprs <pattern(s)>\n\n");
    exit(1);
  }

  print_log(LOGMSG,"num_ecg= %d num_ecgs = %d\n", num_ecg, num_ecgs);
  for (ecg_cnt=1; ecg_cnt<num_ecgs; ecg_cnt++) {
    found_ecg=0;
    cur_ecg_pos = ecg_cnt;
    if (num_ecg == 0) {
      if (strcmp(ECGSTATUS,"UNLOADED")==0) {
	if ( (msg_rcv.cmd == 2001  &&  msg_rcv.subcmd == 1) ||
	     (msg_rcv.cmd == 2011) || (msg_rcv.cmd == 2012) ||
	     (msg_rcv.cmd == 2022) || (msg_rcv.cmd == 2032) ||
	     (msg_rcv.cmd == 2013) || (msg_rcv.cmd == 2023) ||
	     (msg_rcv.cmd == 2033) || (msg_rcv.cmd == 2005) ||
	     (msg_rcv.cmd == 9999) ) {
	  if (init_ecg(result_msg) <0)
	    continue;
	  else {
	    shm_addr.hdr_addr = ECGSHMADDR_HDR;
	    shm_addr.HE_addr = ECGSHMADDR_HE;
	    semhe_id = ECGSEMHEID;
	    found_ecg=1;
	  }
	}
	else
	  continue;
      }
      else {
	shm_addr.hdr_addr = ECGSHMADDR_HDR;
	shm_addr.HE_addr = ECGSHMADDR_HE;
	semhe_id = ECGSEMHEID;
	found_ecg=1;
      }
    }
    else {
      for (tmp=0; tmp<num_ecg; tmp++) {
		//print_log(LOGMSG,"passed name = %s: ECGNAME = %s/%s:status=:%s:\n", ecg[tmp],ECGPATH,ECGNAME, ECGSTATUS); fflush(stdout);
	PUT_FULL_ECG;
	if ((strcmp(ecg[tmp], full_name) ==0) && (strcmp(ECGSTATUS,"UNLOADED") !=0)) {
	  shm_addr.hdr_addr = ECGSHMADDR_HDR;
	  shm_addr.HE_addr = ECGSHMADDR_HE;
	  semhe_id = ECGSEMHEID;
	  found_ecg=1;
	  break;
	}
	else if ((strcmp(ecg[tmp], full_name) ==0) && (strcmp(ECGSTATUS,"UNLOADED") ==0)) {
		    //print_log(LOGMSG,"unloaded ...passed name = %s: ECGNAME = %s/%s:status=%s\n", ecg[tmp],ECGPATH,ECGNAME, ECGSTATUS); fflush(stdout);
	  if ( (msg_rcv.cmd == 2001  &&  msg_rcv.subcmd == 1) ||
	       (msg_rcv.cmd == 2011) || (msg_rcv.cmd == 2012) ||
	       (msg_rcv.cmd == 2022) || (msg_rcv.cmd == 2032) ||
	       (msg_rcv.cmd == 2013) || (msg_rcv.cmd == 2023) ||
	       (msg_rcv.cmd == 2033) || (msg_rcv.cmd == 2005) ||
	       (msg_rcv.cmd == 9999) ) {
	    if (init_ecg(result_msg) <0) // init_ecg not successful
	      continue;
	    else {
	      shm_addr.hdr_addr = ECGSHMADDR_HDR;
	      shm_addr.HE_addr = ECGSHMADDR_HE;
	      semhe_id = ECGSEMHEID;
	      found_ecg=1;
	    }
	  } // msg_rcv
	  else // Simple command, ecg need not be loaded
	    continue;

	}
      }
    }

    if (!found_ecg)
      continue;

    num_entries = orig_list();
    print_log(LOGMSG,"in num = %d\n",num_entries);
    fflush(stdout);

    buildPatterns(argc,argv,&patterns);

    print_log(LOGMSG,"\nMatched list are \n");

    for(list_cnt=0;list_cnt<num_entries;list_cnt++)
    {
      for(i=0;i<argc-1;i++)
      {
	pattern=patterns[i];
	str=list[list_cnt];
	if(isMatched(str,pattern))
	{
	  strcpy(lst[(*num_found)++], str);
	  print_log(LOGMSG," %s \n",str);
	  break;
	}
      }
    }
	/*for ( i = 0; i < argc-1; i++ )
	{
		if ( patterns[i] != NULL ) {
			free(patterns[i]);
			patterns[i]=0;
		}

	}
	if ( pattern != NULL ) {
	   free(pattern);
	   pattern=0;
	}*/

  }

  DBTRACE(DBEXIT,("leave list.c make_list\n"));
}//END OF MAIN

int isMatched(char* str,char* pattern)
{
    int pat_cnt, str_cnt,k,start=0;
    int found_flag=TRUE;
    DBTRACE(DBENTRY,("enter list.c isMatched\n"));

    found_flag=TRUE;
    start=0;

	/*starting text matching i.e before first '*' or find starting pattern counter for rest of the program*/

    while((pattern[start]!='\0')&&(str[start]!='\0')&&(pattern[start]!='*'))
    {
	if(pattern[start]!=str[start])
	{
	    DBTRACE(DBEXIT,("return/a FALSE list.c isMatched\n"));
	    return FALSE;
	}
	start++;
    }// end of while which is used find start counter

	//this check is to find exact match i.e RP to RP

	if(str[start]=='\0'&&pattern[start]=='\0')
	{
	    DBTRACE(DBEXIT,("return/b TRUE list.c isMatched\n"));
	    return TRUE;
	}

    for(k=0 ; str[k];k++)
    {
	str_cnt=k;
	for(pat_cnt = start; pattern[pat_cnt]; pat_cnt++)
	{
	    if (pattern[pat_cnt] == '*')
	    {
		pat_cnt++;
		while ((str[str_cnt] != '\0') && (pattern[pat_cnt] != str[str_cnt]))
		    str_cnt++;
		if(str[str_cnt]=='\0'||pattern[pat_cnt]=='\0')
		    break;
	    }
	    else
	    {
		if (pattern[pat_cnt] != str[str_cnt])
		{
		    break;
		}
	    }
	    str_cnt++;
	}
	if(str[str_cnt]=='\0'&&pattern[pat_cnt]=='\0')
	{
	    DBTRACE(DBEXIT,("return/c TRUE list.c isMatched\n"));
	    return TRUE;
	}
    }
    DBTRACE(DBEXIT,("return/d FALSE list.c isMatched\n"));
    return FALSE;
}// END OF isMatched() function

void buildPatterns(int argc, char argv[][80], char***patterns)
{
    int i;
    DBTRACE(DBENTRY,("enter list.c buildPatterns\n"));

    //allocate memory to patterns

    *patterns=NULL;
    *patterns=(char**)stx_malloc(sizeof(char*)*(argc-1));

    //first squeeze pattern and then allocate memory for it

	for(i=1;i<argc;i++)
	{
	    print_log(LOGMSG,"\npattern  %s   ",argv[i]);
	    (*patterns)[i-1] = NULL;
	    (*patterns)[i-1]=squeeze(argv[i]);
	    print_log(LOGMSG,"squeezed pattern  %s  ",(*patterns)[i-1]);
	}

    DBTRACE(DBEXIT,("leave list.c buildPatterns\n"));
}

char *squeeze(char *pattern)
{
    char temp[100];
    char* target;
    int i=0,j;
    DBTRACE(DBENTRY,("enter list.c squeeze\n"));
    for ( j=0; pattern[j]; )
    {

	temp[i++]=pattern[j];
	//print_log(LOGMSG,"tmpe[%d] = %d\n",j, pattern[j]); fflush(stdout);
	//if this is * then skip all * followed this *

		/*if (!( ((pattern[j] >= 48) && (pattern[j] <= 57)) || ((pattern[j] >= 65) &&  (pattern[j] <= 90)) || ((pattern[j] >= 97) &&  (pattern[j] <= 122)) ))

		{
			while (!(((pattern[j] >= 48) && (pattern[j] <= 57)) || ((pattern[j] >= 65) &&  (pattern[j] <= 90)) || ((pattern[j] >= 97) &&  (pattern[j] <= 122)))) j++;
		}
		else
			j++;*/

	    if((pattern[j]=='*') || (pattern[j]==10) || (pattern[j]==13))

	    {
		while((pattern[j]=='*') || (pattern[j]==10) || (pattern[j]==13)) j++;
	    }
	    else
		j++;
    }

    temp[i]='\0';  /* i indicates length of squeezed string */
    /* allocate memory to target */
    target=(char*)stx_malloc((i+1)*sizeof(char));//allocate 1 more for '\0'
    strcpy(target,temp);
    DBTRACE(DBEXIT,("return list.c squeeze\n"));
    return target;
}
