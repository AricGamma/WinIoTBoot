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
#include <Library/SunxiCheckLib.h>
#include <Protocol/SunxiFlashIo.h>
#include <Library/SunxiMbr.h>
#include <Interinc/sunxi_uefi.h>
#include <IndustryStandard/Mbr.h>


extern EFI_GUID gEfiBlockIoProtocolGuid;


//static int Make_Argv(char *s, int argvsz, char *argv[]);

EFI_RUNTIME_SERVICES              *RuntimeServices = NULL;
EFI_BOOT_SERVICES                 *BootServices = NULL;

#if 1
static void DumpHex(CHAR8 *name, UINT8 *base, INT32 len)
{
  INT32 i;
  AsciiPrint("dump %a: \n", name);
  for (i=0; i<len; i++) {
    AsciiPrint("%02x ",base[i]);
    if((i+1)%16 == 0) {
      AsciiPrint("\n");
    }
  }
  AsciiPrint("\n");
}
#endif
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
  UINT32               TargetMediaIdSd, TargetMediaIdMmc;
  EFI_BLOCK_IO_PROTOCOL *BlockIo = NULL;  
  SUNXI_FLASH_IO_PROTOCOL* eMMCFlashIo = NULL;
  
  UINTN Size = 0x923400;
  UINT8 *WriteDataBuffer = NULL;
  UINT8 *ReadDataBuffer = NULL;
  //EFI_LBA Lba = 0x10010;
  EFI_LBA Length;
  UINT32 i;
  UINT32 Flag = 0;
  //UINT32 WriteCheckSum = 0;
  //UINT32 ReadCheckSum = 0;
  UINT32 MbrSector = 0;    
  //UINT32 GptSector =  1;
  //EFI_LBA LastBlock;
  UINT32                      BlockSize;

  EFI_PARTITION_TABLE_HEADER *PrimaryHeader=NULL;
  EFI_PARTITION_ENTRY         *PartEntry=NULL;
  EFI_PARTITION_ENTRY         *Entry;
  
  EFI_LOADED_IMAGE_PROTOCOL*   LoadedImage;

  UINT32 StartHi, StartLo, EndHi, EndLo, LenHi, LenLo;
  UINT8 name[72];
  UINT64 attr;
  UINT32 attrHi, attrLo;

  RuntimeServices = SystemTable->RuntimeServices;
  BootServices    = SystemTable->BootServices;
  Status = BootServices->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &LoadedImage);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->LocateProtocol (&gSunxiFlashIoProtocolGuid, NULL, (VOID **) &eMMCFlashIo);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //malloc buffer
  WriteDataBuffer = AllocateZeroPool(Size);
  ASSERT(WriteDataBuffer != NULL);
  
  ReadDataBuffer = AllocateZeroPool(Size);
  ASSERT(ReadDataBuffer != NULL);
    
  //get flashio protocol
  TargetMediaIdSd = SIGNATURE_32('e','m','m','c');
  TargetMediaIdMmc = SIGNATURE_32('s','d','h','c');
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &NumHandles, &HandleBuffer);
    
  for(i=0;i<NumHandles;i++) {
    Status = gBS->HandleProtocol(HandleBuffer[i], &gEfiBlockIoProtocolGuid, (VOID **)&BlockIo);
    Print( L"BlockDevice:Device %d Media ID=0x%x\n", i, BlockIo->Media->MediaId);
    if((BlockIo->Media->MediaId == TargetMediaIdSd)||(BlockIo->Media->MediaId == TargetMediaIdMmc))
    {
      Print(L"BlockIo Protocol for Flash exist\n");
      Flag = 1;
      break;
    }
  }

  if(!Flag)
  {
    Print(L"Can't Find BlockIo Protocol for Flash\n");
    goto __END;
  }

    //LastBlock = BlockIo->Media->LastBlock;
  BlockSize = BlockIo->Media->BlockSize;
    /* dump gpt info */

  Status = BlockIo->ReadBlocks(BlockIo,BlockIo->Media->MediaId,MbrSector,512,ReadDataBuffer);
  if (EFI_ERROR (Status)) {
    AsciiPrint("Read ERROR \n");
    goto __END;
  }
  DumpHex("MBR record",ReadDataBuffer,512);

  PrimaryHeader = AllocateZeroPool (BlockSize);
  if (PrimaryHeader == NULL) {
  DEBUG ((EFI_D_ERROR, "Allocate pool error\n"));
    goto __END;
  }

  Status = BlockIo->ReadBlocks(BlockIo,BlockIo->Media->MediaId,1,BlockSize,PrimaryHeader);
  if (EFI_ERROR (Status)) {
    AsciiPrint("Read ERROR \n");
    goto __END;
  }
  DumpHex("Primary GPT table",(UINT8 *)PrimaryHeader,BlockSize);

  PartEntry = AllocatePool (PrimaryHeader->NumberOfPartitionEntries * PrimaryHeader->SizeOfPartitionEntry);
  if (PartEntry == NULL) {
    DEBUG ((EFI_D_ERROR, "Allocate pool error\n"));
    goto __END;
  }

  Status = BlockIo->ReadBlocks (
                   BlockIo,
                   BlockIo->Media->MediaId,
                   PrimaryHeader->PartitionEntryLBA,
                   PrimaryHeader->NumberOfPartitionEntries * (PrimaryHeader->SizeOfPartitionEntry),
                   PartEntry);
  
  DumpHex("Partition table",(UINT8 *)PartEntry,BlockSize*2);

  ZeroMem(name,72);
  for(i = 0; i < 10; i++)
  {
    Entry = (EFI_PARTITION_ENTRY *) ((UINT8 *) PartEntry + i * PrimaryHeader->SizeOfPartitionEntry);
    AsciiPrint("Partition Num:%d\n", i);
    CopyMem(name,Entry->PartitionName,72);
    name[35] = '\0';
    AsciiPrint("Partition Name:%s\n", name);
    
    StartHi = (UINT32)((Entry->StartingLBA>>32) & 0xffffffff);
    StartLo = (UINT32)(Entry->StartingLBA & 0xffffffff);
    AsciiPrint("Partition Start: StartHi 0x%x, StartLo 0x%x\n", StartHi, StartLo);
    
    EndHi = (UINT32)((Entry->EndingLBA>>32) & 0xffffffff);
    EndLo = (UINT32)(Entry->EndingLBA & 0xffffffff);
    AsciiPrint("Partition End: EndHi 0x%x, EndLo 0x%x\n", EndHi, EndLo);

    Length = Entry->EndingLBA - Entry->StartingLBA + 1;
    LenHi = (UINT32)((Length>>32) & 0xffffffff);
    LenLo = (UINT32)(Length & 0xffffffff);
    AsciiPrint("Partition Length: LenHi 0x%x, LenLo 0x%x\n", LenHi, LenLo);

    attr = Entry->Attributes;
    attrHi = (UINT32)((attr>>32) & 0xffffffff);
    attrLo = (UINT32)(attr & 0xffffffff);
    AsciiPrint("Partition attribute: attrHi 0x%x, attrLo 0x%x\n", attrHi, attrLo);
  }
  

#if 0
  for(i = 0; i < Size; i++)
  {
    WriteDataBuffer[i] = i%0xff ;
  }
  WriteCheckSum += SunxiAddSum(WriteDataBuffer, Size);

  Status = BlockIo->WriteBlocks(BlockIo,BlockIo->Media->MediaId,Lba,Size,WriteDataBuffer);
  if (EFI_ERROR (Status)) {
    AsciiPrint("Write ERROR \n");
    goto __END;
  }

  Status = BlockIo->FlushBlocks(BlockIo);
  if (EFI_ERROR (Status)) {
    AsciiPrint("Flush Blocks ERROR \n");
    goto __END;
  }

  Status = BlockIo->ReadBlocks(BlockIo,BlockIo->Media->MediaId,Lba,Size,ReadDataBuffer);
  if (EFI_ERROR (Status)) {
    AsciiPrint("Write ERROR \n");
    goto __END;
  }
  //DumpHex("Read",ReadDataBuffer,Size);
  ReadCheckSum += SunxiAddSum(ReadDataBuffer, Size);
  AsciiPrint("Read Check Sum: 0x%x\n",ReadCheckSum);
#endif

__END:
  if(WriteDataBuffer)
     FreePool(WriteDataBuffer);

  if(ReadDataBuffer)
      FreePool(ReadDataBuffer);
  if(PrimaryHeader)
    FreePool(PrimaryHeader);
  if(PartEntry)
    FreePool(PartEntry);
    
  gBS->FreePool (HandleBuffer);
    return Status;
}

