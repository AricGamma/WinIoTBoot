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

#include <Library/SunxiBootInfoLib.h>
#include <Library/SunxiCommonLib.h>
#include <Library/SunxiMbr.h>

#include <Interinc/sunxi_uefi.h>
#include <Library/SunxiCheckLib.h>
#include <Library/SunxiPartitionLib.h>
#include <Uefi/UefiGpt.h>


#define CARD_ERASE_BLOCK_BYTES    (8 * 1024 * 1024)
#define CARD_ERASE_BLOCK_SECTORS  (CARD_ERASE_BLOCK_BYTES/512)

#define SUNXI_GPT_ENTRY_MAX_NUM         128 
#define SUNXI_GPT_ENTRY_SIZE            128
#define SUNXI_GPT_ENTRY_LBA       2
#define SUNXI_GPT_FIRST_USEABLE_LBA     16
#define DRAM_PARA_STORE_ADDR      (0x41800000)
EFI_GUID SUNXI_EMMC_DISK_GUID = {0xAE420040,0x13DD,0x41F2,{0xAE,0x7F,0x0D,0xC3,0x58,0x54,0xC8,0xD7}};

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
      //printf("don't deal prat's length is 0 (%a) \n", mbr->array[i].name);
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
      //if(EFI_ERROR(This->SunxiFlashIoWrite(This,erase_head_addr,erase_head_sectors,erase_buffer)))
      if(EFI_ERROR(This->SunxiFlashIoWrite(This,erase_tail_addr,erase_tail_sectors,erase_buffer)))
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
  UINT32 BootStorage;
  boot0_file_head_t    *boot0  = (boot0_file_head_t *)Buffer;
  if(boot0->boot_head.length != BufferSize)
  {
    AsciiPrint("sunxi sprite: boot0 Length is error\n");
    return EFI_DEVICE_ERROR;
  }

  if(SunxiSpriteVerifyChecksum(Buffer, boot0->boot_head.length, boot0->boot_head.check_sum))
  {
    printf("sunxi sprite: boot0 checksum is error\n");
    return EFI_DEVICE_ERROR;
  }

  BootStorage = SunxiGetBurnStorage();

  /* Update Emmc configuration param */
  extern int mmc_write_info(int dev_num, void *buffer,u32 buffer_size);
  if(BootStorage == SunxiBurnStorageSDHC)
  {
    if (mmc_write_info(0,(void *)boot0->prvt_head.storage_data, STORAGE_BUFFER_SIZE))
    {
      printf("add sdmmc2 private info fail!\n");
      return EFI_DEVICE_ERROR;
    }
  }
  else if(BootStorage == SunxiBurnStorageEMMC)
  {
    if (mmc_write_info(2,(void *)boot0->prvt_head.storage_data, STORAGE_BUFFER_SIZE))
    {
      printf("add sdmmc2 private info fail!\n");
      return EFI_DEVICE_ERROR;
    }
  }

  /*update dram flag*/
  CopyMem((void *)&boot0->prvt_head.dram_para, (void *)DRAM_PARA_STORE_ADDR, 32 * 4);
  BootDramUpdateFlagSet(boot0->prvt_head.dram_para);
  DumpDramPara(boot0->prvt_head.dram_para,32);    

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
  UINT32 toc_flag = 0;
  UINT32 image_len = 0;
  struct spare_boot_head_t    *Uefi  = (struct spare_boot_head_t *)Buffer;
  toc1_head_info_t *toc1 = NULL;

  image_len = Uefi->boot_head.length;
  if(AsciiStrnCmp((const char *)Uefi->boot_head.magic, UEFI_MAGIC, MAGIC_SIZE))
  {
    AsciiPrint("sunxi sprite: uboot magic is error, try to chek toc magic\n");
    toc1 = (toc1_head_info_t *)Buffer;
    toc_flag = 1;
  }
  if(toc_flag == 1)
  {
    if(toc1->magic != TOC_MAIN_INFO_MAGIC)
    {
      AsciiPrint("sunxi sprite: toc magic is error\n");
      return EFI_DEVICE_ERROR;
    }
    image_len = toc1->valid_len;
  }

  #if 0
  if(Uefi->boot_head.length != BufferSize)
  {
    AsciiPrint("sunxi sprite: uefi Length is error\n");
    return EFI_DEVICE_ERROR;
  }
  #endif
  AsciiPrint("Uefi size = 0x%x\n", image_len);

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

/* Write GPT to flash */
EFI_STATUS eMMCWriteGpt(
  IN SUNXI_FLASH_IO_PROTOCOL *This,
  IN VOID  *Buffer,
  IN UINTN BufferSize
)
{
  MASTER_BOOT_RECORD   *ProtectiveMbr = NULL;
  EFI_PARTITION_TABLE_HEADER  *PrimaryHeader = NULL;
  EFI_PARTITION_TABLE_HEADER  *BackupHeader = NULL;
  EFI_PARTITION_ENTRY         *PartEntry = NULL;
  gpt_table_info_t            table_info;
  UINT32                      BlockSize;
//  UINT32            EntryLbaNum;
  EFI_LBA                     LastBlock;
  EFI_STATUS Status = EFI_SUCCESS;

  Print(L"Write GPT to emmc\n");
  BlockSize     = This->BlockIo->Media->BlockSize;
  LastBlock     = This->BlockIo->Media->LastBlock;

  ProtectiveMbr = AllocateZeroPool (BlockSize);
  if (ProtectiveMbr == NULL) {
    return EFI_NOT_FOUND;
  }

  /* Create Protective MBR */
  Status = CreateProtectiveMbr(ProtectiveMbr, LastBlock);
  if(Status){
    Print(L"Create Protective MBR Error\n");
    goto Done;
  }

  /* Write Protective MBR to Flash */
  Status = This->BlockIo->WriteBlocks(This->BlockIo,This->BlockIo->Media->MediaId,\
  0,1*This->BlockIo->Media->BlockSize,ProtectiveMbr);
  if(Status){
    Print(L"Write Protective MBR Error\n");
    goto Done;
  }

  /* Create GPT Table Header Partition Entry*/
  //PrimaryHeader = AllocateZeroPool (sizeof (EFI_PARTITION_TABLE_HEADER));
  PrimaryHeader = AllocateZeroPool (BlockSize);
  if (PrimaryHeader == NULL) {
    goto Done;
  }

  //EntryLbaNum = (SUNXI_GPT_ENTRY_MAX_NUM * SUNXI_GPT_ENTRY_SIZE)/BlockSize;
  table_info.MyLBA = PRIMARY_PART_HEADER_LBA;
  table_info.AlternateLBA = LastBlock;
  table_info.EntryNum = SUNXI_GPT_ENTRY_MAX_NUM;
  table_info.EntrySize = SUNXI_GPT_ENTRY_SIZE;
  table_info.EntryLBA = SUNXI_GPT_ENTRY_LBA;
  table_info.FirstLBA = SUNXI_GPT_ENTRY_LBA + (table_info.EntryNum * table_info.EntrySize / BlockSize);
  table_info.LastLBA = LastBlock - 1 - (table_info.EntryNum * table_info.EntrySize / BlockSize);
  CopyGuid(&table_info.Guid, &SUNXI_EMMC_DISK_GUID);

  PartEntry = AllocateZeroPool(table_info.EntryNum * table_info.EntrySize);
  if (PartEntry == NULL) {
    goto Done;
  }

  Status = CreateGptTableAndEntry(Buffer,table_info,PrimaryHeader,PartEntry);
  if(Status){
    Print(L"Create GPT table and Partition Entry Error\n");
    goto Done;
  }

  //BackupHeader = AllocateZeroPool (sizeof (EFI_PARTITION_TABLE_HEADER));
  BackupHeader = AllocateZeroPool (BlockSize);
  if (BackupHeader == NULL) {
    goto Done;
  }

  /* Create Backup Header Table and Partition Entry */
  Status = PrimaryConvertBackup(PrimaryHeader, BackupHeader);
  if(Status == FALSE)
  {
    Status = EFI_NOT_READY;
    goto Done;
  }

  /* Write Primary header table and Partition Entry */
  Status = This->BlockIo->WriteBlocks(This->BlockIo,This->BlockIo->Media->MediaId,\
  PrimaryHeader->MyLBA,1*This->BlockIo->Media->BlockSize,PrimaryHeader);
  if(Status){
    Print(L"Write Primary header table error\n");
    goto Done;
  }

  Status = This->BlockIo->WriteBlocks(This->BlockIo,This->BlockIo->Media->MediaId,\
  PrimaryHeader->PartitionEntryLBA,PrimaryHeader->NumberOfPartitionEntries*PrimaryHeader->SizeOfPartitionEntry,PartEntry);
  if(Status){
    Print(L"Write Primary Partition entry error\n");
    goto Done;
  }

  /* Write Backup header table and Partition Entry */
  Status = This->BlockIo->WriteBlocks(This->BlockIo,This->BlockIo->Media->MediaId,\
  BackupHeader->MyLBA,1*This->BlockIo->Media->BlockSize, BackupHeader);
  if(Status){
    Print(L"Write Backup header table Error\n");
    goto Done;
  }

  Status = This->BlockIo->WriteBlocks(This->BlockIo,This->BlockIo->Media->MediaId,\
  BackupHeader->PartitionEntryLBA,BackupHeader->NumberOfPartitionEntries*BackupHeader->SizeOfPartitionEntry,PartEntry);
  if(Status){
    Print(L"Write Backup Partition entry error\n");
    goto Done;
  }

 Done:
  if (ProtectiveMbr != NULL) {
    FreePool (ProtectiveMbr);
  }
  if (PrimaryHeader != NULL) {
    FreePool (PrimaryHeader);
  }
  if (BackupHeader != NULL) {
    FreePool (BackupHeader);
  }
  if (PartEntry != NULL) {
    FreePool (PartEntry);
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
      //Print(L"SunxiBootFileGpt :Not been implemented yet\n");
      Status = eMMCWriteGpt(This,Buffer,BufferSize);
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

