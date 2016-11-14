/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  Martin.Zheng <martinzheng@allwinnertech.com>
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


#include <Uefi.h>

#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UncachedMemoryAllocationLib.h>

#include <Protocol/EfiCheckSig.h>

#define USBFN_DEBUG_ENABLE 1

#if(USBFN_DEBUG_ENABLE)
#define USBFN_DEBUG(ARG...) DEBUG (( EFI_D_INFO,ARG))
#else
#define USBFN_DEBUG(ARG...)
#endif


EFI_STATUS
EFIAPI
EfiCheckSignatureAndHash (
  IN EFI_CHECKSIG_PROTOCOL *This,
  IN UINT8 *pbCatalogData,
  IN UINT32 cbCatalogData,
  IN UINT8 *pbHashTableData,
  IN UINT32 cbHashTableData
)
{
  USBFN_DEBUG("%a:\n", __FUNCTION__);
  return EFI_SUCCESS;
}
  
STATIC EFI_CHECKSIG_PROTOCOL SunxiEfiCheckSigProtocol = 
{
 .Revision                 = EFI_CHECK_SIG_PROTOCOL_REVISION,   
 .EfiCheckSignatureAndHash = EfiCheckSignatureAndHash,

};


EFI_STATUS
EfiCheckSigDxeInitialize (
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
    )
{
  EFI_HANDLE      Handle = NULL;
  EFI_STATUS      Status;
  
  // Install the EfiCheckSig interface
  Status = gBS->InstallMultipleProtocolInterfaces(&Handle, &gEfiCheckSigProtocolGuid, &SunxiEfiCheckSigProtocol, NULL);
  ASSERT_EFI_ERROR(Status);
  
  return Status;
}

