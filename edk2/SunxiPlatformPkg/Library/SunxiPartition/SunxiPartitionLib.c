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




//#include "Crc32.h"
#include <Uefi.h>
#include <Library/BaseLib.h> 
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/SunxiMbr.h>
#include <Library/SunxiCheckLib.h>
#include <Interinc/sunxi_uefi.h>
#include <Protocol/BlockIo.h>

static char mbr_buf[SUNXI_MBR_SIZE];
static int  mbr_status;
static char gpt_buf[SUNXI_GPT_SIZE];
static int  gpt_status;
static int  gptPartSize = 0;
static int  gptPartNum = 0;
static UINT32  SunxiLastPart = 0;


int sunxi_partition_init(void* MbrBuff, unsigned int MbrLen)
//int sunxi_partition_init(EFI_BLOCK_IO_PROTOCOL* pBlockIo)
{
  sunxi_mbr_t    *mbr;
#if 0
  EFI_STATUS Status;
 //read mbr
  Status = pBlockIo->ReadBlocks(pBlockIo,pBlockIo->Media->MediaId, 
      GET_BLOCK_ADDR(0),SUNXI_MBR_SIZE,mbr_buf);
  if(EFI_ERROR (Status))
  {
    AsciiPrint ("Read MBR Error\n");
    return 0;
  }
#else   
  CopyMem(mbr_buf,MbrBuff,MbrLen<SUNXI_MBR_SIZE? MbrLen:SUNXI_MBR_SIZE);
#endif
  mbr = (sunxi_mbr_t*)mbr_buf;
  if(!AsciiStrnCmp((const char*)mbr->magic, SUNXI_MBR_MAGIC, 8))
  {
    int crc = 0;

    crc = SunxiCrc32((void*)&mbr->version, SUNXI_MBR_SIZE-4);
    if(crc == mbr->crc32)
    {
      //DEBUG((DEBUG_INFO,"mbr part count = %d\n", mbr->PartCount));
      mbr_status = 1;
      return mbr->PartCount;
    }
  }
  mbr_status = 0;
  AsciiPrint("ERROR: mbr crc error\n");

  return 0;
}

EFI_STATUS SunxiPartitionInitialize(void* MbrBuff, UINT32 MbrLen)
{
  sunxi_mbr_t    *mbr;

  CopyMem(mbr_buf,MbrBuff,MbrLen<SUNXI_MBR_SIZE? MbrLen:SUNXI_MBR_SIZE);

  mbr = (sunxi_mbr_t*)mbr_buf;
  if(!AsciiStrnCmp((const char*)mbr->magic, SUNXI_MBR_MAGIC, 8))
  {
    int crc = 0;

    crc = SunxiCrc32((void*)&mbr->version, SUNXI_MBR_SIZE-4);
    if(crc == mbr->crc32)
    {
      mbr_status = 1;
      return EFI_SUCCESS;
    }
  }
  mbr_status = 0;
  AsciiPrint("ERROR: mbr crc error\n");

  return EFI_DEVICE_ERROR;

}

int sunxi_partition_get_total_num(void)
{
  sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;
  if(!mbr_status)
  {
    return 0;
  }

  return mbr->PartCount;
}

EFI_STATUS SunxiPartitionGetTotalNumber(OUT UINT32* Number)
{
  sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;
  if(!mbr_status)
  {
    return EFI_DEVICE_ERROR;
  }

  *Number =  mbr->PartCount;
  return EFI_SUCCESS;
}

int sunxi_partition_get_name(int index, char *buf)
{
  sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;

  if(mbr_status)
  {
    AsciiStrnCpy(buf, (const char *)mbr->array[index].name, 16);
  }
  else
  {
    ZeroMem(buf,  16);
  }

  return 0;
}

EFI_STATUS SunxipartitionGetName(IN UINT32 Index, OUT CHAR8 *Buffer)
{
  sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;

  if((!mbr_status) || (Index >= mbr->PartCount))
  {
    return EFI_DEVICE_ERROR;
  }

  AsciiStrnCpy(Buffer, (const char *)mbr->array[Index].name, 16);

  return EFI_SUCCESS;
}
unsigned int sunxi_partition_get_offset(int part_index)
{
  sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;

  if((!mbr_status) || (part_index >= mbr->PartCount))
  {
    return 0;
  }

  return mbr->array[part_index].addrlo;
}

EFI_STATUS SunxipartitionGetOffset(IN UINT32 Index,OUT UINT32* Offset)
{
  sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;

  if((!mbr_status) || (Index >= mbr->PartCount))
  {
    return EFI_DEVICE_ERROR;  
  }

  *Offset =  mbr->array[Index].addrlo;
  return EFI_SUCCESS;
}


unsigned int sunxi_partition_get_size(int part_index)
{
  sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;

  if((!mbr_status) || (part_index >= mbr->PartCount))
  {
    return 0;
  }

  return mbr->array[part_index].lenlo;
}

EFI_STATUS SunxiPartitionGetSize(IN UINT32 Index,OUT UINT32* Size)
{
  sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;

  if((!mbr_status) || (Index >= mbr->PartCount))
  {
    return EFI_DEVICE_ERROR;
  }

  *Size = mbr->array[Index].lenlo;
  return EFI_SUCCESS;
}

unsigned int sunxi_partition_get_offset_byname(const char *part_name)
{
  sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;
  int     i;

  if(!mbr_status)
  {
    return 0;
  }
  for(i=0;i<mbr->PartCount;i++)
  {
    if(!AsciiStrCmp(part_name, (const char *)mbr->array[i].name))
    {
      return mbr->array[i].addrlo;
    }
  }

  return 0;
}

EFI_STATUS SunxiPartitionGetOffsetByName(const CHAR8 *PartName,OUT UINT32 *Offset)
{
  sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;
  int     i;

  if(!mbr_status)
  {
    return EFI_DEVICE_ERROR;
  }
  for(i=0;i<mbr->PartCount;i++)
  {
    if(!AsciiStrCmp(PartName, (const CHAR8 *)mbr->array[i].name))
    {
      *Offset =mbr->array[i].addrlo;
      break;
    }
  }
     
  return i<mbr->PartCount ? EFI_SUCCESS:EFI_DEVICE_ERROR;
}

unsigned int sunxi_partition_get_size_byname(const char *part_name)
{
  sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;
  int     i;

  if(!mbr_status)
  {
    return 0;
  }
  for(i=0;i<mbr->PartCount;i++)
  {
    if(!AsciiStrCmp(part_name, (const char *)mbr->array[i].name))
    {
      return mbr->array[i].lenlo;
    }
  }

  return 0;
}

EFI_STATUS SunxiPartitionGetSizeByName(IN CHAR8 *PartName,OUT UINT32* Size)
{
  sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;
  int     i;

  if(!mbr_status)
  {
    return EFI_DEVICE_ERROR;
  }
  for(i=0;i<mbr->PartCount;i++)
  {
    if(!AsciiStrCmp(PartName, (const char *)mbr->array[i].name))
    {
      *Size = mbr->array[i].lenlo;
      break;
    }
  }

  return i<mbr->PartCount ? EFI_SUCCESS:EFI_DEVICE_ERROR;
}

/* get the partition info, offset and size
 * input: partition name
 * output: part_offset and part_size (in byte)
 */
int sunxi_partition_get_info_byname(const char *part_name, unsigned int *part_offset, unsigned int *part_size)
{
  sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;
  int     i;

  if(!mbr_status)
  {
    return -1;
  }

  for(i=0;i<mbr->PartCount;i++)
  {
    if(!AsciiStrCmp(part_name, (const char *)mbr->array[i].name))
    {
      *part_offset = mbr->array[i].addrlo;
      *part_size = mbr->array[i].lenlo;
      return 0;
    }
  }

  return -1;
}

EFI_STATUS SunxiPartitionGetInfoByName(IN CHAR8 *PartName, OUT UINT32* Offset, OUT UINT32* Size)
{
  sunxi_mbr_t        *mbr  = (sunxi_mbr_t*)mbr_buf;
  int     i;

  if(!mbr_status)
  {
    return -1;
  }

  for(i=0;i<mbr->PartCount;i++)
  {
    if(!AsciiStrCmp(PartName, (const char *)mbr->array[i].name))
    {
      *Offset = mbr->array[i].addrlo;
      *Size = mbr->array[i].lenlo;
      break;
    }
  }

  return i<mbr->PartCount ? EFI_SUCCESS:EFI_DEVICE_ERROR;
}


void *sunxi_partition_fetch_mbr(void)
{
  if(!mbr_status)
  {
    return NULL;
  }

  return mbr_buf;
}

int sunxi_partition_refresh(void *buf, unsigned int bytes)
{
  if(!mbr_status)
  {
    return -1;
  }
  if(bytes != SUNXI_MBR_SIZE)
  {
    return -1;
  }

  CopyMem(mbr_buf, buf, bytes);

  return 0;
}

void SunxiDumpPartitionTable(void)
{
  int index, part_total;
    /* The logical name for this partition, null terminated */
  char name[16];
  /* The start wrt the nand part, must be multiple of nand block size */
  unsigned int start = 0;
  /* The length of the partition, must be multiple of nand block size */
  unsigned int length = 0;
  
  AsciiPrint("--------fastboot partitions--------\n");
  part_total = sunxi_partition_get_total_num();
  if((part_total <= 0) || (part_total > SUNXI_MBR_MAX_PART_COUNT))
  {
    AsciiPrint("mbr not exist\n");
    return ;
  }
  AsciiPrint("-total partitions:%d-\n", part_total);
  AsciiPrint("%-12a  %-12a  %-12a\n", "-name-", "-start-", "-size-");
  for(index = 0; index < part_total && index < SUNXI_MBR_MAX_PART_COUNT; index++)
  {
    sunxi_partition_get_name(index, name);
    start = sunxi_partition_get_offset(index) * 512;
    length = sunxi_partition_get_size(index) * 512;
    AsciiPrint("%-12a: %-12x  %-12x\n", name, start, length);
  }
  AsciiPrint("-----------------------------------\n");
}

void SunxiGetPartitionInfo(int storage_type, char *cmdline_part_info)
{
  int index, part_total;
  char partition_sets[256];
  char part_name[16];
  char *partition_index = partition_sets;
  int offset = 0;

  /* The logical name for this partition, null terminated */
  char name[16];

  part_total = sunxi_partition_get_total_num();
  if((part_total <= 0) || (part_total > SUNXI_MBR_MAX_PART_COUNT))
  {
    AsciiPrint("mbr not exist\n");
    return ;
  }

  ZeroMem(partition_sets, 256);
  partition_sets[255] = '\0';

  for(index = 0; index < part_total && index < SUNXI_MBR_MAX_PART_COUNT; index++)
  {
    sunxi_partition_get_name(index, name);
    ZeroMem(part_name, 16);
    if(!storage_type)
    {
      AsciiSPrint(part_name, sizeof(part_name),"nand%c", 'a' + index);
    }
    else
    {
      if(index == 0)
      {
        AsciiStrCpy(part_name, "mmcblk0p2");
      }
      else if( (index+1)==part_total)
      {
        AsciiStrCpy(part_name, "mmcblk0p1");
      }
      else
      {
        AsciiSPrint(part_name,sizeof(part_name), "mmcblk0p%d", index + 4);
      }
    }
    AsciiSPrint(partition_index,sizeof(partition_sets), "%a@%a:", name, part_name);
    offset += AsciiStrLen(name) + AsciiStrLen(part_name) + 2;
    partition_index = partition_sets + offset;
  }

  partition_sets[offset-1] = '\0';

  AsciiStrCpy(cmdline_part_info, partition_sets);
}

EFI_STATUS
EFIAPI
SunxiPartitionConstructor (
VOID
  )
{
  EFI_STATUS Status = EFI_SUCCESS;
  
  return Status;
}

#if 0
static void DumpDataHex(CHAR8 *name, UINT8 *base, INT32 len)
{
    INT32 i;
    AsciiPrint("dump %a: \n", name);
    for (i=0; i<len; i++) {
        AsciiPrint("%02x ",base[i]);
        if((i+1)%16 == 0) {AsciiPrint("\n");}
    }
    AsciiPrint("\n");
}
#endif
/*
*  convert Primary Header table to Backup Header table or
*  convert Backup Header table to Primary Header table
*/
BOOLEAN 
EFIAPI
PrimaryConvertBackup(
  IN EFI_PARTITION_TABLE_HEADER  *FirstHeader,
  IN EFI_PARTITION_TABLE_HEADER  *SecondHeader
)
{
  EFI_LBA PEntryLBA;

  if(FirstHeader == NULL)
    return FALSE;

  PEntryLBA = (FirstHeader->MyLBA == PRIMARY_PART_HEADER_LBA) ? \
                             (FirstHeader->LastUsableLBA + 1) : \
                             (PRIMARY_PART_HEADER_LBA + 1);

  CopyMem (SecondHeader, FirstHeader, sizeof (EFI_PARTITION_TABLE_HEADER));

  SecondHeader->MyLBA              = FirstHeader->AlternateLBA;
  SecondHeader->AlternateLBA       = FirstHeader->MyLBA;
  SecondHeader->PartitionEntryLBA  = PEntryLBA;
  SecondHeader->Header.CRC32       = 0x0;
  SecondHeader->Header.CRC32       = SunxiCrc32(&SecondHeader->Header,SecondHeader->Header.HeaderSize);

  return TRUE;
}

/*
* Create Protective MBR
*
*/
EFI_STATUS 
EFIAPI
CreateProtectiveMbr
(
  IN MASTER_BOOT_RECORD   *ProtectiveMbr,
  IN EFI_LBA               LastBlock
)
{
  UINT32 block = 0;

  block = (UINT32)(((LastBlock + 1)>>32) & 0xffffffff);
  if(ProtectiveMbr == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  ProtectiveMbr->Partition[0].BootIndicator = 0x0;
  ProtectiveMbr->Partition[0].OSIndicator= 0xEE;
  ProtectiveMbr->Partition[0].StartingLBA[0]= 0x1;

  ProtectiveMbr->Partition[0].SizeInLBA[0]= block & 0xff;
  ProtectiveMbr->Partition[0].SizeInLBA[1]= (block>>8) & 0xff;
  ProtectiveMbr->Partition[0].SizeInLBA[2]= (block>>16) & 0xff;
  ProtectiveMbr->Partition[0].SizeInLBA[3]= (block>>24) & 0xff;

  ProtectiveMbr->Partition[0].StartHead= 0x0;
  ProtectiveMbr->Partition[0].StartSector= 0x0;
  ProtectiveMbr->Partition[0].StartTrack= 0x0;

  ProtectiveMbr->Partition[0].EndHead= 0x0;
  ProtectiveMbr->Partition[0].EndSector= 0x0;
  ProtectiveMbr->Partition[0].EndTrack= 0x0;

  ProtectiveMbr->Signature = MBR_SIGNATURE;

  return EFI_SUCCESS;
}

/*
* Create GPT Header Table and Entry by input parameter
*
*/
EFI_STATUS
EFIAPI
CreateGptTableAndEntry(
  IN VOID *Buffer,
  IN gpt_table_info_t table_info,
  OUT EFI_PARTITION_TABLE_HEADER *PrimaryHeader,
  OUT EFI_PARTITION_ENTRY *PartEntry
)
{
  EFI_PARTITION_ENTRY *Entry;
  sunxi_gpt_t *gpt = (sunxi_gpt_t *)Buffer;
  UINT32 i, j; 
  CHAR16 tmp_name[36];

  if((Buffer == NULL) || (PrimaryHeader == NULL) || (PartEntry == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  ZeroMem(tmp_name,36);

  PrimaryHeader->Header.Signature = EFI_PTAB_HEADER_ID;
  PrimaryHeader->Header.HeaderSize = 0x5c;
  PrimaryHeader->Header.Revision = 0x10000;

  PrimaryHeader->MyLBA = table_info.MyLBA;
  PrimaryHeader->AlternateLBA = table_info.AlternateLBA;
  /* manual create */
  CopyMem(&PrimaryHeader->DiskGUID, &table_info.Guid, sizeof(EFI_GUID));
  PrimaryHeader->FirstUsableLBA = table_info.FirstLBA;
  PrimaryHeader->LastUsableLBA = table_info.LastLBA;
  PrimaryHeader->PartitionEntryLBA = table_info.EntryLBA;
  PrimaryHeader->NumberOfPartitionEntries = table_info.EntryNum;
  PrimaryHeader->SizeOfPartitionEntry = table_info.EntrySize;

  /* Fill Partition entry  */
  SunxiLastPart = gpt->PartCount - 1;
  for(i = 0; i < gpt->PartCount; i++)
  {
    Entry = (EFI_PARTITION_ENTRY *) ((UINT8 *) PartEntry + i * PrimaryHeader->SizeOfPartitionEntry);
    /* Copy Partiton Type GUID and Unique GUID */
    CopyMem(&Entry->PartitionTypeGUID, &gpt->array[i].type, sizeof(EFI_GUID));
    CopyMem(&Entry->UniquePartitionGUID, &gpt->array[i].uniqueguid, sizeof(EFI_GUID));
    Entry->Attributes = ((EFI_LBA)(gpt->array[i].attrhi) << 32) | gpt->array[i].attrlo;
    Entry->StartingLBA = ((EFI_LBA)gpt->array[i].addrhi << 32) | gpt->array[i].addrlo;
    if(i == SunxiLastPart)
    {
      Entry->EndingLBA = PrimaryHeader->LastUsableLBA;
    }
    else
    {
      Entry->EndingLBA = Entry->StartingLBA + (((EFI_LBA)gpt->array[i].lenhi << 32) | gpt->array[i].lenlo) - 1;
    }
  for(j = 0; j < 16; j++)
  {
    tmp_name[j] = (CHAR16)gpt->array[i].name[j];
  }
    CopyMem(&Entry->PartitionName,tmp_name, 36);
  }

  /* Calculate CRC32 value */
  PrimaryHeader->PartitionEntryArrayCRC32 = SunxiCrc32(PartEntry,\
  PrimaryHeader->NumberOfPartitionEntries * PrimaryHeader->SizeOfPartitionEntry);
  PrimaryHeader->Header.CRC32 = SunxiCrc32(&PrimaryHeader->Header,PrimaryHeader->Header.HeaderSize);

  //DumpDataHex("Partition Entry",(UINT8 *)PartEntry, 512);
  return EFI_SUCCESS;
}

EFI_STATUS SunxiGptPartitionInitialize
(
  IN EFI_BLOCK_IO_PROTOCOL*   BlockIo,
  IN void* GptBuff, 
  IN UINT32 GptLen
)
{
  EFI_PARTITION_TABLE_HEADER *PrimaryHeader;
  UINT32 status;
  UINT32 HeadCrc = 0;
  UINT32 EntryCrc = 0;
  UINT32 OrgCrc = 0;

  PrimaryHeader = (EFI_PARTITION_TABLE_HEADER*)GptBuff;
  OrgCrc = PrimaryHeader->Header.CRC32;
  /* clear old crc from header */
  PrimaryHeader->Header.CRC32 = 0;
  /* calculate check crc */
  HeadCrc = SunxiCrc32(&PrimaryHeader->Header, PrimaryHeader->Header.HeaderSize);
  /* restore header crc */
  PrimaryHeader->Header.CRC32 = HeadCrc;
  
  if ((PrimaryHeader->Header.Signature != EFI_PTAB_HEADER_ID) || OrgCrc != HeadCrc 
     ||(PrimaryHeader->SizeOfPartitionEntry < sizeof (EFI_PARTITION_ENTRY)))
  {
    DEBUG ((EFI_D_INFO, "Invalid efi primary partition table header\n"));
    DEBUG ((EFI_D_INFO, "Header size 0x%x,default crc 0x%x, check crc 0x%x\n", PrimaryHeader->Header.HeaderSize, OrgCrc, HeadCrc));
    gpt_status = 0;
    return EFI_INVALID_PARAMETER;
  }

  /* Read GPT Partition Entry */
  status = BlockIo->ReadBlocks(BlockIo, BlockIo->Media->MediaId, PrimaryHeader->PartitionEntryLBA, \
    PrimaryHeader->NumberOfPartitionEntries* PrimaryHeader->SizeOfPartitionEntry, gpt_buf);
  if(status)
  {
    DEBUG ((EFI_D_INFO, "Read gpt partition entry error\n"));
    gpt_status = 0;
    return EFI_DEVICE_ERROR;
  }

  EntryCrc = SunxiCrc32(gpt_buf, PrimaryHeader->NumberOfPartitionEntries * PrimaryHeader->SizeOfPartitionEntry);
  if(PrimaryHeader->PartitionEntryArrayCRC32 != EntryCrc)
  {
    DEBUG ((EFI_D_INFO, "Check gpt partition entry crc error\n"));
    gpt_status = 0;
    return EFI_DEVICE_ERROR;    
  }
  
  gpt_status = 1;
  gptPartSize = PrimaryHeader->SizeOfPartitionEntry;
  gptPartNum = PrimaryHeader->NumberOfPartitionEntries;

  DEBUG ((EFI_D_INFO, "gptPartNum 0x%x, gptPartSize 0x%x\n", gptPartNum, gptPartSize));
  
  return EFI_SUCCESS;
}

EFI_STATUS SunxiGptPartitionGetTotalNumber(OUT UINT32* Number)
{
  if(!gpt_status)
  {
    return EFI_DEVICE_ERROR;
  }

  *Number =  gptPartNum;
  return EFI_SUCCESS;
}

EFI_STATUS SunxiGptPartitionGetName(IN UINT32 Index, OUT CHAR8 *Buffer)
{
  EFI_PARTITION_ENTRY        *EntryIndex;

  if((!gpt_status) || (Index >= gptPartNum))
  {
    return EFI_DEVICE_ERROR;
  }

  EntryIndex = (EFI_PARTITION_ENTRY *)(gpt_buf + Index * gptPartSize);
  
  AsciiStrnCpy(Buffer, (const char *)EntryIndex->PartitionName, 36);

  return EFI_SUCCESS;
}

EFI_STATUS SunxiGptPartitionGetOffset(IN UINT32 Index,OUT UINT32* Offset)
{
  EFI_PARTITION_ENTRY      *EntryIndex;

  if((!gpt_status) || (Index >= gptPartNum))
  {
    return EFI_DEVICE_ERROR;
  }
  EntryIndex = (EFI_PARTITION_ENTRY *)(gpt_buf + Index * gptPartSize);

  *Offset =  (UINTN)EntryIndex->StartingLBA;
  
  return EFI_SUCCESS;
}

EFI_STATUS SunxiGptPartitionGetSize(IN UINT32 Index,OUT UINT32* Size)
{
  EFI_PARTITION_ENTRY    *EntryIndex;

  if((!gpt_status) || (Index >= gptPartNum))
  {
    return EFI_DEVICE_ERROR;
  }

  EntryIndex = (EFI_PARTITION_ENTRY *)(gpt_buf + Index * gptPartSize);
  *Size = (UINTN)(EntryIndex->EndingLBA - EntryIndex->StartingLBA);
  
  return EFI_SUCCESS;
}

EFI_STATUS SunxiGptPartitionGetOffsetByName(const CHAR8 *PartName,OUT UINT32 *Offset)
{
  EFI_PARTITION_ENTRY    *EntryIndex;
  int     i, j;
  CHAR8   tmp_data[36];

  ZeroMem(tmp_data,36);
  if(!gpt_status || PartName == NULL)
  {
    return EFI_DEVICE_ERROR;
  }
  for(i=0;i<gptPartNum;i++)
  {
    EntryIndex = (EFI_PARTITION_ENTRY *)(gpt_buf + i * gptPartSize);
    for(j = 0; j < 36; j++)
      tmp_data[j] = EntryIndex->PartitionName[j];
  
    if(!AsciiStrCmp(PartName, tmp_data))
    {
      *Offset = (UINTN)EntryIndex->StartingLBA;
      break;
    }
  }

  return i<gptPartNum? EFI_SUCCESS:EFI_DEVICE_ERROR;
}

EFI_STATUS SunxiGptPartitionGetSizeByName(IN CHAR8 *PartName,OUT UINT32* Size)
{
  EFI_PARTITION_ENTRY    *EntryIndex;
  int   i, j;
  CHAR8   tmp_data[36];

  ZeroMem(tmp_data,36);
  if(!gpt_status || PartName == NULL)
  {
    return EFI_DEVICE_ERROR;
  }
  for(i=0;i<gptPartNum;i++)
  {
    EntryIndex = (EFI_PARTITION_ENTRY *)(gpt_buf + i * gptPartSize);
    for(j = 0; j < 36; j++)
      tmp_data[j] = EntryIndex->PartitionName[j];
    
    if(!AsciiStrCmp(PartName, tmp_data))
    {
      *Size =(UINTN) (EntryIndex->EndingLBA - EntryIndex->StartingLBA);
      break;
    }
  }

  return i<gptPartNum? EFI_SUCCESS:EFI_DEVICE_ERROR;
}

EFI_STATUS SunxiGptPartitionGetInfoByName(IN CHAR8 *PartName, OUT UINT32* Offset, OUT UINT32* Size)
{
  EFI_PARTITION_ENTRY    *EntryIndex;
  int   i, j;
  CHAR8   tmp_data[36];

  ZeroMem(tmp_data,36);
  if(!gpt_status || PartName == NULL)
  {
    return EFI_DEVICE_ERROR;
  }
  for(i=0;i<gptPartNum;i++)
  {
    EntryIndex = (EFI_PARTITION_ENTRY *)(gpt_buf + i * gptPartSize);
    for(j = 0; j < 36; j++)
      tmp_data[j] = EntryIndex->PartitionName[j];
    
    if(!AsciiStrCmp(PartName, tmp_data))
    {
      *Offset = (UINTN)EntryIndex->StartingLBA;
      *Size = (UINTN)(EntryIndex->EndingLBA - EntryIndex->StartingLBA);
      break;
    }
  }

  return i<gptPartNum? EFI_SUCCESS:EFI_DEVICE_ERROR;
}

void SunxiGptGetPartitionInfo(IN UINT32 storage_type, IN CHAR8 *cmdline_part_info)
{
  unsigned int index, part_total=0;
  char partition_sets[256];
  char part_name[16];
  char *partition_index = partition_sets;
  int offset = 0;

    /* The logical name for this partition, null terminated */
  char name[16];

  SunxiGptPartitionGetTotalNumber(&part_total);
  if((part_total <= 0) || (part_total > 128))
  {
    AsciiPrint("gpt not exist\n");
    return ;
  }

  ZeroMem(partition_sets, 256);
  partition_sets[255] = '\0';

  for(index = 0; index < part_total; index++)
  {
    SunxiGptPartitionGetName(index, name);

    ZeroMem(part_name, 16);
    if(!storage_type)
    {
      AsciiSPrint(part_name, sizeof(part_name),"nand%c", 'a' + index);
    }
    else
    {
      if(index == 0)
      {
        AsciiStrCpy(part_name, "mmcblk0p2");
      }
      else if( (index+1)==part_total)
      {
        AsciiStrCpy(part_name, "mmcblk0p1");
      }
      else
      {
        AsciiSPrint(part_name,sizeof(part_name), "mmcblk0p%d", index + 4);
      }
    }
    AsciiSPrint(partition_index,sizeof(partition_sets), "%a@%a:", name, part_name);
    offset += AsciiStrLen(name) + AsciiStrLen(part_name) + 2;
    partition_index = partition_sets + offset;
  }

  partition_sets[offset-1] = '\0';

  AsciiStrCpy(cmdline_part_info, partition_sets);
}

