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


#ifndef __DOS_PART_H_
#define __DOS_PART_H_

//标准分区信息
#pragma pack(push, 1)
typedef struct tag_part_info_stand
{
  char        indicator;      //表示该分区是否是活动分区
  char        start_head;     //分区开始的磁头
  short       start_sector:6;   //分区开始的扇区
  short       start_cylinder:10;  //分区开始的柱面
  char        part_type;      //分区类型      00H:没有指明  01H:DOS12 02H:xenix 04H:DOS16 05H:扩展分区  06H:FAT16
                      //          07H:NTFS    0BH:FAT32
  char        end_head;     //分区结束的磁头
  short       end_sector:6;   //分区结束的扇区
  short       end_cylinder:10;  //分区结束的柱面
  //int         start_sectors;    //起始扇区，前面说的扇区是硬盘的概念，这里的扇区是逻辑概念
  //int         total_sectors;    //分区中的扇区总数
  short       start_sectorl;
  short       start_sectorh;
  short       total_sectorsl;
  short       total_sectorsh;
}
part_info_stand;
#pragma pack(pop)

//标准MBR
#pragma pack(push, 1)
typedef struct tag_mbr_stand
{
  char        mbr[0x89];      //主引导记录
  char        err_info[0x135];  //出错信息
  part_info_stand   part_info[4];   //分区表项
  short       end_flag;     //固定值 0x55aa
}
mbr_stand;
#pragma pack(pop)


#endif  /* __BOOT_STANDBY_H_ */
