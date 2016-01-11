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


