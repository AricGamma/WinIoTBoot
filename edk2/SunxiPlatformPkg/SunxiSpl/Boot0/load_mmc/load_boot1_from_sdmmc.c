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

#include <Interinc/private_uefi.h>
#include <Interinc/private_toc.h>

#include <boot0_include/config.h>
#include <boot0_include/boot0_helper.h>

#include "mmc.h"

extern signed int check_magic( signed int *mem_base, const char *magic );
extern int verify_addsum( void *mem_base, unsigned int size );


extern const boot0_file_head_t  *boot0_head;


typedef struct _boot_sdcard_info_t
{
  signed int  card_ctrl_num;                //总共的卡的个数
  signed int  boot_offset;                  //指定卡启动之后，逻辑和物理分区的管理
  signed int  card_no[4];                   //当前启动的卡号, 16-31:GPIO编号，0-15:实际卡控制器编号
  signed int  speed_mode[4];                //卡的速度模式，0：低速，其它：高速
  signed int  line_sel[4];                  //卡的线制，0: 1线，其它，4线
  signed int  line_count[4];                //卡使用线的个数
}
boot_sdcard_info_t;

//card num: 0-sd 1-card3 2-emmc
int GetCardNum(void)
{
  int card_num = 0;

  card_num = boot0_head->boot_head.platform[0] & 0xf;
  card_num = (card_num == 1)? 3: card_num;
  return card_num;
}

void UpdateFlashPara(void)
{
  int card_num;
  struct spare_boot_head_t  *bfh = (struct spare_boot_head_t *) CONFIG_SYS_TEXT_BASE;
  card_num = GetCardNum();
  if(card_num == 0)
  {
    bfh->boot_data.storage_type = STORAGE_SD;
  }
  else if(card_num == 2)
  {
    bfh->boot_data.storage_type = STORAGE_EMMC;
    set_mmc_para(2,(void *)&boot0_head->prvt_head.storage_data);
  }
  else if(card_num == 3)
  {
    bfh->boot_data.storage_type = STORAGE_EMMC3;
    set_mmc_para(3,(void *)&boot0_head->prvt_head.storage_data);
  }
}

int LoadToc1FromSdmmc(char *buf)
{
  unsigned char *tmp_buff = (unsigned char *)CONFIG_BOOTPKG_STORE_IN_DRAM_BASE;
  unsigned total_size;
  sbrom_toc1_head_info_t  *toc1_head;
  int  card_no;
  int ret =0;
  int start_sector;
  boot_sdcard_info_t  *sdcard_info = (boot_sdcard_info_t *)buf;

  card_no = GetCardNum();
  start_sector = UBOOT_START_SECTOR_IN_SDMMC;

  printf("card no is %d\n", card_no);
  if(card_no < 0)
  {
    printf("bad card number %d in card boot\n", card_no);
    goto __ERROR_EXIT;
  }

  if(!sdcard_info->line_sel[card_no])
  {
    sdcard_info->line_sel[card_no] = 4;
  }
  printf("sdcard %d line count %d\n", card_no, sdcard_info->line_sel[card_no]);

  if( SunxiMmcInit(card_no, sdcard_info->line_sel[card_no], boot0_head->prvt_head.storage_gpio, 16, (void *)(sdcard_info) ) == -1) 
  {
    printf("sunxi_flash_init err: sunxi_mmc_init failed\n");
    goto __ERROR_EXIT;;
  }
  //read 64 sectors 
  ret = MmcBread(card_no, start_sector, 64, tmp_buff);
  if(!ret)
  {
    printf("PANIC : sunxi_flash_init() error --1--\n");
    goto __ERROR_EXIT;
  }
  toc1_head = (struct sbrom_toc1_head_info *)tmp_buff;
  if(toc1_head->magic != TOC_MAIN_INFO_MAGIC)
  {
    printf("PANIC : sunxi_flash_init() error --2--,toc1 magic error\n");
    goto __ERROR_EXIT;
  }
  total_size = toc1_head->valid_len;
  if(total_size > 64 * 512)
  {
    tmp_buff += 64*512;
    ret = MmcBread(card_no, start_sector + 64, (total_size - 64*512 + 511)/512, tmp_buff);
    if(!ret)
    {
      printf("PANIC : sunxi_flash_init() error --3--\n");
      goto __ERROR_EXIT;
    }
  }

  if( verify_addsum( (unsigned int *)CONFIG_BOOTPKG_STORE_IN_DRAM_BASE, total_size) != 0 )
  {
    printf("Fail in checking uboot.\n");
    goto __ERROR_EXIT;
  }
  printf("Succeed in loading uboot from sdmmc flash.\n");
  SunxiMmcExit( card_no, boot0_head->prvt_head.storage_gpio, 16 );
  return 0;

__ERROR_EXIT:
  SunxiMmcExit(card_no, boot0_head->prvt_head.storage_gpio, 16 );
  return -1;

}


int LoadBoot1(void)
{
  memcpy((void *)DRAM_PARA_STORE_ADDR, (void *)boot0_head->prvt_head.dram_para, 
    SUNXI_DRAM_PARA_MAX * 4);

  return LoadToc1FromSdmmc((char *)boot0_head->prvt_head.storage_data);
}
