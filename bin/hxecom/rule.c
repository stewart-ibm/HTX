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

/* @(#)00	1.3.1.16  src/htx/usr/lpp/htx/bin/hxecom/rule.c, exer_com, htxubuntu 3/17/15 03:08:45 */

/*
 *   COMPONENT_NAME: exer_com
 *
 *   FUNCTIONS: SetHwDefaults
 *		SetStanzaDefaults
 *		get_rule
 *		get_line
 */

#include <string.h>
#include <ctype.h>

#ifdef NTDEF
#include <winsock.h>
#include <stdio.h>
#include <limits.h>
#include "exit.h"
#include "ntdef.h"
#else
#include "hxecomdef.h"
#endif

static int  get_line(FILE *fd, char *s, int lim);
static void SetStanzaDefaults(struct rule_format * r_ptr);
static void SetHwDefaults(struct rule_format * r_ptr);

extern   struct htx_data htx_ds;

/****************************************************************************/
/*****  g e t _ r u l e ( )  ************************************************/
/****************************************************************************/
/*                                                                          */
/* FUNCTION NAME =     get_rule()                                           */
/*                                                                          */
/* DESCRIPTIVE NAME =  Get rule from rules file.                            */
/*                                                                          */
/* FUNCTION =          Reads a rule from the rules file, checks each        */
/*                     keyword value, and assigns value to rules data       */
/*                     structure.                                           */
/*                                                                          */
/* INPUT =             line - pointer to integer containing line number in  */
/*                            number in file where to begin rule            */
/*                            processing.                                   */
/*                     r_ptr - pointer to rules data structure.             */
/*                     pattern_nm - string array for name of pattern file.  */
/*                                                                          */
/* OUTPUT =            Integer pointed to by line argument is updated to   */
/*                            value of line number at the end of the        */
/*                            current rule.                                 */
/*                     Updated rules data structure.                        */
/*                     pattern_nm array set to contain the name of the      */
/*                            specified pattern file.                       */
/*                                                                          */
/* NORMAL RETURN =     0 to indicate successful completion.                 */
/*                     -1 to indicate EOF.                                  */
/*                                                                          */
/* ERROR RETURNS =     1 - Invalid rule.                                    */
/*                                                                          */
/* EXTERNAL REFERENCES                                                      */
/*                                                                          */
/*    OTHER ROUTINES = hxsmsg() - updates data for supervisor and sends a   */
/*                                message to the message handler program    */
/*                                (hxsmsg).                                 */
/*                                                                          */
/****************************************************************************/

int get_rule(FILE *fd, int *line, struct rule_format * r_ptr)
{
   	char            error;			/* global error flag                */
   	char            lerror;			/* field error flag                 */
   	char            keywd[80];		/* keyword buffer                   */
   	char            s[200];			/* rules file line buffer           */
   	char            ss[5][80];		/* buffer for operands              */
   	int             first_line;		/* line # of 1st stanza keyword     */
   	int             i;				/* loop counter                     */
   	int             keywd_count;	/* keyword count                    */
   	int             rc;				/* return code                      */
   	int		   		LoadDefFlag; 	/* Only load defaults once ea stanza*/
   	int             Stanza1Flag; 	/* First stanza?                    */
	char 			msg_text [1024]; 
	struct htx_data * stats = (struct htx_data *)&htx_ds;
	
	/****************************************************************************/
	/* The hardware parameters can only be set once.  If multiple HW values are */
	/* in the rule file, the last value will be used.                           */
	/****************************************************************************/
   	if(*line == 0) {
      	SetHwDefaults(r_ptr);
      	Stanza1Flag = 1;
   	}
   	else {
      	Stanza1Flag = 0;
   	}
   	LoadDefFlag = 1;
   	keywd_count = 0;
   	error  = 'n';
   	lerror = 'n';
   	first_line = *line + 1;
   	while ((get_line(fd, s, 200)) > 1) {
      	/* We don't want to disturb settings on a  blank line         */
      	if(LoadDefFlag) {
         	LoadDefFlag = 0;
         	SetStanzaDefaults(r_ptr);
      	}
      	*line = *line + 1;
      	if (s[0] == '*')
	 		continue;
      	for (i = 0; s[i] != '\n'; i++) {
		 	if (s[i] == '=')
	    		s[i] = ' ';
      	}				/* endfor */
      	keywd_count++;
      	sscanf(s, "%s", keywd);
      	for(i=0;i < (int)strlen(keywd);i++)
	 		keywd[i] = (char) toupper((int) keywd[i]);

   		if ((strcmp(keywd, "RULE_ID")) == 0) {
 			sscanf(s, "%*s %s", r_ptr->rule_id);
 			if ((strlen(r_ptr->rule_id)) > 8) {
    			sprintf(msg_text, "line# %d %s = %s (must be 8 characters or less)\n", *line, keywd, r_ptr->rule_id);
    			hxfmsg(stats, HERINFO1, HTX_HE_SOFT_ERROR, msg_text);
    			error = 'y';
 			}			/* endif */
   		} else if ((strcmp(keywd, "PATTERN_ID")) == 0) {
			sscanf(s, "%*s %s", r_ptr->pattern_id);
 			if ((strlen(r_ptr->pattern_id)) > 8 || Stanza1Flag == 0) {
    			sprintf(msg_text, "line# %d %s = %s (must be 8 characters or less) FIRST STANZA ONLY.\n", *line, keywd, r_ptr->pattern_id);
    			hxfmsg(stats, HERINFO2, HTX_HE_SOFT_ERROR, msg_text);
    			error = 'y';
       		}
   		} else if ((strcmp(keywd, "COMNET_NAME")) == 0) {
 			sscanf(s, "%*s %s", r_ptr->ComName);
 			if ((strlen(r_ptr->ComName)) > HXECOM_HOSTNAMELEN || Stanza1Flag == 0) {
    			sprintf(msg_text, "line# %d %s = %s (must be %d characters or less) FIRST STANZA ONLY.\n", *line, keywd, r_ptr->ComName, HXECOM_HOSTNAMELEN);
    			hxfmsg(stats, HERINFO3, HTX_HE_SOFT_ERROR, msg_text);
    			error = 'y';
       		}
   		} else if ((strcmp(keywd, "TESTNET_NAME")) == 0) {
 			sscanf(s, "%*s %s", r_ptr->TestNetName);
 			if ((strlen(r_ptr->TestNetName)) > HXECOM_HOSTNAMELEN || Stanza1Flag == 0) {
    			sprintf(msg_text, "line# %d %s = %s (must be %d characters or less) FIRST STANZA ONLY.\n", *line, keywd, r_ptr->TestNetName, HXECOM_HOSTNAMELEN);
    			hxfmsg(stats, HERINFO3, HTX_HE_SOFT_ERROR, msg_text);
    			error = 'y';
       		}
   		} else if ((strcmp(keywd, "LAYER")) == 0) {
 			sscanf(s, "%*s %s", ss[0]);
       		for(i=0;i < (int)strlen(ss[0]);i++)
    			ss[0][i] = (char) toupper((int) ss[0][i]);
       		if(strcmp(ss[0], "UDP") == 0 && Stanza1Flag == 1) {
       			r_ptr->layer = UDP_LAYER;
       		}
       		else if(strcmp(ss[0], "TCP") == 0 && Stanza1Flag == 1) {
       			r_ptr->layer = TCP_LAYER;
       		}
       		else if(strcmp(ss[0], "RDMA") == 0 && Stanza1Flag == 1) {
       			r_ptr->layer = RDMA_LAYER;
				printf("RDMA_LAYER selected = %d\n", r_ptr->layer);
       		}
 			else {
    			sprintf(msg_text, "line# %d %s = %s (must be UDP or TCP)  FIRST STANZA ONLY.\n",*line, keywd, ss[0]);
    			hxfmsg(stats, HERINFO4, HTX_HE_SOFT_ERROR, msg_text);
    			error = 'y';
       		}
   		} else if ((strcmp(keywd, "COM_STREAM_PORT")) == 0) {
 			sscanf(s, "%*s %hu", &(r_ptr->ComPortStream));
 			if (r_ptr->ComPortStream < 5000 || Stanza1Flag == 0) {
    			sprintf(msg_text, "line# %d %s = %hu (must be greater than 5000) FIRST STANZA ONLY.\n",*line, keywd, r_ptr->ComPortStream);
    			hxfmsg(stats, HERINFO5, HTX_HE_SOFT_ERROR, msg_text);
    			error = 'y';
 			}			/* endif */
   		} else if ((strcmp(keywd, "COM_DGRAM_PORT")) == 0) {
 			sscanf(s, "%*s %hu", &(r_ptr->ComPortDgram));
 			if (r_ptr->ComPortDgram < 5000 || Stanza1Flag == 0) {
    			sprintf(msg_text, "line# %d %s = %hu (must be greater than 5000) FIRST STANZA ONLY.\n", *line, keywd, r_ptr->ComPortDgram);
    			hxfmsg(stats, HERINFO6, HTX_HE_SOFT_ERROR, msg_text);
    			error = 'y';
			}			/* endif */
   		} else if ((strcmp(keywd, "COM_RDMA_PORT")) == 0) {
 			sscanf(s, "%*s %hu", &(r_ptr->ComPortDapl));
 			if (r_ptr->ComPortDapl < 5000 || Stanza1Flag == 0) {
    			sprintf(msg_text, "line# %d %s = %hu (must be greater than 5000) FIRST STANZA ONLY.\n",*line, keywd, r_ptr->ComPortStream);
    			hxfmsg(stats, HERINFO5, HTX_HE_SOFT_ERROR, msg_text);
    			error = 'y';
 			}			/* endif */
			printf("COM_RDMA_PORT = 0x%x\n", r_ptr->ComPortDapl);
   		} else if ((strcmp(keywd, "REPLICATES")) == 0) {
 			sscanf(s, "%*s %hu", &(r_ptr->replicates));
 			if (r_ptr->replicates < 1 || r_ptr->replicates >50 || Stanza1Flag == 0) {
    			sprintf(msg_text, "line# %d %s = %hu (must be >= 1 and <=50) FIRST STANZA ONLY.\n", *line, keywd, r_ptr->replicates);
    			hxfmsg(stats, HERINFO7, HTX_HE_SOFT_ERROR, msg_text);
    			error = 'y';
 			}			/* endif */
   		} else if ((strcmp(keywd, "BUFMIN")) == 0) {
 			sscanf(s, "%*s %hu", &(r_ptr->bufmin));
 			if (r_ptr->bufmin < 1) {
    			sprintf(msg_text, "line# %d %s = %hu (must be 1 or greater) \n", *line, keywd, r_ptr->bufmin);
    			hxfmsg(stats, HERINFO9, HTX_HE_SOFT_ERROR, msg_text);
    			error = 'y';
 			}			/* endif */
   		} else if ((strcmp(keywd, "BUFMAX")) == 0) {
 			sscanf(s, "%*s %hu", &(r_ptr->bufmax));
 			if (r_ptr->bufmax < 1) {
    			sprintf(msg_text, "line# %d %s = %hu (must be 1 or greater) \n", *line, keywd, r_ptr->bufmax);
    			hxfmsg(stats, HERINFO10, HTX_HE_SOFT_ERROR, msg_text);
    			error = 'y';
 			}			/* endif */
   		} else if ((strcmp(keywd, "BUFINC")) == 0) {
 			sscanf(s, "%*s %hi", &(r_ptr->bufinc));
 			if (r_ptr->bufinc < -1) {
    			sprintf(msg_text, "line# %d %s = %hu (must be -1 or greater) \n", *line, keywd, r_ptr->bufinc);
    			hxfmsg(stats, HERINFO11, HTX_HE_SOFT_ERROR, msg_text);
    			error = 'y';
 			}			/* endif */
		} else if((strcmp(keywd, "BUFSEED")) == 0) { 
			sscanf(s, "%*s %hu", &(r_ptr->bufseed));
			if (r_ptr->bufseed < 1 || Stanza1Flag == 0) {	
				sprintf(msg_text, "line# %d %s = %hu (must be 0 or greater) FIRST STANZA ONLY. \n", *line, keywd, r_ptr->bufseed);
				hxfmsg(stats, HERINFO11, HTX_HE_SOFT_ERROR, msg_text);
				error = 'y';
			} 
   		} else if ((strcmp(keywd, "ACK_TRIG")) == 0)  {
 			sscanf(s, "%*s %hu", &(r_ptr->ack_trig));
			/*	 if (r_ptr->ack_trig < 0) {
    				sprintf(msg_text,
		   				"line# %d %s = %hu (must be 0 or greater) \n",
		   				*line, keywd, r_ptr->ack_trig);
   					hxfmsg(stats, HERINFO12, HTX_HE_SOFT_ERROR, msg_text);
	    			error = 'y';
	 			}
		     */
      	} else if ((strcmp(keywd, "NUM_OPER")) == 0) {
	 		sscanf(s, "%*s %hu", &(r_ptr->num_oper));
	 		if (r_ptr->num_oper < 1) {
	    		sprintf(msg_text, "line# %d %s = %hu (must be 1 or greater) \n", *line, keywd, r_ptr->num_oper);
	    		hxfmsg(stats, HERINFO13, HTX_HE_SOFT_ERROR, msg_text);
	    		error = 'y';
	 		}			/* endif */
      	} else if ((strcmp(keywd, "WRITE_AHEAD")) == 0) {
	 		sscanf(s, "%*s %hu", &(r_ptr->write_ahead));
	 		if (r_ptr->write_ahead < 1 || r_ptr->write_ahead > ACK_MAX) {
	    		sprintf(msg_text, "line# %d %s = %hu (must be > 1 and <= %d) \n", *line, keywd, r_ptr->write_ahead, ACK_MAX); 
	    		hxfmsg(stats, HERINFO14, HTX_HE_SOFT_ERROR, msg_text);
	    		error = 'y';
	 		}			/* endif */
      	} else if ((strcmp(keywd, "ONESYS")) == 0) {
			sscanf(s, "%*s %hu", &(r_ptr->onesys));
		 	if (r_ptr->onesys < 0 || r_ptr->onesys > 1) {
				sprintf(msg_text, "line# %d %s = %hu (must be 0 or 1) \n", *line, keywd, r_ptr->onesys);
				hxfmsg(stats, HERINFO14, HTX_HE_SOFT_ERROR, msg_text);
				error = 'y';
		 	}			/* endif */
      	} else if ((strcmp(keywd, "DEBUG_PATTERN")) == 0) {
			sscanf(s, "%*s %hu", &(r_ptr->debug_pattern));
		 	if (r_ptr->debug_pattern < 0 || r_ptr->debug_pattern > 1) {
				sprintf(msg_text, "line# %d %s = %hu (must be 0 or 1) \n", *line, keywd, r_ptr->debug_pattern);
				hxfmsg(stats, HERINFO14, HTX_HE_SOFT_ERROR, msg_text);
				error = 'y';
		 	}			/* endif */
      	} else if ((strcmp(keywd, "NO_COMPARE")) == 0) {
			sscanf(s, "%*s %hu", &(r_ptr->no_compare));
		 	if (r_ptr->no_compare < 0 || r_ptr->no_compare > 1) {
				sprintf(msg_text, "line# %d %s = %hu (must be 0 or 1) \n", *line, keywd, r_ptr->no_compare);
				hxfmsg(stats, HERINFO14, HTX_HE_SOFT_ERROR, msg_text);
				error = 'y';
		 	}			/* endif */
      	} else if ((strcmp(keywd, "WRITE_SLEEP")) == 0) {
	 		sscanf(s, "%*s %d", &i);
         	r_ptr->write_sleep = 0xffff & i;
	 		if (i < 0) {
	    		sprintf(msg_text, "line# %d %s = %d (must be >= 0) \n", *line, keywd, i);
	    		hxfmsg(stats, HERINFO15, HTX_HE_SOFT_ERROR, msg_text);
	    		error = 'y';
	 		}			/* endif */
      	} else if ((strcmp(keywd, "IO_ALARM_TIME")) == 0) {
	 		sscanf(s, "%*s %d", &i);
         	r_ptr->alarm = 0xffff & i;
	 		if (i < 0) {
	    		sprintf(msg_text, "line# %d %s = %d (must be >= 0) \n", *line, keywd, i);
	    		hxfmsg(stats, HERINFO16, HTX_HE_SOFT_ERROR, msg_text);
	    		error = 'y';
	 		}			/* endif */
      	} else if ((strcmp(keywd, "SHUTDOWN_FLAGS")) == 0) {
	 		sscanf(s, "%*s %hx", &(r_ptr->shutdown));
	 		if (r_ptr->shutdown < 0) {
	    		sprintf(msg_text, "line# %d %s = %d (must be >= 0).\nShutdown flag should be positive value. Check hxecom.readme for possible values.\n", *line, keywd, r_ptr->shutdown); 
	    		hxfmsg(stats, HERINFO17, HTX_HE_SOFT_ERROR, msg_text);
	    		error = 'y';
	 		}
      	} else if ((strcmp(keywd, "MASTER")) == 0) {
	 		ss[0][0] = '\0';
	 		sscanf(s, "%*s %s", ss[0]);
	 		switch (ss[0][0]) {
	 			case '\0':
           			break;
	 			case 'Y':
         		case 'y':
	    			r_ptr->transact |= MASTER_VAL;
           			break;
	 			case 'N':
         		case 'n':
	    			r_ptr->transact &= ~((u_short)MASTER_VAL);
	    			break;
	 			default:
	    			lerror = 'y';
	    			break;
	 		}			/* endswitch */
         	if(lerror == 'y' || Stanza1Flag == 0) {
	    		sprintf(msg_text, "line# %d %s = %s (value must be Y(yes) or N(no)) FIRST STANZA ONLY.\n", *line, keywd, ss[0]);
	    		hxfmsg(stats, HERINFO18, HTX_HE_SOFT_ERROR, msg_text);
           		error  = 'y';
           		lerror = 'n';
         	}
      	} else if ((strcmp(keywd, "TCP_NODELAY")) == 0) {
	 		ss[0][0] = '\0';
	 		sscanf(s, "%*s %s", ss[0]);
	 		switch (ss[0][0]) {
	 			case '\0':
           			break;
	 			case 'Y':
         		case 'y':
	    			r_ptr->transact |= TCP_NODELAY_VAL;
           			break;
	 			case 'N':
         		case 'n':
	    			r_ptr->transact &= ~((u_short)TCP_NODELAY_VAL);
	    			break;
	 			default:
	    			lerror = 'y';
	    			break;
	 		}			/* endswitch */
         	if(lerror == 'y' || Stanza1Flag == 0) {
	    		sprintf(msg_text, "line# %d %s = %s (value must be Y(yes) or N(no)) FIRST STANZA ONLY (%d).\n", *line, keywd, ss[0], Stanza1Flag);
	    		hxfmsg(stats, HERINFO18, HTX_HE_SOFT_ERROR, msg_text);
           		error  = 'y';
           		lerror = 'n';
         	}
      	} else if ((strcmp(keywd, "SO_LINGER")) == 0) {
	 		ss[0][0] = '\0';
	 		sscanf(s, "%*s %s", ss[0]);
	 		switch (ss[0][0]) {
	 			case '\0':
           			break;
	 			case 'Y':
         		case 'y':
	    			r_ptr->transact |= SO_LINGER_VAL;
           			break;
	 			case 'N':
         		case 'n':
	    			r_ptr->transact &= ~((u_short)SO_LINGER_VAL);
	    			break;
	 			default:
	   				lerror = 'y';
	    			break;
	 		}			/* endswitch */
         	if(lerror == 'y' || Stanza1Flag == 0) {
	    		sprintf(msg_text, "line# %d %s = %s (value must be Y(yes) or N(no)) FIRST STANZA ONLY (%d).\n", *line, keywd, ss[0], Stanza1Flag); 
	    		hxfmsg(stats, HERINFO18, HTX_HE_SOFT_ERROR, msg_text);
           		error  = 'y';
           		lerror = 'n';
         	}
      	} else if ((strcmp(keywd, "OPER")) == 0) {
	 		sscanf(s, "%*s %s", ss[0]);
	 		if (Stanza1Flag == 0 || (strcmp(ss[0],"R") != 0 && strcmp(ss[0], "W") != 0 && strcmp(ss[0], "RW") != 0
              						 && strcmp(ss[0], "WR") != 0)) {
	    		sprintf(msg_text, "line# %d %s = %s (must be \"R\", \"W\", \"RW\", or \"WR\") FIRST STANZA ONLY.\n", *line, keywd, ss[0]);
	    		hxfmsg(stats, HERINFO1, HTX_HE_SOFT_ERROR, msg_text);
	    		error = 'y';
	 		}
         	else {
           		if(ss[0][0] == 'R' || ss[0][1] == 'R')
           			r_ptr->transact |= READ_VAL;
           		else
           			r_ptr->transact &= ~((u_short)READ_VAL);
           		if(ss[0][0] == 'W' || ss[0][1] == 'W')
              		r_ptr->transact |= WRITE_VAL;
            	else
               		r_ptr->transact &= ~((u_short)WRITE_VAL);
         	}
       } else {
	 		sprintf(msg_text, "line# %d keywd = %s (invalid) \n",
						*line, keywd);
	 		hxfmsg(stats, HERINFO19, HTX_HE_SOFT_ERROR, msg_text);
	 		error = 'y';
      	}				/* endif */
   	}				/* endwhile */
   	*line = *line + 1;
   	if (keywd_count > 0) {
   		if (r_ptr->rule_id[0] == '\0') {
 			sprintf(msg_text, "line# %d rule_id not specified \n", first_line);
 			hxfmsg(stats, HERINFO20, HTX_HE_SOFT_ERROR, msg_text);
 			rc = 1;
   		} else { 
 			rc = 0;
		}
   		if(r_ptr->bufinc == USHRT_MAX)
   			r_ptr->bufinc = 1;
  	 	if(r_ptr->bufmin == USHRT_MAX)
   			r_ptr->bufmin = r_ptr->bufmax = 100;
   		if(r_ptr->bufmax == USHRT_MAX)
   			r_ptr->bufmax = r_ptr->bufmin;
   		if(r_ptr->ack_trig == USHRT_MAX)
   			r_ptr->ack_trig = r_ptr->write_ahead -1;

   		if(error == 'n' && r_ptr->bufmin > r_ptr->bufmax) {
   			sprintf(msg_text, "BUFMIN = %d, BUFMAX = %d, BUFINC = %d, Invalid combination.\n",
             				r_ptr->bufmin, r_ptr->bufmax, r_ptr->bufinc);
   			hxfmsg(stats, HERINFO21, HTX_HE_SOFT_ERROR, msg_text);
   			error = 'y';
    	}	
	   	if(error == 'n' && r_ptr->bufmin < r_ptr->bufmax && r_ptr->bufinc == 0) {
   			sprintf(msg_text, "BUFMIN = %d, BUFMAX = %d, BUFINC = %d, Invalid combination.\n",
              					r_ptr->bufmin, r_ptr->bufmax, r_ptr->bufinc);
   			hxfmsg(stats, HERINFO22, HTX_HE_SOFT_ERROR, msg_text);
   			error = 'y';
   		}
		/****************************************************************************/
		/* We have just added OPER rule.  To make backward compatible for rules     */
 		/* that don't specify OPER -- if Master, OPER=RW. if not master, OPER=W.    */
		/****************************************************************************/
    	if(error == 'n' && (r_ptr->transact & (WRITE_VAL | READ_VAL)) == 0) {
       		if(r_ptr->transact & MASTER_VAL == 0)
           		r_ptr->transact |= WRITE_VAL;
       		else
           		r_ptr->transact |= (WRITE_VAL | READ_VAL);
   		}
   		if(error != 'y' && Stanza1Flag == 1 && (strcmp(r_ptr->ComName, "") == 0 || strcmp(r_ptr->TestNetName, "") == 0)) {
      		sprintf(msg_text, "COMNET_NAME and TESTNET_NAME must be specified.\n");
      		hxfmsg(stats, HERINFO22, HTX_HE_SOFT_ERROR, msg_text);
      		error = 'y';
   		}
   		if (error == 'y') {
      		rc = 1;
   		}
	} else { 
		rc = EOF; 
	}	
   	return (rc);
}				/* get_rule() */



/****************************************************************************/
/*****  g e t l i n e ( )  **************************************************/
/****************************************************************************/
/*                                                                          */
/* FUNCTION NAME =     get_line()                                            */
/*                                                                          */
/* DESCRIPTIVE NAME =  Get a line from stdin.                               */
/*                                                                          */
/* FUNCTION =          This routine reads a line from "stdin" into the      */
/*                     specified string.  It returns the length of the      */
/*                     string.  If the length is 1 the line is blank.  When */
/*                     it reaches EOF the length is set to 0,               */
/*                                                                          */
/* INPUT =             Specified by rules file.                             */
/*                                                                          */
/* OUTPUT =            Specified by rules file.                             */
/*                                                                          */
/* NORMAL RETURN =     0 to indicate EOF.                                   */
/*                     1 to indicate blank line.                            */
/*                     Otherwise, the length of the line.                   */
/*                                                                          */
/* ERROR RETURNS =     None.                                                */
/*                                                                          */
/* EXTERNAL REFERENCES = None.                                              */
/*                                                                          */
/****************************************************************************/

static int get_line(FILE *fd, char *s, int lim)
{
   	int             c;				/* input character                         */
   	int             i;				/* array index                             */

   	i = 0;							/* set array index to 0                    */
   	while ((--lim > 0) && ((c = getc(fd)) != EOF) && (c != '\n')) {
      	s[i++] = c;					/* copy char to array                      */
   	}								/* endwhile                                */
   	if (c == '\n') {				/* newline character?                      */
      	s[i++] = c;					/* copy char to array                      */
   	}								/* endif                                   */
   	s[i] = '\0';					/* copy string terminator to array         */
   	return (i);						/* return number of chars in line          */
}									/* get_line()                               */



/****************************************************************************/
/*****  S e t S t a n z a D e f a u l t s ( ) *******************************/
/****************************************************************************/
/*                                                                          */
/* FUNCTION NAME =     SetStanzaDefaults                                    */
/*                                                                          */
/* DESCRIPTIVE NAME =  Sets the stanza default values in the rules          */
/*                     structure.                                           */
/*                                                                          */
/* FUNCTION =          Sets the default values in the rules structure.      */
/*                                                                          */
/* INPUT =             r_ptr - pointer value to the rules structure.        */
/*                                                                          */
/* OUTPUT =            Modified rules structure.                            */
/*                                                                          */
/* NORMAL RETURN =     None - void function.                                */
/*                                                                          */
/* ERROR RETURNS =     None - void function.                                */
/*                                                                          */
/* EXTERNAL REFERENCES = None.                                              */
/*                                                                          */
/****************************************************************************/

static void SetStanzaDefaults(struct rule_format * r_ptr)
{
   return;
}



/****************************************************************************/
/*****  S e t H w D e f a u l t s ( ) ***************************************/
/****************************************************************************/
/*                                                                          */
/* FUNCTION NAME =     SetHwDefaults                                        */
/*                                                                          */
/* DESCRIPTIVE NAME =  Sets the hardware default values in the rules        */
/*                     structure.                                           */
/*                                                                          */
/* FUNCTION =          Sets the default values in the rules structure.      */
/*                                                                          */
/* INPUT =             r_ptr - pointer value to the rules structure.        */
/*                                                                          */
/* OUTPUT =            Modified rules structure.                            */
/*                                                                          */
/* NORMAL RETURN =     None - void function.                                */
/*                                                                          */
/* ERROR RETURNS =     None - void function.                                */
/*                                                                          */
/* EXTERNAL REFERENCES = None.                                              */
/*                                                                          */
/****************************************************************************/

static void SetHwDefaults(struct rule_format * r_ptr)
{
    strcpy(r_ptr->rule_id, "00000000");
    strcpy(r_ptr->pattern_id, "HEX255");
    strcpy(r_ptr->ComName, "");
    strcpy(r_ptr->TestNetName, "");
    r_ptr->ComPortStream = COM_STREAM_DEFAULT;
    r_ptr->ComPortDgram  = COM_DGRAM_DEFAULT;
    r_ptr->ComPortDapl  = COM_UDAPL_DEFAULT;
    r_ptr->ComNetMask = COM_NET_MASK;
    r_ptr->nettype = 99;
    r_ptr->transact = MASTER_VAL | TCP_NODELAY_VAL;
    r_ptr->write_ahead = WRITEAHEAD;
    r_ptr->replicates = 1;
    r_ptr->write_sleep = 0;
    r_ptr->num_oper = 1000;
    r_ptr->bufmin = USHRT_MAX;
    r_ptr->bufmax = USHRT_MAX;
    r_ptr->bufinc = USHRT_MAX;
	r_ptr->bufseed = 1234 ; 
    r_ptr->ack_trig = USHRT_MAX;
    r_ptr->layer  = TCP_LAYER;
    r_ptr->alarm  = 300;
    r_ptr->idle_time  = USHRT_MAX;
    r_ptr->shutdown = 0;
    r_ptr->onesys = 0;
    r_ptr->debug_pattern = 1;
    r_ptr->no_compare = 0;
}
