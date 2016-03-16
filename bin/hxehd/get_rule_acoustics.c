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
/* @(#)08       1.8  src/htx/usr/lpp/htx/bin/hxehd/get_rule_acoustics.c, exer_hd, htxubuntu 8/7/13 02:05:49 */
/******************************************************************************
 
 * FUNCTION: Get a rule stanza from the rules file. For each keyword
 *           parameter of the rules stanza:
 *              1. assign a default value to corresponding program variable
 *              2. assign specified value to corresponding program variable
 *              3. validity check each variable.
 *           This routine will work through the entire rules file stanza by
 *           stanza till EOF is reached. It will then issue the following
 *           return codes:
 *                0 = valid rules file
 *               -1 = invalid rules file.
 *           The HTX message log will contain a listing of all errors found
 *           in the rules file.
 *
 * CHANGE LOG: programmer  | date     | change
 *            -------------|----------|---------------------------------------
 *             D. Stauffer | 03/28/97 | Initial release to HTX.
 *             D. Stauffer | 01/09/98 | Make sure last stanza next ruleptr
 *                         |          | points to NULL - chg 1
 *                         |          | Allow an negative increment - chg 2
 *                         |          | Allow a percentage on min & max block
 *                         |          | number - chg 3
 *             D. Stauffer | 07/07/99 | Added three new global variables that
 *                         |          | a user is allowed to set (misc_run_cmd,
 *                         |          | run_on_misc, run_reread).
 *             R. Gebhardt | 10/21/00 | Corrected logic to get trailing
 *                         |          | digits from rule_id.
 ******************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include "hxehd.h"
#include "hxehd_proto.h"

int num_sub_blks_psect=0;
extern int     crash_on_miscom, rule_stanza[MAX_STANZA], seeds_were_saved, la_on_miscom;
extern int    crash_on_anyerr, saved_num_blocks, hang_time, threshold,open_retry_count,cpu_bind,accous_rules;
extern int    turn_attention_on;
extern int    is_bwrc_rule_present; /* D502299 */

#ifndef __HTX43X__
extern int    pass_thru, debug_passthru;
#endif

extern char   crash_on_hang, default_patternid[9], msg_swtch[4];
extern char   misc_run_cmd[100], run_on_misc, run_reread;
extern char   pipe_name[15], saved_last_type_len;
extern struct ruleinfo *first_ruleptr, *prev_ruleptr;

int get_rule(struct htx_data *ps, char rules_file_name[100], unsigned long long maxblk, unsigned long blksize)
{
  int        pattern_des, rc = 0, rec_lgth, tvar, rule_id_len;
  int        keywd_count, i, j, k, cnt, first_line, digitct;
  int align_tmp = 0;
  static int line = 0;
  FILE       *fptr;
  char       *p, s[200], t[200], varstr[80], optswtch = 'N';
  char       digit[3], eof_file = 'N', keywd[80], msg[MSG_TEXT_SIZE];
  char       error = 'n', pipe_swtch[3], pattern_nm[100];
  double     tminblk, tmaxblk;
  unsigned long numb;

  if ( (fptr = fopen(rules_file_name, "r")) == NULL ) {
	 sprintf(msg,"error open %s ",rules_file_name);
     user_msg(ps, errno, HARD, msg);
     return(-1);
  }
  keywd_count = 0;
  cnt = 0;
  first_line = line + 1;
  fgets(s, 200, fptr);
  rec_lgth = strlen(s);
  while ( eof_file != 'Y' ) {
    line += 1;
    if ( rec_lgth > 1 ) {
      if ( s[0] != '*' ) {
        for ( i = 0; s[i] != '\n'; i++ ) {
          t[i] = s[i];
          s[i] = toupper(s[i]);
          if ( s[i] == '=' ) {
            s[i] = ' ';
            t[i] = ' ';
          }
        }
        if ( keywd_count == 0 ) {
          current_ruleptr = (struct ruleinfo*) malloc(sizeof(struct ruleinfo));
          strcpy(current_ruleptr->rule_id, "        ");
          strcpy(current_ruleptr->pattern_id, default_patternid);
          strcpy(current_ruleptr->addr_type, "RANDOM");
          strcpy(current_ruleptr->oper, "R");
          strcpy(current_ruleptr->starting_block, "BOT");
          strcpy(current_ruleptr->direction, "UP");
          strcpy(current_ruleptr->type_length, "RANDOM");
          strcpy(current_ruleptr->messages, msg_swtch);
          current_ruleptr->sleep                 = 0;
          current_ruleptr->start                 = 'Y';
          current_ruleptr->timed                 = 'N';
          current_ruleptr->offset                = 0;
          current_ruleptr->pat_cnt               = 0;
          current_ruleptr->percent               = 0.0;
          current_ruleptr->num_blks              = 1;
          current_ruleptr->num_oper              = 1;
          current_ruleptr->min_blkno             = 0;
          current_ruleptr->max_blkno             = maxblk;
          current_ruleptr->increment             = 0;
          current_ruleptr->om_invert             = 0;
          current_ruleptr->om_invert1            = 0;
          current_ruleptr->align                 = 0;
          current_ruleptr->om_prevent            = 0;
          current_ruleptr->no_mallocs            = 2;
          current_ruleptr->repeat_neg            = 0;
          current_ruleptr->repeat_pos            = 0;
          current_ruleptr->rule_offset           = 0;
          current_ruleptr->rule_options          = 0;
          current_ruleptr->loop_on_offset        = 0;
          current_ruleptr->max_number_of_threads = 2;
          current_ruleptr->min_blklen            = 0; /* RDT changes */
          current_ruleptr->op_rate               = 0;
          current_ruleptr->rule_time             = 0;
          current_ruleptr->stanza_time           = 0;
        }
        keywd_count++;
        sscanf(s, "%s", keywd);
        if ( (strcmp(keywd, "RULE_ID")) == 0 ) {
           sscanf(s, "%*s %s", current_ruleptr->rule_id);
           if ( ((strlen(current_ruleptr->rule_id)) < 1) ||
                ((strlen(current_ruleptr->rule_id)) > 16) ) {
              sprintf(msg, "line# %d %s = %s (must be 1 to 16 characters) \n",
                      line, keywd, current_ruleptr->rule_id);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           } else {  /* get trailing digits for stanza number */
              rule_id_len = j = strlen(current_ruleptr->rule_id);
              digitct = 0;
              while ( (j > 0) && isdigit(current_ruleptr->rule_id[--j]) )
                  digitct++;            /* count the trailing digits */
              for ( i = rule_id_len-digitct, j=0; j < digitct; ) { /* move 'em */
                  digit[j++] = current_ruleptr->rule_id[i++];
              digit[digitct]='\0';       /* terminate digit string */
              }
              if ( digitct > 0 ) {
                 rule_stanza[cnt] = atoi(digit);
                 for ( i = 0; i < cnt; i++ ) {
                    if ( rule_stanza[cnt] == rule_stanza[i] ) {
                       sprintf(msg, "line# %d %s = %s (Duplicate Stanza "
                                    "number found in RULE_ID) \n",
                                line, keywd, current_ruleptr->rule_id);
                       user_msg(ps, 0, INFO, msg);
                       error = 'y';
                    }
                 }
                 cnt++;
              } else {
                 sprintf(msg, "line# %d %s = %s (NO Stanza number found "
                              "in RULE_ID) \n",
                         line, keywd, current_ruleptr->rule_id);
                 user_msg(ps, 0, INFO, msg);
                 error = 'y';
              }
           }
        } else if ( (strcmp(keywd, "MIN_BLKNO")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           tminblk = atof(varstr);
           if ( tminblk > 0 && tminblk < 1 )          /* chg 3 */
              current_ruleptr->min_blkno = tminblk * current_ruleptr->max_blkno;
           else
              current_ruleptr->min_blkno = tminblk;
           if ( current_ruleptr->min_blkno < 0 ) {
              sprintf(msg, "line# %d ( MIN_BLKNO (%d) must be > 0) \n",
                      line, current_ruleptr->min_blkno);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "MAX_BLKNO")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           tmaxblk = atof(varstr);
           if ( tmaxblk > 0 && tmaxblk < 1 )           /* chg 3 */
              current_ruleptr->max_blkno = tmaxblk * current_ruleptr->max_blkno;
           else
              current_ruleptr->max_blkno = tmaxblk;
           if ( current_ruleptr->max_blkno < 0 ) {
              sprintf(msg, "line# %d ( MAX_BLKNO (%d) must be > 0) \n",
                      line, current_ruleptr->max_blkno);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           } else
              current_ruleptr->om_prevent = 1; /* set switch for background */
        } else if ( (strcmp(keywd, "MIN_LEN")) == 0 ) {  /* RDT changes */
           sscanf(s, "%*s %s", varstr);
           tminblk = atoi(varstr);
           current_ruleptr->min_blklen = tminblk;
					 if (current_ruleptr->min_blklen < 0) current_ruleptr->min_blklen = 0;
        } else if ( (strcmp(keywd, "OP_RATE")) == 0 ) {  /* RDT changes */
           sscanf(s, "%*s %s", varstr);
           tminblk = atoi(varstr);
           current_ruleptr->op_rate = tminblk;
					 if (current_ruleptr->op_rate < 0) current_ruleptr->op_rate = 0;
        } else if ( (strcmp(keywd, "RULE_TIME")) == 0 ) {  /* RDT changes */
           sscanf(s, "%*s %s", varstr);
           tminblk = atoi(varstr);
           current_ruleptr->rule_time = tminblk;
					 if (current_ruleptr->rule_time < 0) current_ruleptr->rule_time = 0;
        } else if ( (strcmp(keywd, "STANZA_TIME")) == 0 ) {  /* accoustic changes */
           sscanf(s, "%*s %s", varstr);
           tminblk = atoi(varstr);
           current_ruleptr->stanza_time = tminblk;
					 if (current_ruleptr->stanza_time < 0) current_ruleptr->stanza_time = 0;
        } else if ( (strcmp(keywd, "PATTERN_FORM")) == 0 ) {
           sscanf(s, "%*s %n", &i);
           while ( isspace(s[i]) ) i++;
           strcpy(varstr, &s[i]);
           p = varstr;
           i = 0;
           while ( *p && i < 8 ) {
             if ( isxdigit(*p) ) {
                current_ruleptr->form[i] = (unsigned short)strtol(p, NULL, 16);
                i++;
                while ( isxdigit(*(p++)) );
             } else
                p++;
           }
           if ( i == 1 || i == 2 || i == 4 || i == 8 );
           else {
              sprintf(msg, "line# %d: Only 1, 2, 4, or 8 arguments allowed!\n",
                      line);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
           current_ruleptr->pat_cnt = i;
        } else if ( (strcmp(keywd, "PATTERN_ID")) == 0 ) {
           sscanf(s, "%*s %s", current_ruleptr->pattern_id);
           strcpy (pattern_nm, PATLIB_PATH);
           strcat (pattern_nm, current_ruleptr->pattern_id);
           if ( current_ruleptr->pattern_id[0] != '#' &&
                current_ruleptr->pattern_id[0] != '!' ) {
              pattern_des = open(pattern_nm, 0);
              if ( pattern_des == -1 ) {
                 sprintf(msg, "line# %d %s = %s (not found)\n",
                         line, keywd, pattern_nm);
                 user_msg(ps, 0, INFO, msg);
                 error = 'y';
              }
              close(pattern_des);
           }
        } else if ( (strcmp(keywd, "DEFAULT_PATTERN")) == 0 ) {
           sscanf(s, "%*s %s", default_patternid);
           strcpy (pattern_nm, PATLIB_PATH);
           strcat (pattern_nm, default_patternid);
           if ( default_patternid[0] != '#' && default_patternid[0] != '!' ) {
              pattern_des = open(pattern_nm, 0);
              if ( pattern_des == -1 ) {
                 sprintf(msg, "line# %d %s = %s (not found)\n",
                         line, keywd, pattern_nm);
                 user_msg(ps, 0, INFO, msg);
                 error = 'y';
              }
              close(pattern_des);
           }
        } else if ( (strcmp(keywd, "MESSAGES")) == 0 ) {
           sscanf(s, "%*s %s", current_ruleptr->messages);
           if ( strcmp(current_ruleptr->messages, "NO")   == 0  ||
                strcmp(current_ruleptr->messages, "YES")  == 0  ||
                strcmp(current_ruleptr->messages, "DBUG") == 0 );
           else {
              sprintf(msg, "line# %d %s = %s (must be YES or NO) \n",
                      line, keywd, current_ruleptr->messages);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( strcmp(keywd, "RULE_OPTIONS") == 0 ) {
           sscanf(s, "%*s %s", varstr);
           if ( strcmp(varstr, "SAVE_SEEDS") == 0 ) {
              if ( optswtch == 'N' )
                 if ( (current_ruleptr->rule_options &
                       RESTORE_SEEDS_FLAG) == 0 ) {
                    current_ruleptr->rule_options |= SAVE_SEEDS_FLAG;
                    seeds_were_saved = 1;
                    optswtch = 'S';
                 } else {
                    sprintf(msg, "line #%d %s = %s (RESTORE_SEEDS already "
                                 "specified)\n",
                            line, keywd, varstr);
                    user_msg(ps, 0, INFO, msg);
                    error = 'y';
                 }
              else if ( optswtch == 'R' ) {
                 sprintf(msg, "line #%d: Restore Seeds and Save Seeds is NOT "
                              "Allowed in the same stanza!\n", line);
                 user_msg(ps, 0, INFO, msg);
                 error = 'y';
              } else {
                 sprintf(msg, "line #%d: Specify Seeds and Save Seeds is NOT "
                              "Allowed in the same stanza!\n", line);
                 user_msg(ps, 0, INFO, msg);
                 error = 'y';
              }
           } else if ( strcmp(varstr, "RESTORE_SEEDS") == 0 ) {
              if ( optswtch == 'N' )
                 if ( seeds_were_saved &&
                      !(current_ruleptr->rule_options & SAVE_SEEDS_FLAG) ) {
                    current_ruleptr->rule_options |= RESTORE_SEEDS_FLAG;
                    optswtch = 'R';
                 } else if ( current_ruleptr->rule_options & SAVE_SEEDS_FLAG ) {
                    sprintf(msg, "line #%d %s = %s (SAVE_SEEDS "
                                 "already specified)\n",
                            line, keywd, varstr);
                    user_msg(ps, 0, INFO, msg);
                    error = 'y';
                 } else {
                    sprintf(msg, "line #%d - RESTORE_SEEDS option "
                                 "not allowed unless\n"
                                 "saved in previous stanza.", first_line);
                    user_msg(ps, 0, INFO, msg);
                    error = 'y';
                 }
              else if ( optswtch == 'S' ) {
                 printf(msg, "line #%d: Save Seeds and Restore Seeds is NOT "
                              "Allowed in the same stanza!\n", line);
                 user_msg(ps, 0, INFO, msg);
                 error = 'y';
              } else {
                 sprintf(msg, "line #%d: Specify Seeds and Restore Seeds is "
                              "NOT Allowed in the same stanza!\n", line);
                 user_msg(ps, 0, INFO, msg);
                 error = 'y';
              }
           } else if ( strncmp(varstr, "SEEDS", 5) == 0 ) {
              if ( optswtch == 'N' ) {
                 sscanf(s, "%*s %*s %n", &i);
                 while ( isspace(s[i]) ) i++;
                 strcpy(varstr, &s[i]);
                 p = varstr;
                 i = 0;
                 while ( *p && i < 6 ) {
                    if ( isdigit(*p) ) {
                       current_ruleptr->useed[i] = atoi(p);
                       i++;
                       while ( isdigit(*(p++)) );
                    } else
                       p++;
                 }
                 current_ruleptr->useed_lba = atoi(p);
                 current_ruleptr->rule_options |= USER_SEEDS_FLAG;
                 optswtch = 'U';
              } else if ( optswtch == 'S' ) {
                 sprintf(msg, "line #%d: Save Seeds and Specify Seeds is NOT "
                              "Allowed in the same stanza!\n", line);
                 user_msg(ps, 0, INFO, msg);
                 error = 'y';
              } else {
                 sprintf(msg, "line #%d: Restore Seeds and Specify Seeds is "
                              "NOT Allowed in the same stanza!\n", line);
                 user_msg(ps, 0, INFO, msg);
                 error = 'y';
              }
           } else if ( strcmp(varstr, "TIMED") == 0 )
              current_ruleptr->timed = 'Y';
           else {
              sprintf(msg, "line #%d %s = %s (invalid option "
                           "for RULE_OPTIONS)\n",
                      line, keywd, varstr);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "ADDR_TYPE")) == 0 ) {
           sscanf(s, "%*s %s", current_ruleptr->addr_type);
           if ( strcmp(current_ruleptr->addr_type, "SEQ") == 0 ||
                strcmp(current_ruleptr->addr_type, "RANDOM") == 0 ) ;
           else {
              sprintf(msg, "line# %d %s = %s (must be SEQ or RANDOM) \n",
                      line, keywd, current_ruleptr->addr_type);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "NUM_OPER")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           current_ruleptr->num_oper = atoi(varstr);
           if ( current_ruleptr->num_oper < 0 ) {
              sprintf(msg, "line# %d %s = %d (must be >= 0) \n",
                      line, keywd, current_ruleptr->num_oper);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "OPER")) == 0 ) {

    	   sscanf(s, "%*s %s", current_ruleptr->oper);

           /* D502299 */
           if (strcmp(current_ruleptr->oper, "BWRC") == 0){
		      is_bwrc_rule_present = 1;
		   }

           if ( strcmp(current_ruleptr->oper, "R")     == 0 ||
                strcmp(current_ruleptr->oper, "S")     == 0 ||
                strcmp(current_ruleptr->oper, "W")     == 0 ||
#ifndef __HTX43X__
                strcmp(current_ruleptr->oper, "V")     == 0 ||
#endif
                strcmp(current_ruleptr->oper, "RC")    == 0 ||
                strcmp(current_ruleptr->oper, "RS")    == 0 ||
                strcmp(current_ruleptr->oper, "RW")    == 0 ||
                strcmp(current_ruleptr->oper, "WR")    == 0 ||
                strcmp(current_ruleptr->oper, "WS")    == 0 ||
                strcmp(current_ruleptr->oper, "WRC")   == 0 ||
                strcmp(current_ruleptr->oper, "BWRC")  == 0 ||
                strcmp(current_ruleptr->oper, "CARR")  == 0 ||
                strcmp(current_ruleptr->oper, "CARW")  == 0 ||
                strcmp(current_ruleptr->oper, "CAWR")  == 0 ||
                strcmp(current_ruleptr->oper, "CAWW")  == 0 ||
                strcmp(current_ruleptr->oper, "OMEX")  == 0 ||
                strcmp(current_ruleptr->oper, "OMMV")  == 0 ||
                strcmp(current_ruleptr->oper, "RWRC")  == 0 ||
                strcmp(current_ruleptr->oper, "WRCW")  == 0 ||
                strcmp(current_ruleptr->oper, "XCMD")  == 0 ||
                strcmp(current_ruleptr->oper, "OMINF") == 0 ||
                strcmp(current_ruleptr->oper, "OMINI") == 0 ||
                strcmp(current_ruleptr->oper, "OMINQ") == 0 ||
                strcmp(current_ruleptr->oper, "OMINV") == 0 ||
                strcmp(current_ruleptr->oper, "OMPRE") == 0 ) ;
           else {

#ifndef __HTX43X__
                sprintf(msg, "line# %d %s = %s - must be R,S,V,W,RC,RS,RW,WR,WS,"
                             "WRC,\nBWRC,CARR,CARW,CAWR,CAWW,OMEX,OMMV,RWRC,"
                             "WRCW,XCMD,OMINF,OMINI,OMINQ,OMINV,or OMPRE"
                             "separated by White-space character.\n",
                        line, keywd, current_ruleptr->oper);
#else
                sprintf(msg, "line# %d %s = %s - must be R,S,W,RC,RS,RW,WR,WS,"
                             "WRC,\nBWRC,CARR,CARW,CAWR,CAWW,OMEX,OMMV,RWRC,"
                             "WRCW,XCMD,OMINF,OMINI,OMINQ,OMINV,or OMPRE"
                             "separated by White-space character.\n",
                        line, keywd, current_ruleptr->oper);
#endif

                user_msg(ps, 0, INFO, msg);
                error = 'y';
           }
        } else if ( (strcmp(keywd, "STARTING_BLOCK")) == 0 ) {
           sscanf(s, "%*s %s", current_ruleptr->starting_block);
        } else if ( (strcmp(keywd, "DIRECTION")) == 0 ) {
           sscanf(s, "%*s %s", current_ruleptr->direction);
           if ( strcmp(current_ruleptr->direction, "UP") == 0   ||
                strcmp(current_ruleptr->direction, "DOWN") == 0 ||
                strcmp(current_ruleptr->direction, "OUT") == 0  ||
                strcmp(current_ruleptr->direction, "IN") == 0 ) ;
           else {
              sprintf(msg, "line# %d %s = %s (must be UP, DOWN, IN or OUT) \n",
                      line, keywd, current_ruleptr->direction);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "INCREMENT")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           current_ruleptr->increment = atoi(varstr);
        } else if ( (strcmp(keywd, "TYPE_LENGTH")) == 0 ) {
           sscanf(s, "%*s %s", current_ruleptr->type_length);
           if ( strcmp(current_ruleptr->type_length, "FIXED") == 0 ||
                strcmp(current_ruleptr->type_length, "RANDOM") == 0 ) ;
           else {
              sprintf(msg, "line# %d %s = %s (must be FIXED or RANDOM) \n",
                      line, keywd, current_ruleptr->type_length);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "NUM_BLKS")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           current_ruleptr->num_blks = atoi(varstr);
           if ( current_ruleptr->num_blks < 1 ||
                current_ruleptr->num_blks > 10000 ) {
              sprintf(msg, "line# %d %s = %d (must be > 1 and <= 10,000) \n",
                      line, keywd, current_ruleptr->num_blks);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "MAX_NUMBER_OF_THREADS")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           current_ruleptr->max_number_of_threads = atoi(varstr);
           if ( current_ruleptr->max_number_of_threads > 8 ||
                current_ruleptr->max_number_of_threads < 2 ) {
              sprintf(msg, "line# %d %s = %d (must be >= 2 and <= 8) \n",
                      line, keywd, current_ruleptr->max_number_of_threads);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
              current_ruleptr->max_number_of_threads = 1;
           }
        } else if ( (strcmp(keywd, "SCALE_OPER")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           current_ruleptr->percent = atof(varstr);
           if ( current_ruleptr->percent <= 0.0 ) {
              sprintf(msg, "line# %d %s = %s (must be greater than 0) \n",
                      line, keywd, varstr);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "SKIP")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           if ( varstr[0] == '-' ) {
              varstr[0] = '0';
              current_ruleptr->repeat_neg = atoi(varstr);
           } else
              current_ruleptr->repeat_pos = atoi(varstr);
        } else if ( (strcmp(keywd, "SLEEP")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           current_ruleptr->sleep = atoi(varstr);
           if ( current_ruleptr->sleep < 0 ) {
              sprintf(msg, "line# %d %s = %d (must be >= 0) \n",
                      line, keywd, current_ruleptr->sleep);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "CRASH_ON_ANYERR")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           if      ( strcmp(varstr, "YES") == 0 ) crash_on_anyerr = 1;
           else if ( strcmp(varstr, "NO")  == 0 ) crash_on_anyerr = 0;
           else {
              sprintf(msg, "line# %d %s = %s (must be YES or NO) \n",
                      line, keywd, varstr);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "TURN_ATTENTION_ON")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           if      ( strcmp(varstr, "YES") == 0 ) turn_attention_on = 1;
           else if ( strcmp(varstr, "NO")  == 0 ) turn_attention_on = 0;
           else {
              sprintf(msg, "line# %d %s = %s (must be YES or NO) \n",
                      line, keywd, varstr);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        }

#ifndef __HTX43X__
         else if ( (strcmp(keywd, "PASS_THRU")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           if      ( strcmp(varstr, "YES") == 0 ) pass_thru = 1;
           else if ( strcmp(varstr, "NO")  == 0 ) pass_thru = 0;
           else {
              sprintf(msg, "line# %d %s = %s (must be YES or NO) \n",
                      line, keywd, varstr);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "DEBUG_PASSTHRU")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           if      ( strcmp(varstr, "YES") == 0 ) debug_passthru = 1;
           else if ( strcmp(varstr, "NO")  == 0 ) debug_passthru = 0;
           else {
              sprintf(msg, "line# %d %s = %s (must be YES or NO) \n",
                      line, keywd, varstr);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        }
#endif
	 else if ( (strcmp(keywd, "CRASH_ON_MIS")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           if      ( strcmp(varstr, "YES") == 0 ) crash_on_miscom = 1;
           else if ( strcmp(varstr, "NO")  == 0 ) crash_on_miscom = 0;
           else {
              sprintf(msg, "line# %d %s = %s (must be YES or NO) \n",
                      line, keywd, varstr);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "LA_ON_MIS")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           if      ( strcmp(varstr, "YES") == 0 ) la_on_miscom = 1;
           else if ( strcmp(varstr, "NO")  == 0 ) la_on_miscom = 0;
           else {
              sprintf(msg, "line# %d %s = %s (must be YES or NO) \n",
                      line, keywd, varstr);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "LOOP_ON_OFFSET")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           if ( strcmp(varstr, "YES") == 0 )
              current_ruleptr->loop_on_offset = 1;
           else if ( strcmp(varstr, "NO")  == 0 )
              current_ruleptr->loop_on_offset = 0;
           else {
              sprintf(msg, " line# %d %s = %s (must be YES or NO) \n",
                      line, keywd, varstr);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "OFFSET")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           current_ruleptr->offset = atoi(varstr);
           if ( current_ruleptr->offset < 0 || current_ruleptr->offset > 63 ) {
              sprintf(msg, "line# %d %s = %d (must be >= 0 and <= 63 ) \n",
                      line, keywd, current_ruleptr->offset);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "NO_MALLOCS")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           current_ruleptr->no_mallocs = atoi(varstr);
           if ( current_ruleptr->no_mallocs < 0 ||
                current_ruleptr->no_mallocs > 100 ) {
              sprintf(msg, "line# %d %s = %d (must be >= 0 and <= 100) \n",
                      line, keywd, current_ruleptr->no_mallocs);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "START_STANZA")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           if      ( strcmp(varstr, "YES") == 0 ) current_ruleptr->start = 'Y';
           else if ( strcmp(varstr, "NO")  == 0 ) current_ruleptr->start = 'N';
           else {
              sprintf(msg, "line# %d %s = %s (must be YES or NO) \n",
                      line, keywd, varstr);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "GLOBAL_VAR")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           if ( strcmp(varstr, "HANG_TIME") == 0 ) {
              sscanf(s, "%*s %*s %s", varstr);
              hang_time = atoi(varstr);
           } else if ( strcmp(varstr, "HANG_THRESHOLD") == 0 ) {
              sscanf(s, "%*s %*s %s", varstr);
              threshold = atoi(varstr);
           } else if ( strcmp(varstr, "ACCOUS_RULES") == 0 ) {
              sscanf(s, "%*s %*s %s", varstr);
	      if      ( strcmp(varstr, "YES") == 0) accous_rules = 1;
	      else if ( strcmp(varstr, "NO")  == 0) accous_rules = 0;
              else {
                 sprintf(msg, "line# %d %s = %s (must be YES or NO) \n",
                         line, keywd, varstr);
                 user_msg(ps, 0, INFO, msg);
                 error = 'y';
              }
           } else if ( strcmp(varstr, "CRASH_ON_HANG") == 0 ) {
              sscanf(s, "%*s %*s %s", varstr);
              if      ( strcmp(varstr, "YES") == 0 ) crash_on_hang = 'Y';
              else if ( strcmp(varstr, "NO")  == 0 ) crash_on_hang = 'N';
              else {
                 sprintf(msg, "line# %d %s = %s (must be YES or NO) \n",
                         line, keywd, varstr);
                 user_msg(ps, 0, INFO, msg);
                 error = 'y';
              }
           } else if ( strcmp(varstr, "MISC_CMD") == 0 ) {
              j = 0;
              for ( i = 24; s[i] != '\n' && s[i] != '\0'; i++, j++ )
                  misc_run_cmd[j] = t[i];
              misc_run_cmd[j] = '\0';
           } else if ( strcmp(varstr, "RUN_ON_MISC") == 0 ) {
              sscanf(s, "%*s %*s %s", varstr);
              if      ( strcmp(varstr, "YES") == 0 ) run_on_misc = 'Y';
              else if ( strcmp(varstr, "NO")  == 0 ) run_on_misc = 'N';
              else {
                 sprintf(msg, "line# %d %s = %s (must be YES or NO) \n",
                         line, keywd, varstr);
                 user_msg(ps, 0, INFO, msg);
                 error = 'y';
              }
           } else if ( strcmp(varstr, "RUN_REREAD") == 0 ) {
              sscanf(s, "%*s %*s %s", varstr);
              if      ( strcmp(varstr, "YES") == 0 ) run_reread = 'Y';
              else if ( strcmp(varstr, "NO")  == 0 ) run_reread = 'N';
              else {
                 sprintf(msg, "line# %d %s = %s (must be YES or NO) \n",
                         line, keywd, varstr);
                 user_msg(ps, 0, INFO, msg);
                 error = 'y';
              }
           } else {
              sprintf(msg, "line# %d %s = %s\n "
                           "(ONLY the variables HANGTIME and CRASH_ON_HANG\n"
                           " can be UPDATED at this time!) \n",
                      line, keywd, varstr);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "DEV_NAME")) == 0 ) {
           for ( i = 0; s[i] != '\n'; i++ )
              s[i] = tolower(s[i]);
           sscanf(s, "%*s %s", current_ruleptr->dev_name);
        } else if ( (strcmp(keywd, "OM_MAXSLOTS")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           current_ruleptr->om_maxslots = atoi(varstr);
        } else if ( (strcmp(keywd, "OM_SOURCE")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           current_ruleptr->om_source = atoi(varstr);
           if ( current_ruleptr->om_source < 0 ||
                current_ruleptr->om_source > current_ruleptr->om_maxslots ) {
              sprintf(msg, "line# %d %s = %d (must be >= 0 and <= %d) \n",
                      line, keywd, current_ruleptr->om_source,
                      current_ruleptr->om_maxslots);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if  ( (strcmp(keywd, "OM_TARGET")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           current_ruleptr->om_target = atoi(varstr);
           if ( current_ruleptr->om_target < 0 ||
                current_ruleptr->om_target > current_ruleptr->om_maxslots ) {
              sprintf(msg, "line# %d %s = %d (must be >= 0 and <= %d) \n",
                      line, keywd, current_ruleptr->om_target,
                      current_ruleptr->om_maxslots);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "OM_TARGET1")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           current_ruleptr->om_target1 = atoi(varstr);
           if ( current_ruleptr->om_target1 < 0 ||
                current_ruleptr->om_target1 > current_ruleptr->om_maxslots ) {
              sprintf(msg, "line# %d %s = %d (must be >= 0 and <= %d) \n",
                      line, keywd, current_ruleptr->om_target1,
                      current_ruleptr->om_maxslots);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "OM_INVERT")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           if      ( strcmp(varstr,"YES") == 0 ) current_ruleptr->om_invert = 1;
           else if ( strcmp(varstr,"NO")  == 0 ) current_ruleptr->om_invert = 0;
           else {
              sprintf(msg, "line# %d %s = %s (must be YES or NO) \n",
                      line, keywd, varstr);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "OM_INVERT1")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           if ( strcmp(varstr, "YES") == 0 )
              current_ruleptr->om_invert1 = 1;
           else if ( strcmp(varstr, "NO")  == 0 )
              current_ruleptr->om_invert1 = 0;
           else {
              sprintf(msg, "line# %d %s = %s (must be YES or NO) \n",
                      line, keywd, varstr);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "OM_PREVENT")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           if ( strcmp(varstr, "YES") == 0 )
              current_ruleptr->om_prevent = 1;
           else if ( strcmp(varstr, "NO")  == 0 )
              current_ruleptr->om_prevent = 0;
           else {
              sprintf(msg, "line# %d %s = %s (must be YES or NO) \n",
                      line, keywd, varstr);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "COMMAND")) == 0 ) {
           j = 0;
           for ( i = 10; s[i] != '\n' && s[i] != '\0'; i++, j++ )
               current_ruleptr->cmd_list[j] = t[i];
           current_ruleptr->cmd_list[j] = '\0';
        } else if ( (strcmp(keywd, "ALIGN")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
               align_tmp = atoi(varstr);
             if( align_tmp >= 0 )
               {
                current_ruleptr->align = align_tmp;
               }
            else {
                sprintf(msg, "line# %d %s = %d ( A positive value for align must be specified) \n", line, keywd, current_ruleptr->offset);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
              }
	} else if ( (strcmp(keywd, "CPU_BIND")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           if      ( strcmp(varstr, "YES") == 0 ) cpu_bind = 1;
           else if ( strcmp(varstr, "NO")  == 0 ) cpu_bind = 0;
           else {
              sprintf(msg, "line# %d %s = %s (must be YES or NO) \n",
                      line, keywd, varstr);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "OPEN_RETRY_COUNT")) == 0 ) {
           sscanf(s,"%*s %s",varstr);
           open_retry_count = atoi(varstr);
           if ( open_retry_count < 0 ) {
              sprintf(msg, "line# %d %s = %d (must be >= 0) \n",
                      line, keywd, open_retry_count);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
        } else if ( (strcmp(keywd, "UNIQUE_PATTERN")) == 0 ) {
           sscanf(s, "%*s %s", varstr);
           if      ( strcmp(varstr, "YES") == 0 ) num_sub_blks_psect = (blksize/128)-1;
           else if ( strcmp(varstr, "NO")  == 0 ) num_sub_blks_psect = 0;
           else {
              sprintf(msg, "line# %d %s = %s (must be YES or NO) \n",
                      line, keywd, varstr);
              user_msg(ps, 0, INFO, msg);
              error = 'y';
           }
				} else {
           sprintf(msg, "line# %d keywd = %s (invalid)\n"
                        "Suspect line is:\n\"%s\"\n",
                   line, keywd,  s);
           user_msg(ps, 0, INFO, msg);
           error = 'y';
        }
      }
      rec_lgth = 0;
    } else {
     if(current_ruleptr != NULL)
     {
       static char sysvol[1024];
       char *hdisk;
       char *p;
       if ( getsysvol(sysvol, sizeof(sysvol)) == NULL ) {
          sprintf(msg, "Error getting system volume names from "
                       "file sysdisk in directory /tmp.\n"
                       "Check to make sure file sysdisk exists!\n");
          user_msg(ps, 0, INFO, msg);
          error = 'y';
       }
                          /* check ps->sdev_id against each volume in sysvol */
       if ( index(sysvol, ':') == 0 )
          strcat(sysvol, ":");             /* old style sysvol, append a ':' */
       for ( hdisk = sysvol; ; hdisk = p ) {
                                  /* terminate hdisk with a NULL char at ':' */
          if ( (p = (char *)index(hdisk, ':')) == 0 )
             break;
          *p++ = '\0';                                 /* change ':' to NULL */
                    /* now check ps->sdev_id (+6 to skip past "/dev/r" part) */
          if ( strcmp((ps->sdev_id)+6,hdisk) == 0 &&
               strcmp(current_ruleptr->oper,"R") != 0 ) {
             sprintf(msg, "Write to %s not allowed \n", hdisk);
             user_msg(ps, 0, INFO, msg);
             error = 'y';
          }
       }
       if ( strcmp(current_ruleptr->rule_id, "        ") == 0 ) {
          sprintf(msg, "line# %d rule_id not specified \n", first_line);
          user_msg(ps, 0, INFO, msg);
          error = 'y';
       }
       if ( (tvar = current_ruleptr->num_blks + current_ruleptr->increment)
             < 0 ) {           /* chg 2 */
          sprintf(msg, "line# %d %s = %d (negative increment must be "
                       "less than the number of blocks specified) \n",
                  line, keywd, current_ruleptr->increment);
          user_msg(ps, 0, INFO, msg);
          error = 'y';
       }
       current_ruleptr->first_block = set_first_blk(current_ruleptr);
       if ( strcmp(current_ruleptr->direction, "OUT") == 0 ) {
          if ( current_ruleptr->first_block !=  (current_ruleptr->min_blkno +
               (current_ruleptr->max_blkno - current_ruleptr->min_blkno) / 2)) {
             current_ruleptr->first_block = current_ruleptr->min_blkno +
                 (current_ruleptr->max_blkno - current_ruleptr->min_blkno) / 2;
             sprintf(msg, "line #%d - The STARTING_BLOCK has been "
                          "reset to a MIDDLE LBA for this direction\n",
                     first_line);
             user_msg(ps, 0, INFO, msg);
          }
       }
       if ( (current_ruleptr->rule_options &
            (SAVE_SEEDS_FLAG | RESTORE_SEEDS_FLAG)) ) {
          if ( strcmp(current_ruleptr->addr_type, "RANDOM") ) {
             sprintf(msg, "Line %d: SAVE/RESTORE seed options must be "
                          "used with\nRANDOM addressing only.", first_line);
             user_msg(ps, 0, INFO, msg);
             error = 'y';
          } else if ( strcmp(current_ruleptr->type_length, "RANDOM") == 0 ) {
             if ( !(strcmp(current_ruleptr->pattern_id, "#003") == 0 ||
                    strcmp(current_ruleptr->pattern_id, "#004") == 0 ||
                    strcmp(current_ruleptr->pattern_id, "#005") == 0 ||
                    strcmp(current_ruleptr->pattern_id, "#006") == 0 ||
                    strcmp(current_ruleptr->pattern_id, "#007") == 0) ) {
                sprintf(msg, "Line #%d - When using SAVE/RESTORE_SEEDS option "
                             "with\nRANDOM lengths, always use pattern #003, "
                             "#004, #005, #006 or #007 to avoid\nmiscompares due to " "overlapping sequences.", first_line);
                user_msg(ps, 0, INFO, msg);
                error = 'y';
             } else if ( current_ruleptr->rule_options & SAVE_SEEDS_FLAG ) {
                saved_num_blocks = current_ruleptr->num_blks;
                saved_last_type_len = 'R';
             } else if ( saved_last_type_len == 'F' ) {
                sprintf(msg, "Line #%d - Last SAVE_SEEDS stanza used FIXED\n"
                             "This stanza uses RANDOM lengths - they must be\n"
                             "the same type length!", first_line);
                user_msg(ps, 0, INFO, msg);
                error = 'y';
             } else if ( saved_num_blocks != current_ruleptr->num_blks ) {
                sprintf(msg, "Line %d - NUM_BLKS in RESTORE_SEEDS stanza "
                             "using\nTYPE_LENGTH = RANDOM must use same NUM_"
                             "BLKS\n as stanza where seeds were saved.",
                        first_line);
                user_msg(ps, 0, INFO, msg);
                error = 'y';
             }
          } else if ( current_ruleptr->rule_options & SAVE_SEEDS_FLAG ) {
             saved_num_blocks = current_ruleptr->num_blks;
             saved_last_type_len = 'F';
          } else if ( saved_last_type_len == 'R' ) {
             sprintf(msg, "Line #%d - Last SAVE_SEEDS stanza used RANDOM\n"
                          "This stanza uses FIXED lengths - they must be\n"
                          "the same type length!", first_line);
             user_msg(ps, 0, INFO, msg);
             error = 'y';
          } else if ( current_ruleptr->num_blks > saved_num_blocks ) {
             sprintf(msg, "Line #%d - When using RESTORE_SEEDS option with\n"
                          "FIXED lengths, always use NUM_BLKS <= to number\n"
                          "used in preceeding SAVE_SEEDS stanza!\n",
                     first_line);
             user_msg(ps, 0, INFO, msg);
             error = 'y';
          }
       }
       if ( current_ruleptr->num_oper == 0 &&
            strcmp(current_ruleptr->addr_type, "SEQ") != 0 ) {
          sprintf(msg, "line# %d num_oper = 0 but addr_type not SEQ\n",
                  first_line);
          user_msg(ps, 0, INFO, msg);
          error = 'y';
       }
       if ( first_ruleptr == NULL )
          first_ruleptr = current_ruleptr;
       else
          prev_ruleptr->next_ruleptr = current_ruleptr;
       prev_ruleptr = current_ruleptr;
       prev_ruleptr->next_ruleptr = NULL;    /* chg 1 */
       first_line = line + 1;
       keywd_count = 0;
       optswtch = 'N';
       if ( eof_file == 'F' )
          eof_file = 'Y';
    }
    }
    if ( eof_file == 'N' ) {
       if ( fgets(s, 200, fptr) == NULL ) {
          fclose(fptr);
          eof_file = 'F';
          rec_lgth = 0;
       } else
          rec_lgth = strlen(s);
    }
  }
  current_ruleptr = NULL;
  if ( error == 'n' )
     return(0);
  else
     return(-1);
}
