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

/****************************************************************************

Function(s) Parse/Extract HTX Rule File

These collection of functions are responsible for parsing out the
rules file by the hardware exerciser.  Functions attempt to locate
keywords in the rules file as defined specifically for the device
in the structure, XXXXXXXX.

*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#ifdef LIBC_23
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <string.h>
#include <ctype.h>
#include "getrule.h"
#include "hxihtx.h"
#include "htx_exer.h"
#include "sevcodes.h"
#define BUFF 1023

void skip_to_end (FILE *rule_fp,int *rule_file_lineno);
int get_fmt_line (FILE *rule_fp,struct htx_data *htx_sp,char *buf,int new_rule_flag,int *rule_file_lineno);
struct rule_def_struct *lookup_rule (struct rule_def_struct rule_def_table[],char *keyword);
int check_long (char *value,long *long_ptr);

int check_keyword_value(char *keyword,char *value,struct htx_data *htx_sp,int *rule_file_lineno, FILE *rule_fp,int *ruleid_count,struct rule_def_struct **master_ptr,struct rule_def_struct rule_def_table[],char *buf);
int assign_value (struct rule_def_struct rule_def_table[], struct rule_def_struct *master_ptr,char *value, union bind_union *bind_table);

/*****************************************************************************

rule_err

Function sends a RULE error message HTX via the "htx_err()" function.
Function expects (IN ORDER):

	1)  the htx_data structure pointer
	2)  rule error code
	3)  the line number within the rules file

	4)  pointer to a "printf()" format string, OR NULL.

	5)  if pointer from 4) is not null, then remaining args
	    may exist to support the printf string.

*****************************************************************************/
#ifdef LIBC_23
void rule_err( char *fmt, ...)
#else
void rule_err( va_alist )
va_dcl
#endif
{
	va_list args;
	struct htx_data *htx_sp;
	int error_code;
	int rule_file_lineno;
	char *fmtstring;
	char buf[BUFF];
#ifdef LIBC_23
	va_start(args, fmt);
#else
	va_start(args);
#endif

	htx_sp = va_arg(args, struct htx_data *);
	error_code = va_arg(args, int);
	rule_file_lineno = va_arg(args, int);
	fmtstring = va_arg(args, char *);


	switch (error_code)
	   {
		case RULE_READ_ERR:
			sprintf(buf,
		     "Line number:  %d.  Read error occurred in rules file.\n",
			rule_file_lineno);
				     break;
		case RULE_SYN_ERR:
			sprintf(buf,
			"Line number:  %d.  Syntax error in rules file.\n",
			rule_file_lineno);
				     break;
		case RULE_NOID_ERR:
			sprintf(buf,
		   "Line number:  %d.  No rule id specified before keyword.\n",
			rule_file_lineno);
				     break;
		case RULE_NOBLANK_ERR:
			sprintf(buf,
	       "Line number:  %d.  Missing blank line between rule stanzas.\n",
			rule_file_lineno);
				     break;
		case RULE_BADKW_ERR:
			sprintf(buf,
	"Line number:  %d.  Illegal or undefined keyword in rules file.\n",
			rule_file_lineno);
				     break;
		case RULE_BADVAL_ERR:
			sprintf(buf,
    "Line number:  %d.  Illegal VALUE in rules file.\n",
			rule_file_lineno);
				     break;
		case RULE_RANGE_ERR:
			sprintf(buf,
 "Line number:  %d.  VALUE out of range in rules file.\n", rule_file_lineno);
				     break;
		case RULE_TYPE_ERR:
			sprintf(buf,
 "Line number:  %d.  Illegal data type in rules definition table.\n",
			rule_file_lineno);
				     break;

		case RULE_REDEF_ERR:
			sprintf(buf,
 "Line number:  %d.  WARNING - Keyword redefined within stanza.\n",
			rule_file_lineno);
				     break;
		default:
			sprintf(buf,
			"Unknown rule error code %d.", error_code);
	   }
	if (fmtstring != NULL)
		vsprintf(buf+strlen(buf),fmtstring,args);

	htx_err(5, "%p%d%d%p",htx_sp, NO_ERRNO, SYSERR, buf);
	va_end(args);
   }
/*****************************************************************************

skip_to_end

Function reads from rule file (via the file pointer, "rule_fp") until
the end of the current rule stanza is reached, or EOF.  "rule_file_lineno"
is passed in so that we can increment our position within the file.

*****************************************************************************/

void skip_to_end (FILE *rule_fp,int *rule_file_lineno)
   {
	char buf[BUFF+1];
	char *cp;

	while (fgets(buf, BUFF, rule_fp) != NULL)
	   {
		(*rule_file_lineno)++;
		cp = buf;
		while (isspace(*cp))
			cp++;
		if (*cp == '\0')
			break;
	   }
   }
/*****************************************************************************

get_fmt_line

Function reads in a line from rules file pointed to by "rule_fp" and
checks syntax.  If a non-comment line, then function edits line
into format,  "keyword   value".  NOTE, that function
formats keyword to upper case.

IF successful
THEN RETURNs 0
ELSE IF (EOF or at end of rule stanza)
     THEN RETURNs -1
     ELSE IF ERROR
	  THEN RETURNs a rule_file_error_code

*****************************************************************************/

int get_fmt_line (FILE *rule_fp,struct htx_data *htx_sp,char *buf,int new_rule_flag,int *rule_file_lineno)
   {
	char *cp;

	/*
	 * if new_rule_flag is set (1), then a new rule
	 * stanza is expected to be parsed so skip over any
	 * blank lines (or comments).  If new_rules_flag is NOT set (0),
	 * then a blank (non-comment) line encountered here should signal
	 * back to getrule() that the end of the rule stanza
	 * has been reached.
	 */
	while (1)
	   {
		/*
		 * read a line from the rules file.
		 */
		if (fgets(buf, BUFF, rule_fp) == NULL)
			return(-1);

		/*
		 * increment line number counter in rules file.
		 */

		(*rule_file_lineno)++;

		/*
		 * point to start of line and skip over blanks.
		 */
		cp = buf;
		while (isspace(*cp))
			cp++;

		if (new_rule_flag)
		   {
			/*
			 * if something is on the line (other than
			 * a comment), then break out
			 */
			 if ((*cp != '\0') && (*cp != '*'))
				break;
		   }
		else
		   {
			if (*cp == '\0')
				return(-1);
			if (*cp != '*')
				break;
		   }
	   }


	/*
	 * if not an alpha char, then no keyword, so error.
	 */
	if (!isalpha(*cp))
	   {
		rule_err("%p%d%d%p%d",htx_sp, RULE_SYN_ERR, *rule_file_lineno,
	"Keyword began with non-alpha character (dec.= %d).", *cp);
		skip_to_end(rule_fp, rule_file_lineno);
		return(RULE_SYN_ERR);
	   }

	/*
	 * scan over potential keyword and convert to upper case
	 */
	while (!isspace(*cp) && (*cp != '=') && (*cp != '\0'))
	   {
		if (isalpha(*cp))
			*cp = toupper(*cp);
		cp++;
	   }

	/*
	 * if hit end of line, then no keyword value, so error
	 */
	if (*cp == '\0')
	   {
		rule_err("%p%d%d%p",htx_sp, RULE_SYN_ERR, *rule_file_lineno,
			"Missing assignment operator '='.");
		skip_to_end(rule_fp, rule_file_lineno);
		return(RULE_SYN_ERR);
	   }

	/*
	 * scan over to the assignment operator (equal sign)
	 */
	while (isspace(*cp))
		cp++;

	if (*cp != '=')
	   {
		rule_err("%p%d%d%p",htx_sp, RULE_SYN_ERR, *rule_file_lineno,
			"Missing assignment operator '='.");
		skip_to_end(rule_fp, rule_file_lineno);
		return(RULE_SYN_ERR);
	   }

	/*
	 * blank out the equal sign
	 */
	*cp = ' ';

	/*
	 * parse out remainder of line for keyword value.
	 * If hit null, then none, so error.
	 */
	cp++;
	while (isspace(*cp))
		cp++;

	if (*cp == '\0')
	   {
		rule_err("%p%d%d%p",htx_sp, RULE_SYN_ERR, *rule_file_lineno,
			"No keyword value found on line.");
		skip_to_end(rule_fp, rule_file_lineno);
		return(RULE_SYN_ERR);
	   }

	return(0);
   }

/*****************************************************************************

lookup_rule

Function attempts to lookup "keyword" in the master definition rules
table defined by "rule_def_table".

IF found
THEN RETURNs pointer to entry in table
ELSE RETURNs NULL

*****************************************************************************/

struct rule_def_struct *lookup_rule (struct rule_def_struct rule_def_table[],char *keyword)
   {
	static struct rule_def_struct *rds_p;

	rds_p = rule_def_table;
	while (rds_p->keyword[0] != '\0')
		if (strcmp(rds_p->keyword, keyword) == 0)
			return(rds_p);
		else
			rds_p++;

	return(NULL);
   }

#if 0
/*
 * commented out temporarily due
 * to problems with floating point on DD2 machines
 */

/*****************************************************************************

check_double

Function verifies that the string "value" passed in can be converted
to a legal double and places the contents in "*double_ptr".

IF successful
THEN RETURNs 0
ELSE RETURNs 1

*****************************************************************************/

int check_double (value, double_ptr)
   char *value;
   double *double_ptr;
   {
	char *ptr;
	double strtod();
	double double_value;

	double_value = strtod(value, &ptr);
	*double_ptr = double_value;
	if ((ptr == value) && (double_value == 0.0))
		return(1);

	if (!isspace(*ptr) && (*ptr != '\0'))
		return(1);

	return(0);
   }
#endif /* end if-zero */

/*****************************************************************************

check_long

Function verifies that the string "value" passed in can be converted
to a legal long and places the contents in "*long_ptr".

IF successful
THEN RETURNs 0
ELSE RETURNs 1

*****************************************************************************/

int check_long (char *value,long *long_ptr)
   {
	char *ptr;
	long long_value;

	long_value = strtol(value, &ptr, 10);
	*long_ptr = long_value;
	if ((ptr == value) && (long_value == 0))
		return(1);

	if (!isspace(*ptr) && (*ptr != '\0'))
		return(1);

	return(0);
   }

/*****************************************************************************

check_keyword_value

Function takes formatted line ("buf") and attempts to pull out the
keyword and value.  Function checks for legal keyword name
and value and returns them in "keyword" and "value", respectively.
If all is legal, "master_ptr" is set to point to the rule definition
in the "rule_def_table" for future reference (i.e., when the item
actually gets added to the linked_list).  Also, ruleid_count gets
incremented if the keyword encountered is a "RULE_ID".

*****************************************************************************/

int check_keyword_value(char *keyword,char *value,struct htx_data *htx_sp,int *rule_file_lineno, FILE *rule_fp,int *ruleid_count,struct rule_def_struct **master_ptr,struct rule_def_struct rule_def_table[],char *buf)
   {
	register int len;
	register char *cp, *cp2;
	long long_value;
#if 0
/*
 * commented out temporarily due
 * to problems with floating point on DD2 machines
 */
	double double_value;
#endif

	extern void strlencpy(char *,char *,int);
	extern int validate_value(struct htx_data *htx_sp,struct rule_def_struct *master_ptr,char *value,long lvalue);

	/*
	 * point to start of keyword line and skip past spaces.
	 */
	cp = buf;
	while (isspace(*cp))
		cp++;

	/*
	 * copy the keyword off the line.  After copying check
	 * that we now point to a space.  If not, then length
	 * of keyword was too long, so error.
	 */
	cp2 = cp;
	while (!isspace(*cp2))
		cp2++;
	len = cp2 - cp;
	if (len > MAX_KEYWORD_LEN)
	   {
		rule_err("%p%d%d%p",htx_sp, RULE_BADKW_ERR, *rule_file_lineno, NULL);
		skip_to_end(rule_fp, rule_file_lineno);
		return(RULE_BADKW_ERR);
	   }
	strlencpy(keyword, cp, len);
	cp += strlen(keyword);
	if (!isspace(*cp))
	   {
		rule_err("%p%d%d%p",htx_sp, RULE_BADKW_ERR, *rule_file_lineno, NULL);
		skip_to_end(rule_fp, rule_file_lineno);
		return(RULE_BADKW_ERR);
	   }

	/*
	 * go search for the read-in keyword in the master rules
	 * definition table to insure that it is legal.
	 */
	if ((*master_ptr = lookup_rule(rule_def_table, keyword)) == NULL)
	   {
		rule_err("%p%d%d%p%p",htx_sp, RULE_BADKW_ERR, *rule_file_lineno,
			"Invalid keyword:  <%s>.\n", keyword);
		skip_to_end(rule_fp, rule_file_lineno);
		return(RULE_BADKW_ERR);
	   }

	/*
	 * If this keyword is "RULE_ID" (i.e. start of new rule stanza)
	 * then increment the rule_id count.  This will be checked later
	 * to insure that we don't have more than one "RULE_ID" within
	 * a single rule stanza.
	 */
	if (strcmp((*master_ptr)->keyword,"RULE_ID") == 0)
		(*ruleid_count)++;


	/*
	 * skip past the spaces now and go read the value.
	 */
	while (isspace(*cp))
		cp++;

	if (*cp == '\0')
	   {
		rule_err("%p%d%d%p",htx_sp, RULE_SYN_ERR, *rule_file_lineno, NULL);
		skip_to_end(rule_fp, rule_file_lineno);
		return(RULE_SYN_ERR);
	   }

	cp2 = cp;
	while (!isspace(*cp2))
		cp2++;
	len = cp2 - cp;
	if (len > MAX_KEYVAL_LEN)
	   {
		rule_err("%p%d%d%p",htx_sp, RULE_BADVAL_ERR, *rule_file_lineno, NULL);
		skip_to_end(rule_fp, rule_file_lineno);
		return(RULE_BADVAL_ERR);
	   }
	strlencpy(value, cp, len);

	/*
	 * if this keyword expects a char value, check
	 * for legal length.
	 */
	if ((*master_ptr)->data_type == RSTRING_TYPE)
	   {
		if (len > (*master_ptr)->len)
		   {
			rule_err("%p%d%d%p%p%d%d",htx_sp, RULE_BADVAL_ERR, *rule_file_lineno,
	 "Bad value:  %s.\nLength of value (%d) greater than allowed (%d).",
	 value, len, (*master_ptr)->len);
			skip_to_end(rule_fp, rule_file_lineno);
			return(RULE_BADVAL_ERR);
		   }
	   }
	cp += len;
	if (!isspace(*cp) && (*cp != '\0'))
	   {
		rule_err("%p%d%d%p",htx_sp, RULE_BADVAL_ERR, *rule_file_lineno, NULL);
		skip_to_end(rule_fp, rule_file_lineno);
		return(RULE_BADVAL_ERR);
	   }

	/*
	 * if data_type is numerical (long or double),
	 * verify of legal value.
	 */
	switch ((*master_ptr)->data_type)
	   {
		case RSTRING_TYPE:
				break;

#if 0
/*
 * commented out temporarily due
 * to problems with floating point on DD2 machines
 */
		case RDOUBLE_TYPE:
				if (check_double(value, &double_value))
				   {
					rule_err(htx_sp, RULE_BADVAL_ERR,
						*rule_file_lineno,
						"Bad double:  <%s>.",
						value);
					skip_to_end(rule_fp,
						rule_file_lineno);
					return(RULE_BADVAL_ERR);
				   }
				break;
#endif

		case RLONG_TYPE:
				if (check_long(value, &long_value))
				   {
					rule_err("%p%d%d%p%p",htx_sp, RULE_BADVAL_ERR,
						*rule_file_lineno,
						"Bad long:  <%s>.",
						value);
					skip_to_end(rule_fp,
						rule_file_lineno);
					return(RULE_BADVAL_ERR);
				   }

#if 0
/*
 * commented out temporarily due
 * to problems with floating point on DD2 machines
 */
				double_value = (double) long_value;
#endif
				break;

		default:
				rule_err("%p%d%d%p%d",htx_sp, RULE_BADVAL_ERR,
				  *rule_file_lineno,
				  "Unknown data type (dec. = %d)",
					(*master_ptr)->data_type);
				skip_to_end(rule_fp, rule_file_lineno);
				return(RULE_TYPE_ERR);
	   }

	if (validate_value(htx_sp, *master_ptr, value, long_value))
	   {
		rule_err("%p%d%d%p%p%p",htx_sp, RULE_BADVAL_ERR,
			*rule_file_lineno,
	"Bad value:  %s.\nLegal values defined include:  %s.",
	value, (*master_ptr)->value_list);
		return(RULE_BADVAL_ERR);
	   }

	return(0);
   }

/*****************************************************************************

assign_value

Function assigns the read-in value "value" to the pointer whose address
is pointed to by an entry in the array "bind_table".

*****************************************************************************/

int assign_value (struct rule_def_struct rule_def_table[], struct rule_def_struct *master_ptr,char *value, union bind_union *bind_table)
   {
	register long table_index;
	union bind_union *bu_p;
#if 0
/*
 * commented out temporarily due
 * to problems with floating point on DD2 machines
 */
	double strtod();
#endif

	/*
	 * calculate the index into the rule definition table
	 * of this keyword pointed by "master_ptr" so we know
	 * how far to go down into the array of pointers
	 * pointed by "bind_table" to get the address to store
	 * this particular value.
	 */
	table_index = master_ptr - rule_def_table;
	bu_p = bind_table + table_index;
	switch (master_ptr->data_type)
	   {
		case RSTRING_TYPE:
				strcpy(bu_p->cp, value);
				break;
		case RLONG_TYPE:
				*bu_p->lp = strtol(value,(char **) NULL , 10);
				break;

#if 0
/*
 * commented out temporarily due
 * to problems with floating point on DD2 machines
 */
		case RDOUBLE_TYPE:
				*bu_p->dp = strtod(value, (char **) NULL);
				break;
#endif
		default:
				return(1);
	   }
	return(0);
   }
/*****************************************************************************

getrule

Function parses rule file pointed to by "rule_fp" and attempts to
read a rule stanza and build a list of keywords and their associated
values for the rule.  Keywords and values are checked against the
master definition list (from "rule_def_table") and, if all is ok,
built into a linked list.  Any errors incurred, are sent to HTX
via hxfmsg() using the specially defined structure "htx_sp".

IF successful
THEN RETURNs 0
ELSE IF EOF
     THEN RETURNs -1
     ELSE RETURNs  1

*****************************************************************************/

int getrule (FILE *rule_fp,int *rule_file_lineno,struct htx_data *htx_sp,struct rule_def_struct rule_def_table[],union bind_union *bind_table)
   {
	int at_end_of_stanza, cc;
	char buf[BUFF+1];
	char keyword[MAX_KEYWORD_LEN+1];
	char value[MAX_KEYVAL_LEN+1];
	struct rule_def_struct *master_ptr;
	int ruleid_count;
	int new_rule_flag;


	/*
	 * init flag that we are now processing a new rule stanza.
	 */
	new_rule_flag = 1;
	at_end_of_stanza = 0;
	ruleid_count = 0;
	while (!at_end_of_stanza)
	   {
		/*
		 * go read and check syntax of a line in a rule
		 * stanza.
		 */
		if ((cc = get_fmt_line(rule_fp, htx_sp, buf, new_rule_flag,
					rule_file_lineno)))
		   {
			/*
			 * we hit the end of a rule stanza, or EOF
			 */
			if (cc < 0)
			{
				at_end_of_stanza = 1;
			}
			else
			    {
				/*
				 * hit some kind of rule error
				 */
				return(1);
			    }
		   }
		else
		   {
			/*
			 * since we successfully checked the syntax
			 * of a line in the rule stanza, set our
			 * flag to zero so that we now know we
			 * are in the middle of a rule stanza.
			 */
			new_rule_flag = 0;

			/*
			 * go parse and validate the keyword and
			 * value from the line "buf" we read in.
			 */
			if ((cc = check_keyword_value(keyword, value,
				htx_sp, rule_file_lineno, rule_fp,
				&ruleid_count, &master_ptr,
				rule_def_table, buf)))
			   {
				return(1);
			   }
			if (ruleid_count != 1)
			   {
				/*
				 * read more than one RULE_ID keyword
				 * within rule stanza, so report error.
				 */
				rule_err("%p%d%d%p",htx_sp, RULE_NOBLANK_ERR,
					*rule_file_lineno, NULL);
				skip_to_end(rule_fp, rule_file_lineno);
				return(1);
			   }
			/*
			 * If bind_table is NULL, then getrule() is
			 * being invoked just to check format/syntax of
			 * rule file.  Else, we actually are processing
			 * the rule file, so assign value.
			 */
			if (bind_table != NULL)
			   {
				if ((cc = assign_value(rule_def_table,
					master_ptr, value,
					bind_table)))
				   {
					rule_err("%p%d%d%p",htx_sp, cc,
						*rule_file_lineno, NULL);
					skip_to_end(rule_fp, rule_file_lineno);
					return(1);
				   }
			   }
		   }
	   }

	/*
	 * now, the only way to make it down to this far, is if
	 * we don't have any rule file errors.  That is,
	 * get_fmt_line() will have returned a negative return code
	 * specifying that we have hit a blank line to terminate
	 * a rule stanza -OR- we have hit EOF.   We check our
	 * "new_rule_flag".  If it is set, then we were expecting a
	 * new rule to be read in, but got EOF before ANY keywords
	 * so we return -1.  If "new_rule_flag" is NOT set, then
	 * we have successfully read in a rule (terminated by either
	 * a blank line or EOF) so we return 0.
	 */
	if (new_rule_flag)
		return(-1);	/* no rule read, EOF */
	else
		return(0);	/* rule read, end of rule stanza */
   }
