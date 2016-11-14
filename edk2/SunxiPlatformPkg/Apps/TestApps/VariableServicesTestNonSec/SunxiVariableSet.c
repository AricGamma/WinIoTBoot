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
#include <Library/PrintLib.h>

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
  
  UINT32        i = 0;
  CHAR16        VariableValue[128];
  CHAR16        VariableKey[128];

  for (i = 0; i < 10; i++)
  {
    ZeroMem(VariableValue,sizeof(VariableValue));
    ZeroMem(VariableKey,sizeof(VariableKey));
    UnicodeSPrint(VariableKey,sizeof(VariableKey),L"SunxiVar%d", i);
    UnicodeSPrint(VariableValue,sizeof(VariableValue),L"This is Val%d", i);
    Status = gRT->SetVariable (
      VariableKey,
      &gSunxiVariableGuid,
      (i%2)? (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS) : (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS) ,
      StrLen(VariableValue)*sizeof(CHAR16) + sizeof(CHAR16), //add '\0' in the end
      VariableValue);
        
    if (EFI_ERROR(Status))
    {
      Print (L" EFI SetVariable Variables Error:%r\n",Status);
    }
  }
  
  return Status;
}
