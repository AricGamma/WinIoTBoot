/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  Jerry Wang <wangflord@allwinnertech.com>
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


#ifndef __SYS_PARTITION_H
#define __SYS_PARTITION_H

#include <Library/SunxiMbr.h>
#include <Protocol/BlockIo.h>

extern int sunxi_partition_get_total_num(void);

extern int sunxi_partition_get_name(int index, char *buf);

extern unsigned int  sunxi_partition_get_offset(int part_index);

extern unsigned int sunxi_partition_get_size(int part_index);

extern unsigned int sunxi_partition_get_offset_byname(const char *part_name);

extern unsigned int sunxi_partition_get_size_byname(const char *part_name);

extern int sunxi_partition_get_info_byname(const char *part_name, unsigned int *part_offset, unsigned int *part_size);

extern void *sunxi_partition_fetch_mbr(void);

extern int sunxi_partition_refresh(void *buf, unsigned int bytes);

extern int sunxi_partition_init(void* MbrBuff, unsigned int MbrLen);
//extern int sunxi_partition_init(EFI_BLOCK_IO_PROTOCOL* pBlockIo);

extern void SunxiGetPartitionInfo(int storage_type, char *cmdline_part_info);


extern EFI_STATUS SunxiPartitionInitialize(void* MbrBuff, UINT32 MbrLen);
extern EFI_STATUS SunxiPartitionGetTotalNumber(OUT UINT32* Number);
extern EFI_STATUS SunxipartitionGetName(IN UINT32 Index, OUT CHAR8 *Buffer);
extern EFI_STATUS SunxipartitionGetOffset(IN UINT32 Index,OUT UINT32* Offset);
extern EFI_STATUS SunxiPartitionGetSize(IN UINT32 Index,OUT UINT32* Size);
extern EFI_STATUS SunxiPartitionGetOffsetByName(const CHAR8 *PartName,OUT UINT32 *Offset);
extern EFI_STATUS SunxiPartitionGetSizeByName(IN CHAR8 *PartName,OUT UINT32* Size);
extern EFI_STATUS SunxiPartitionGetInfoByName(IN CHAR8 *PartName, OUT UINT32* Offset, OUT UINT32* Size);
extern void       SunxiDumpPartitionTable(void);

extern BOOLEAN PrimaryConvertBackup(
  IN EFI_PARTITION_TABLE_HEADER  *FirstHeader,
  IN EFI_PARTITION_TABLE_HEADER  *SecondHeader
);

extern EFI_STATUS CreateProtectiveMbr
(
  IN MASTER_BOOT_RECORD   *ProtectiveMbr,
  IN UINT32 LastBlock
);

extern EFI_STATUS  CreateGptTableAndEntry(
  IN VOID *Buffer, 
  IN gpt_table_info_t table_info,
  OUT EFI_PARTITION_TABLE_HEADER *PrimaryHeader,
  OUT EFI_PARTITION_ENTRY *PartEntry
);

extern EFI_STATUS SunxiGptPartitionInitialize
(
  IN EFI_BLOCK_IO_PROTOCOL*   BlockIo,
  IN void* GptBuff, 
  IN UINT32 GptLen
);

extern EFI_STATUS SunxiGptPartitionGetTotalNumber(OUT UINT32* Number);
extern EFI_STATUS SunxiGptPartitionGetName(IN UINT32 Index, OUT CHAR8 *Buffer);
extern EFI_STATUS SunxiGptPartitionGetOffset(IN UINT32 Index,OUT UINT32* Offset);
extern EFI_STATUS SunxiGptPartitionGetSize(IN UINT32 Index,OUT UINT32* Size);
extern EFI_STATUS SunxiGptPartitionGetOffsetByName(const CHAR8 *PartName,OUT UINT32 *Offset);
extern EFI_STATUS SunxiGptPartitionGetSizeByName(IN CHAR8 *PartName,OUT UINT32* Size);
extern EFI_STATUS SunxiGptPartitionGetInfoByName(IN CHAR8 *PartName, OUT UINT32* Offset, OUT UINT32* Size);
void SunxiGptGetPartitionInfo(IN UINT32 storage_type, IN CHAR8 *cmdline_part_info);

#endif //__SYS_PARTITION_H
