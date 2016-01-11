
/* @(#)76	1.4  src/htx/usr/lpp/htx/lib/gr64/rule_file.c, htx_libgr, htxubuntu 6/29/04 08:12:43 */

/*****************************************************************************

Function(s) HTX Rule File Processor

Contains main functions for invoking other functions responsible for
opening/reading/parsing/closing the rules file used by the hardware
exercisers.

*****************************************************************************/
#include <stdio.h>
#ifdef LIBC_23
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <stdlib.h>
#include "hxihtx.h"
#include "getrule.h"

#define ROPEN		0
#define RCLOSE		1
#define RREWIND		2
#define RREAD		3
#define RGETKV		4

#define RNOT_OPEN_ERR  -99
#define ROPEN_ERR      -98
#define RUNKNOWN_ERR   -97

/*****************************************************************************

bind_ptrs

Function mallocs an array of "bind_union" unions and then
assigns an address to a member of each union (based on data type
as derived from the "rule_def_table").  Each address is a pointer
passed in through the variable argument list in "var_stack_ptr".

*****************************************************************************/

int bind_ptrs (struct rule_def_struct *rule_def_table,union bind_union **bind_table,va_list var_stack_ptr)
   {
	register struct rule_def_struct *rds_p;
	register union bind_union *bu_p;
	long rule_count;

	/*
	 * point to start of rule definition table and then
	 * loop through to end so we can count the number
	 * of keywords defined.
	 */
	rds_p = rule_def_table;
	rule_count = 0;
	while (rds_p->keyword[0] != '\0')
	   {
		rule_count++;
		rds_p++;
	   }

	/*
	 * ok, allocate an array space big enough for a structure
	 * for each keyword entry in the rule definition table.
	 */
	*bind_table = (union bind_union *) calloc(rule_count, 
			sizeof(union bind_union));

	if (*bind_table == NULL)
		return(1);

	/*
	 * point back to the beginning of the rule definition table.
	 * Now, we're gonna loop through the table so that we
	 * can determine the data type so we know which member of
	 * the union gets the address passed in through the variable
	 * argument list of pointers pointed by "var_stack_ptr".
	 */
	rds_p = rule_def_table;
	bu_p = *bind_table;
	while (rds_p->keyword[0] != '\0')
	   {
		switch(rds_p->data_type)
		   {
			case RSTRING_TYPE:
					bu_p->cp = va_arg(var_stack_ptr,
								char *);
					break;
			case RLONG_TYPE:
					bu_p->lp = va_arg(var_stack_ptr,
								long *);
					break;
#if 0
/*
 * commented out temporarily due
 * to problems with floating point on DD2 machines
 */
			case RDOUBLE_TYPE:
					bu_p->dp = va_arg(var_stack_ptr,
								double *);
					break;
#endif
			default:
				free(*bind_table);
				return(1);
		   }
		rds_p++;
		bu_p++;
	   }
	return(0);
   }
/*****************************************************************************

set_default_var

Function assigns the first value in the value_list for each keyword
in the "rule_def_table" to the variables passed in
(pointed to by "bind_table").

IF successful
THEN RETURNs 0
ELSE RETURNs 1

*****************************************************************************/

int set_default_var (struct htx_data *htx_sp,struct rule_def_struct *rule_def_table,union bind_union *bind_table)
   {
	register struct rule_def_struct *rds_p;
	register union bind_union *bu_p;
	char arg[2*MAX_KEYVAL_LEN+1];
	char value[MAX_KEYVAL_LEN+1];
	char *vlp;
#if 0
/*
 * commented out temporarily due
 * to problems with floating point on DD2 machines
 */
	double double_value;
#endif
	long long_value;

	extern void strlencpy(char *,char *,int);
	extern int next_arg(char * , char *);
	extern int chk_arg(struct htx_data *htx_sp,struct rule_def_struct *rdt_sp,char *arg,char *v1,char *v2,long *l1,long *l2);


	/*
	 * point to start of rule definition table.  We are now
	 * going to loop through the rule definition table and
	 * step through the array of pointers to the variables
	 * which will hold the read-in keyword values.  We're doing
	 * this now to assign default values as the first item
	 * specified in the "value_list" for each keyword.
	 */
	rds_p = rule_def_table;
	bu_p = bind_table;
	while (rds_p->keyword[0] != '\0')
	   {
		/*
		 * grab the first value in the value list.  If the
		 * first item is a range specification, then just get
		 * the lower bound value of the range and assign it
		 * as the default value.
		 */
		vlp = rds_p->value_list;

		/*
		 * grabs the first value or value range in the
		 * value_list.
		 */
		if (next_arg(vlp, arg) == 0)
			return(1);
		
		/*
		 * verifies syntax and gets string value of first
		 * value in value_list into "value" and also into
		 * "double_value" in case of type long or double.
		 *
		 * NULLs are passed in place of a possible value
		 * representing an end range which we do not need.
		 */
		if (chk_arg(htx_sp, rds_p, arg, value, NULL,
				&long_value, NULL))
			return(1);


		switch(rds_p->data_type)
		   {
			case RSTRING_TYPE:
					strlencpy(bu_p->cp, value, rds_p->len);
					break;
			case RLONG_TYPE:
					*bu_p->lp =
						(long) long_value;
					break;
#if 0
/*
 * commented out temporarily due
 * to problems with floating point on DD2 machines
 */
				
			case RDOUBLE_TYPE:
					*bu_p->dp = double_value;
					break;
#endif
			default:
				return(1);
		   }
		rds_p++;
		bu_p++;
	   }
	return(0);
   }
/*****************************************************************************

rule_file

Function acts as main driver for the operations (open, close, rewind,
read, get) for the rules file.  As such, function keeps track of position
within rule file using a static FILE pointer ("rule_fp") and a static
int ("rule_file_lineno").

Since the operations requested invoke specific functions which require
different argument lists, rule_file() uses the variable-argument list
strategy in pulling off the args. needed for the specified operation.

IF successful
THEN RETURNs 0
ELSE RETURNs non-zero error code

*****************************************************************************/
#ifdef LIBC_23
int rule_file (char *var_stack_ptr, ...)
#else
int rule_file (va_alist)
va_dcl
#endif
   {
	va_list args;
#ifndef LIBC_23
	char *var_stack_ptr;
#endif

	int operation;
	static FILE *rule_fp = NULL;
	static int rule_file_lineno = 0;
	static int rc;
	extern int getrule(FILE *rule_fp,int *rule_file_lineno,struct htx_data *htx_sp,struct rule_def_struct rule_def_table[],union bind_union *bind_table);
	extern int chk_rdt_syntax(struct htx_data *htx_sp,struct rule_def_struct rule_def_table[]);

	char *filename;
	struct htx_data *htx_sp;
	struct rule_def_struct *rule_def_table;
	static union bind_union *bind_table;

#ifdef LIBC_23
	va_start(args, var_stack_ptr);
#else
	va_start(args);
#endif

	operation = va_arg(args, int);

	if ((operation != ROPEN) && (rule_fp == NULL))
	   {
		va_end(args);
		return(1);
	   }
	switch (operation)
	   {
		case ROPEN:	if (rule_fp != NULL)
				   {
					rc = ROPEN_ERR;
					break;
				   }
				rc = 0;
				htx_sp = va_arg(args, struct htx_data *);
				rule_def_table = va_arg(args, 
						struct rule_def_struct *);
				filename = va_arg(args, char *);
				if ((rule_fp = fopen(filename,"r")) == NULL)
				   {
					rc = 1;
					break;
				   }
				if (chk_rdt_syntax(htx_sp, rule_def_table))
				   {
					rc = 1;
					break;
				   }
				rule_file_lineno = 0;
				while ((rc = getrule(rule_fp,
					&rule_file_lineno, htx_sp,
					rule_def_table, NULL)) == 0);
				if (rc > 0)
				   {
					fclose(rule_fp);
					rule_fp = NULL;
				   }
				else
				   {
					rc = 0;
					rewind(rule_fp);
				   }
				rule_file_lineno = 0;
				break;

		case RCLOSE:	fclose(rule_fp);
				rule_fp = NULL;
				rc = 0;
				rule_file_lineno = 0;
				break;

		case RREWIND:	rewind(rule_fp);
				rule_file_lineno = 0;
				rc = 0;
				break;

		case RREAD:	htx_sp = va_arg(args, struct htx_data *);
				rule_def_table = va_arg(args, 
						struct rule_def_struct *);
				var_stack_ptr = va_arg(args, char *);
				if (bind_ptrs(rule_def_table, &bind_table,
						var_stack_ptr))
				   {
					rc = 1;
					break;
				   }
				set_default_var(htx_sp, rule_def_table,
						bind_table);
				rc = getrule(rule_fp, &rule_file_lineno,
					htx_sp, rule_def_table,
					bind_table);
				free(bind_table);
				bind_table = NULL;
				break;

		default:
				rc = RUNKNOWN_ERR;
				break;
	   }
	va_end(args);
	return(rc);
   }
/*****************************************************************************

open_rf

Function attempts to open rules file, "filename".  Note that
function "fails" if an attempt is made to open an already opened file.

IF successful
THEN RETURNs 0
ELSE RETURNs 1

*****************************************************************************/

int open_rf (struct htx_data *htx_sp,struct rule_def_struct rule_def_table[],char *filename)
   {

#ifdef LIBC_23
	if (rule_file("%d%p%p%p",ROPEN, htx_sp, rule_def_table, filename))
#else
	if (rule_file(ROPEN, htx_sp, rule_def_table, filename))
#endif
		return(1);

	return(0);
   }
/*****************************************************************************

close_rf

Function attempts to close the rules file.  If not presently open,
then returns an error.

IF successful
THEN RETURNs 0
ELSE RETURNs 1

*****************************************************************************/

int close_rf (void)
   {
#ifdef LIBC_23
	if (rule_file("%d",RCLOSE))
#else
	if (rule_file(RCLOSE))
#endif
		return(1);
	return(0);
   }
/*****************************************************************************

rewind_rf

Function attempts to rewind the rules file.  If not presently opened,
then returns an error.

IF successful
THEN RETURNs 0
ELSE RETURNs 1

*****************************************************************************/

int rewind_rf (void)
   {
#ifdef LIBC_23
	if (rule_file("%d",RREWIND))
#else
	if (rule_file(RREWIND))
#endif
		return(1);
	return(0);
   }
/*****************************************************************************

read_rf

Function attempts to read a rule stanza and build a list of all the
keywords and their associated values.  Note that the "htx_sp" (htx_data
struct) is passed in.  "Lower" functions use this var. to report
errors in the rules file to HTX.

IF successful
THEN RETURNs 0
ELSE IF EOF
     THEN RETURNs -1
     ELSE RETURNs  1.

*****************************************************************************/

#ifdef LIBC_23
int read_rf (char *fmt, ...)
#else
int read_rf (va_alist)
   va_dcl
#endif
   {
	va_list args;
	struct htx_data *htx_sp;
	struct rule_def_struct *rule_def_table;
	static int rc;

	rc = 0;
#ifdef LIBC_23
	htx_sp = (struct htx_data *)fmt;
	va_start(args, fmt);
#else
	va_start(args);

	htx_sp = va_arg(args, struct htx_data *);
#endif
	rule_def_table = va_arg(args, struct rule_def_struct *);

#ifdef LIBC_23
	rc = rule_file("%d%p%p%p",RREAD, htx_sp, rule_def_table, args);
#else
	rc = rule_file(RREAD, htx_sp, rule_def_table, args);
	
#endif
	va_end(args);
	return(rc);
   }
