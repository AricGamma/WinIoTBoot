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

#include <Library/NandLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/Cpu.h>
#include <Protocol/SunxiFlashIo.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/SunxiMbr.h>
#include <Library/SunxiCheckLib.h>
#include <Interinc/sunxi_uefi.h>

#include "NandExt.h"

#define debug(ARG...)  


EFI_STATUS NandFlashIoInit (
  IN SUNXI_FLASH_IO_PROTOCOL *This,
  IN UINTN   BootFlag
)
{
  if(Nand_Uefi_Init(BootFlag))
  {
    return EFI_DEVICE_ERROR;
  }
  if(This->BlockIo->Media->LastBlock== 0)
  {
    This->BlockIo->Media->LastBlock = (INT32)get_nftl_cap();
  }
  return EFI_SUCCESS;
}

EFI_STATUS NandFlashIoExit(
  IN SUNXI_FLASH_IO_PROTOCOL *This,
  IN UINTN   ForceExit
)
{
  if(Nand_Uefi_Exit(ForceExit))
  {
    return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;
}


EFI_STATUS NandFlashIoRead(  
  IN SUNXI_FLASH_IO_PROTOCOL *This,  
  IN EFI_LBA Lba,  
  IN UINTN   NumberBlocks,  
  IN VOID  *Buffer
)
{
  return This->BlockIo->ReadBlocks(This->BlockIo,This->BlockIo->Media->MediaId,\
  Lba+This->LogicOffSetBlock,NumberBlocks*This->BlockIo->Media->BlockSize,Buffer);
}

EFI_STATUS NandFlashIoWrite (  
  IN SUNXI_FLASH_IO_PROTOCOL *This,  
  IN EFI_LBA Lba,  
  IN UINTN   NumberBlocks, 
  IN VOID  *Buffer
)
{
  return This->BlockIo->WriteBlocks(This->BlockIo,This->BlockIo->Media->MediaId,\
  Lba+This->LogicOffSetBlock,NumberBlocks*This->BlockIo->Media->BlockSize,Buffer);

}

EFI_STATUS NandFlashIoFlush(  
  IN SUNXI_FLASH_IO_PROTOCOL *This
)
{
  return This->BlockIo->FlushBlocks(This->BlockIo);
}

EFI_STATUS NandFlashIoErase ( 
  IN SUNXI_FLASH_IO_PROTOCOL *This,  
  IN UINTN Erase,  
  IN VOID  *SunxiMbrBuffer
)

{
  Nand_Get_Mbr(SunxiMbrBuffer,16*1024);
  Nand_Uefi_Erase(Erase);
  return EFI_SUCCESS;
}
    
EFI_STATUS NandWriteBoot0(
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

  //get nand info
  Nand_Uefi_Get_Flash_Info((void *)boot0->prvt_head.storage_data, STORAGE_BUFFER_SIZE);

  /* regenerate check sum */
  boot0->boot_head.check_sum = SunxiSpriteGenerateChecksum(Buffer, boot0->boot_head.length, boot0->boot_head.check_sum);

  if(SunxiSpriteVerifyChecksum(Buffer, boot0->boot_head.length, boot0->boot_head.check_sum))
  {
    AsciiPrint("sunxi sprite: boot0 checksum is error\n");

    return EFI_DEVICE_ERROR;
  }
  if (0 != Nand_Download_Boot0(boot0->boot_head.length, Buffer))
  {
    return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;
    
}

EFI_STATUS NandWriteUefi(
  IN SUNXI_FLASH_IO_PROTOCOL *This,  
  IN VOID  *Buffer,  
  IN UINTN BufferSize
)
{
  struct spare_boot_head_t    *Uefi  = (struct spare_boot_head_t *)Buffer;

  if(Uefi->boot_head.length != BufferSize)
  {
    AsciiPrint("sunxi sprite: Uefi Length is error\n");
    return EFI_DEVICE_ERROR;
  }
  //get FLASH info
  Nand_Uefi_Get_Flash_Info((void *)Uefi->boot_data.nand_spare_data, STORAGE_BUFFER_SIZE);

  /* regenerate check sum */
  Uefi->boot_head.check_sum = SunxiSpriteGenerateChecksum(Buffer, Uefi->boot_head.length, Uefi->boot_head.check_sum);

    //Verify
  if(SunxiSpriteVerifyChecksum(Buffer, Uefi->boot_head.length, Uefi->boot_head.check_sum))
  {
    AsciiPrint("sunxi sprite: Uefi checksum is error\n");
    return EFI_DEVICE_ERROR;
  }

  AsciiPrint("Uefi size = 0x%x\n", Uefi->boot_head.length);
  if (0 != Nand_Download_Uefi(Uefi->boot_head.length, Buffer))
  {
    return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;

}


EFI_STATUS NandWriteMbr(
  IN SUNXI_FLASH_IO_PROTOCOL *This,  
  IN VOID  *Buffer,  
  IN UINTN BufferSize
)
{
  return EFI_SUCCESS;
}


EFI_STATUS NandFlashWriteBootFiles(  
  IN SUNXI_FLASH_IO_PROTOCOL *This,  
  IN SUNXI_BOOT_FILES_ID BootFile, 
  IN VOID  *Buffer,  
  IN UINTN BufferSize
)

{
  EFI_STATUS Status = EFI_SUCCESS;
  
  switch(BootFile){
    case SunxiBootFileBoot0:
      Status = NandWriteBoot0(This,Buffer,BufferSize);
      break;
    case SunxiBootFileUefi:
      Status = NandWriteUefi(This,Buffer,BufferSize);
      break;
    case SunxiBootFileMbr:
      Status = NandWriteMbr(This,Buffer,BufferSize);
      break;
    case SunxiBootFileGpt:
      Print(L"SunxiBootFileGpt :Not been implemented yet\n");
      Status = EFI_UNSUPPORTED;
      break;
  }

  return Status;
}

extern UINT32 NAND_GetLogicPageSize(void);

EFI_STATUS
InstallNandFlashIoProtocol(
  EFI_HANDLE* NandBlockIOHandle,
  EFI_BLOCK_IO_PROTOCOL* BlockIo
  )
{
  EFI_STATUS Status;
  
  SUNXI_FLASH_IO_PROTOCOL* NandFlashIo = NULL;

  NandFlashIo = (SUNXI_FLASH_IO_PROTOCOL*)AllocateZeroPool(sizeof(SUNXI_FLASH_IO_PROTOCOL));
  
  NandFlashIo->LogicOffSetBlock            = 0;
  NandFlashIo->LogicPageSize               = NAND_GetLogicPageSize();
  NandFlashIo->BlockIo                     = BlockIo;
  NandFlashIo->SunxiFlashIoInit            = NandFlashIoInit;
  NandFlashIo->SunxiFlashIoExit            = NandFlashIoExit;
  NandFlashIo->SunxiFlashIoRead            = NandFlashIoRead;
  NandFlashIo->SunxiFlashIoWrite           = NandFlashIoWrite;
  NandFlashIo->SunxiFlashIoFlush           = NandFlashIoFlush;
  NandFlashIo->SunxiFlashIoErase           = NandFlashIoErase;
  NandFlashIo->SunxiFlashIoWriteBootFiles  = NandFlashWriteBootFiles;
  Status = gBS->InstallProtocolInterface (
                NandBlockIOHandle,
                &gSunxiFlashIoProtocolGuid,
                EFI_NATIVE_INTERFACE,
                NandFlashIo
                );
  return Status;
}

