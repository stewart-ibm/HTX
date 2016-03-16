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


#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <syslog.h>
#include "eservd.h"
#include "global.h"


void print_log(int a, const char *fmt, ...) /* This function takes variable no. of arguments, first as the debug level and rest any message to be printed */
{
        va_list args;
        int i, lenstr;
        int priority; 
	char *buf = NULL ;
        char *msg_buf = NULL ;
        long long j;
        char *newstr;
	char *tfmt;
#if 0
	tfmt = fmt;
        lenstr = strlen(fmt);
        /*printf("start string:%d\n", lenstr);*/
        va_start(args, fmt);
        while (*fmt ) {
           if ( *fmt++ == '%') {
               switch(*fmt) {
                   case 's':
                          newstr = va_arg(args, char *);
                          /*printf("got a string %s, len:%d\n", newstr, strlen(newstr));*/
                          lenstr += strlen(newstr);
                          break;

                    case 'c' :
                          newstr = va_arg(args, char *);
                          lenstr += 1;
                          /*printf("added 1, lenstr:%d\n", lenstr);*/
                          break;

                   default :
                          if ( *(fmt+1) == 'l' ) {
                               j = va_arg(args, long long);
                               lenstr += 8;
                               /*printf("added 8, lenstr:%d\n", lenstr);*/
                               break;
                          } else {
                               i = va_arg(args, int);
                               lenstr += 4;
                               /*printf("added 4, lenstr:%d\n", lenstr);*/
                               break;
                          }

                }
           }
        }

        /*printf("obtained string len : %d\n", lenstr );*/
        if ( lenstr > (4096-100) ) {
             buf = malloc(lenstr);
             msg_buf = malloc(lenstr+ 100);
        } else  {
             buf = malloc(4096);
             msg_buf = malloc(4096);
        }
        va_end(args);
#endif
             buf = malloc(4096); 
             msg_buf = malloc(4096);

        va_start(args, fmt);
        vsnprintf(buf,4096 - 100,fmt, args); /* 11 for msg_buf data*/
        va_end(args);

        switch(a){ /*based on tag add the corresponding string to the message buffer */

                case LOGERR:
                         strcpy(msg_buf,"ERROR :");
                        priority = LOG_ERR;  
			break;
                case LOGMSG:
                        strcpy(msg_buf,"MESSAGE  :");
                         priority = LOG_INFO; 
			  break;
                case LOGDB1:
                       strcpy(msg_buf,"DB1 :");
                       priority = LOG_DEBUG; 
			break;
                case LOGDB2:
                         strcpy(msg_buf,"DB2 :");
                       priority = LOG_DEBUG; 
                        break;
                default:
                         strcpy(msg_buf,"UNKNOWN LEVEL: ");
                         priority = LOG_INFO;
			break;
                }
	/*printf("a = %d, msg_buf = %s, buf = %s\n",a,msg_buf,buf);*/
        if(a<=LEVEL)   /* compare the level of current message level with level defined...add to log file only if its less than or equals */
        {
                strncat(msg_buf,buf,2000); /* concatinate the message with message buffer */
                openlog("STX_LOG",LOG_NDELAY, LOG_LOCAL5); 
                syslog( priority,msg_buf); /* adding message to syslog */
		closelog();  
                }
      if ( buf != NULL ) {
            /*printf("buf = 0x%x\n", buf );*/
            free(buf);
      }
      if ( msg_buf != NULL ) {
            /*printf("msg_buf = 0x%x\n", msg_buf );*/
            free(msg_buf);
      }

}
