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
/* @(#)42  1.6.4.2  src/htx/usr/lpp/htx/inc/htx_local.h, htx_libhtx, htxubuntu 1/7/16 04:26:05 */
#ifndef HTX_LOCAL_H
#define HTX_LOCAL_H


/*
 * htx_local.h -- HTX local include file
 */                                                                  

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <sys/types.h>
#include <errno.h>
#include <stddef.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#ifndef __HTX_LINUX__
	#include "htx_port.h"
#endif

#ifndef AIXVER2
#define PARMS(x) x              /* define for ANSI C */
#else
#define PARMS(x) ()             /* define for non-ANSI C */
#endif

/* next 5 names require no local changes */
typedef char tbool;

#if defined(__HTX_LINUX__) || defined(__OS400__)

	extern	size_t	htx_strlen(const char *string);
	extern	char	*htx_strcpy(char *dest, const char *src);
	extern	char	*htx_strncpy(char *dest, const char *src, size_t n);
	extern	char	*htx_strcat(char *dest, const char *src);
	extern	char	*htx_strchr(const char *s, int c);
	extern	int		htx_strcmp(const char *s1, const char *s2);
	extern	int		htx_strncmp(const char *s1, const char *s2, size_t n);
	extern	size_t	htx_strspn(const char *s, const char *accept);
	extern	size_t	htx_strcspn(const char *s, const char *reject);

#else

	#define htx_strcpy strcpy
	#define htx_strlen strlen
	#define htx_strcat strcat
	#define htx_strcmp strcmp
	#define htx_strncmp strncmp
	#define htx_strncpy strncpy
	#define htx_strchr  strchr
	#define htx_strspn  strspn
	#define htx_strcspn strcspn

#endif

#if defined(__HTX_LINUX__) || defined(__OS400__)
	#ifdef FALSE
		#undef FALSE
	#endif
	#define FALSE (tbool) 0     /* Boolean value */
#endif

#ifndef FOREVER
	#define FOREVER for (;;)
#endif  /* FOREVER */

#ifndef GOOD
	#define GOOD 0              /* short return value */
#endif  /* GOOD */

#ifndef NO
	#define NO (tbool) 0        /* Boolean value */
#endif  /* NO */

#if defined(__HTX_LINUX__) || defined(__OS400__)
	#ifdef TRUE
		#undef TRUE
	#endif
	#define TRUE (tbool) 1      /* Boolean value */
#endif

#ifndef YES
	#define YES (tbool) 1       /* Boolean value */
#endif  /* YES */

#define GETLN(s,n)  ((fgets(s,n,stdin)==NULL) ? EOF : htx_strlen(s))

#define ABS(x)      (((x) < 0) ? -(x) : (x))

/*
 * Need to undef MAX for Linux ...
 */
#ifdef	__HTX_LINUX__
	#ifdef	MAX
		#undef	MAX
	#endif	/* MAX */
	#ifdef	MIN
		#undef	MIN
	#endif	/* MIN */
#endif	/* __HTX_LINUX__ */

#define MAX(x,y)    (((x) < (y)) ? (y) : (x))
#define MIN(x,y)    (((x) < (y)) ? (x) : (y))

#define DIM(a)      (sizeof(a) / sizeof(a[0]))
#define IN_RANGE(n, lo, hi)  ((lo) <= (n) && (n) <= (hi))

#ifndef NDEBUG
	#define ASSERTS(cond,str) {if (!(cond)) fprintf(stderr, "Assertion '%s' failed.", str);}
#else
	#define ASSERTS(cond,str)
#endif

#define SWAP(a,b,t) ((t) = (a), (a) = (b), (b) = (t))
#define LOOPDN(r,n) for ((r) = (n)+1; --(r) > 0; )
#define STREQ(s,t) (htx_strcmp(s,t) == 0)
#define STRLT(s,t) (htx_strcmp(s,t) < 0)
#define STRGT(s,t) (htx_strcmp(s,t) > 0)


#endif    /* HTX_LOCAL_H */

