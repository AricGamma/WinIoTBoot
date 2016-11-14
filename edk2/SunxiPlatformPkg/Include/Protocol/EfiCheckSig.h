/** @file

  Copyright (c) 2007 - 2014, Allwinner Technology Co., Ltd. <www.allwinnertech.com>

  Martin.zheng <zhengjiewen@allwinnertech.com>
  
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __EFI_Check_Sig_H__
#define __EFI_Check_Sig_H__

#include <IndustryStandard/Usb.h>

// {E52500C3-4BF4-41A5-9692-6DF73DBFB9FE}
#define EFI_CHECKSIG_PROTOCOL_GUID \
  { 0xe52500c3, 0x4bf4, 0x41a5, { 0x96, 0x92, 0x6d, 0xf7, 0x3d, 0xbf, 0xb9, 0xfe } }


#define EFI_CHECK_SIG_PROTOCOL_REVISION   0x00010001


typedef struct _EFI_CHECKSIG_PROTOCOL EFI_CHECKSIG_PROTOCOL;

 
typedef EFI_STATUS
(EFIAPI * EFI_CHECK_SIG_AND_HASH) (
  IN EFI_CHECKSIG_PROTOCOL *This,
  IN UINT8 *pbCatalogData,
  IN UINT32 cbCatalogData,
  IN UINT8 *pbHashTableData,
  IN UINT32 cbHashTableData
);

struct _EFI_CHECKSIG_PROTOCOL {
  UINT32 Revision;
  EFI_CHECK_SIG_AND_HASH EfiCheckSignatureAndHash;
};


extern EFI_GUID gEfiCheckSigProtocolGuid;

#endif
