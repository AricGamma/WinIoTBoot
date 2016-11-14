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

//��׼������Ϣ
#pragma pack(push, 1)
typedef struct tag_part_info_stand
{
  char        indicator;      //��ʾ�÷����Ƿ��ǻ����
  char        start_head;     //������ʼ�Ĵ�ͷ
  short       start_sector:6;   //������ʼ������
  short       start_cylinder:10;  //������ʼ������
  char        part_type;      //��������      00H:û��ָ��  01H:DOS12 02H:xenix 04H:DOS16 05H:��չ����  06H:FAT16
                      //          07H:NTFS    0BH:FAT32
  char        end_head;     //���������Ĵ�ͷ
  short       end_sector:6;   //��������������
  short       end_cylinder:10;  //��������������
  //int         start_sectors;    //��ʼ������ǰ��˵��������Ӳ�̵ĸ��������������߼�����
  //int         total_sectors;    //�����е���������
  short       start_sectorl;
  short       start_sectorh;
  short       total_sectorsl;
  short       total_sectorsh;
}
part_info_stand;
#pragma pack(pop)

//��׼MBR
#pragma pack(push, 1)
typedef struct tag_mbr_stand
{
  char        mbr[0x89];      //��������¼
  char        err_info[0x135];  //������Ϣ
  part_info_stand   part_info[4];   //��������
  short       end_flag;     //�̶�ֵ 0x55aa
}
mbr_stand;
#pragma pack(pop)


#endif  /* __BOOT_STANDBY_H_ */
