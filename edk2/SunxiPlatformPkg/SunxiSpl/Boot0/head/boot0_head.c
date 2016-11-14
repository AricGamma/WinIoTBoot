/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  tangmanliang <tangmanliang@allwinnertech.com>
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


#include "private_boot0.h"
#include "config.h"


const boot0_file_head_t  BT0_head = 
{
  {
    /* jump_instruction */         
    ( 0xEA000000 | ( ( ( 0x184 + sizeof( boot0_file_head_t ) + sizeof( int ) - 1 ) / sizeof( int ) - 2 ) & 0x00FFFFFF ) ),
    BOOT0_MAGIC,
    STAMP_VALUE,
#ifdef ALIGN_SIZE_8K
    0x2000,
#else
    0x4000,
#endif
    sizeof( boot_file_head_t ),
    BOOT_PUB_HEAD_VERSION,
    CONFIG_BOOT0_RET_ADDR,
    CONFIG_BOOT0_RUN_ADDR,
    {0},
    {
      //brom modify: nand-4bytes sdmmc-2bytes
      0, 0,0,0, '4','.','0',0
    }
  },

  {
    //__u32 prvt_head_size;
    0,
    //char prvt_head_vsn[4];      
    {1},
    //unsigned int                dram_para[32] ; 
    {0},
    //__s32          uart_port;   
    0,
    //normal_gpio_cfg       uart_ctrl[2];  
    {
      { 6, 2, 4, 1, 1, 0, {0}},//PB8: 4--RX
      { 6, 4, 4, 1, 1, 0, {0}},//PB9: 4--TX
    },
    //__s32                         enable_jtag;  
    0,
    //normal_gpio_cfg       jtag_gpio[5];   
    {{0},{0},{0},{0},{0}},
    //normal_gpio_cfg        storage_gpio[32]; 
    {
      { 0, 0, 2, 1, 2, 0, {0}},//PF0-5: 2--SDC
      { 0, 1, 2, 1, 2, 0, {0}},
      { 0, 2, 2, 1, 2, 0, {0}},
      { 0, 3, 2, 1, 2, 0, {0}},
      { 0, 4, 2, 1, 2, 0, {0}},
      { 0, 5, 2, 1, 2, 0, {0}},
    },
    //char                             storage_data[512 - sizeof(normal_gpio_cfg) * 32]; 
    {0}
  }
};


/*******************************************************************************
*
*                  ����Boot_file_head�е�jump_instruction�ֶ�
*
*  jump_instruction�ֶδ�ŵ���һ����תָ�( B  BACK_OF_Boot_file_head )������
*תָ�ִ�к󣬳�����ת��Boot_file_head�����һ��ָ�
*
*  ARMָ���е�Bָ��������£�
*          +--------+---------+------------------------------+
*          | 31--28 | 27--24  |            23--0             |
*          +--------+---------+------------------------------+
*          |  cond  | 1 0 1 0 |        signed_immed_24       |
*          +--------+---------+------------------------------+
*  ��ARM Architecture Reference Manual�����ڴ�ָ�������½��ͣ�
*  Syntax :
*  B{<cond>}  <target_address>
*    <cond>    Is the condition under which the instruction is executed. If the
*              <cond> is ommitted, the AL(always,its code is 0b1110 )is used.
*    <target_address>
*              Specified the address to branch to. The branch target address is
*              calculated by:
*              1.  Sign-extending the 24-bit signed(wro's complement)immediate
*                  to 32 bits.
*              2.  Shifting the result left two bits.
*              3.  Adding to the contents of the PC, which contains the address
*                  of the branch instruction plus 8.
*
*  �ɴ˿�֪����ָ���������8λΪ��0b11101010����24λ����Boot_file_head�Ĵ�С��
*̬���ɣ�����ָ�����װ��������:
* 0x184��Ϊ������UEFI FD��ͷ��
*  ( sizeof( boot_file_head_t ) + sizeof( int ) - 1 ) / sizeof( int )
*                                              ����ļ�ͷռ�õġ��֡��ĸ���
*  - 2                                         ��ȥPCԤȡ��ָ������
*  & 0x00FFFFFF                                ���signed-immed-24
*  | 0xEA000000                                ��װ��Bָ��
*
*******************************************************************************/

