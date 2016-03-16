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
/* @(#)69  1.3  src/htx/usr/lpp/htx/inc/cfg04.h, htx_libcfgc, htxubuntu 10/19/03 23:39:11 */
/* Component = htx_libcfgc_lx */

#ifndef _H_CFG04
#define _H_CFG04


/*  (c) Copyright IBM Corp. 1985  */
/*  Licensed Material - Program Property of IBM */

/*      Return codes for common routines */
#define CFG_SUCC  0            /* successful termination */
#define CFG_EOF   4004         /* end of file encountered */
#define CFG_EXBF -4013         /* input exceeds buffer size */
#define CFG_SZNF -4015         /* stanza not found */
#define CFG_SZBF -4016         /* stanza size exceeds buffer size */
#define CFG_UNIO -4017         /* unrecoverable I/O error */

#define CFG_SPCE -4028         /* malloc failed -- out of space */
#define CFG_FRMT -4029         /* attribute file format corrupted */
#define CFG_EOPN -4030         /* error opening file */
#define CFG_ECLS -4032         /* error closing file */
#define CFG_NSCSI -4040        /* attempt to add duplicate SCSI device   */

/* declarations for standard lengths */

#define MAXVAL   128    /* maximum size for keyword value */
#define MAXKWD   75     /* maximum keyword length */
#define MAXDDISZ 4096   /* maximum size for DDI stanza */
#define MAXATRSZ 2048   /* maximum size for attribute file stanza */
#define MAXDIR   100    /* maximum directory path length */

/*
 *  Structure for character level access to attribute files
 */

typedef struct {
	FILE *sfile;       /* file pointer returned from fopen */
    	char *defbuf;      /* pointer to buffer containing default stanza */
     	short *defmap;     /* pointer to array of line indexes in default */
    	char spath[MAXDIR];/* filename for re-opens */
} CFG__SFT;

CFG__SFT *cfgcopsf(char *);  /* open routine returns structure pointer */



/*      Macro for rewinding attribute file (handles default info)     
#define cfgcrwnd(sfptr) { \
	if (sfptr->defbuf != NULL)  {	\
		*sfptr->defbuf = '\0';	\
		rewind(sfptr->sfile);	\
	}				\
}*/

#define cfgcrwnd(sfptr) { \
	if (sfptr->defbuf != NULL)  {	\
		*sfptr->defbuf = '\0';	\
	}				\
	rewind(sfptr->sfile);	\
}

#endif /* _H_CFG04 */


