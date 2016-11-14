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
#include <mmc_def.h>
#include <mmc_op.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/Cpu.h>
#include <Protocol/SunxiFlashIo.h>

#include <Library/DevicePathLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Library/SunxiMbr.h>

#include <IndustryStandard/Mbr.h>
#include <Interinc/sunxi_uefi.h>
#include <Library/SunxiCheckLib.h>


#define CARD_ERASE_BLOCK_BYTES    (8 * 1024 * 1024)
#define CARD_ERASE_BLOCK_SECTORS  (CARD_ERASE_BLOCK_BYTES/512)

EFI_STATUS eMMCFlashIoInit (
  IN SUNXI_FLASH_IO_PROTOCOL *This,
  IN UINTN   BootFlag
)
{
  return EFI_SUCCESS;
}

EFI_STATUS eMMCFlashIoExit(
  IN SUNXI_FLASH_IO_PROTOCOL *This,
  IN UINTN   ForceExit
)
{
  return EFI_SUCCESS;
}

EFI_STATUS eMMCFlashIoRead(  
  IN SUNXI_FLASH_IO_PROTOCOL *This,  
  IN EFI_LBA Lba,  
  IN UINTN   NumberBlocks,  
  IN VOID  *Buffer
)
{
  return This->BlockIo->ReadBlocks(This->BlockIo,This->BlockIo->Media->MediaId,\
    Lba+This->LogicOffSetBlock,NumberBlocks*This->BlockIo->Media->BlockSize,Buffer);
}

EFI_STATUS eMMCFlashIoWrite (  
  IN SUNXI_FLASH_IO_PROTOCOL *This,  
  IN EFI_LBA Lba,  
  IN UINTN   NumberBlocks, 
  IN VOID  *Buffer
)
{
  return This->BlockIo->WriteBlocks(This->BlockIo,This->BlockIo->Media->MediaId,\
    Lba+This->LogicOffSetBlock,NumberBlocks*This->BlockIo->Media->BlockSize,Buffer);
}

EFI_STATUS eMMCFlashIoFlush(  
  IN SUNXI_FLASH_IO_PROTOCOL *This
)
{
  return This->BlockIo->FlushBlocks(This->BlockIo);
}

EFI_STATUS eMMCFlashIoErase ( 
  IN SUNXI_FLASH_IO_PROTOCOL *This,  
  IN UINTN Erase,  
  IN VOID  *SunxiMbrBuffer
)

{
  CHAR8 *erase_buffer;
  sunxi_mbr_t *mbr = (sunxi_mbr_t *)SunxiMbrBuffer;
  UINT32 erase_head_sectors;
  UINT32 erase_head_addr;
  UINT32 erase_tail_sectors;
  UINT32 erase_tail_addr;
  int  i;

  //tick_printf("erase all part start\n");
  if(!Erase)
  {
    return EFI_SUCCESS;
  }
  erase_buffer = (char *)AllocateZeroPool(CARD_ERASE_BLOCK_BYTES);
  if(!erase_buffer)
  {
    Print(L"card erase fail: unable to malloc memory for card erase\n");
    return EFI_OUT_OF_RESOURCES;
  }

  if(EFI_ERROR(This->BlockIo->WriteBlocks(This->BlockIo,This->BlockIo->Media->MediaId,\
    BOOT0_SDMMC_START_ADDR,32 * 1024,erase_buffer)))
    {
      FreePool(erase_buffer);
      Print(L"erase boot0, size:32k, write 0x00 Fail\n");
      return EFI_DEVICE_ERROR;
    }
  
    if(0>SDMMC_BootFileWrite(BOOT0_SDMMC_START_ADDR,32 * 1024/This->BlockIo->Media->BlockSize,erase_buffer,2)){
    return EFI_DEVICE_ERROR;
  }
  Print(L"erase boot0, size:32k, write 0x00\n");

  if(EFI_ERROR(This->BlockIo->WriteBlocks(This->BlockIo,This->BlockIo->Media->MediaId,\
  UEFI_SDMMC_START_ADDR,1024 * 1024,erase_buffer)))
  {
    FreePool(erase_buffer);
    Print(L"erase UEFI, size:1024k, write 0x00 Fail\n");
    return EFI_DEVICE_ERROR;
  }

  if(0>SDMMC_BootFileWrite(UEFI_SDMMC_START_ADDR,1024 * 1024/This->BlockIo->Media->BlockSize,erase_buffer,2)){
    return EFI_DEVICE_ERROR;
  }
  Print(L"erase UEFI, size:1024k, write 0x00\n");

  for(i=1;i<mbr->PartCount;i++)
  {
    Print(L"erase %a part\n", mbr->array[i].name);
    if (mbr->array[i].lenlo > CARD_ERASE_BLOCK_SECTORS * 2)  // part > 16M
    {
      erase_head_sectors = CARD_ERASE_BLOCK_SECTORS;
      erase_head_addr = mbr->array[i].addrlo;
      //erase_tail_sectors = CARD_ERASE_BLOCK_SECTORS;
      erase_tail_sectors = 2 * 1024 * 1024 / 512;
      erase_tail_addr = mbr->array[i].addrlo + mbr->array[i].lenlo - CARD_ERASE_BLOCK_SECTORS;
    }
    else if (mbr->array[i].lenlo > CARD_ERASE_BLOCK_SECTORS) // 8M < part <= 16M
    {
      erase_head_sectors = CARD_ERASE_BLOCK_SECTORS;
      erase_head_addr = mbr->array[i].addrlo;
      //erase_tail_sectors = mbr->array[i].lenlo - CARD_ERASE_BLOCK_SECTORS;
      erase_tail_sectors = 2 * 1024 * 1024 / 512;
      erase_tail_addr = mbr->array[i].addrlo + mbr->array[i].lenlo - erase_tail_sectors;
    }
    else if (mbr->array[i].lenlo > 0)                                           // 0 < part <= 8M
    {
      erase_head_sectors = mbr->array[i].lenlo;
      erase_head_addr = mbr->array[i].addrlo;
      erase_tail_sectors = 0;
      erase_tail_addr = mbr->array[i].addrlo;
    }
    else {
      //printf("don't deal prat's length is 0 (%s) \n", mbr->array[i].name);
      //break;
      erase_head_sectors = CARD_ERASE_BLOCK_SECTORS;
      erase_head_addr = mbr->array[i].addrlo;
      erase_tail_sectors = 0;
      erase_tail_addr = mbr->array[i].addrlo; 
    }
    // erase head for partition
    if(EFI_ERROR(This->SunxiFlashIoWrite(This,erase_head_addr,erase_head_sectors,erase_buffer)))
    //if(!sunxi_sprite_write(erase_head_addr, erase_head_sectors, erase_buffer))
    {
      Print(L"card erase fail in erasing part %a\n", mbr->array[i].name);
      FreePool(erase_buffer);
      return EFI_DEVICE_ERROR;
    }
    Print(L"erase prat's head from sector 0x%x to 0x%x\n", erase_head_addr, erase_head_addr + erase_head_sectors);

    // erase tail for partition
    if (erase_tail_sectors) 
    {
      if(EFI_ERROR(This->SunxiFlashIoWrite(This,erase_head_addr,erase_head_sectors,erase_buffer)))
      //if(!sunxi_sprite_write(erase_tail_addr, erase_tail_sectors, erase_buffer))
      {
        Print(L"card erase fail in erasing part %a\n", mbr->array[i].name);
        FreePool(erase_buffer);
        return EFI_DEVICE_ERROR;
      }
      Print(L"erase part's tail from sector 0x%x to 0x%x\n", erase_tail_addr, erase_tail_addr + erase_tail_sectors);
    }
  }
  Print(L"card erase all\n");
  FreePool(erase_buffer);
  //tick_printf("erase all part end\n");
  return EFI_SUCCESS;

}

EFI_STATUS eMMCWriteBoot0(
  IN SUNXI_FLASH_IO_PROTOCOL *This,  
  IN VOID  *Buffer,  
  IN UINTN BufferSize
)
{
  boot0_file_head_t    *boot0  = (boot0_file_head_t *)Buffer;
  if(boot0->boot_head.length != BufferSize)
  {
    AsciiPrint("sunxi sprite: boot0 Length is error\n");
    return EFI_DEVICE_ERROR;
  }
    
  //get emmc info
  //EMMCGetInfo((void *)boot0->prvt_head.storage_data, STORAGE_BUFFER_SIZE);

  /* regenerate check sum */
  boot0->boot_head.check_sum = SunxiSpriteGenerateChecksum(Buffer, boot0->boot_head.length, boot0->boot_head.check_sum);

  if(SunxiSpriteVerifyChecksum(Buffer, boot0->boot_head.length, boot0->boot_head.check_sum))
  {
    AsciiPrint("sunxi sprite: boot0 checksum is error\n");
    return EFI_DEVICE_ERROR;
  }
  
  #ifdef USE_EMMC_BOOT_PART
  if((BufferSize % This->BlockIo->Media->BlockSize) != 0) {
    AsciiPrint("sunxi sprite: boot0 Length is not aligned to block size\n");
    return EFI_INVALID_PARAMETER;
  }
  if(0>SDMMC_BootFileWrite(BOOT0_SDMMC_START_ADDR,BufferSize/This->BlockIo->Media->BlockSize,Buffer,2))
  {
    return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;
  #else
  return This->BlockIo->WriteBlocks(This->BlockIo,This->BlockIo->Media->MediaId,\
  BOOT0_SDMMC_START_ADDR,BufferSize,Buffer);
  #endif
}

EFI_STATUS eMMCWriteUefi(
  IN SUNXI_FLASH_IO_PROTOCOL *This,  
  IN VOID  *Buffer,  
  IN UINTN BufferSize
)
{
  struct spare_boot_head_t    *Uefi  = (struct spare_boot_head_t *)Buffer;
  if(Uefi->boot_head.length != BufferSize)
  {
    AsciiPrint("sunxi sprite: uefi Length is error\n");
    return EFI_DEVICE_ERROR;
  }
  AsciiPrint("Uefi size = 0x%x\n", Uefi->boot_head.length);

  #ifdef USE_EMMC_BOOT_PART
  if((BufferSize % This->BlockIo->Media->BlockSize) != 0) {
    AsciiPrint("sunxi sprite: uefi Length is not aligned to block size\n");
    return EFI_INVALID_PARAMETER;
  }

  if(0>SDMMC_BootFileWrite(UEFI_SDMMC_START_ADDR,BufferSize/This->BlockIo->Media->BlockSize,Buffer,2)){
    return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;
  #else
  return This->BlockIo->WriteBlocks(This->BlockIo,This->BlockIo->Media->MediaId,\
  UEFI_SDMMC_START_ADDR,BufferSize,Buffer);
  #endif

}

#define GET_BYTE(DATA,BYTEINDEX) ((DATA>>(BYTEINDEX<<3))&0XFF)

EFI_STATUS eMMCWriteMbr(
  IN SUNXI_FLASH_IO_PROTOCOL *This,  
  IN VOID  *Buffer,  
  IN UINTN BufferSize
)
{
  MASTER_BOOT_RECORD   *StandardMbr;
  sunxi_mbr_t *mbr = (sunxi_mbr_t *)Buffer;
  UINT8        StandardMbrBuffer[512];
  UINTN        i;
  UINTN        Sectors,UnusedSectors,StartingLBA;
  EFI_STATUS Status = EFI_SUCCESS;
  
  Sectors = 0;
  
  for(i=1;i<mbr->PartCount-1;i++)
  {
    ZeroMem(StandardMbrBuffer,512);
    StandardMbr = (MASTER_BOOT_RECORD *)StandardMbrBuffer;

    Sectors += mbr->array[i].lenlo;

    StandardMbr->Partition[0].OSIndicator= 0x83;

    StartingLBA = (mbr->array[i].addrlo - i + 20 * 1024 * 1024/512 );
    StandardMbr->Partition[0].StartingLBA[0]  = GET_BYTE(StartingLBA,0);
    StandardMbr->Partition[0].StartingLBA[1]  = GET_BYTE(StartingLBA,1);
    StandardMbr->Partition[0].StartingLBA[2]  = GET_BYTE(StartingLBA,2);
    StandardMbr->Partition[0].StartingLBA[3]  = GET_BYTE(StartingLBA,3);
    
    StartingLBA = mbr->array[i].lenlo;
    StandardMbr->Partition[0].SizeInLBA[0] = GET_BYTE(StartingLBA,0);
    StandardMbr->Partition[0].SizeInLBA[1] = GET_BYTE(StartingLBA,1);
    StandardMbr->Partition[0].SizeInLBA[2] = GET_BYTE(StartingLBA,2);
    StandardMbr->Partition[0].SizeInLBA[3] = GET_BYTE(StartingLBA,3);

    if(i != mbr->PartCount-2)
    {
      StandardMbr->Partition[1].OSIndicator      = 0x05;

      StartingLBA = i;
      StandardMbr->Partition[1].StartingLBA[0]  = GET_BYTE(StartingLBA,0);
      StandardMbr->Partition[1].StartingLBA[1]  = GET_BYTE(StartingLBA,1);
      StandardMbr->Partition[1].StartingLBA[2]  = GET_BYTE(StartingLBA,2);
      StandardMbr->Partition[1].StartingLBA[3]  = GET_BYTE(StartingLBA,3);

      StartingLBA = mbr->array[i].lenlo;
      StandardMbr->Partition[1].SizeInLBA[0] = GET_BYTE(StartingLBA,0);
      StandardMbr->Partition[1].SizeInLBA[1] = GET_BYTE(StartingLBA,1);
      StandardMbr->Partition[1].SizeInLBA[2] = GET_BYTE(StartingLBA,2);
      StandardMbr->Partition[1].SizeInLBA[3] = GET_BYTE(StartingLBA,3);
    }

    StandardMbr->Signature = MBR_SIGNATURE;

    Status = This->BlockIo->WriteBlocks(This->BlockIo,This->BlockIo->Media->MediaId,\
    i,1*This->BlockIo->Media->BlockSize,StandardMbrBuffer);
    if(Status){
      Print(L"Write Standard MBR Error\n");
      return Status;
    }
  }
  ZeroMem(StandardMbrBuffer, 512);
  StandardMbr = (MASTER_BOOT_RECORD *)StandardMbrBuffer;

  UnusedSectors = This->BlockIo->Media->LastBlock + 1 - 20 * 1024 * 1024/512 - Sectors;
  StandardMbr->Partition[0].BootIndicator = 0x80;
  StandardMbr->Partition[0].OSIndicator = 0x0B;
  
  StartingLBA = (mbr->array[mbr->PartCount-1].addrlo + 20 * 1024 * 1024/512 );
  StandardMbr->Partition[0].StartingLBA[0]  =  GET_BYTE(StartingLBA,0);
  StandardMbr->Partition[0].StartingLBA[1]  =  GET_BYTE(StartingLBA,1);
  StandardMbr->Partition[0].StartingLBA[2]  =  GET_BYTE(StartingLBA,2);
  StandardMbr->Partition[0].StartingLBA[3]  =  GET_BYTE(StartingLBA,3);
  StartingLBA = UnusedSectors;
  StandardMbr->Partition[0].SizeInLBA[0] = GET_BYTE(StartingLBA,0);
  StandardMbr->Partition[0].SizeInLBA[1] = GET_BYTE(StartingLBA,1);
  StandardMbr->Partition[0].SizeInLBA[2] = GET_BYTE(StartingLBA,2);
  StandardMbr->Partition[0].SizeInLBA[3] = GET_BYTE(StartingLBA,3);

  StandardMbr->Partition[1].OSIndicator = 0x06;
  StartingLBA = (mbr->array[0].addrlo + 20 * 1024 * 1024/512);
  StandardMbr->Partition[1].StartingLBA[0] = GET_BYTE(StartingLBA,0);
  StandardMbr->Partition[1].StartingLBA[1] = GET_BYTE(StartingLBA,1);
  StandardMbr->Partition[1].StartingLBA[2] = GET_BYTE(StartingLBA,2);
  StandardMbr->Partition[1].StartingLBA[3] = GET_BYTE(StartingLBA,3);
  StartingLBA = mbr->array[0].lenlo;
  StandardMbr->Partition[1].SizeInLBA[0] = GET_BYTE(StartingLBA,0);
  StandardMbr->Partition[1].SizeInLBA[1] = GET_BYTE(StartingLBA,1);
  StandardMbr->Partition[1].SizeInLBA[2] = GET_BYTE(StartingLBA,2);
  StandardMbr->Partition[1].SizeInLBA[3] = GET_BYTE(StartingLBA,3);


  StandardMbr->Partition[2].OSIndicator = 0x05;
  StandardMbr->Partition[2].StartingLBA[0]  = 1;
  StartingLBA  =Sectors;
  StandardMbr->Partition[2].SizeInLBA[0] = GET_BYTE(StartingLBA,0);
  StandardMbr->Partition[2].SizeInLBA[1] = GET_BYTE(StartingLBA,1);
  StandardMbr->Partition[2].SizeInLBA[2] = GET_BYTE(StartingLBA,2);
  StandardMbr->Partition[2].SizeInLBA[3] = GET_BYTE(StartingLBA,3);


  StandardMbr->Signature = MBR_SIGNATURE;
  Status = This->BlockIo->WriteBlocks(This->BlockIo,This->BlockIo->Media->MediaId,\
  0,1*This->BlockIo->Media->BlockSize,StandardMbrBuffer);
  if(Status){
    Print(L"Write Standard MBR Error\n");
    return Status;
  }

  return Status;

}


EFI_STATUS eMMCFlashIoWriteBootFiles(  
  IN SUNXI_FLASH_IO_PROTOCOL *This,  
  IN SUNXI_BOOT_FILES_ID BootFile, 
  IN VOID  *Buffer,  
  IN UINTN BufferSize
)

{
  EFI_STATUS Status = EFI_SUCCESS;
  
  switch(BootFile){
    case SunxiBootFileBoot0:
      Status = eMMCWriteBoot0(This,Buffer,BufferSize);
      break;
    case SunxiBootFileUefi:
      Status = eMMCWriteUefi(This,Buffer,BufferSize);
      break;
    case SunxiBootFileMbr:
      Status = eMMCWriteMbr(This,Buffer,BufferSize);
      break;
    case SunxiBootFileGpt:
      Print(L"SunxiBootFileGpt :Not been implemented yet\n");
      Status = EFI_UNSUPPORTED;
      break;
  }

  return Status;
}

EFI_STATUS
InstallMMCFlashIoProtocol(
  EFI_HANDLE* eMMCBlockIOHandle,
  EFI_BLOCK_IO_PROTOCOL* BlockIo
  )
{
  EFI_STATUS Status;
  
  SUNXI_FLASH_IO_PROTOCOL* eMMCFlashIo;

  eMMCFlashIo = (SUNXI_FLASH_IO_PROTOCOL*)AllocateZeroPool(sizeof(SUNXI_FLASH_IO_PROTOCOL));
  eMMCFlashIo->LogicOffSetBlock = MMC_LOGICAL_OFFSET;
  eMMCFlashIo->LogicPageSize = 64*1024;
  eMMCFlashIo->SunxiFlashIoInit =eMMCFlashIoInit;
  eMMCFlashIo->SunxiFlashIoExit =eMMCFlashIoExit;
  eMMCFlashIo->SunxiFlashIoRead = eMMCFlashIoRead;
  eMMCFlashIo->SunxiFlashIoWrite = eMMCFlashIoWrite;
  eMMCFlashIo->SunxiFlashIoFlush = eMMCFlashIoFlush;
  eMMCFlashIo->SunxiFlashIoErase = eMMCFlashIoErase;
  eMMCFlashIo->SunxiFlashIoWriteBootFiles = eMMCFlashIoWriteBootFiles;
  eMMCFlashIo->BlockIo  = BlockIo;

  Status = gBS->InstallProtocolInterface (
                eMMCBlockIOHandle,
                &gSunxiFlashIoProtocolGuid,
                EFI_NATIVE_INTERFACE,
                eMMCFlashIo
                );
  return Status;
}

