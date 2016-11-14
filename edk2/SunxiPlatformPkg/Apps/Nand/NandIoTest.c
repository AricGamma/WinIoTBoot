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
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/DebugLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/LoadedImage.h>

extern EFI_GUID gEfiBlockIoProtocolGuid;


static int Make_Argv(char *s, int argvsz, char *argv[]);

EFI_RUNTIME_SERVICES              *RuntimeServices = NULL;
EFI_BOOT_SERVICES                 *BootServices = NULL;

static void DumpHex(CHAR8 *name, UINT8 *base, INT32 len)
{
  INT32 i;
  AsciiPrint("dump %a: \n", name);
  for (i=0; i<len; i++) {
    AsciiPrint("%02x ",base[i]);
    if((i+1)%16 == 0) {AsciiPrint("\n");} 
  }
  AsciiPrint("\n");
}

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
  EFI_STATUS           Status;
  UINTN                NumHandles;
  EFI_HANDLE           *HandleBuffer = NULL;
  UINT32               TargetMediaId;
  EFI_BLOCK_IO_PROTOCOL *BlockIo = NULL;
  
  UINTN Size = 512;
  UINT8 *DataBuffer = NULL;
  EFI_LBA Lba = 0;
  UINT32 i;
  UINT32 Flag = 0;
  
  CHAR8 TmpStr[512];
  CHAR8 *Argv[64];
  UINT32 Argc = 0;
  EFI_LOADED_IMAGE_PROTOCOL*   LoadedImage;
  
  RuntimeServices = SystemTable->RuntimeServices;
  BootServices    = SystemTable->BootServices;
  Status = BootServices->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &LoadedImage);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem(TmpStr,sizeof(TmpStr));
  UnicodeStrToAsciiStr((CHAR16*)LoadedImage->LoadOptions,TmpStr);
  Argc = Make_Argv(TmpStr,sizeof(Argv)/sizeof(Argv[0]),Argv);

  for( i = 0; i < Argc; i ++)
  {
    AsciiPrint("arg %d : %a\n",i, Argv[i]);
  }
  if(Argc != 4)
  {
    AsciiPrint("Usage: NandIoTest [read|write] address length\n");
    goto __END;
  }

  //get address 
  if(!AsciiStrnCmp(Argv[2],"0x",2) || !AsciiStrnCmp(Argv[2],"0X",2))
  {
    Lba = AsciiStrHexToUintn(Argv[2]);
  }
  else
  {
    Lba = AsciiStrDecimalToUintn(Argv[2]);
  }
  //get  data length
  if(!AsciiStrnCmp(Argv[3],"0x",2) || !AsciiStrnCmp(Argv[3],"0X",2))
  {
    Size = AsciiStrHexToUintn(Argv[3]);
  }
  else
  {
    Size = AsciiStrDecimalToUintn(Argv[3]);
  }
  if(Size == 0) {Size = 512;}

  //malloc buffer
  DataBuffer = AllocateZeroPool(Size);
  ASSERT(DataBuffer != NULL);
  
  //get nandio protocol
  TargetMediaId = SIGNATURE_32('n','a','n','d');
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &NumHandles, &HandleBuffer);   
    
  for(i=0;i<NumHandles;i++) {
    Status = gBS->HandleProtocol(HandleBuffer[i], &gEfiBlockIoProtocolGuid, (VOID **)&BlockIo);
    Print( L"BlockDevice:Device %d Media ID=0x%x\n", i, BlockIo->Media->MediaId);
    if(BlockIo->Media->MediaId == TargetMediaId)
    {
      Print(L"BlockIo Protocol for Nand exist\n");
      Flag = 1;
      break;
    }
  }
  
  if(!Flag)
  {
    Print(L"Can't Find BlockIo Protocol for Nand\n");
    goto __END;
  }
    
  //execute cmd
  AsciiPrint("cmd = %a addr = %d, len = %d\n",Argv[1], (UINTN)Lba, Size);
  if( 0 == AsciiStrCmp(Argv[1],"write"))
  {
    for(i = 0; i < Size; i++)
    {
      DataBuffer[i] = i%0xff ;
    }
    DumpHex("Write",DataBuffer,Size);
        
    Status = BlockIo->WriteBlocks(BlockIo,BlockIo->Media->MediaId,Lba,Size,DataBuffer);
    if (EFI_ERROR (Status)) {
      AsciiPrint("Write ERROR \n"); 
      goto __END;
    }

    Status = BlockIo->FlushBlocks(BlockIo);
    if (EFI_ERROR (Status)) {
      AsciiPrint("Flush Blocks ERROR \n"); 
      goto __END;
    }
  }
  else if ( 0 == AsciiStrCmp(Argv[1],"read"))
  {
    Status = BlockIo->ReadBlocks(BlockIo,BlockIo->Media->MediaId,Lba,Size,DataBuffer);
    if (EFI_ERROR (Status)) {
      AsciiPrint("Write ERROR \n"); 
      goto __END;
    }
    DumpHex("Read",DataBuffer,Size);
  }
  else
  {
    AsciiPrint("unknow cmd\n");
  }
  
__END:  
  if(DataBuffer)
    FreePool(DataBuffer);
  
  gBS->FreePool (HandleBuffer);
  return Status;
}


static int Make_Argv(CHAR8 *s, int argvsz, CHAR8 *argv[])
{
  int argc = 0;

  /* split into argv */
  while (argc < argvsz - 1) {

    /* skip any white space */
    while ((*s == ' ') || (*s == '\t'))
      ++s;

    if (*s == '\0') /* end of s, no more args */
      break;

    argv[argc++] = s; /* begin of argument string */
       

    /* find end of string */
    while (*s && (*s != ' ') && (*s != '\t'))
      ++s;

    if (*s == '\0')   /* end of s, no more args */
      break;

    *s++ = '\0';    /* terminate current arg   */
  }
  argv[argc] = NULL;

  return argc;
}

