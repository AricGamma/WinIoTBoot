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

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/SunxiFlashIo.h>
#include <Library/SunxiMbr.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/SunxiCheckLib.h>


#define  VERIFY_ONCE_BYTES    (16 * 1024 * 1024)
#define  VERIFY_ONCE_SECTORS  (VERIFY_ONCE_BYTES/512)

#define  SUNXI_SPRITE_PROTECT_DATA_MAX    (16)
#define  SUNXI_SPRITE_PROTECT_PART        "private"

struct private_part_info
{
  UINT32 part_sectors;
  CHAR8 *part_buf;
  CHAR8  part_name[32];
};

struct private_part_info  part_info[SUNXI_SPRITE_PROTECT_DATA_MAX];


static SUNXI_FLASH_IO_PROTOCOL *PrivateFlashIo = NULL;

static INT32 SunxiPrivateRead (UINT32 start_block, UINT32 nblock, void *buffer)
{
  EFI_STATUS           Status;
  Status = PrivateFlashIo->SunxiFlashIoRead(PrivateFlashIo,start_block,nblock, buffer);
  if(EFI_ERROR(Status))
  {
    return 0;
  }
  return nblock;
}

static INT32 SunxiPrivateWrite(UINT32 start_block, UINT32 nblock, void *buffer)
{
  EFI_STATUS           Status;
  Status = PrivateFlashIo->SunxiFlashIoWrite(PrivateFlashIo,start_block,nblock, buffer);
  if(EFI_ERROR(Status))
  {
    return 0;
  }
  return nblock;
}


/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
BOOLEAN SunxiSpriteStorePrivateData(void *buffer)
{
  INT32 i, j;

  j = 0;
  sunxi_mbr_t  *mbr = (sunxi_mbr_t *)buffer;
  for(i=0;i<mbr->PartCount;i++)
  {
    AsciiPrint("part name %a\n", mbr->array[i].name);
    AsciiPrint("keydata = 0x%x\n", mbr->array[i].keydata);
    if( (!AsciiStrCmp((const char *)mbr->array[i].name, SUNXI_SPRITE_PROTECT_PART)) || (mbr->array[i].keydata == 0x8000))
    {
      AsciiPrint("find keypart %a\n", mbr->array[i].name);
      AsciiPrint("keypart read start: 0x%x, sectors 0x%x\n", mbr->array[i].addrlo, mbr->array[i].lenlo);

      part_info[j].part_buf = (char *)AllocateZeroPool(mbr->array[i].lenlo * 512);
      if(!part_info[j].part_buf)
      {
        AsciiPrint("sprite protect private data fail: cant malloc memory for part %s, sectors 0x%x\n", mbr->array[i].name, mbr->array[i].lenlo);
        goto __sunxi_sprite_store_part_data_fail;
      }

      if(mbr->array[i].lenlo != SunxiPrivateRead(mbr->array[i].addrlo, mbr->array[i].lenlo, (void *)part_info[j].part_buf))
      {
        AsciiPrint("sunxi sprite error : read private data error\n");
        goto __sunxi_sprite_store_part_data_fail;
      }
      AsciiPrint("keypart part %s read end: 0x%x, sectors 0x%x\n", mbr->array[i].name, mbr->array[i].addrlo, mbr->array[i].lenlo);

      part_info[j].part_sectors = mbr->array[i].lenlo;
      AsciiStrCpy(part_info[j].part_name, (const char *)mbr->array[i].name);

      j ++;
    }
  }
  if(!j)
  {
    AsciiPrint("there is no keypart part on local flash\n");
  }

  return TRUE;

__sunxi_sprite_store_part_data_fail:
  for(i=0;i<j;i++)
  {
    if(part_info[i].part_buf)
    {
      FreePool(part_info[i].part_buf);
    }
    else
    {
      break;
    }
  }

  return FALSE;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
BOOLEAN SunxiSpriteRestorePrivateData(void *buffer)
{
  INT32 i, j;
  INT32 ret = -1;
  UINT32      down_sectors;
  sunxi_mbr_t  *mbr = (sunxi_mbr_t *)buffer;
    
  j = 0;
  while(part_info[j].part_buf)
  {
    for(i=0;i<mbr->PartCount;i++)
    {
      if(!AsciiStrCmp(part_info[j].part_name, (const char*)mbr->array[i].name))
      {
        if(part_info[j].part_sectors > mbr->array[i].lenlo)
        {
          AsciiPrint("origin sectors 0x%x, new part sectors 0x%x\n", part_info[j].part_sectors, mbr->array[i].lenlo);
          AsciiPrint("fix it, only store less sectors\n");

          down_sectors = mbr->array[i].lenlo;
        }
        else
        {
          down_sectors = part_info[j].part_sectors;
        }

        AsciiPrint("keypart write start: 0x%x, sectors 0x%x\n", mbr->array[i].addrlo, down_sectors);
  
        if(down_sectors != SunxiPrivateWrite(mbr->array[i].addrlo, down_sectors, (void *)part_info[j].part_buf))
        {
          AsciiPrint("sunxi sprite error : write private data error\n");
          goto __sunxi_sprite_restore_part_data_fail;
        }

        AsciiPrint("keypart write end: 0x%x, sectors 0x%x\n", mbr->array[i].addrlo, down_sectors);

        break;
      }
    }
    j ++;
  }
  if(!j)
  {
    AsciiPrint("there is no private part need rewrite\n");
  }
  ret = 0;

__sunxi_sprite_restore_part_data_fail:
  for(i=0;i<j;i++)
  {
    if(part_info[i].part_buf)
    {
      FreePool(part_info[i].part_buf);
    }
    else
    {
      break;
    }
  }

  return ret == 0 ? TRUE: FALSE;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
EFI_STATUS SunxiSpriteProbePrivatePartition(void  *buffer)
{
  INT32 i;

  sunxi_mbr_t  *mbr = (sunxi_mbr_t *)buffer;
  for(i=0;i<mbr->PartCount;i++)
  {
    if( (!AsciiStrCmp((const char *)mbr->array[i].name, SUNXI_SPRITE_PROTECT_PART)) || (mbr->array[i].keydata == 0x8000))
    {
      AsciiPrint("private part exist\n");

      return TRUE;
    }
  }

  return FALSE;
}


UINT32 SunxiSpritePartRawDataVerify(UINT32 base_start, UINT64 base_bytes)
{
  UINT32 checksum = 0;
  UINT32 unaligned_bytes, last_time_bytes;
  UINT32 rest_sectors;
  UINT32 crt_start;
  CHAR8 *tmp_buf = NULL;
  
  tmp_buf = (CHAR8 *)AllocateZeroPool(VERIFY_ONCE_BYTES);
  if(!tmp_buf)
  {
    AsciiPrint("sunxi sprite err: unable to malloc memory for verify\n");
    return 0;
  }
  crt_start       = base_start;
  rest_sectors    = (UINT32)((base_bytes + 511)>>9);
  unaligned_bytes = (UINT32)base_bytes & 0x1ff;

  //debug("read total sectors %d\n", rest_sectors);
  //debug("read part start %d\n", crt_start);
  while(rest_sectors >= VERIFY_ONCE_SECTORS)
  {
    if(SunxiPrivateRead(crt_start, VERIFY_ONCE_SECTORS, tmp_buf) != VERIFY_ONCE_SECTORS)
    {
      AsciiPrint("sunxi sprite: read flash error when verify\n");
      checksum = 0;
      goto __rawdata_verify_err;
    }
    crt_start    += VERIFY_ONCE_SECTORS;
    rest_sectors -= VERIFY_ONCE_SECTORS;
    checksum     += SunxiAddSum(tmp_buf, VERIFY_ONCE_BYTES);
    //debug("check sum = 0x%x\n", checksum);
  }
  if(rest_sectors)
  {
    if(SunxiPrivateRead(crt_start, rest_sectors, tmp_buf) != rest_sectors)
    {
      AsciiPrint("sunxi sprite: read flash error when verify\n");
      checksum = 0;
      goto __rawdata_verify_err;
    }
    if(unaligned_bytes)
    {
      last_time_bytes = (rest_sectors - 1) * 512 + unaligned_bytes;
    }
    else
    {
      last_time_bytes = rest_sectors * 512;
    }
    checksum += SunxiAddSum(tmp_buf, last_time_bytes);
    //debug("check sum = 0x%x\n", checksum);
  }
    
__rawdata_verify_err:
  if(tmp_buf)
  {
    FreePool(tmp_buf);
  }

  return checksum;

}

static void __mbr_map_dump(CHAR8 *buf)
{
  sunxi_gpt_t     *mbr_info = (sunxi_gpt_t *)buf;
  gpt_partition *part_info;
  UINT32 i, j;
  CHAR8  *buffer[32];

  AsciiPrint("*************MBR DUMP***************\n");
  AsciiPrint("total mbr part %d\n", mbr_info->PartCount);
  AsciiPrint("\n");
  for(part_info = mbr_info->array, i=0;i<mbr_info->PartCount;i++, part_info++)
  {
    ZeroMem(buffer, 32);
    CopyMem(buffer, part_info->name, 16);
    AsciiPrint("part[%d] name      :%a\n", i, buffer);
    ZeroMem(buffer, 32);
    CopyMem(buffer, part_info->classname, 16);
    AsciiPrint("part[%d] classname :%a\n", i, buffer);
    AsciiPrint("part[%d] type : 0x%x, 0x%x, 0x%x", i, part_info->type.Data1, part_info->type.Data2, part_info->type.Data3);
    for(j = 0; j < 8; j++)
    {
      AsciiPrint(",0x%x", part_info->type.Data4[j]);
    }
    AsciiPrint("\n");
    AsciiPrint("part[%d] addrlo    :0x%x\n", i, part_info->addrlo);
    AsciiPrint("part[%d] lenlo     :0x%x\n", i, part_info->lenlo);
    AsciiPrint("part[%d] attrhi    :0x%x\n", i, part_info->attrhi);
    AsciiPrint("part[%d] attrlo    :0x%x\n", i, part_info->attrlo);
    AsciiPrint("part[%d] user_type :%d\n", i, part_info->user_type);
    AsciiPrint("part[%d] keydata   :%d\n", i, part_info->keydata);
    AsciiPrint("part[%d] ro        :%d\n", i, part_info->ro);
    AsciiPrint("\n");
  }
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
BOOLEAN SunxiSpriteVerifyMbr(void *buffer)
{
  sunxi_mbr_t  *local_mbr;
  CHAR8        *tmp_buf = (CHAR8 *)buffer;
  INT32        i;

  tmp_buf = buffer;
  for(i=0;i<SUNXI_MBR_COPY_NUM;i++)
  {
    local_mbr = (sunxi_mbr_t *)tmp_buf;
    if(SunxiCrc32((const unsigned char *)(tmp_buf + 4), SUNXI_MBR_SIZE - 4) != local_mbr->crc32)
    {
      AsciiPrint("the %d mbr table is bad\n", i);
      return FALSE;
    }
    else
    {
      AsciiPrint("the %d mbr table is ok\n", i);
      tmp_buf += SUNXI_MBR_SIZE;
    }
  }
#if 1
  __mbr_map_dump(buffer);
#endif

    return TRUE;
}


EFI_STATUS
EFIAPI
SunxiSpriteHelperConstructor (VOID)
{
  EFI_STATUS Status = EFI_SUCCESS;

   //get FlashIO protocol
  Status = gBS->LocateProtocol(&gSunxiFlashIoProtocolGuid, NULL, (VOID **)&PrivateFlashIo);
  if (EFI_ERROR (Status)) 
  {
    Print(L"Get FlashIO Fail\n");  
  }

  return Status;
}


