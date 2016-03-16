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
/* @(#)70  1.3  src/htx/usr/lpp/htx/inc/cfgclibdef.h, htx_libcfgc, htxubuntu 10/19/03 23:39:14 */

/*
 *  cfgclibdef.h -- libcfgc.a Function Declarations
 *          
 *    This include file is the respository for all libcfgc.a Library
 *    function declarations.
 *
 */                                                          

#ifndef CFGCLIBDEF_H
#define CFGCLIBDEF_H

#include <cfg04.h>

     
/*
 *  Function to close attribute file...
 */
int cfgcclsf PARMS((CFG__SFT *));
     
/*
 *  Function to copy keyword...
 */
int cfgckwd PARMS((char *, char *));
     
/*
 *  Function to merge default stanza...
 */
int cfgcmrgdflt PARMS((char *, CFG__SFT *, int));
     
/*
 *  Function to get keyword and value from a line...
 */
int cfgcprsln PARMS((char *, char *, char *));
     
/*
 *  Function to read a stanza...
 */
int cfgcrdsz PARMS((CFG__SFT *, char *, int, char *));
     
/*
 *  Function to search for a stanza name to define a start of the
 *  stanza...
 */
int cfgcread PARMS((CFG__SFT *, char *, int));
     
/*
 *  Function to scan for keyword and return value...
 */
int cfgcskwd PARMS((char *, char *, char *));
     
/*
 *  Function to process default stanza...
 */
int cfgcsvdflt PARMS((CFG__SFT *, char *));
     
/*
 *  Function to remove current default information from stanza...
 */
int cfgcunmrgdft PARMS((CFG__SFT *, char *));
     
#endif  /* CFGCLIBDEF_H */


