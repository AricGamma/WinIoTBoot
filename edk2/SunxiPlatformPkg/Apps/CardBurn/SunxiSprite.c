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

#include "SunxiSprite.h"

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/DebugLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/SunxiFlashIo.h>
#include <Library/SunxiMbr.h>
#include <Library/SunxiPartitionLib.h>
#include <Interinc/sunxi_uefi.h>
#include <Library/SunxiBootInfoLib.h>
#include <Library/SunxiCheckLib.h>

static EFI_BLOCK_IO_PROTOCOL *BlockIo = NULL;
static SUNXI_FLASH_IO_PROTOCOL *FlashIo = NULL;

//interface for sprite
int sunxi_sprite_init(int stage)
{
  EFI_STATUS           Status;
  Status = FlashIo->SunxiFlashIoInit(FlashIo,stage);
  if(EFI_ERROR(Status))
  {
    return -1;
  }
  return 0;
}

int sunxi_sprite_read (uint start_block, uint nblock, void *buffer)
{
  EFI_STATUS           Status;
  Status = FlashIo->SunxiFlashIoRead(FlashIo,start_block,nblock, buffer);
  if(EFI_ERROR(Status))
  {
    return 0;
  }
  return nblock;
}

int sunxi_sprite_write(uint start_block, uint nblock, void *buffer)
{
  EFI_STATUS           Status;
  Status = FlashIo->SunxiFlashIoWrite(FlashIo,start_block,nblock, buffer);
  if(EFI_ERROR(Status))
  {
    return 0;
  }
  return nblock;
}

int sunxi_sprite_erase(int erase, void *mbr_buffer)
{
  EFI_STATUS           Status;
  Status = FlashIo->SunxiFlashIoErase(FlashIo,erase, mbr_buffer);
  if(EFI_ERROR(Status))
  {
    return -1;
  }
  return 0;
}

int sunxi_sprite_exit(int force)
{
  EFI_STATUS           Status;
  Status = FlashIo->SunxiFlashIoExit(FlashIo,force);
  if(EFI_ERROR(Status))
  {
    return -1;
  }
  return 0;
}


int sunxi_sprite_download_mbr(void *buffer, uint buffer_size)
{
  EFI_STATUS           Status;

  if(buffer_size != (SUNXI_MBR_SIZE * SUNXI_MBR_COPY_NUM))
  {
    AsciiPrint("the mbr size is bad\n");
    return -1;
  }

  Status = FlashIo->SunxiFlashIoWrite(FlashIo,0,buffer_size/512, buffer);
  if(EFI_ERROR(Status))
  {
    return -1;
  }
  if(SunxiBurnStorageEMMC == SunxiGetBurnStorage())
  {
    Status = FlashIo->SunxiFlashIoWriteBootFiles(FlashIo,SunxiBootFileMbr,buffer, buffer_size);
    if(EFI_ERROR(Status))
    {
      return -1;
    }
  }
  return 0;
}

int sunxi_sprite_download_uboot(void *buffer)
{
  EFI_STATUS           Status;
  struct spare_boot_head_t    *Uefi  = (struct spare_boot_head_t *)buffer;

  if(AsciiStrnCmp((const char *)Uefi->boot_head.magic, UEFI_MAGIC, MAGIC_SIZE))
  {
    AsciiPrint("sunxi sprite: uefi magic is error\n");
    return -1;
  }
  
  if(SunxiSpriteVerifyChecksum(buffer, Uefi->boot_head.length, Uefi->boot_head.check_sum))
  {
    AsciiPrint("sunxi sprite: uefi checksum is error\n");
    return -1;
  }
  Status = FlashIo->SunxiFlashIoWriteBootFiles(FlashIo,SunxiBootFileUefi,buffer, Uefi->boot_head.length);
  if(EFI_ERROR(Status))
  {
    return -1;
  }
  return 0;

}

int sunxi_sprite_download_boot0(void *buffer)
{
  EFI_STATUS           Status;
  boot0_file_head_t    *boot0  = (boot0_file_head_t *)buffer;

  if(AsciiStrnCmp((const char *)boot0->boot_head.magic, BOOT0_MAGIC, MAGIC_SIZE))
  {
    AsciiPrint("sunxi sprite: boot0 magic is error\n");
    return -1;
  }

  if(SunxiSpriteVerifyChecksum(buffer, boot0->boot_head.length, boot0->boot_head.check_sum))
  {
    AsciiPrint("sunxi sprite: boot0 checksum is error\n");
    return -1;
  }
  Status = FlashIo->SunxiFlashIoWriteBootFiles(FlashIo,SunxiBootFileBoot0,buffer, boot0->boot_head.length);
  if(EFI_ERROR(Status))
  {
    return -1;
  }
  return 0;
}


//interface for storage card0 
int sunxi_flash_read (uint start_block, uint nblock, void *buffer)
{
  //debug("sunxi flash read : start %d, sector %d\n", start_block, nblock);
  //return sunxi_flash_read_pt(start_block, nblock, buffer);
  EFI_STATUS           Status;
  Status = BlockIo->ReadBlocks(BlockIo,BlockIo->Media->MediaId,start_block+SUNXI_MBR_OFFSET,nblock*512,buffer);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "ReadBlocks Failed %r\n",  Status));
    return 0;
  }
  return nblock;
}

int sunxi_flash_write(uint start_block, uint nblock, void *buffer)
{
  //debug("sunxi flash write : start %d, sector %d\n", start_block, nblock);
  //return sunxi_flash_write_pt(start_block, nblock, buffer);
  EFI_STATUS           Status;
  Status = BlockIo->WriteBlocks(BlockIo,BlockIo->Media->MediaId,start_block+SUNXI_MBR_OFFSET,nblock*512,buffer);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "WriteBlocks Failed %r\n",  Status));
    return 0;
  }
  return nblock;
}

uint sunxi_flash_size(void)
{
  return (uint)(UINTN)(BlockIo->Media->LastBlock);
}

int SunxiSpriteInit(void)
{
  
  UINTN                NumHandles;
  EFI_HANDLE           *HandleBuffer = NULL;
  UINT32               TargetMediaId;
  UINT32               i=0,Flag = 0;
  EFI_STATUS           Status;
  CHAR8                MbrBuffer[SUNXI_MBR_SIZE];

    //get nandio protocol
  TargetMediaId = SIGNATURE_32('s','d','h','c');
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &NumHandles, &HandleBuffer); 
  if (EFI_ERROR (Status)) {
    AsciiPrint("Write ERROR \n"); 
    return -1;
  }  
    
  for(i=0;i<NumHandles;i++) {
    Status = gBS->HandleProtocol(HandleBuffer[i], &gEfiBlockIoProtocolGuid, (VOID **)&BlockIo);
    Print( L"BlockDevice:Device %d Media ID=0x%x\n", i, BlockIo->Media->MediaId);
    if(BlockIo->Media->MediaId == TargetMediaId)
    {
      Print(L"BlockIo Protocol for Card0 exist\n");
      Flag = 1;
      break;
    }
  }
  if(!Flag)
  {
    Print(L"Can't Find BlockIo Protocol for Card0\n");
    return -1;
  }
  //syspartion init
  if(!sunxi_flash_read(0, SUNXI_MBR_SIZE/512, MbrBuffer))
  {
    Print(L"read mbr failed\n");
    return -1;
  }
  Status = SunxiPartitionInitialize(MbrBuffer, SUNXI_MBR_SIZE);
  if(EFI_ERROR(Status))
  {
    Print(L"mbr init failed\n");
    return -1;
  }

  //get FlashIO protocol
  Status = gBS->LocateProtocol(&gSunxiFlashIoProtocolGuid, NULL, (VOID **)&FlashIo);
  if (EFI_ERROR (Status)) 
  {
    Print(L"Get FlashIO Fail\n");
    return -1;    
  }
    
  gBS->FreePool (HandleBuffer);
  return 0;
}
