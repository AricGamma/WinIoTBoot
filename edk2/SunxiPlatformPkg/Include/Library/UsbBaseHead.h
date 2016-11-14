/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  Jerry Wang <wangflord@allwinnertech.com>
*  
*  This program and the accompanying materials                          
*  are licensed and made available under the terms and conditions of the BSD License         
*  which accompanies this distribution.  The full text of the license may be found at        
*  http://opensource.org/licenses/bsd-license.php                                            
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
*
**/

#ifndef  __USB_BASE_H__
#define  __USB_BASE_H__

#define CBWCDBLENGTH  16
#define CBWSIGNATURE  (0x43425355)
#define CSWSIGNATURE  (0x53425355)

typedef struct umass_bbb_cbw_t UMASS_CBW_T;
typedef struct umass_bbb_csw_t UMASS_CSW_T;

#pragma pack(1)

/* Command Block Wrapper */
struct umass_bbb_cbw_t
{
  UINT32    dCBWSignature;
  UINT32    dCBWTag;
  UINT32    dCBWDataTransferLength;
  UINT8   bCBWFlags;
  UINT8   bCBWLUN;
  UINT8   bCDBLength;
  UINT8   CBWCDB[CBWCDBLENGTH];
};


/* Command Status Wrapper */
struct umass_bbb_csw_t
{
  UINT32    dCSWSignature;
  UINT32    dCSWTag;
  UINT32    dCSWDataResidue;
  UINT8   bCSWStatus;
};
#pragma pack()

#endif
