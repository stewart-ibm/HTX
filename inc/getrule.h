/* @(#)25	1.5  src/htx/usr/lpp/htx/inc/getrule.h, htx_libgr, htxubuntu 4/4/05 14:33:40 */

#ifndef GETRULE_H
#define GETRULE_H
/*****************************************************************************

Header Parse/Extract HTX Rule File
Module Name:  getrule.h

*****************************************************************************/
#include <linux/version.h>
#include "hxihtx.h"
#include <stdarg.h>

#define MAX_KEYWORD_LEN	         14
#define MAX_KEYVAL_LEN           78

#define MAX_RULE_ID_LEN           8

#define RSTRING_TYPE		  0
#define RLONG_TYPE		  1
#if 0
/*
 * commented out temporarily due
 * to problems with floating point on DD2 machines
 */
#define RDOUBLE_TYPE		  2
#endif

#define RULE_READ_ERR		  1
#define RULE_SYN_ERR		  2
#define RULE_NOID_ERR		  3
#define RULE_NOBLANK_ERR	  4
#define RULE_BADKW_ERR		  5
#define RULE_BADVAL_ERR		  6
#define RULE_RANGE_ERR		  7
#define RULE_TYPE_ERR		  8
#define RULE_MEM_ERR		  9
#define RULE_REDEF_ERR           10

union bind_union
   {
	char *cp;
	long *lp;
#if 0
/*
 * commented out temporarily due
 * to problems with floating point on DD2 machines
 */
	double *dp;
#endif
   };

struct rule_def_struct
   {
	char keyword[MAX_KEYWORD_LEN+1];
	int data_type;
	int len;
	char *value_list;
   };

extern int open_rf(struct htx_data *, struct rule_def_struct *,char *);
extern int close_rf(void);
extern int rewind_rf(void);

#endif
