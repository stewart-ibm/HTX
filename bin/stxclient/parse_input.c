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

/* @(#)59	1.8  src/htx/usr/lpp/htx/bin/stxclient/parse_input.c, eserv_gui, htxubuntu 5/24/04 17:08:03 */

#include "hxssup.h"
tfull_info   info_rcv;
extern tscn_1 scn_1_in;
extern char level[];

int parse_input_5(char *str1,int *rc, char *result_msg, int len)
{
int i,num,j,msglen;

char *s2[512],*s1[512],tmpstr[80],*str;


   i=0;
   str = str1;
   str[len] = 0;
   s2[i] = strtok(str,"*");
   if (s2[i] == (char *)0)
      s2[i] = strtok(str,"@");

   do {
     i++;
     s2[i] = strtok((char *)0,"*");
   } while(s2[i] != (char *)0);

   num = i-1;
   i=0;
   j=-1;

   do {
     j++;
   if (s2[j] == (char *)0)
      break;
     s1[i] = strtok(s2[j],"@");

     do {
       i++;
       s1[i] = strtok((char *)0,"@");
       //printf("s1[%d] = %s\n",i,s1[i]);
     } while(s1[i] != (char *)0);
   } while(s2[j] != (char *)0);

   num = i-1;
   i=0;

   memset(tmpstr,0,80);
   *rc = atoi(s1[i++]);

   msglen = (strlen(s1[i]) > 79)?79:strlen(s1[i]);
   strncpy(result_msg,s1[i],msglen);
   result_msg[msglen] = '\0';
   s1[i++];

   strcpy(tmpstr,s1[i++]); // No need to grab the level
   memset(tmpstr,0,80);
   sprintf(info_rcv.sys_hdr_info.str_curr_time,"%s",s1[i++]);
     sprintf(tmpstr,"  %s",s1[i++]);
     strcat(info_rcv.sys_hdr_info.str_curr_time,tmpstr);
     sprintf(tmpstr,"  %s",s1[i++]);
     strcat(info_rcv.sys_hdr_info.str_curr_time,tmpstr);
   sprintf(info_rcv.cur_info.str_start_time,"%s",s1[i++]);
     sprintf(tmpstr,"  %s",s1[i++]);
     strcat(info_rcv.cur_info.str_start_time,tmpstr);
     sprintf(tmpstr,"  %s",s1[i++]);
     strcat(info_rcv.cur_info.str_start_time,tmpstr);
   if (*rc == -99) {
      info_rcv.cur_info.max_entries = 0;
      info_rcv.cur_info.num_entries = 0;
      *rc = 0;
      return 0;
   }
   info_rcv.cur_info.error_flag = atoi(s1[i++]);
   info_rcv.cur_info.max_cycles_done = atoi(s1[i++]);
   info_rcv.cur_info.min_cycles_done = atoi(s1[i++]);
   info_rcv.cur_info.max_entries = atoi(s1[i++]);
   info_rcv.cur_info.num_entries = atoi(s1[i++]);

   /*printf("the number of max entries: %d\n",info_rcv.cur_info.max_entries); fflush(stdout);
     getch();*/
   if ( info_rcv.cur_info.max_entries > 0 ) {
     j=0;
     memset(tmpstr,0,80);
     do {
       sprintf(info_rcv.scn_num.scn_5_info[j].status,"%s",s1[i++]);

       strcpy(info_rcv.scn_num.scn_5_info[j].sdev_id,s1[i++]);

       memset(info_rcv.scn_num.scn_5_info[j].upd_time,0,13);
       sprintf(info_rcv.scn_num.scn_5_info[j].upd_time,"%s",s1[i++]);
       sprintf(tmpstr," %s",s1[i++]);

       strcat(info_rcv.scn_num.scn_5_info[j].upd_time,tmpstr);

       info_rcv.scn_num.scn_5_info[j].cycles = atoi(s1[i++]);
       info_rcv.scn_num.scn_5_info[j].test_id = atoi(s1[i++]);
       info_rcv.scn_num.scn_5_info[j].err_ack = atoi(s1[i++]);

       sprintf(info_rcv.scn_num.scn_5_info[j].err_time,"%s",s1[i++]);
       sprintf(tmpstr," %s",s1[i++]);
       tmpstr[strlen(tmpstr)] = '\0';
       strcat(info_rcv.scn_num.scn_5_info[j].err_time,tmpstr);

       j++;
     } while(i<num);
   }

}

int parse_input_2(char *str1,int *rc, char *result_msg, int len)
{
int i,num,j,msglen;

char *s2[512],*s1[512],tmpstr[80],*str;


   i=0;
   str = str1;
   str[len] = '\0';
   s2[i] = strtok(str,"*");
   if (s2[i] == (char *)0)
      s2[i] = strtok(str,"@");

   do {
     i++;
     s2[i] = strtok((char *)0,"*");
   } while(s2[i] != (char *)0);

   num = i-1;
   i=0;
   j=-1;

   do {
     j++;
     if (s2[j] == (char *)0)
        break;
     s1[i] = strtok(s2[j],"@");

     do {
       i++;
       s1[i] = strtok((char *)0,"@");
      // printf("s1[%d] = %s\n",i,s1[i]);
       //fflush(stdout);
       //getch();
     } while(s1[i] != (char *)0);
   } while(s2[j] != (char *)0);

   num = i-1;
   i=0;

   *rc = atoi(s1[i++]);

   msglen = (strlen(s1[i]) > 79)?79:strlen(s1[i]);
   strncpy(result_msg,s1[i],msglen);
   result_msg[msglen] = '\0';
   s1[i++];

   sprintf(level,"%s",s1[i++]);
   s1[i++];
   sprintf(info_rcv.sys_hdr_info.str_curr_time,"%s",s1[i++]);
     sprintf(tmpstr," %s",s1[i++]);
     strcat(info_rcv.sys_hdr_info.str_curr_time,tmpstr);
   if (*rc == -99) {
      info_rcv.cur_info.num_entries = 0;
      *rc = 0;
      return 0;
   }
   if (*rc != 0) 
      return 0;
   
   info_rcv.cur_info.num_entries = atoi(s1[i++]);

   j=1;
   memset(tmpstr,0,80);
   while(i<num) {
     sprintf(info_rcv.scn_num.scn_2_4_info[j].status,"%s",s1[i++]);

     strcpy(info_rcv.scn_num.scn_2_4_info[j].sdev_id,s1[i++]);

     sprintf(info_rcv.scn_num.scn_2_4_info[j].adapt_desc,"%s",s1[i++]);
     sprintf(info_rcv.scn_num.scn_2_4_info[j].device_desc,"%s",s1[i++]);
     sprintf(info_rcv.scn_num.scn_2_4_info[j].slot_port,"%s",s1[i++]);
     j++;
   }
}

int parse_input_6(char *str1,int *rc, char *result_msg,char *file, int len)
{
int i,num,j,msglen;

char *s2[1024],*str;


   i=0;
   str = str1;
   str[len] = '\0';
   s2[i] = strtok(str,"@");

   do {
     i++;
     s2[i] = strtok((char *)0,"@");
   } while(s2[i] != (char *)0);

   num = i-1;
   i=0;
   j=-1;

   *rc = atoi(s2[i++]);
   /*if (*rc == -99) {
      sprintf(result_msg,"System not started");
      sprintf(file," ");
      *rc = 0;
      return 0;
   }*/
   msglen = (strlen(s2[i]) > 79)?79:strlen(s2[i]);
   strncpy(result_msg,s2[i],msglen);
   result_msg[msglen] = '\0';
   s2[i++];
   if (*rc < 0) {
      sprintf(file," ");
      return 0;
   }
   //strncpy(result_msg,s2[i++],79);

   //sprintf(file,s2[i++]);
   strcpy(file,s2[i++]);
}

void parse_input_9(char *str1,int *rc, char *result_msg, int len)
{
int i,num,j,msglen;

char *s2[1024],*str;


   i=0;
   str = str1;
   str[len] = '\0';
   s2[i] = strtok(str,"@");

   do {
     i++;
     s2[i] = strtok((char *)0,"@");
   } while(s2[i] != (char *)0);

   num = i-1;
   i=0;
   j=-1;

   *rc = atoi(s2[i++]);
   msglen = (strlen(s2[i]) > 79)?79:strlen(s2[i]);
   strncpy(result_msg,s2[i],msglen);
   result_msg[msglen] = '\0';
   s2[i++];
   //strncpy(result_msg,s2[i++],79);

}


void parse_input_1(char *str1,int *rc, char *result_msg, int len, char *ecg)
{
int i,num,j,msglen;

char *s2[1024],*str,tmpstr[80];


   i=0;
   str = str1;
   //printf("s = %s\n",str1);
   //fflush(stdout);
   //getch();
   str[len] = '\0';
   s2[i] = strtok(str,"@");

     //printf("s2[%d] = %s\n",i,s2[i]);
   do {
     i++;
     s2[i] = strtok((char *)0,"@");
     //printf("s2[%d] = %s\n",i,s2[i]);
   } while(s2[i] != (char *)0);

   num = i-1;
   i=0;
   j=-1;

   memset(tmpstr,0,80);
   *rc = atoi(s2[i++]);

   memset(result_msg,0,80);
   msglen = (strlen(s2[i]) > 79)?79:strlen(s2[i]);
   strncpy(result_msg,s2[i],msglen);
   result_msg[msglen] = '\0';

   s2[i++];
   sprintf(level,"%s",s2[i++]);
   level[strlen(level)] = '\0';
   s2[i++];
   sprintf(scn_1_in.sys_hdr_info.str_curr_time,"%s",s2[i++]);
   sprintf(tmpstr," %s",s2[i++]);
   strcat(scn_1_in.sys_hdr_info.str_curr_time,tmpstr);
   sprintf(ecg,s2[i++]);
   if (*rc == -99) {
      *rc = 0;
      //scn_1_in.sys_hdr_info.running_halted = 99;
      //strcpy(scn_1_in.sys_hdr_info.running_halted,"INACTIVE");
      //sprintf(ecg,"ecg.bu");
      //return;
   }
   //scn_1_in.sys_hdr_info.running_halted = atoi(s2[i++]);
   sprintf(scn_1_in.sys_hdr_info.running_halted,"%s",s2[i++]);
}

int parse_input_0(char *str1,int *rc, char *result_msg, int len)
{
int i,num,j,msglen;

char *s2[512],*s1[512],tmpstr[80],*str;

   if (strcmp(result_msg,"No ecg directory existent")== 0 )
           return 0;
   i=0;
   str = str1;
   str[len] = '\0';
   s2[i] = strtok(str,"*");
   if (s2[i] == (char *)0)
      s2[i] = strtok(str,"@");

   do {
     i++;
     s2[i] = strtok((char *)0,"*");
   } while(s2[i] != (char *)0);

   num = i-1;
   i=0;
   j=-1;

   do {
     j++;
     if (s2[j] == (char *)0)
        break;
     s1[i] = strtok(s2[j],"@");

     do {
       i++;
       s1[i] = strtok((char *)0,"@");
     } while(s1[i] != (char *)0);
   } while(s2[j] != (char *)0);

   num = i-1;
   i=0;

   *rc = atoi(s1[i++]);

   msglen = (strlen(s1[i]) > 79)?79:strlen(s1[i]);
   strncpy(result_msg,s1[i],msglen);
   result_msg[msglen] = '\0';
   s1[i++];

   sprintf(level,"%s",s1[i++]);
   s1[i++];
   sprintf(info_rcv.sys_hdr_info.str_curr_time,"%s",s1[i++]);
     sprintf(tmpstr," %s",s1[i++]);
     strcat(info_rcv.sys_hdr_info.str_curr_time,tmpstr);
   if (*rc == -99) {
      info_rcv.cur_info.num_entries = 0;
      *rc = 0;
      return 0;
   }
   //if (*rc != 0) 
   //   return 0;
   
   info_rcv.cur_info.num_entries = atoi(s1[i++]);

   j=1;
   memset(tmpstr,0,80);
   while(i<num) {
     sprintf(info_rcv.scn_num.scn_0_info[j].ecg_status,"%s",s1[i++]);

     strcpy(info_rcv.scn_num.scn_0_info[j].ecg_abs_name,s1[i++]);

     sprintf(info_rcv.scn_num.scn_0_info[j].ecg_desc,"%s",s1[i++]);
     j++;
   }
}
