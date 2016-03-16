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
/* @(#)96       1.7.1.1  src/htx/usr/lpp/htx/bin/hxecom/exit.h, exer_com, htx53A 8/11/97 17:27:59 */
/*
 *   COMPONENT_NAME: exer_com
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *   OBJECT CODE ONLY SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef HXECOMEXIT_H
#define HXECOMEXIT_H


/* When the htxerr file is referred to below and you are testing in           */
/* standalone mode, we are refering to stderr.                                */

/******************************************************************************/
/* An invalid number of arguments were passed at the invocation of "hxecom".  */
/* Correct invocation of hxecom can be displayed by executing hxecom without  */
/* any arguments.                                                             */
#define EX_INV_ARGV     1
/******************************************************************************/

/******************************************************************************/
/* There was an error reading the rules file.                                 */
/* Specific information should have been printed in the htxerr log file.      */
#define EX_RULEREAD1    2
/******************************************************************************/

/******************************************************************************/
/* The exerciser may have been unable to generate a valid pattern file name.  */
/* Check the following:                                                       */
/* htxerr message  - There are more than a few failure modes and this will    */
/*                   help isolate problem.                                    */
/* The file name   - Specified in rule file and should be a valid file in the */
/*                   pattern directory. The name must be all upper-case       */
/*                   letters.  If the file name contains some lower-case      */
/*                   letters, rename the file.                                */
/* The path      1 - Path will be assumed to be part of the file name.        */
/*               2 - If file name begins with a '.' or a '/'. no other paths  */
/*                   will be tried (This is hard to do with a limit of 8      */
/*                   characters for the filename.)                            */
/*            or 2 - Next a hardwired path '../pattern' will be tried.        */
/*               3 - Last, path HTXPATTERNS from environment will be tried.   */
#define EX_RULEREAD2    3
/******************************************************************************/

/******************************************************************************/
/* The file had been opened previously, check the htxerr message for clues.   */
#define  EX_RULEREAD3	4
/******************************************************************************/

/******************************************************************************/
/* Device passed into hxecom on the command line was invalid.                 */
#define EX_INVDEV	6
/******************************************************************************/

/******************************************************************************/
/* Either the paging volume is too small, not enough memory, or too many      */
/* processes/threads are running on this machine.                             */
/* "lsps -a" will show amount of paging space.  "lsdev -C|grep mem" will      */
/* show total memory in machine.                                              */
#define EX_SIGDANGER    7
/******************************************************************************/

/******************************************************************************/
/* Save htxerr log and any aditional information.                             */
#define EX_FORK0        10
#define EX_FORK1        11
#define EX_FORK2        12
#define EX_FORK3        13
#define EX_FORK4        14
#define EX_FORK5        15
#define EX_FORK6        16
#define EX_FORK7        17
#define EX_FORK8        18

#define EX_THREAD1      21
#define EX_THREAD2      22
#define EX_THREAD3      23
#define EX_THREAD4      24
/******************************************************************************/

/******************************************************************************/
/* The exerciser was unable to generate a valid rule file pathname.  Check    */
/* the following:                                                             */
/* htxerr message  - There are more than a few failure modes and this will    */
/*                   help isolate problem.                                    */
/* The file name   - If specified in mdt test file, it should be a valid file */
/*                   in the rule directory.  If running standalone, the rule  */
/*                   file name is specified as the 3rd argument on the        */
/*                   command line.                                            */
/* The path      1 - Path will be assumed to be part of the file name.        */
/*               2 - If file name begins with a '.' or a '/'. no other paths  */
/*                   will be tried.                                           */
/*            or 2 - A hardwired path '../rules/reg/hxecom' will be tried     */
/*                   if in "REG" mode or "OTH" mode.  If in "EMC" mode,       */
/*                   '../rules/emc/hxecom' will be tried.                     */
/*               3 - Last, path HTXRULES from environment will be tried.      */
#define EX_FNAME        25
/******************************************************************************/

/******************************************************************************/
/* Unable to lookup the IP address for the name represented by COMNET_NAME in */
/* the rule file.  Check :                                                    */
/*     - COMNET_NAME (TESTNET_NAME) was correct in rule file                  */
/*     - COMNET_NAME (TESTNET_NAME) is represented in the /etc/hosts file     */
/*     - If a nameserver is active, can it resolve this name? Use of          */
/*       nameserver for testing is not recommended.                           */
/*     - Review messages in htxerr and htxmsg for additionl information.      */
#define EX_GETH1	26
#define EX_GETH2	27
/* This is the same as EX_GETH1 above except it may be for TESTNET_NAME.      */
/* Review the error message to determine which network has the problem and    */
/* follow the advice for EX_GETH1 above.                                      */
#define EX_GETH3	28
/******************************************************************************/

/******************************************************************************/
/* Unable to bind IP address represented by COMNET_NAME in the rule file.     */
/* Check:                                                                     */
/*     - IP address in /etc/hosts or nameserver.                              */
/*     - IP address must be a valid address on THIS host.                     */
/*     - Review messages in htxerr and htxmsg for additionl information.      */
#define EX_BIND1	29
/* This is the same as EX_BIND1 above except it may be for TESTNET_NAME.      */
/* Review the error message to determine which network has the problem and    */
/* follow the advice for EX_BIND1 above.                                      */
#define EX_BIND5	30
/******************************************************************************/

/******************************************************************************/
/* Was unable to make a socket connection to address printed in htxerr log.   */
/* Try pinging the same address as                                            */
/* ping xx.xx.xx.xx  where xx.xx.xx.xx is address printed in the log.         */
/* Always make sure all remote connections can be pinged before using HXECOM  */
#define EX_GETH4	42
/******************************************************************************/

/******************************************************************************/
/* There is insufficient memory to run the exerciser.  If you get these errors*/
/* and you are running under HTX, check the memory exerciser.  It may be      */
/* taking too much memory and not leaving enough for hxecom.  Depending on    */
/* the order of execution of processes, the problem may appear to be random.  */
#define EX_MALLOC1      31
#define EX_MALLOC2      32
#define EX_MALLOC3      33
#define EX_MALLOC4      34
#define EX_MALLOC5      35
#define EX_MALLOC6      36
#define EX_MALLOC7      37
#define EX_MALLOC8      38
#define EX_MALLOC9      39
#define EX_MALLOC10     40
#define EX_MALLOC11     41
#define EX_MALLOC12     42
/******************************************************************************/

/******************************************************************************/
/* You have exceeded a maximum array limit.  Study the error message in the   */
/* htxerr log and if you think you still need more space allocated.  Contact  */
/* the exerciser owner for a change.                                          */
#define  EX_EXERNO      43
#define  EX_COORD1	44
#define  EX_COORD16	45
#define  EX_COORD22	46
#define  EX_COMSEM7	47
/******************************************************************************/

/******************************************************************************/
/* Old shared memory existed when processes were started.  These errors       */
/* occurred while removing the old memory.  Additional information should be  */
/* in the htxerr log.                                                         */
#define EX_BACK1        50
#define EX_BACK2        51
#define EX_BACK3        52
#define EX_BACK4        53
#define EX_BACK5        54
#define EX_BACK6        55
#define EX_BACK7        56
/******************************************************************************/

/******************************************************************************/
/* Error message provided should provide additional information on nature of  */
/* problem.                                                                   */
#define EX_SHMGET1      61
#define EX_SHMGET2      62
#define EX_SHMGET3      63
#define EX_SHMGET4      64
#define EX_SHMGET5      65
#define EX_SHMAT1       66
#define EX_SHMAT2       67
#define EX_SHMAT3       68
#define EX_SHMAT4       69
#define EX_SHMAT5       70
#define EX_SHMAT6       71
#define EX_SHMAT7       72
#define EX_SHMAT8       73
#define EX_SHMCTL       74
/******************************************************************************/

#define EX_RULEREAD5	5

#define EX_SEMDWN2      75
#define EX_SEMCREAT1    76
#define EX_SEMCREAT2    77
#define EX_SEMCREAT3    78
#define EX_SEMCREAT4    79
#define EX_SEMUP1       80
#define EX_SEMUP2       81
#define EX_SEMUP3       82
#define EX_SEMINIT1     83
#define EX_SEMGET1      84
#define EX_SEMGET2      85
#define EX_DGERR        86


#define EX_SOCK1	111
#define EX_SOCK2	112
#define EX_SOCK3	113
#define EX_SOCK4	114
#define EX_SOCK5	115
#define EX_SOCK6	116
#define EX_SOCK7	117
#define EX_SOCK8	118
#define EX_SOCK9	119
#define EX_SOCK10	120
#define EX_BIND2	122
#define EX_BIND3	123
#define EX_BIND4	124
#define EX_SETB1	126
#define EX_SETB2	127
#define EX_SETB3	128
#define EX_GETB1	129
#define EX_SETREUSE1    130
#define EX_SETREUSE2    131

#define EX_GETS1	141
#define EX_GETS2	142
#define EX_EXEC1	151
#define EX_EXEC2	152
#define EX_EXEC3	153
#define EX_EXEC4	154
#define EX_EXEC5	155

#define EX_SHMGET0      160
#define EX_SHMAT0       161

#define  HEINFO1	201
#define  HEINFO2	202
#define  HEINFO3	203
#define  HEINFO4	204
#define  HEINFO5	205
#define  HEINFO6	206
#define  HEINFO7	207
#define  HEINFO8	208
#define  HEINFO9	209
#define  HEINFO10	210
#define  HEINFO11	211
#define  HEINFO12	212
#define  HEINFO13	213

#define  EX_LAYER1      215
#define  EX_SHUTDOWN    216

#define  EX_READ1	217
#define  EX_READ2	218
#define  EX_READ3	219
#define  EX_READ4	220
#define  EX_READ5	221
#define  EX_READ6	222
#define  EX_READ7	223
#define  EX_READ8	224
#define  EX_READ9	225
#define  EX_READ10	226
#define  EX_READ11	227
#define  EX_READ12	228
#define  EX_READ13	229
#define  EX_READ14	230
#define  EX_READ15	231

#define  EX_LAYER2      232

#define  EX_WRITE1	233
#define  EX_WRITE2	234
#define  EX_WRITE3	235
#define  EX_WRITE4	236
#define  EX_WRITE5	237
#define  EX_WRITE6	238
#define  EX_WRITE7	239
#define  EX_WRITE8	240
#define  EX_WRITE9	241
#define  EX_WRITE10	242
#define  EX_WRITE11	243
#define  EX_WRITE12	244
#define  EX_WRITE13	245
#define  EX_WRITE14	246
#define  EX_WRITE15	247
#define  EX_WRITE16	248
#define  EX_WRITE17	249
#define  EX_WRITE18	250
#define  EX_WRITE19	251
#define  EX_WRITE20     252

#define  EX_PAT1	253
#define  EX_PAT2	254
#define  EX_PAT3	255
#define  EX_PAT4        258
#define  EX_SERV1	256
#define  EX_SERV2	257

#define  EX_WRITE21     259
#define  EX_WRITE22     260

#define  EX_COMSEM1	263
#define  EX_COMSEM2	264
#define  EX_COMSEM3	265
#define  EX_COMSEM4	266
#define  EX_COMSEM5	267
#define  EX_COMSEM6	268
#define  EX_COMSEM8	270
#define  EX_COMSEM9	271
#define  EX_COMSEM10	272
#define  EX_COMSEM11	273
#define  EX_COMSEM12	274
#define  EX_COMSEM13	275
#define  EX_COMSEM14	276
#define  EX_COMSEM15	277
#define  EX_COMSEM16	278
#define  EX_COMSEM17	279
#define  EX_COMSEM18	280
#define  EX_COMSEM19	281
#define  EX_COMSEM20	282
#define  EX_COMSEM21	283
#define  EX_COMSEM22	284
#define  EX_COMSEM23	285
#define  EX_COMSEM24	286
#define  EX_COMSEM25	287
#define  EX_COMSEM26	288
#define  EX_COMSEM27	289
#define  EX_COMSEM28	290

#define  EX_COORD2	294
#define  EX_COORD3	295
#define  EX_COORD4	296
#define  EX_COORD5	297
#define  EX_COORD6	298
#define  EX_COORD7	299
#define  EX_COORD8	300
#define  EX_COORD9	301
#define  EX_COORD10	302
#define  EX_COORD11	303
#define  EX_COORD12	304
#define  EX_COORD13	305
#define  EX_COORD14	306
#define  EX_COORD15	307
#define  EX_COORD17	309
#define  EX_COORD18	310
#define  EX_COORD19	311
#define  EX_COORD20	312
#define  EX_COORD21	313

#define  EX_PATT1       320
#define  EX_PATT2       321
#define  EX_PATT3       322
#define  EX_PATT4       327
#define  EX_RULE1       323
#define  EX_RULE2       324
#define  EX_RULE3       325
#define  EX_RULE4       326
#define  EX_SHUTD       330

#define EX_THREAD5      350
#define EX_THREAD6      351

#define EX_FILE1	360
#define EX_FILE2	361

#define HERINFO1        0x20010000
#define HERINFO2        0x20020000
#define HERINFO3        0x20030000
#define HERINFO4        0x20040000
#define HERINFO5        0x20050000
#define HERINFO6        0x20060000
#define HERINFO7        0x20070000
#define HERINFO8        0x20080000
#define HERINFO9        0x20090000
#define HERINFO10       0x200a0000
#define HERINFO11       0x200b0000
#define HERINFO12       0x200c0000
#define HERINFO13       0x200d0000
#define HERINFO14       0x200e0000
#define HERINFO15       0x200f0000
#define HERINFO16       0x20100000
#define HERINFO17       0x20110000
#define HERINFO18       0x20120000
#define HERINFO19       0x20130000
#define HERINFO20       0x20140000
#define HERINFO21       0x20150000
#define HERINFO22       0x20160000
#define HERINFO23       0x20170000
#define HERINFO24       0x20180000
#define HERINFO25       0x20190000
#define HERINFO26       0x201a0000
#define HERINFO27       0x201b0000
#define HERINFO28       0x201c0000
#define HERINFO29       0x201d0000
#define EX_COM1         400
#define EX_COM2         401

#define EX_SEMCREAT0    402

#define EX_PSEUDO1      420
#define EX_PSEUDO2      421

#define EX_LISTEN1      431
#define EX_SELECT1      432
#define EX_SELECT2      433
#define EX_THREAD7      436

#define EX_OPEN1        450
#define EX_SEEK1        451

#define EX_WSA1         600
#define EX_WSA2         601

#define EX_IOCTL1       610
#define EX_IOCTL2       611

#define EX_OTHERID      612
#define EX_OTHERID1     613
#define EX_OTHERID2     614
#define EX_OTHERID3     615

#define EX_DAPL_INIT		700
#define EX_DAPL_REGISTER	701
#define EX_DAPL_POST_RCV	702
#define EX_DAPL_CONNECT		703
#define EX_DAPL_ADDR_XCHG	704
#define EX_DAPL_RDMAW		705
#define EX_DAPL_RDMAR		706
#define EX_DAPL_LISTEN	    707
#define EX_DAPL_ACCEPT	    708
#define EX_DAPL_READ_EVENT	709

#endif  /* HXECOMEXIT_H  */
