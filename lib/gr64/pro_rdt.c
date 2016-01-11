
/* @(#)75	1.3  src/htx/usr/lpp/htx/lib/gr64/pro_rdt.c, htx_libgr, htxubuntu 6/24/04 09:26:46 */

/*****************************************************************************

Function(s) for Processing Rule Definition Table

Set of functions for checking syntax and validating values of the
rule definition value list which describes the legal domains for
keyword values from the rule file.

*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "sevcodes.h"
#include "getrule.h"
#ifdef LIBC_23
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include "htx_exer.h"

#define RANGE_DELIM	'-'


int validate_value (struct htx_data *htx_sp,struct rule_def_struct *master_ptr,char *value,long lvalue);

/*****************************************************************************

next_arg

Function retrieves string of next value or value range from the
remainder of the value_list passed in through "list_remainder".
Result is placed in "arg."

IF successful
THEN RETURNs ptr to remainder of list
ELSE RETURNs NULL

*****************************************************************************/

char *next_arg (char *list_remainder,char *arg)
   {
	static char *cp;
	char *cp2;
	char *htx_strchr(const char *s ,int c);

	/*
	 * point to beginning of remainder of list and
	 * then scan past any spaces or value separators (commas).
	 */
	cp = list_remainder;
	while (isspace(*cp) || (*cp == ','))
		cp++;
	
	/*
	 * if hit NULL, then at at end of value list.
	 */
	if (*cp == '\0')
		return(NULL);

	/*
	 * at a non-blank, non-comma, so copy EVERYTHING
	 * until hit another comma or the end of the list (null char).
	 */
	cp2 = arg;
	while ( (*cp != ',') && (*cp != '\0') )
		*cp2++ = *cp++;
	*cp2 = '\0';

	/*
	 * return the remainder of the value list.
	 */
	return(cp);
   }

/*****************************************************************************

chk_a_value

Function verifies length (if char) and correct data type of argument
"value" for the keyword definition pointed by "rdt_sp".  Numerical
conversions are performed on doubles and longs and returned in the
supplied double pointer, "double_p".  Any errors incurred are reported
to HTX via htx_err through the "htx_sp" pointer.

IF successful
THEN RETURNs 0
ELSE RETURNs 1

*****************************************************************************/

int chk_a_value (struct htx_data *htx_sp,struct rule_def_struct *rdt_sp,char *value,long *long_p)
   {
#if 0
/*
 * commented out temporarily due
 * to problems with floating point on DD2 machines
 */
	extern int check_double();
	double double_value;
#endif
	extern int check_long(char *value,long *long_ptr);
	long long_value;


	switch(rdt_sp->data_type)
	   {
		case RSTRING_TYPE:
				/*
				 * insure that pgmr. did not include
				 * the unary operator (also range
				 * separator) as apart of the
				 * string.
				 */
				 if (*value == RANGE_DELIM)
				    {
			htx_err(htx_sp, NO_ERRNO, SOFTERR,
"Illegal string <%s> in rule_def_table\nfor keyword %s.  Hyphen not allowed.",
	value, rdt_sp->keyword, rdt_sp->len);
					return(1);
				    }
				/*
				 * check for legal length.
				 */
				if (strlen(value) > rdt_sp->len)
				   {
			htx_err(htx_sp, NO_ERRNO, SOFTERR,
"Value string <%s> too long in rule_def_table\nfor keyword %s (max = %d).",
	value, rdt_sp->keyword, rdt_sp->len);
					return(1);
				   }
				break;

		case RLONG_TYPE:
				if (check_long(value, &long_value))
				   {
			htx_err(htx_sp, NO_ERRNO, SOFTERR,
	"Illegal long <%s> in rule_def_table for keyword %s.",
	value,rdt_sp->keyword);
					return(1);
				   }
				*long_p = (double) long_value;
				break;

#if 0 
/*
 * commented out temporarily due
 * to problems with floating point on DD2 machines
 */
		case RDOUBLE_TYPE:
				if (check_double(value, &double_value))
				   {
			htx_err(htx_sp, NO_ERRNO, SOFTERR,
	"Illegal double <%s> in rule_def_table for keyword %s.",
	value,rdt_sp->keyword);
					return(1);
				   }
				*double_p = double_value;
				break;
#endif

		default:
			htx_err(htx_sp, NO_ERRNO, SOFTERR,
	"Illegal data_type %d in chk_a_value for keyword %s.",
	rdt_sp->data_type, rdt_sp->keyword);
			return(-1);
	   }
	return(0);
   }
/*****************************************************************************

chk_arg

Function parses out value string as specified in "arg" attempting
to pull out the single value or value range and then calls
chk_a_value() to validate that the value is legal.  If the pointers
"v1", "v2", "d1", and "d2" are NOT NULL, then function will return
the string representations of the values in "v1" and "v2" and the
numerical conversions (if type is long or double) in "d1" and "d2".

*****************************************************************************/

int chk_arg (struct htx_data *htx_sp,struct rule_def_struct *rdt_sp,char *arg,char *v1,char *v2,long *l1,long *l2)
   {
	char value1[MAX_KEYVAL_LEN+1];
	char value2[MAX_KEYVAL_LEN+1];
	char *cp, *cp2;
	register int hit_non_delim;
#if 0
/*
 * commented out temporarily due
 * to problems with floating point on DD2 machines
 */
	double num_val1, num_val2;
#endif
	long num_val1, num_val2;


	/*
	 * point to value or value range and copy it until we hit
	 * the end of the string or the range delimiter or space.
	 * Note that we use the variable flag "hit_non_delim"
	 * so we can first scan past any possible unary operators.
	 */
	cp = arg;
	cp2 = value1;
	hit_non_delim = 0;
	while ( !isspace(*cp) && (*cp != '\0') )
	   {
		if (*cp == RANGE_DELIM)
		   {
			if (hit_non_delim)
				break;
		   }
		else
			hit_non_delim = 1;

		*cp2++ = *cp++;
	   }
	*cp2 = '\0';


	/*
	 * now, go check that the value is of legal length (if type char)
	 * or legal data type conversion (if double or long).  If
	 * double or long, value should be returned in "num_val1".
	 */
	if (chk_a_value(htx_sp, rdt_sp, value1, &num_val1))
		return(1);
	
	/*
	 * if "v1" is not NULL, then function is being invoked not to
	 * just check syntax, but to actually retrieve the value,
	 * so copy it.
	 */
	if (v1 != NULL)
		strcpy(v1, value1);
	
	/*
	 * if "d1" is not NULL, then function is being invoked not to
	 * just check syntax, but to actually retrieve the value,
	 * so grab it if of type long or double, else zero it out.
	 */
	if (l1 != NULL)
	{
		if (rdt_sp->data_type == RLONG_TYPE)
			*l1 = num_val1;
		else
			*l1 = 0;
	}	

	/*
	 * now scan over any potentially trailing white space.
	 */
	while (isspace(*cp))
		cp++;
	
	/*
	 * assume just one arg (i.e., no range).  By setting "*v2" with
	 * a null char, the invoking function will know that "arg" was
	 * just one value rather than a range value unless it gets
	 * filled in later below.
	 */

	if (v2 != NULL)
		*v2 = '\0';

	/*
	 * we hit a null char so just one arg (i.e., not a range
	 * value) so just return.
	 */
	if (*cp == '\0')
		return(0);
	
	/*
	 * ok, we hit a non space and NOT the end of the string, so
	 * the next char had better be the range delimiter or
	 * it's a syntax error.
	 */
	if (*cp != RANGE_DELIM)
	   {
		htx_err(htx_sp, NO_ERRNO, SOFTERR,
	"Syntax error in rule_def_table after value <%s>\nfor keyword %s.",
			value1, rdt_sp->keyword);
		return(1);
	   }

	/*
	 * ok, char hit WAS the range delimiter so we should have another
	 * value after it.  Skip over any possible spaces until you get
	 * to it.
	 */
	cp++;
	while (isspace(*cp))
		cp++;
	
	/*
	 * hit end of string without ever getting the value to specify
	 * the end of the range, so syntax error.
	 */
	if (*cp == '\0')
	   {
		htx_err(htx_sp, NO_ERRNO, SOFTERR,
"Syntax error in rule_def_table after value <%s>\nfor keyword %s.\n%s",
			value1, rdt_sp->keyword,
"Missing range end value.");
		return(1);
	   }
	
	/*
	 * we should be pointing to the value that specifies the
	 * end of the range, so let's copy it in.  Note the
	 * use of the "hit_non_delim" variable.  We do this so we can
	 * skip over any multiple hyphens which may be legally representing
	 * a unary operator.  For example, the range "-10 - -1" is
	 * certainly legal, so we use "hit_non_delim" so we can scan 
	 * past all the leading unary hyphens.  However, if we do encounter
	 * a hyphen AFTER already hitting a non-hyphen, then we have a syntax
	 * error (e.g.,  "-10 - 4-3").
	 */
	cp2 = value2;
	hit_non_delim = 0;
	while ( !isspace(*cp) && (*cp != '\0') )
	   {
		if (*cp == RANGE_DELIM)
		   {
			if (hit_non_delim)
			   {
				*cp2 = '\0';
		htx_err(htx_sp, NO_ERRNO, SOFTERR,
"Syntax error in rule_def_table after value <%s>\nfor keyword %s.\n%s",
			value2, rdt_sp->keyword,
"Encountered range delimiter after second value.");
				return(1);
			   }
		   }
		else
			hit_non_delim = 1;

		*cp2++ = *cp++;
	   }
	*cp2 = '\0';

	/*
	 * scan past any trailing white space.  If you don't hit
	 * the end of the string or a comma, then we hit another
	 * type of char. which is a syntax error.
	 */
	while (isspace(*cp))
		cp++;
	
	if ((*cp != ',') && (*cp != '\0'))
	   {
		htx_err(htx_sp, NO_ERRNO, SOFTERR,
"Syntax error in rule_def_table after value <%s>\nfor keyword %s.",
			value2, rdt_sp->keyword);
		return(1);
	   }


	/*
	 * now, go check that the value is of legal length (if type char)
	 * or legal data type conversion (if double or long).  If
	 * double or long, value should be returned in "num_val2".
	 */
	if (chk_a_value(htx_sp, rdt_sp, value2, &num_val2))
		return(1);

	/*
	 * if "v2" is not NULL, then function is being invoked not to
	 * just check syntax, but to actually retrieve the value,
	 * so copy it.  Now, since "v2" has a value in it
	 * (we overwrote the \0 in position 0), the invoking function
	 * will know that a range was specified instead of just
	 * a single value.
	 */
	if (v2 != NULL)
		strcpy(v2, value2);
	
	if (l2 != NULL)
	/*
	 * if "d2" is not NULL, then function is being invoked not to
	 * just check syntax, but to actually retrieve the value,
	 * so grab it if of type long or double, else zero it out.
	 */
	{
		if (rdt_sp->data_type == RLONG_TYPE)
			*l2 = num_val2;
		else
			*l2 = 0;
	}
	/*
	 * now, since range was specified we need to compare the
	 * two values to insure that the lower_bound value is
	 * less than or equal to the upper bound, else error.
	 */
	if (rdt_sp->data_type == RSTRING_TYPE)
	   {
		if (strcmp(value1, value2) > 0)
		   {
			htx_err(htx_sp, NO_ERRNO, SOFTERR,
	"Illegal range in rule_def_table for keyword %s.\n%s > %s.",
	rdt_sp->keyword, value1, value2);
			return(1);
		   }
	   }
	else
	   {
		if (num_val1 > num_val2)
		   {
			htx_err(htx_sp, NO_ERRNO, SOFTERR,
	"Illegal range in rule_def_table for keyword %s.\n"
        "%d (string: %s) > %d (string: %s).",
	rdt_sp->keyword, num_val1, value1, num_val2, value2);

			return(1);
		   }
	   }
	
	return(0);
   }

/*****************************************************************************

chk_rdt_syntax

Function steps through entire rule definition table and checks
the syntax if each value_list for each keyword.  Reporting of
errors to HTX is accomplished through the "htx_sp" pointer.

IF successful (no errors)
THEN RETURNs 0
ELSE RETURNs 1

*****************************************************************************/

int chk_rdt_syntax (struct htx_data *htx_sp,struct rule_def_struct rule_def_table[])
   {
	struct rule_def_struct *rdt_sp;
	register char *vlp;
	char arg[2*MAX_KEYVAL_LEN+1];


	/*
	 * point to start of rule definition table and then
	 * loop through it.
	 */
	rdt_sp = rule_def_table;
	while (*rdt_sp->keyword != '\0')
	   {
		/*
		 * point to beginning of value_list for this keyword
		 * and then loop through it checking each value
		 * or value range.
		 */
		vlp = rdt_sp->value_list;
		vlp = next_arg(vlp, arg);
		while (vlp != NULL)
		   {
			/*
			 * we pass NULL pointers here since this
			 * function is being invoked at the beginning
			 * of the hardware exerciser to validate
			 * a legal rule_def_table, so we don't
			 * need any of the actual values, yet.
			 */
			if (chk_arg(htx_sp, rdt_sp, arg, NULL, NULL,
				NULL, NULL))
			   {
				return(1);
			   }
			vlp = next_arg(vlp, arg);
		   }
		rdt_sp++;
	   }
	return(0);
   }
/*****************************************************************************

validate_value

This function validates that the string passed in "value" (or the
value passed in "dvalue" in case type is long or double) meets
the requirements of the domain of legal values as specified by
the value list for the keyword pointed to by "master_ptr."
Any errors are reported to HTX via the "htx_sp" pointer.

IF successful
THEN RETURNs 0
ELSE RETURNs 1

*****************************************************************************/

int validate_value (struct htx_data *htx_sp,struct rule_def_struct *master_ptr,char *value,long lvalue)
   {
	register char *vlp;
	char arg[2*MAX_KEYVAL_LEN+1];
	char string_value1[MAX_KEYVAL_LEN+1];
	char string_value2[MAX_KEYVAL_LEN+1];
#if 0
/*
 * commented out temporarily due
 * to problems with floating point on DD2 machines
 */
	double double_value1, double_value2;
#endif
	long long_value1, long_value2;


	/*
	 * point to start of value_list for this keyword.  Then
	 * loop through each value or value range in the list
	 * to compare the value.  If equal or in the specified
	 * range, then return 0.
	 */
	vlp = master_ptr->value_list;
	vlp = next_arg(vlp, arg);
	while (vlp != NULL)
	   {
		if (chk_arg(htx_sp, master_ptr, arg,
			string_value1, string_value2,
			&long_value1, &long_value2))
		   {
			return(1);
		   }
		switch(master_ptr->data_type)
		   {
			case RSTRING_TYPE:
				if (*string_value2 == '\0')
				   {
					if (strcmp(string_value1,
							value) == 0)
						return(0);
				   }
				else
				   {
					if ((strcmp(string_value1,
							value) <= 0) &&
					    (strcmp(value,
							string_value2) <= 0))
						return(0);
				   }
					break;

			case RLONG_TYPE:
				if (*string_value2 == '\0')
				   {
					if ((long) long_value1 ==
						(long) lvalue)
						return(0);
				   }
				else
				   {
					if ( ((long) long_value1 <=
						(long) lvalue) &&
					     ((long) lvalue <=
						(long) long_value2) )
						return(0);
				   }
					break;

#if 0
/*
 * commented out temporarily due
 * to problems with floating point on DD2 machines
 */
			case RDOUBLE_TYPE:
				if (*string_value2 == '\0')
				   {
					if (double_value1 == dvalue)
						return(0);
				   }
				else
				   {
					if ((double_value1 <= dvalue) &&
					    (dvalue <= double_value2))
						return(0);
				   }
					break;
#endif

			default:
				htx_err(htx_sp, NO_ERRNO, SOFTERR,
	"Illegal data_type %d in validate_value for keyword %s.",
	master_ptr->data_type, master_ptr->keyword);
				return(-1);
		   }
		vlp = next_arg(vlp, arg);
	   }

	/*
	 * went all the way through the value_list that specified
	 * the legal values for the keyword, but did not find a
	 * match, so illegal value and returns 1.
	 */
	return(1);
   }
