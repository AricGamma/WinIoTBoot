/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  tangmanliang <tangmanliang@allwinnertech.com>
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
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Guid/VariableFormat.h>

extern EFI_GUID gSunxiVariableGuid;

/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the image goes into a library that calls this
  function.


  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS            Status;
  /* For GetNextVariableName */
  UINTN     VariableNameBufferSize;
  UINTN     VariableNameSize;
  CHAR16        *VariableName;
  UINTN     VariableDataBufferSize;
  UINTN     VariableDataSize;
  VOID      *VariableData;
  EFI_GUID    VendorGuid;
  UINT32        VariableAttributes;
 
  VariableNameSize = sizeof (CHAR16);
  VariableName = AllocateZeroPool (VariableNameSize);
  VariableNameBufferSize = VariableNameSize;
  
  VariableDataSize = sizeof(CHAR8);
  VariableData = AllocateZeroPool (VariableNameSize);
  VariableDataBufferSize = VariableDataSize;
  
  VariableAttributes = 0;
  
  Print (L"GetNextVariableName EFI Variables:\n");
  
  /* Check GetNextVariableName */
  do {
    VariableNameSize = VariableNameBufferSize;
    Status = gRT->GetNextVariableName(
          &VariableNameSize,
          VariableName,
          &VendorGuid
          );
    
    if(Status == EFI_BUFFER_TOO_SMALL) {    
      VariableName = ReallocatePool (VariableNameBufferSize, VariableNameSize, VariableName);
      VariableNameBufferSize = VariableNameSize;
    
      Status = gRT->GetNextVariableName(
          &VariableNameSize,
          VariableName,
          &VendorGuid
          );
    }
    
    if(Status == EFI_NOT_FOUND) {
      Status = EFI_SUCCESS;
      break;
    }
    
    VariableDataSize = VariableDataBufferSize;
    ZeroMem(VariableData,VariableDataSize);
    Status = gRT->GetVariable(
        VariableName,
        &VendorGuid,
        &VariableAttributes,
        &VariableDataSize,
        VariableData
        );
    if(Status == EFI_BUFFER_TOO_SMALL) {   
      VariableData = ReallocatePool (VariableDataBufferSize, VariableDataSize, VariableData);
      VariableDataBufferSize = VariableDataSize;
      ZeroMem(VariableData,VariableDataSize);
      Status = gRT->GetVariable(
        VariableName,
        &VendorGuid,
        &VariableAttributes,
        &VariableDataSize,
        VariableData
        );
    }
    if(CompareGuid(&VendorGuid, &gSunxiVariableGuid))
    {
      Print (L"%.-36g %.-20s : %s\n", &VendorGuid, VariableName, (CHAR16*)VariableData);
    }  
  } while (Status == EFI_SUCCESS);
  
  if(VariableName != NULL) {
    FreePool (VariableName);
  }
  if(VariableData != NULL) {
    FreePool (VariableData);
  }

  return Status;
}
