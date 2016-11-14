/** @file

  Copyright (c) 2007 - 2014, Allwinner Technology Co., Ltd. <www.allwinnertech.com>

  Martin.zheng <zhengjiewen@allwinnertech.com>
  
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __SUNXI_FLASH_IO_H__
#define __SUNXI_FLASH_IO_H__

#include <Protocol/BlockIo.h>

#define SUNXI_FLASH_IO_PROTOCOL_GUID \
  { 0x1984c6f2, 0x4a91, 0x4aa9, { 0x97, 0xdc, 0xf5, 0x2e, 0xef, 0xb0, 0x13, 0x70 } }


typedef enum _SUNXI_BOOT_FILES_ID
{
  SunxiBootFileBoot0 = 0,
  SunxiBootFileUefi,
  SunxiBootFileMbr,
  SunxiBootFileGpt
} SUNXI_BOOT_FILES_ID;


typedef struct _SUNXI_FLASH_IO_PROTOCOL SUNXI_FLASH_IO_PROTOCOL;

typedef EFI_STATUS
(EFIAPI * SUNXI_FLASH_IO_INIT) (
  IN SUNXI_FLASH_IO_PROTOCOL *This,
  IN UINTN   BootFlag
);

typedef EFI_STATUS
(EFIAPI * SUNXI_FLASH_IO_EXIT) (
  IN SUNXI_FLASH_IO_PROTOCOL *This,
  IN UINTN   ForceExit
);

typedef EFI_STATUS
(EFIAPI * SUNXI_FLASH_IO_READ) (
  IN SUNXI_FLASH_IO_PROTOCOL *This,
  IN EFI_LBA Lba,
  IN UINTN   NumberBlocks,
  IN VOID  *Buffer
);

typedef EFI_STATUS
(EFIAPI * SUNXI_FLASH_IO_WRITE) (
  IN SUNXI_FLASH_IO_PROTOCOL *This,
  IN EFI_LBA Lba,
  IN UINTN   NumberBlocks,
  IN VOID  *Buffer
);

typedef EFI_STATUS
(EFIAPI * SUNXI_FLASH_IO_FLUSH) (
  IN SUNXI_FLASH_IO_PROTOCOL *This
);

typedef EFI_STATUS
(EFIAPI * SUNXI_FLASH_IO_ERASE) (
  IN SUNXI_FLASH_IO_PROTOCOL *This,
  IN UINTN Erase,
  IN VOID  *MbrBuffer
);

typedef EFI_STATUS
(EFIAPI * SUNXI_FLASH_IO_WRITE_BOOT_FILES) (
  IN SUNXI_FLASH_IO_PROTOCOL *This,
  IN SUNXI_BOOT_FILES_ID BootFile,
  IN VOID  *Buffer,
  IN UINTN BufferSize
);


struct _SUNXI_FLASH_IO_PROTOCOL {
  UINT32 LogicOffSetBlock;
  UINT32 LogicPageSize;
  SUNXI_FLASH_IO_INIT      SunxiFlashIoInit;
  SUNXI_FLASH_IO_EXIT      SunxiFlashIoExit;
  SUNXI_FLASH_IO_READ      SunxiFlashIoRead;
  SUNXI_FLASH_IO_WRITE     SunxiFlashIoWrite;
  SUNXI_FLASH_IO_FLUSH     SunxiFlashIoFlush;
  SUNXI_FLASH_IO_ERASE     SunxiFlashIoErase;
  SUNXI_FLASH_IO_WRITE_BOOT_FILES SunxiFlashIoWriteBootFiles;
  EFI_BLOCK_IO_PROTOCOL*   BlockIo;
};


extern EFI_GUID gSunxiFlashIoProtocolGuid;

#endif
