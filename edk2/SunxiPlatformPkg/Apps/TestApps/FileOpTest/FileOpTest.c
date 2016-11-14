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
#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/SunxiFileLib.h>
#include <Protocol/LoadedImage.h>

static INT32 Make_Argv(CHAR16 *s, INT32 argvsz, CHAR16 *argv[])
{
  INT32 argc = 0;

  /* split into argv */
  while (argc < argvsz - 1) {
    /* skip any white space */
    while ((*s == L' ') || (*s == L'\t'))
      ++s;

    if (*s == L'\0')  /* end of s, no more args */
      break;

    argv[argc++] = s; /* begin of argument string */
       
    /* find end of string */
    while (*s && (*s != L' ') && (*s != L'\t'))
      ++s;

    if (*s == L'\0')    /* end of s, no more args */
      break;

    *s++ = L'\0';   /* terminate current arg   */
  }
  argv[argc] = NULL;

  return argc;
}


/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/

EFI_STATUS
EFIAPI
FileOpTestMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_LOADED_IMAGE_PROTOCOL*   LoadedImage;
  CHAR16 Para[128];
  CHAR16 *Argv[32];
  INT32 Argc = 0;
  CHAR16 FileName[64] = {0};
  CHAR8* FileBuffer = NULL;
  UINTN  FileSize;

  Status = gBS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &LoadedImage);
  if (EFI_ERROR (Status)) 
  {
    return Status;
  }
  ZeroMem(Para,sizeof(Para));
  if(LoadedImage->LoadOptionsSize != 0)
  {
    CopyMem(Para,LoadedImage->LoadOptions,LoadedImage->LoadOptionsSize);
    Print(L"%s\n",Para);
  }
  else
  {
    Print(L"Usage: FileOpTest FileName\n");
    return Status;
  }

  Argc = Make_Argv(Para,sizeof(Argv)/sizeof(Argv[0]),Argv);
  if(Argc != 3)
  {
    Print(L"Usage: FileOpTest FileName read|write\n");
    return Status;
  }
  StrCpy(FileName,Argv[1]);
  Print(L"FileName:%s\n", FileName);

  if(StrCmp(Argv[2],L"read") == 0)
  {
    Status = ReadFileFromAwFs(FileName,(VOID**) &FileBuffer, &FileSize);
    if(EFI_ERROR(Status))
    {
      Print(L"ReadFile Error: %r\n", Status);
    }
    else
    {
      Print(L"ReadFile Ok: size is %d, data0=0x%x\n", FileSize, *(UINT32*)(FileBuffer));
    }
  }
  else if(StrCmp(Argv[2],L"write") == 0)
  {
    UINT32 i = 0;
    FileBuffer = AllocateZeroPool(1024);
    FileSize = 1024;
    for(i = 0; i < FileSize; i++)
    {
      FileBuffer[i] = i%0xff;
    }
    Status = WriteFileToAwFs(FileName, FileBuffer, FileSize);
    if(EFI_ERROR(Status))
    {
      Print(L"Write File Error: %r\n", Status);
    }
    else
    {
      Print(L"Write File Ok: size %d, data is ordered num", FileSize);
    }
  }
  else 
  {
    Print(L"cmd %s is not support\n", Argv[2]);
  }

  if(FileBuffer != NULL)
  {
    FreePool(FileBuffer);
  }
  
  return Status;
}



