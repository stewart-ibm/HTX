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

/* @(#)91	1.6.1.4  src/htx/usr/lpp/htx/bin/hxecom/comrw.c, exer_com, htx53A 5/24/04 17:27:12 */

/*
 *   COMPONENT_NAME: exer_com
 *
 *   FUNCTIONS: Free
 *		GetPattern
 *		GetRules
 *		ShutdownSocket
 *		ShutdownTest
 *		bad_other
 *		good_other
 */

#   include <string.h>
#   include <unistd.h>
#   include <stdlib.h>
#   include <memory.h>
#   include "hxecomdef.h"
#   include "comrw.h"


	char tmpaddr[30];

void bad_other(struct cum_rw * Lstats, struct htx_data * pstats, struct htx_data * sstats)
{
    Lstats->totals.bad_others  += 1;
    Lstats->current.bad_others += 1;
    sstats->bad_others          += 1;
}


void good_other(struct cum_rw * Lstats, struct htx_data * pstats, struct htx_data * sstats)
{
    Lstats->totals.good_others  += 1;
    Lstats->current.good_others += 1;
    sstats->good_others          += 1;
}


int GetPattern(struct sockaddr_in ServerID, char *patternbuf, int pattern_max, struct htx_data * stats)
{
    SOCKET ToServerSock;
    int rc;
    int i;
    struct CoordMsg CMsg;
    int pflength;
	char msg_text[1024]; 

    ToServerSock = SetUpConnect(&ServerID, stats, 0); 
    memset(&CMsg, '\0', sizeof(CMsg));
    CMsg.msg_type = htonl(CM_REQ_PATTERN);
    CMsg.ID.Wsize.size = htonl((uint32_t)pattern_max);
    rc = StreamWrite(ToServerSock, (char *) &CMsg, sizeof(CMsg));
    if(rc == -1) {
        sprintf(msg_text, "GetPattern: Error writing to Server - %s\n", STRERROR(errno));
        hxfmsg(stats, HTXERROR(EX_PATT1,ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        HE_exit(EX_PAT1);
    }
    rc = StreamRead(ToServerSock, (char *) &CMsg, sizeof(CMsg));
    CMsg.msg_type = ntohl(CMsg.msg_type);
    if(CMsg.msg_type != CM_PATTERN) {
        sprintf(msg_text, "GetPattern: Unable to obtain pattern file - %s\n", STRERROR(errno));
        hxfmsg(stats, HTXERROR(EX_PATT2,ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        HE_exit(EX_PAT2);
    }
    pflength = ntohl(CMsg.ID.Wsize.size);
    if(pflength > pattern_max || pflength <= 0) {
        sprintf(msg_text, "GetPattern: Server wrote too many characters - %s\n", STRERROR(errno));
        hxfmsg(stats, HTXERROR(EX_PATT3,ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        HE_exit(EX_PAT3);
    }
    rc = StreamRead(ToServerSock, (char *) patternbuf, pflength);
    if(rc != pflength) {
        sprintf(msg_text, "GetPattern: Unable to receive pattern file - %s\n", STRERROR(errno));
        hxfmsg(stats, HTXERROR(EX_PATT4,ERRNO), HTX_HE_SOFT_ERROR, msg_text);
        HE_exit(EX_PAT4);
    }

    closesocket(ToServerSock);

/********************************************************************/
/* Fill remainder of patternbuf.                                    */
/********************************************************************/
    for(i=pflength; i<pattern_max; i++)
        patternbuf[i] = patternbuf[i%pflength];
    return pattern_max;
}



int GetRules(struct sockaddr_in ServerID, struct rule_format rule[], int max_rules, struct htx_data * stats)
{
    SOCKET ToServerSock;
    struct CoordMsg CMsg;
    int rc;
    int i;
    int NoStanzas;
	char msg[1024]; 
	errno = 0;     

    ToServerSock = SetUpConnect(&ServerID, stats, 0); 
    memset(&CMsg, '\0', sizeof(CMsg));
    CMsg.msg_type = htonl(CM_REQ_RULES);
    CMsg.ID.Wsize.size  = htonl((uint32_t)sizeof(struct rule_format));
    rc = StreamWrite(ToServerSock, (char *) &CMsg, sizeof(CMsg));
    if(rc == -1) {
        sprintf(msg, "GetRules: Error writing to Server - %s\n", STRERROR(errno));
        hxfmsg(stats, HTXERROR(EX_RULE1,ERRNO), HTX_HE_SOFT_ERROR, msg);
        HE_exit(EX_RULE1);
    }
    StreamRead(ToServerSock, (char *) &CMsg, sizeof(CMsg));
    CMsg.msg_type = ntohl(CMsg.msg_type);
    CMsg.ID.Wsize.size  = ntohl(CMsg.ID.Wsize.size);
    if(CMsg.msg_type != CM_RULES_STANZA) {
        sprintf(msg, "GetRules: Illegal Packet recvd.\n"); 
        hxfmsg(stats, HTXERROR(EX_RULE3,0), HTX_HE_SOFT_ERROR, msg);
        HE_exit(EX_RULE3);
    }
	#ifdef __DEBUG__
		sprintf(msg, "GetRules: Sock = %d, Recvd msg = %x of size = %#x, rule_format size = %x, pid = %d \n", ToServerSock, CMsg.msg_type, sizeof(CMsg), (uint32_t)sizeof(struct rule_format), getpid()); 
		hxfmsg(stats, 0, 7, msg); 
	#endif 
    memset(rule, '\0', sizeof(struct rule_format) * max_rules);
	memset(&CMsg, '\0', sizeof(CMsg));
    for(i=0; i < max_rules; i++) {

        rc = StreamRead(ToServerSock, (char *) &CMsg.msg_type, sizeof(CMsg.msg_type));
        CMsg.msg_type = ntohl(CMsg.msg_type);
	#ifdef __DEBUG__
		sprintf(msg, "i = %#x, GetRules: Recvd msg = %x, of size = %x \n", i, CMsg.msg_type, sizeof(CMsg.msg_type)); 
		hxfmsg(stats, 0, 7, msg); 
	#endif 
        if(CMsg.msg_type == CM_RULES_FINISHED)
            break;
		 
        if(CMsg.msg_type != CM_RULES_STANZA) {
            sprintf(msg, "GetRules: Unable to obtain rules - type %x\n", (int)CMsg.msg_type);
            hxfmsg(stats, HTXERROR(EX_RULE2,ERRNO), HTX_HE_SOFT_ERROR, msg);
            HE_exit(EX_RULE2);
        }
		 
        rc = StreamRead(ToServerSock, (char*) &rule[i], sizeof(struct rule_format));
        if(rc != sizeof(struct rule_format)) {
            sprintf(msg, "GetRules: Unable to read rules from server - %s\n", STRERROR(errno));
            hxfmsg(stats, HTXERROR(EX_RULE4,ERRNO), HTX_HE_SOFT_ERROR, msg);
			HE_exit(EX_RULE3); 

        }

        NetToHostRules(&rule[i]);
    }
    NoStanzas = i;
    closesocket(ToServerSock);
    return NoStanzas;
}



void ShutdownTest(struct sockaddr_in ServerID, struct htx_data * stats)
{
    SOCKET ToServerSock;
    struct CoordMsg CMsg;
    int rc;
    
    ToServerSock = SetUpConnect(&ServerID, stats, SH_FORCE); 
    memset(&CMsg, '\0', sizeof(CMsg));
    CMsg.msg_type = htonl(CM_SHUTDOWN);
    rc = StreamWrite(ToServerSock, (char *) &CMsg, sizeof(CMsg));
    if(rc == -1) {
		/* I am shutting down don't care about error here. just exit.
			sprintf(msg_text, "ShutdownTest: Error writing to Server - %s\n", STRERROR(errno));
			hxfmsg(stats, HTXERROR(EX_SHUTD,ERRNO), HTX_HE_SOFT_ERROR, msg_text);
		*/
    }
    closesocket(ToServerSock);
}



void Free(void * arg)
{
    free((char *)arg);
}



void ShutdownSocket(void * arg)
{
    if(arg != NULL)
        close(*(int *)arg);
}



