/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef  __toc_v2_h
#define  __toc_v2_h


#include "spare_head.h"

#define  TOC0_MAGIC             "TOC0.GLH"
#define  TOC_MAIN_INFO_MAGIC    0x89119800

#define  SECURE_SWITCH_OTHER   0
#define  SECURE_SWITCH_NORMAL  1
#define  SECURE_NON_SECUREOS   2

//增加安全启动下，toc0的头部数据结构
typedef struct
{
  unsigned char  name[8];   //字符串，可以更改，没有作限制
  unsigned int magic;       //必须是0x89119800
  unsigned int check_sum;    //整个数据的校验和，参考现在boot0做法

  unsigned int serial_num;   //序列号，可以更改，没有限制
  unsigned int status;       //可以更改，没有限制

  unsigned int items_nr;    //总的项目个数，对TOC0来说，必须是2
  unsigned int length;        //TOC0的长度
  unsigned char  platform[4];  //toc_platform[0]标示启动介质
                      //0：nand；1：卡0；2：卡2；3：spinor
  unsigned int reserved[2];  //保留位
  unsigned int end;          //表示头部结构体结束，必须是0x3b45494d

}
toc0_private_head_t;

//增加安全启动下，toc1的头部数据结构
typedef struct sbrom_toc1_head_info
{
  char name[16] ; //user can modify
  unsigned int  magic ; //must equal TOC_U32_MAGIC
  unsigned int  add_sum ;

  unsigned int  serial_num  ; //user can modify
  unsigned int  status    ; //user can modify,such as TOC_MAIN_INFO_STATUS_ENCRYP_NOT_USED

  unsigned int  items_nr; //total entry number
  unsigned int  valid_len;
  unsigned int  version_main; //only one byte
  unsigned int  version_sub;   //two bytes
  unsigned int  reserved[3];  //reserved for future

  unsigned int  end;
}
sbrom_toc1_head_info_t;


typedef struct sbrom_toc1_item_info
{
  char name[64];      //such as ITEM_NAME_SBROMSW_CERTIF
  unsigned int  data_offset;
  unsigned int  data_len;
  unsigned int  encrypt;      //0: no aes   //1: aes
  unsigned int  type;       //0: normal file, dont care  1: key certif  2: sign certif 3: bin file
  unsigned int  run_addr;          //if it is a bin file, then run on this address; if not, it should be 0
  unsigned int  index;             //if it is a bin file, this value shows the index to run; if not
                         //if it is a certif file, it should equal to the bin file index
                         //that they are in the same group
                         //it should be 0 when it anyother data type
  unsigned int  reserved[69];    //reserved for future;
  unsigned int  end;
}sbrom_toc1_item_info_t;


typedef struct sbrom_toc0_config
{
  unsigned char     config_vsn[4];
  unsigned int        dram_para[32];    // dram参数
  int           uart_port;        // UART控制器编号
  normal_gpio_cfg     uart_ctrl[2];     // UART控制器GPIO
  int               enable_jtag;      // JTAG使能
  normal_gpio_cfg     jtag_gpio[5];     // JTAG控制器GPIO
  normal_gpio_cfg   storage_gpio[50];   // 存储设备 GPIO信息
                              // 0-23放nand，24-31存放卡0，32-39放卡2
                              // 40-49存放spi
  char          storage_data[384];  // 0-159,存储nand信息；160-255,存放卡信息
  unsigned int       secure_dram_mbytes; //
  unsigned int       drm_start_mbytes;   //
  unsigned int       drm_size_mbytes;    //
  unsigned int       boot_cpu;           //
  special_gpio_cfg    a15_power_gpio;  //the gpio config is to a15 extern power enable gpio
  unsigned int       next_exe_pa;
    unsigned int       secure_without_OS;   //secure boot without semelis
    unsigned int       debug_mode;         //1:turn on printf; 0 :turn off printf
  unsigned int    card_work_mode;
  unsigned int        res[2];         // 总共1024字节

}
sbrom_toc0_config_t;

#define ITEM_SCP_NAME             "scp"
#define ITEM_MONITOR_NAME         "monitor"
#define ITEM_UBOOT_NAME           "u-boot"

#endif     //  ifndef __toc_h

/* end of toc.h */
