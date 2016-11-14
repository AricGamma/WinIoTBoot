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

#include <Protocol/EfiSampleWinPhoneIo.h>

#define USBFN_DEBUG_ENABLE 1

#if(USBFN_DEBUG_ENABLE)
#define USBFN_DEBUG(ARG...) DEBUG (( EFI_D_INFO,ARG))
#else
#define USBFN_DEBUG(ARG...)
#endif


EFI_STATUS
EFIAPI 
Initialize 
(
  IN EFI_SIMPLE_WINPHONE_IO_PROTOCOL    *This,
  IN UINTN                              ConnectionTimeout,
  IN UINTN                              ReadWriteTimeout
  )
{
  USBFN_DEBUG("%a:\n", __FUNCTION__);
  return EFI_SUCCESS;
}

 

EFI_STATUS
EFIAPI 
Read (
  IN EFI_SIMPLE_WINPHONE_IO_PROTOCOL    *This,
  IN UINTN                              NumberOfBytesToRead,
  IN OUT UINTN                          *NumberOfBytesRead,
  OUT VOID                              *Buffer
  )
{
  USBFN_DEBUG("%a:\n", __FUNCTION__);
  return EFI_SUCCESS;
}
 
 

EFI_STATUS
EFIAPI  
Write (
  IN EFI_SIMPLE_WINPHONE_IO_PROTOCOL    *This,
  IN UINTN                              NumberOfBytesToWrite,
  IN OUT UINTN                          *NumberOfBytesWritten,
  IN VOID                               *Buffer
  )
{
  USBFN_DEBUG("%a:\n", __FUNCTION__);
  return EFI_SUCCESS;   
}
 
 

EFI_STATUS 
EFIAPI GetMaxPacketSize( 
  IN EFI_SIMPLE_WINPHONE_IO_PROTOCOL    *This, 
  OUT UINTN                             *MaxPacketSize 
  )
{
  USBFN_DEBUG("%a:\n", __FUNCTION__);
  return EFI_SUCCESS;
}
 

STATIC EFI_SIMPLE_WINPHONE_IO_PROTOCOL SunxiEfiSampleWinPhoneIoProtocol = 
{
 .Revision                 = EFI_SIMPLE_WINPHONE_IO_PROTOCOL_REVISION,   
 .Initialize           = Initialize,
 .Read             = Read,
 .Reserved                 = NULL,
 .Write            = Write,
 .GetMaxPacketSize         = GetMaxPacketSize,
};


EFI_STATUS
EfiSampleWinPhoneIoDxeInitialize (
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
    )
{
  EFI_HANDLE      Handle = NULL;
  EFI_STATUS      Status;

  // Install the EfiUsbFnIo interface
  Status = gBS->InstallMultipleProtocolInterfaces(&Handle, &gEfiSimpleWinPhoneIoProtocolGuid, &SunxiEfiSampleWinPhoneIoProtocol, NULL);
  ASSERT_EFI_ERROR(Status);

  return Status;
}

