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
static char sccsid[] = "@(#)76	1.1  src/htx/usr/lpp/htx/lib/htxmp64/hxfupdatemp_new.c, htx_libhtxmp, htxubuntu 9/22/10 07:32:59";

/*
 *   COMPONENT_NAME: HTX_LIBHTXMP
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *   OBJECT CODE ONLY SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "htx_local.h"
#include "hxihtx64.h"
#include "hxiconv.h"
#include "htxlibdef.h"

int hxfupdate_tunsafe(char call, struct htx_data *data)
     /* call -- call type: START, UPDATE, ERROR, or FINISH                  */
     /* data -- HTX Hardware Exerciser data structure                       */
{
  int rc;

  rc=hxfupdate(call, data);
  return (rc);
  
} /* hxfupdate_tunsafe() */
