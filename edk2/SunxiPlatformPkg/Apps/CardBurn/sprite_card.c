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

#include "../CardBurn.h"
#include <Interinc/sunxi_uefi.h>
#include "sparse/sparse.h"
#include "sprite_verify.h"
#include "firmware/imgdecode.h"
//#include "../dos_part.h"
#include "../sprite_cartoon.h"

#define  SPRITE_CARD_HEAD_BUFF       (32 * 1024)
#define  SPRITE_CARD_ONCE_DATA_DEAL    (16 * 1024 * 1024)
#define  SPRITE_CARD_ONCE_SECTOR_DEAL  (SPRITE_CARD_ONCE_DATA_DEAL/512)

static void *imghd = NULL;
static void *imgitemhd = NULL;

static int __download_normal_part(dl_one_part_info *part_info,  uchar *source_buff);
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
uint sprite_card_firmware_start(void)
{
  return sunxi_partition_get_offset(1);
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
int sprite_card_firmware_probe(char *name)
{
  debug("firmware name %s\n", name);
  imghd = Img_Open(name);
  if(!imghd)
  {
    return -1;
  }

  return 0;
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
int sprite_card_fetch_download_map(sunxi_download_info  *dl_map)
{
  imgitemhd = Img_OpenItem(imghd, "12345678", "1234567890DLINFO");
  if(!imgitemhd)
  {
    return -1;
  }
  debug("try to read item dl map\n");
  if(!Img_ReadItem(imghd, imgitemhd, (void *)dl_map, sizeof(sunxi_download_info)))
  {
    printf("sunxi sprite error : read dl map failed\n");
    return -1;
  }
  Img_CloseItem(imghd, imgitemhd);
  imgitemhd = NULL;
  //检查获取的dlinfo是否正确
  return sunxi_sprite_verify_dlmap(dl_map);
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
int sprite_card_fetch_mbr(void  *img_mbr)
{
  imgitemhd = Img_OpenItem(imghd, "12345678", "1234567890___MBR");
  if(!imgitemhd)
  {
    return -1;
  }
  debug("try to read item dl map\n");
  if(!Img_ReadItem(imghd, imgitemhd, img_mbr, sizeof(sunxi_mbr_t) * SUNXI_MBR_COPY_NUM))
  {
    printf("sunxi sprite error : read mbr failed\n");
    return -1;
  }
  Img_CloseItem(imghd, imgitemhd);
  imgitemhd = NULL;

  if(SunxiSpriteVerifyMbr(img_mbr) == FALSE)
  {
    return -1;
  }
  return 0;
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
static int __download_udisk(dl_one_part_info *part_info,  uchar *source_buff)
{
  HIMAGEITEM imgitemhd = NULL;
  u32  flash_sector;
  s64  packet_len;
  s32  ret = -1, ret1;

  //打开分区镜像
  imgitemhd = Img_OpenItem(imghd, "RFSFAT16", (char *)part_info->dl_filename);
  if(!imgitemhd)
  {
    printf("sunxi sprite error: open part %s failed\n", part_info->dl_filename);
    return -1;
  }
  //获取分区镜像字节数
  packet_len = Img_GetItemSize(imghd, imgitemhd);
  if (packet_len <= 0)
  {
    printf("sunxi sprite error: fetch part len %s failed\n", part_info->dl_filename);
    goto __download_udisk_err1;
  }
  if (packet_len <= FW_BURN_UDISK_MIN_SIZE)
  {
    printf("download UDISK: the data length of udisk is too small, ignore it\n");
    ret = 1;
    goto __download_udisk_err1;
  }
  //分区镜像够大，需要进行烧录
  flash_sector = sunxi_flash_size();
  if(!flash_sector)
  {
    printf("sunxi sprite error: download_udisk, the flash size is invalid(0)\n");
    goto __download_udisk_err1;
  }
  printf("the flash size is %d MB\n", flash_sector/2/1024); //计算出M单位
  part_info->lenlo = flash_sector - part_info->addrlo;
  part_info->lenhi = 0;
  printf("UDISK low is 0x%x Sectors\n", part_info->lenlo);
  printf("UDISK high is 0x%x Sectors\n", part_info->lenhi);

  ret = __download_normal_part(part_info, source_buff);
__download_udisk_err1:
  ret1 = Img_CloseItem(imghd, imgitemhd);
  if(ret1 != 0 )
  {
    printf("sunxi sprite error: __download_udisk, close udisk image failed\n");
    return -1;
  }

  return ret;
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
static int __download_normal_part(dl_one_part_info *part_info,  uchar *source_buff)
{
  uint partstart_by_sector;   //分区起始扇区
  uint tmp_partstart_by_sector;

  s64  partsize_by_byte;      //分区大小(字节单位)

  s64  partdata_by_byte;      //需要下载的分区数据(字节单位)
  s64  tmp_partdata_by_bytes;

  uint onetime_read_sectors;    //一次读写的扇区数
  uint first_write_bytes;

  uint imgfile_start;       //分区数据所在的扇区
  uint tmp_imgfile_start;

  u8 *down_buffer       = source_buff + SPRITE_CARD_HEAD_BUFF;

  int  partdata_format;

  int  ret = -1;
  //*******************************************************************
  //获取分区起始扇区
  tmp_partstart_by_sector = partstart_by_sector = part_info->addrlo;
  //获取分区大小，字节数
  partsize_by_byte     = part_info->lenlo;
  partsize_by_byte   <<= 9;
  //打开分区镜像
  imgitemhd = Img_OpenItem(imghd, "RFSFAT16", (char *)part_info->dl_filename);
  if(!imgitemhd)
  {
    printf("sunxi sprite error: open part %s failed\n", part_info->dl_filename);
    return -1;
  }
  //获取分区镜像字节数
  partdata_by_byte = Img_GetItemSize(imghd, imgitemhd);
  if (partdata_by_byte <= 0)
  {
    printf("sunxi sprite error: fetch part len %s failed\n", part_info->dl_filename);
    goto __download_normal_part_err1;
  }
  printf("partdata hi 0x%x\n", (uint)(partdata_by_byte>>32));
  printf("partdata lo 0x%x\n", (uint)partdata_by_byte);
  //如果分区数据超过分区大小
  if(partdata_by_byte > partsize_by_byte)
  {
    printf("sunxi sprite: data size 0x%x is larger than part %s size 0x%x\n", (uint)(partdata_by_byte/512), part_info->dl_filename, (uint)(partsize_by_byte/512));
    goto __download_normal_part_err1;
  }
  //准备读取分区镜像数据
  tmp_partdata_by_bytes = partdata_by_byte;
  if(tmp_partdata_by_bytes >= SPRITE_CARD_ONCE_DATA_DEAL)
  {
    onetime_read_sectors = SPRITE_CARD_ONCE_SECTOR_DEAL;
    first_write_bytes    = SPRITE_CARD_ONCE_DATA_DEAL;
  }
  else
  {
    onetime_read_sectors = (tmp_partdata_by_bytes + 511)>>9;
    first_write_bytes    = (uint)tmp_partdata_by_bytes;
  }
  //开始获取分区数据
  imgfile_start = Img_GetItemStart(imghd, imgitemhd);
  if(!imgfile_start)
  {
    printf("sunxi sprite err : cant get part data imgfile_start %s\n", part_info->dl_filename);
    goto __download_normal_part_err1;
  }
  tmp_imgfile_start = imgfile_start;
  //读出第一笔固件中的分区数据，大小为buffer字节数
  if(sunxi_flash_read(tmp_imgfile_start, onetime_read_sectors, down_buffer) != onetime_read_sectors)
  {
    printf("sunxi sprite error : read sdcard block %d, total %d failed\n", tmp_imgfile_start, onetime_read_sectors);
    goto __download_normal_part_err1;
  }
  //下一个要读出的数据
  tmp_imgfile_start += onetime_read_sectors;
  //尝试查看是否sparse格式
  partdata_format = unsparse_probe((char *)down_buffer, first_write_bytes, partstart_by_sector);    //判断数据格式
  if(partdata_format != ANDROID_FORMAT_DETECT)
  {
    //写入第一笔数据
    if(sunxi_sprite_write(tmp_partstart_by_sector, onetime_read_sectors, down_buffer) != onetime_read_sectors)
    {
      printf("sunxi sprite error: download rawdata error %s\n", part_info->dl_filename);
      goto __download_normal_part_err1;
    }
    tmp_partdata_by_bytes   -= first_write_bytes;
    tmp_partstart_by_sector += onetime_read_sectors;

    while(tmp_partdata_by_bytes >= SPRITE_CARD_ONCE_DATA_DEAL)
    {
      //继续读出固件中的分区数据，大小为buffer字节数
      if(sunxi_flash_read(tmp_imgfile_start, SPRITE_CARD_ONCE_SECTOR_DEAL, down_buffer) != SPRITE_CARD_ONCE_SECTOR_DEAL)
      {
        printf("sunxi sprite error : read sdcard block %d, total %d failed\n", tmp_imgfile_start, SPRITE_CARD_ONCE_SECTOR_DEAL);
        goto __download_normal_part_err1;
      }
      //写入flash
      if(sunxi_sprite_write(tmp_partstart_by_sector, SPRITE_CARD_ONCE_SECTOR_DEAL, down_buffer) != SPRITE_CARD_ONCE_SECTOR_DEAL)
      {
        printf("sunxi sprite error: download rawdata error %s, start 0x%x, sectors 0x%x\n", part_info->dl_filename, tmp_partstart_by_sector, SPRITE_CARD_ONCE_SECTOR_DEAL);
        goto __download_normal_part_err1;
      }
      tmp_imgfile_start       += SPRITE_CARD_ONCE_SECTOR_DEAL;
      tmp_partdata_by_bytes   -= SPRITE_CARD_ONCE_DATA_DEAL;
      tmp_partstart_by_sector += SPRITE_CARD_ONCE_SECTOR_DEAL;
    }
    if(tmp_partdata_by_bytes > 0)
    {
      uint rest_sectors = (tmp_partdata_by_bytes + 511)>>9;
      //继续读出固件中的分区数据，大小为buffer字节数
      if(sunxi_flash_read(tmp_imgfile_start, rest_sectors, down_buffer) != rest_sectors)
      {
        printf("sunxi sprite error : read sdcard block %d, total %d failed\n", tmp_imgfile_start, rest_sectors);
        goto __download_normal_part_err1;
      }
      //写入flash
      if(sunxi_sprite_write(tmp_partstart_by_sector, rest_sectors, down_buffer) != rest_sectors)
      {
        printf("sunxi sprite error: download rawdata error %s, start 0x%x, sectors 0x%x\n", part_info->dl_filename, tmp_partstart_by_sector, rest_sectors);
        goto __download_normal_part_err1;
      }
    }
  }
  else
  {
    if(unsparse_direct_write(down_buffer, first_write_bytes))
    {
      printf("sunxi sprite error: download sparse error %s\n", part_info->dl_filename);
      goto __download_normal_part_err1;
    }
    tmp_partdata_by_bytes   -= first_write_bytes;

    while(tmp_partdata_by_bytes >= SPRITE_CARD_ONCE_DATA_DEAL)
    {
      //继续读出固件中的分区数据，大小为buffer字节数
      if(sunxi_flash_read(tmp_imgfile_start, SPRITE_CARD_ONCE_SECTOR_DEAL, down_buffer) != SPRITE_CARD_ONCE_SECTOR_DEAL)
      {
        printf("sunxi sprite error : read sdcard block 0x%x, total 0x%x failed\n", tmp_imgfile_start, SPRITE_CARD_ONCE_SECTOR_DEAL);
        goto __download_normal_part_err1;
      }
      //写入flash
      if(unsparse_direct_write(down_buffer, SPRITE_CARD_ONCE_DATA_DEAL))
      {
        printf("sunxi sprite error: download sparse error %s\n", part_info->dl_filename);
        goto __download_normal_part_err1;
      }
      tmp_imgfile_start       += SPRITE_CARD_ONCE_SECTOR_DEAL;
      tmp_partdata_by_bytes   -= SPRITE_CARD_ONCE_DATA_DEAL;
    }
    if(tmp_partdata_by_bytes > 0)
    {
      uint rest_sectors = (tmp_partdata_by_bytes + 511)>>9;
      //继续读出固件中的分区数据，大小为buffer字节数
      if(sunxi_flash_read(tmp_imgfile_start, rest_sectors, down_buffer) != rest_sectors)
      {
        printf("sunxi sprite error : read sdcard block 0x%x, total 0x%x failed\n", tmp_imgfile_start, rest_sectors);
        goto __download_normal_part_err1;
      }
      //写入flash
      if(unsparse_direct_write(down_buffer, tmp_partdata_by_bytes))
      {
        printf("sunxi sprite error: download sparse error %s\n", part_info->dl_filename);
        goto __download_normal_part_err1;
      }
    }
  }

  tick_printf("successed in writting part %s\n", part_info->name);
  ret = 0;
  if(imgitemhd)
  {
    Img_CloseItem(imghd, imgitemhd);
    imgitemhd = NULL;
  }
  //判断是否需要进行校验
  if(part_info->verify)
  {
    uint active_verify;
    uint origin_verify;
    uchar verify_data[1024];

    ret = -1;
    if(part_info->vf_filename[0])
    {
      imgitemhd = Img_OpenItem(imghd, "RFSFAT16", (char *)part_info->vf_filename);
      if(!imgitemhd)
      {
        printf("sprite update warning: open part %s failed\n", part_info->vf_filename);
        goto __download_normal_part_err1;
      }
      if(!Img_ReadItem(imghd, imgitemhd, (void *)verify_data, 1024))   //读出数据
      {
        printf("sprite update warning: fail to read data from %s\n", part_info->vf_filename);
        goto __download_normal_part_err1;
      }
      if(partdata_format == ANDROID_FORMAT_DETECT)
      {
        active_verify = sunxi_sprite_part_sparsedata_verify();
      }
      else
      {
        active_verify=SunxiSpritePartRawDataVerify(partstart_by_sector, partdata_by_byte);
        //active_verify = sunxi_sprite_part_rawdata_verify(partstart_by_sector, partdata_by_byte);
      }
      {
        uint *tmp = (uint *)verify_data;
        origin_verify = *tmp;
      }
      printf("origin_verify value = %x, active_verify value = %x\n", origin_verify, active_verify);
      if(origin_verify != active_verify)
      {
        printf("origin checksum=%x, active checksum=%x\n", origin_verify, active_verify);
        printf("sunxi sprite: part %a verify error\n", part_info->dl_filename);
        goto __download_normal_part_err1;
      }
      ret = 0;
    }
    else
    {
      printf("sunxi sprite err: part %s unablt to find verify file\n", part_info->dl_filename);
    }
    tick_printf("successed in verify part %s\n", part_info->name);
  }
  else
  {
    printf("sunxi sprite err: part %s not need to verify\n", part_info->dl_filename);
  }

__download_normal_part_err1:
  if(imgitemhd)
  {
    Img_CloseItem(imghd, imgitemhd);
    imgitemhd = NULL;
  }

  return ret;
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
static int __download_sysrecover_part(dl_one_part_info *part_info,  uchar *source_buff)
{
  uint partstart_by_sector;   //分区起始扇区
  uint tmp_partstart_by_sector;

  s64  partsize_by_byte;      //分区大小(字节单位)

  s64  partdata_by_byte;      //需要下载的分区数据(字节单位)
  s64  tmp_partdata_by_bytes;

  uint onetime_read_sectors;    //一次读写的扇区数

  uint imgfile_start;       //分区数据所在的扇区
  uint tmp_imgfile_start;

  u8 *down_buffer       = source_buff + SPRITE_CARD_HEAD_BUFF;

  int  ret = -1;
  //*******************************************************************
  //获取分区起始扇区
  tmp_partstart_by_sector = partstart_by_sector = part_info->addrlo;
  //获取分区大小，字节数
  partsize_by_byte     = part_info->lenlo;
  partsize_by_byte   <<= 9;
  //打开分区镜像

  //获取分区镜像字节数
  partdata_by_byte = Img_GetSize(imghd);
  if (partdata_by_byte <= 0)
  {
    printf("sunxi sprite error: fetch part len %s failed\n", part_info->dl_filename);
    goto __download_sysrecover_part_err1;
  }
  //如果分区数据超过分区大小
  if(partdata_by_byte > partsize_by_byte)
  {
    printf("sunxi sprite: data size 0x%x is larger than part %s size 0x%x\n", (uint)(partdata_by_byte/512), part_info->dl_filename, (uint)(partsize_by_byte/512));
    goto __download_sysrecover_part_err1;
  }
  //准备读取分区镜像数据
  tmp_partdata_by_bytes = partdata_by_byte;
  if(tmp_partdata_by_bytes >= SPRITE_CARD_ONCE_DATA_DEAL)
  {
    onetime_read_sectors = SPRITE_CARD_ONCE_SECTOR_DEAL;
  }
  else
  {
    onetime_read_sectors = (tmp_partdata_by_bytes + 511)>>9;
  }
  //开始获取分区数据
  imgfile_start = sprite_card_firmware_start();
  if(!imgfile_start)
  {
    printf("sunxi sprite err : cant get part data imgfile_start %s\n", part_info->dl_filename);
    goto __download_sysrecover_part_err1;
  }
  tmp_imgfile_start = imgfile_start;

  while(tmp_partdata_by_bytes >= SPRITE_CARD_ONCE_DATA_DEAL)
  {
    //继续读出固件中的分区数据，大小为buffer字节数
    if(sunxi_flash_read(tmp_imgfile_start, onetime_read_sectors, down_buffer) != onetime_read_sectors)
    {
      printf("sunxi sprite error : read sdcard block %d, total %d failed\n", tmp_imgfile_start, onetime_read_sectors);
      goto __download_sysrecover_part_err1;
    }
    //写入flash
    if(sunxi_sprite_write(tmp_partstart_by_sector, onetime_read_sectors, down_buffer) != onetime_read_sectors)
    {
      printf("sunxi sprite error: download rawdata error %s, start 0x%x, sectors 0x%x\n", part_info->dl_filename, tmp_partstart_by_sector, onetime_read_sectors);
      goto __download_sysrecover_part_err1;
    }
    tmp_imgfile_start       += onetime_read_sectors;
    tmp_partdata_by_bytes   -= onetime_read_sectors*512;
    tmp_partstart_by_sector += onetime_read_sectors;
  }
  if(tmp_partdata_by_bytes > 0)
  {
    uint rest_sectors = (tmp_partdata_by_bytes + 511)/512;
    //继续读出固件中的分区数据，大小为buffer字节数
    if(sunxi_flash_read(tmp_imgfile_start, rest_sectors, down_buffer) != rest_sectors)
    {
      printf("sunxi sprite error : read sdcard block %d, total %d failed\n", tmp_imgfile_start, rest_sectors);
      goto __download_sysrecover_part_err1;
    }
    //写入flash
    if(sunxi_sprite_write(tmp_partstart_by_sector, rest_sectors, down_buffer) != rest_sectors)
    {
      printf("sunxi sprite error: download rawdata error %s, start 0x%x, sectors 0x%x\n", part_info->dl_filename, tmp_partstart_by_sector, rest_sectors);
      goto __download_sysrecover_part_err1;
    }
  }
  ret = 0;

__download_sysrecover_part_err1:
  if(imgitemhd)
  {
    Img_CloseItem(imghd, imgitemhd);
    imgitemhd = NULL;
  }

  return ret;
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
int sunxi_sprite_deal_part(sunxi_download_info *dl_map)
{
    dl_one_part_info    *part_info;
  int         ret  = -1;
  int         ret1;
  int           i  = 0;
  uchar *down_buff         = NULL;
  int         rate;

  if(!dl_map->download_count)
  {
    printf("sunxi sprite: no part need to write\n");
    return 0;
  }
  rate = (70-10)/dl_map->download_count;
  //初始化flash，nand或者mmc
  #if 0
  if(sunxi_sprite_init(1))
  {
    printf("sunxi sprite err: init flash err\n");

    return -1;
  }
    #endif
  //申请内存
  down_buff = (uchar *)malloc(SPRITE_CARD_ONCE_DATA_DEAL + SPRITE_CARD_HEAD_BUFF);
  if(!down_buff)
  {
    printf("sunxi sprite err: unable to malloc memory for sunxi_sprite_deal_part\n");
    goto __sunxi_sprite_deal_part_err1;
  }
  for(part_info = dl_map->one_part_info, i = 0; i < dl_map->download_count; i++, part_info++)
  {
    tick_printf("begin to download part %s\n", part_info->name);
    if(!strncmp("UDISK", (char*)part_info->name, strlen("UDISK")))
    {
      ret1 = __download_udisk(part_info, down_buff);
      if(ret1 < 0)
      {
        printf("sunxi sprite err: sunxi_sprite_deal_part, download_udisk failed\n");
        goto __sunxi_sprite_deal_part_err2;
      }
      else if(ret1 > 0)
      {
        printf("do NOT need download UDISK\n");
      }
    }//如果是sysrecovery分区，烧录完整分区镜像
    else if(!strncmp("sysrecovery", (char*)part_info->name, strlen("sysrecovery")))
    {
      ret1 = __download_sysrecover_part(part_info, down_buff);
      if(ret1 != 0)
      {
        printf("sunxi sprite err: sunxi_sprite_deal_part, download sysrecovery failed\n");
        goto __sunxi_sprite_deal_part_err2;
      }
    }//如果是private分区，检查是否需要烧录
    else if(!strncmp("private", (char*)part_info->name, strlen("private")))
    {
      if(1)
      {
        //需要烧录此分区
        printf("NEED down private part\n");
        ret1 = __download_normal_part(part_info, down_buff);
        if(ret1 != 0)
        {
          printf("sunxi sprite err: sunxi_sprite_deal_part, download private failed\n");
          goto __sunxi_sprite_deal_part_err2;
        }
      }
      else
      {
        printf("IGNORE private part\n");
      }
    }
    else
    {
      ret1 = __download_normal_part(part_info, down_buff);
      if(ret1 != 0)
      {
        printf("sunxi sprite err: sunxi_sprite_deal_part, download normal failed\n");
        goto __sunxi_sprite_deal_part_err2;
      }
    }
    sprite_cartoon_upgrade(10 + rate * (i+1));
    tick_printf("successed in download part %s\n", part_info->name);
  }

  ret = 0;

__sunxi_sprite_deal_part_err1:
  //sunxi_sprite_exit(1);

__sunxi_sprite_deal_part_err2:

  if(down_buff)
  {
    free(down_buff);
  }

  return ret;
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
int sunxi_sprite_deal_uboot(void)
{
  //char buffer[1024 * 1024];
  char *buffer = NULL;
  uint item_original_size;

  buffer = malloc(4*1024*1024);
  if(buffer == NULL)
  {
    printf("sunxi sprite malloc for uefi fail\n");
    return -1;
  }

  imgitemhd = Img_OpenItem(imghd, "12345678", "UBOOT_0000000000");
  if(!imgitemhd)
  {
    printf("sprite update error: fail to open uboot item\n");
    goto __ERR_END;
  }
  //uboot长度
  item_original_size = Img_GetItemSize(imghd, imgitemhd);
  if(!item_original_size)
  {
    printf("sprite update error: fail to get uboot item size\n");
    goto __ERR_END;
  }
  /*获取uboot的数据*/
  if(!Img_ReadItem(imghd, imgitemhd, (void *)buffer, 4*1024 * 1024))
  {
    printf("update error: fail to read data from for uboot\n");
    goto __ERR_END;
  }
  Img_CloseItem(imghd, imgitemhd);
  imgitemhd = NULL;

  if(sunxi_sprite_download_uboot(buffer))
  {
    printf("update error: fail to write uboot\n");
    goto __ERR_END;
  }
  printf("sunxi_sprite_deal_uboot ok\n");

  return 0;
__ERR_END:
  if(buffer)
    free(buffer);
  
  return -1;    
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
int sunxi_sprite_deal_boot0(void)
{
  //char buffer[32 * 1024];
  char *buffer = NULL;
  uint item_original_size;
    
  buffer = malloc(32*1024);
  if(buffer == NULL)
  {
    printf("sunxi sprite malloc for boot0 fail\n");
    goto __ERR_END;
  }

  if(SunxiGetBurnStorage()== SunxiBurnStorageNand)
  {
    imgitemhd = Img_OpenItem(imghd, "BOOT    ", "BOOT0_0000000000");
  }
  else
  {
    imgitemhd = Img_OpenItem(imghd, "12345678", "1234567890BOOT_0");
  }
  if(!imgitemhd)
  {
    printf("sprite update error: fail to open boot0 item\n");
    goto __ERR_END;
  }
  //boot0长度
  item_original_size = Img_GetItemSize(imghd, imgitemhd);
  if(!item_original_size)
  {
    printf("sprite update error: fail to get boot0 item size\n");
    goto __ERR_END;
  }
  /*获取boot0的数据*/
  if(!Img_ReadItem(imghd, imgitemhd, (void *)buffer, 32 * 1024))
  {
    printf("update error: fail to read data from for boot0\n");
    goto __ERR_END; 
  }
  Img_CloseItem(imghd, imgitemhd);
  imgitemhd = NULL;

  if(sunxi_sprite_download_boot0(buffer))
  {
    printf("update error: fail to write boot0\n");
    goto __ERR_END;
  }

  return 0;
    
__ERR_END:
  if(buffer)
    free(buffer);
  return -1;
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :  一键恢复烧写分区
*
*    parmeters     :
*
*    return        :
*
*    note          :  yanjianbo@allwinnertech.com
*
*
************************************************************************************************************
*/
int sunxi_sprite_deal_part_from_sysrevoery(sunxi_download_info *dl_map)
{
  dl_one_part_info    *part_info;
  int         ret  = -1;
  int         ret1;
  int           i  = 0;
  uchar *down_buff         = NULL;
  int         rate;

  if(!dl_map->download_count)
  {
    printf("sunxi sprite: no part need to write\n");
    return 0;
  }
  rate = (80)/dl_map->download_count;
  //初始化flash，nand或者mmc
  //
/*
  if(sunxi_sprite_init(1))
  {
    printf("sunxi sprite err: init flash err\n");

    return -1;
  }
*/  
  //申请内存
  down_buff = (uchar *)malloc(SPRITE_CARD_ONCE_DATA_DEAL + SPRITE_CARD_HEAD_BUFF);
  if(!down_buff)
  {
    printf("sunxi sprite err: unable to malloc memory for sunxi_sprite_deal_part\n");
    goto __sunxi_sprite_deal_part_err1;
  }

  for(part_info = dl_map->one_part_info, i = 0; i < dl_map->download_count; i++, part_info++)
  {
    tick_printf("begin to download part %s\n", part_info->name);
    if (!strcmp("env", (const char *)part_info->name))
    {
      printf("env part do not need to rewrite\n");
      sprite_cartoon_upgrade(20 + rate * (i+1));
      continue;
    }
    else if (!strcmp("sysrecovery", (const char *)part_info->name))
    {
      printf("THIS_IMG_SELF_00 do not need to rewrite\n");
      sprite_cartoon_upgrade(20 + rate * (i+1));
      continue;
    }
    else if (!strcmp("UDISK", (const char *)part_info->name))
    {
      printf("UDISK do not need to rewrite\n");
      sprite_cartoon_upgrade(20 + rate * (i+1));
      continue;
    }
    else if (!strcmp("private", (const char *)part_info->name))
    {
      printf("private do not need to rewrite\n");
      sprite_cartoon_upgrade(20 + rate * (i+1));
      continue;
    }
    else
    {
      ret1 = __download_normal_part(part_info, down_buff);
      if(ret1 != 0)
      {
        printf("sunxi sprite err: sunxi_sprite_deal_part, download normal failed\n");
        goto __sunxi_sprite_deal_part_err2;
      }
    }
    sprite_cartoon_upgrade(20 + rate * (i+1));
    tick_printf("successed in download part %s\n", part_info->name);
  }

  ret = 0;

__sunxi_sprite_deal_part_err1:
  sunxi_sprite_exit(1);

__sunxi_sprite_deal_part_err2:

  if(down_buff)
  {
    free(down_buff);
  }

  return ret;
}


/*
************************************************************************************************************
*
*                                             function
*
*    name          :  这个函数主要主要为了用sprite的函数接口
*
*    parmeters     :
*
*    return        :
*
*    note          :  yanjianbo@allwinnertech.com
*
*
************************************************************************************************************
*/
int __imagehd(HIMAGE tmp_himage)
{
  imghd = tmp_himage;
  if (imghd)
  {
    return 0;
  }
  return -1;
}

