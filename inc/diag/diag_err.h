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

/* This file defines the macros for the error codes */

#define DGX_KMALLOC_FAILED				0x00000001
#define DGX_HANDLE_NOT_FOUND				0x00000002
#define DGX_DEVICE_NOT_FOUND				0x00000003
#define DGX_INTR_HANDLER_NOT_FOUND  			0x00000004
#define DGX_INTR_HANDLER_NOT_INSTALLED			0x00000005
#define DGX_DMA_SETUP_FAILED				0x00000006
#define DGX_NO_DMA_DESC_FOUND				0x00000007
#define DGX_DMA_DESC_NOT_FOUND				0x00000008
#define DGX_GET_USER_FAILED				0x00000009
#define DGX_PUT_USER_FAILED				0x0000000a
#define DGX_DEVICE_NOT_AVAILABLE_FOR_DIAGNOSIS		0x0000000b
#define DGX_DEVICE_ALREADY_UNDER_DIAGNOSIS		0x0000000c
#define DGX_USER_ADDR_NOT_ALIGNED			0x0000000d
#define DGX_USER_BUF_SIZE_NOT_SUPPORTED			0x0000000e
#define DGX_ALLOC_KIOVEC_FAILED				0x0000000f
#define DGX_MAP_USER_KIOBUF_FAILED			0x00000010
#define DGX_LOCK_KIOVEC_FAILED				0x00000020
#define DGX_PCI_MAP_SINGLE_FAILED			0x00000030
#define DGX_GET_USER_PAGES_FAILED                       0x00000040
#define DGX_OK						0x00000000
