* @(#)37        1.11.4.2  src/htx/usr/lpp/htx/rules/reg/hxecd/cdrom.mm, exer_cd, htxubuntu 12/23/10 07:50:08
*---------------------------------------------------------------------------*
*- Set CDROM disc part number - Required part number = 81F8902             -*
*---------------------------------------------------------------------------*
DISC_PN = 81F8902
*---------------------------------------------------------------------------*
*- STANZA 1: Mode Select - CD-ROM Mode 1                                   -*
*---------------------------------------------------------------------------*
RULE_ID = MODESEL1
OPER = MS
MODE = M1

*---------------------------------------------------------------------------*
*- STANZA 2: Read/Read/Compare - This operation will read Mode 1 data      -*
*-           starting at block 150. It will read the data twice and then   -*
*-           compare the results of the two reads. The total amount of     -*
*-           amount of data is determined by multiplying NUM_BLKS by       -*
*-           NUM_OPER. The reads are done in increments of 25 blocks at a  -*
*-           blocks at a time. Each block is 2048 bytes.                   -*
*---------------------------------------------------------------------------*
RULE_ID = RDRDCMP2
OPER = RRC
NUM_OPER = 173
NUM_BLKS = 25
INCREMENT = 0
STARTING_BLOCK = 150

*---------------------------------------------------------------------------*
*- STANZA 3: Read Compare - Read and Compare 10 blocks 10 times of Mode 1  -*
*-           data.                                                         -*
*---------------------------------------------------------------------------*
RULE_ID = RDCMP3
OPER = RC
NUM_OPER = 10
NUM_BLKS = 10
PATTERN_ID = CDM102
INCREMENT = 0
STARTING_BLOCK = 04:48:00

*---------------------------------------------------------------------------*
*- STANZA 4: Mode Select - CD-ROM Mode 1                                   -*
*---------------------------------------------------------------------------*
RULE_ID = MODESEL4
OPER = MS
MODE = M1

*---------------------------------------------------------------------------*
*- STANZA 5: Read - Stream 2 minutes of sequential mode 1 data, 5 blocks   -*
*-           per read                                                      -*
*---------------------------------------------------------------------------*
RULE_ID = STREAM5
OPER = R
NUM_OPER = 1800
NUM_BLKS = 5
INCREMENT = 0
STARTING_BLOCK = 04:48:00

*---------------------------------------------------------------------------*
*- STANZA 6: Read - Stream 2 minutes of sequential mode 1 data, 15 blocks  -*
*-           per read                                                      -*
*---------------------------------------------------------------------------*
RULE_ID = STREAM6
OPER = R
NUM_OPER = 600
NUM_BLKS = 15
INCREMENT = 0
STARTING_BLOCK = 04:48:00

*---------------------------------------------------------------------------*
*- STANZA 7: Read - Stream 10 minutes of sequential mode 1 data, 25 blocks -*
*-           per read                                                      -*
*---------------------------------------------------------------------------*
RULE_ID = STREAM7
OPER = R
NUM_OPER = 1800
NUM_BLKS = 25
INCREMENT = 0
STARTING_BLOCK = 38:00:00

*---------------------------------------------------------------------------*
*- STANZA 8: Mode Select - Mode 2 Form 1                                   -*
*---------------------------------------------------------------------------*
RULE_ID = MODSEL8
OPER = MS
MODE = M2F1

*---------------------------------------------------------------------------*
*- STANZA 9: Read/Read/Compare - This operation will read Mode 2 Form 1    -*
*-           data starting at block 58:51:00. It will read the data twice  -*
*-           and then compare the results of the two reads. The total      -*
*-           amount of data is determined by multiplying NUM_BLKS by       -*
*-           NUM_OPER. The reads are done in increments of 1 block at a    -*
*-           time. Each block is 2048 bytes. This is done because the M2F1 -*
*-           data is interspersed between M2F2 data.                       -*
*---------------------------------------------------------------------------*
RULE_ID = RRCMP9
OPER = RRC
NUM_OPER = 2200
NUM_BLKS = 1 
INCREMENT = 1
STARTING_BLOCK = 58:51:00

*---------------------------------------------------------------------------*
*- STANZA 10: Read Compare - Read and Compare 10 blocks 10 times.          -*
*---------------------------------------------------------------------------*
RULE_ID = RDCMP10
OPER = RC
NUM_OPER = 10
NUM_BLKS = 10
INCREMENT = 1
PATTERN_ID = CDM2F102
STARTING_BLOCK = 58:51:00

*---------------------------------------------------------------------------*
*- STANZA 11: Mode Select - CD-ROM Mode 1                                  -*
*---------------------------------------------------------------------------*
RULE_ID = MODSEL11
OPER = MS
MODE = M1

*---------------------------------------------------------------------------*
*- STANZA 12: Read/Read/Compare - This operation will read Mode 1 data     -*
*-            starting at block 7425. It will read the data twice and then -*
*-            compare the results of the two reads. The total amount of    -*
*-            data is determined by multiplying NUM_BLKS by NUM_OPER. The  -*
*-            reads are done in increments of 25 blocks at a time. Each    -*
*-            block is 2048 bytes.                                         -*
*---------------------------------------------------------------------------*
RULE_ID = RRCMP12
OPER = RRC
NUM_OPER = 173
NUM_BLKS = 25
INCREMENT = 0
STARTING_BLOCK = 7425

*---------------------------------------------------------------------------*
*- STANZA 13: Mode Select - Mode 2 Form 2                                  -*
*---------------------------------------------------------------------------*
RULE_ID = MODSEL13
OPER = MS
MODE = M2F2

*---------------------------------------------------------------------------*
*- STANZA 14: Read/Read/Compare - This operation will read Mode 2 Form 2   -*
*-            data starting at block 58:51:01. It will read the data twice -*
*-            and then compare the results of the two reads. The total     -*
*-            amount of data is determined by multiplying NUM_BLKS by      -*
*-            NUM_OPER. This done in one block increments as the M2F2 data -*
*-            is interspersed between M2F1 data.                           -*
*---------------------------------------------------------------------------*
RULE_ID = RRCMP14
OPER = RRC
NUM_OPER = 2200
NUM_BLKS = 1
INCREMENT = 1
STARTING_BLOCK = 58:51:01

*---------------------------------------------------------------------------*
*- STANZA 15: Read Compare - Read and Compare 10 blocks 10 times.          -*
*---------------------------------------------------------------------------*
RULE_ID = RDCMP15
OPER = RC
NUM_OPER = 10
NUM_BLKS = 10
INCREMENT = 1
PATTERN_ID = CDM2F202
STARTING_BLOCK = 58:51:01

*---------------------------------------------------------------------------*
*- STANZA 16: Mode Select - DA Mode                                        -*
*---------------------------------------------------------------------------*
RULE_ID = MODSEL16
OPER = MS
MODE = DA

*---------------------------------------------------------------------------*
*- STANZA 17: Read 10 blocks 10 times starting at block 03:45:00.          -*
*---------------------------------------------------------------------------*
RULE_ID = READ17
OPER = R
NUM_OPER = 10
NUM_BLKS = 10
ADDR_TYPE = SEQ
STARTING_BLOCK = 03:45:00

*---------------------------------------------------------------------------*
*- STANZA 18: Mode Select - CD-ROM Mode 1                                  -*
*---------------------------------------------------------------------------*
RULE_ID = MODSEL18
OPER = MS
MODE = M1

*---------------------------------------------------------------------------*
*- STANZA 19: SCSI Play audio command for 1 block starting at block 2.     -*
*---------------------------------------------------------------------------*
RULE_ID = PAUDIO19
OPER = AMM
STARTING_BLOCK = 01:06:00
NUM_BLKS = 2245

*---------------------------------------------------------------------------*
*- STANZA 20: Read/Read/Compare - This operation will read Mode 1 data     -*
*-            starting at block 21600. It will read the data twice and     -*
*-            then compare the results of the two reads. The total amount  -*
*-            of data is determined by multiplying NUM_BLKS by NUM_OPER.   -*
*-            The reads are done in increments of 25 blocks at a time.     -*
*-            Each block is 2048 bytes.                                    -*
*---------------------------------------------------------------------------*
RULE_ID = RRCMP20
OPER = RRC
NUM_OPER = 9500
NUM_BLKS = 25
INCREMENT = 0
STARTING_BLOCK = 21600

