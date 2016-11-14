/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  Martin.Zheng <martinzheng@allwinnertech.com>
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
#include "mmc_def.h"
#include "mmc.h"
#define EMMC_BOOT_START_BLOCK (0)
#define SD_BOOT_START_BLOCK (16)

static unsigned bootcard_offset;

extern int sunxi_mmc_init(int sdc_no);
extern int sunxi_mmc_exit(int sdc_no);

INT32 SDMMC_PhyInit(UINT32 card_no)
{
  int ret = 0;
  struct mmc *mmc = NULL;

  MMCINFO("gch card%d SDMMC_PhyInit start\n",card_no);

  ret = sunxi_mmc_init(card_no);
  if ( ret < 0) {
    MMCINFO("SDMMC_PhyInit:Init SD/MMC card failed !\n");
    return -1;
  }
  MMCDBG("SDMMC_PhyInit:Phy Init SD/MMC card succeed!\n");

#ifndef NO_USE_BOOT0
  mmc = find_mmc_device(card_no);
  if (!(mmc->part_support & PART_SUPPORT)) {
    MMCINFO("Card doesn't support part_switch\n");
    MMCDBG("card%d SDMMC_PhyInit end\n",card_no);
    return 0;
  }
#else
  return 0;
#endif

  ret = mmc_switch_boot_bus_cond(card_no, MMC_SWITCH_BOOT_SDR_NORMAL, MMC_SWITCH_BOOT_RST_BUS_COND, MMC_SWITCH_BOOT_BUS_SDRx4_DDRx4);
  if (ret) {
    MMCINFO("SDMMC_PhyInit:mmc switch boot cond failed\n");
    ret = -1;
    goto out;
  }
  MMCDBG("SDMMC_PhyInit:mmc switch boot cond succeed\n");

  ret = mmc_switch_boot_part(card_no, MMC_SWITCH_PART_BOOT_ACK_ENB, MMC_SWITCH_PART_BOOT_PART_1); //正常模式的配置
  if (ret) {
    MMCINFO("SDMMC_PhyInit:mmc switch boot part failed\n");
    ret = -1;
    goto out;
  }
  MMCDBG("SDMMC_PhyInit:mmc switch boot part succeed\n");

  ret = mmc_switch_part(card_no, MMC_SWITCH_PART_BOOT_PART_1);
  if (ret) {
    MMCINFO("SDMMC_PhyInit:mmc switch access boot0 failed\n");
    ret = -1;
    goto out;
  }

  MMCDBG("SDMMC_PhyInit:mmc switch access boot0 succeed\n");

  mmc->part_num = BOOT1_PART;//boot0
  mmc->lba = mmc->boot1_lba;
  MMCDBG("SDMMC_PhyInit:part num:%d\n",mmc->part_num);
  MMCDBG("card%d SDMMC_PhyInit end\n",card_no);
out:  
  return ret;
}

INT32 SDMMC_PhyExit(UINT32 card_no)
{
  MMCDBG("SDMMC_PhyExit start\n");
  sunxi_mmc_exit(card_no);
  MMCDBG("SDMMC_PhyExit end\n");
  return 0;
}

INT32 SDMMC_PhyRead(UINT32 start_sector, UINT32 nsector, void *buf, UINT32 card_no)
{
  struct mmc *mmc = find_mmc_device(card_no);
  int ret = 0;
  MMCDBG("SDMMC_PhyRead start\n");

#ifndef NO_USE_BOOT0
  if (!(mmc->part_support & PART_SUPPORT))
#endif
  {
    MMCDBG("Card doesn't support part_switch\n");
    ret = mmc_bread(card_no, start_sector, nsector, buf);
    if(!ret)
      ret = -1;
    if(ret==-1)
      MMCINFO("phy read failed\n");
    MMCDBG("SDMMC_PhyRead end\n");
    return ret;
  }

  if(mmc->part_num!=BOOT1_PART){
    MMCINFO("SDMMC_PhyRead part_num%d should be %d\n",mmc->part_num,BOOT1_PART);
    return -1;//
  }

  //eMMC boot from boot0 0 sector
  if(start_sector<(EMMC_BOOT_START_BLOCK+SD_BOOT_START_BLOCK)){
    MMCINFO("SDMMC_PhyRead wrong start_sector%d\n",start_sector);
    return -1;//
  }

  ret = mmc_bread(card_no, start_sector-SD_BOOT_START_BLOCK, nsector, buf);
  if(!ret)
    ret = -1;
  if(ret==-1)
    MMCINFO("phy read failed\n"); 
  MMCDBG("SDMMC_PhyRead end\n");
  return ret;
}

INT32 SDMMC_PhyWrite(UINT32 start_sector, UINT32 nsector, void *buf, UINT32 card_no)
{
  struct mmc *mmc = find_mmc_device(card_no);
  int ret = 0;
  MMCDBG("SDMMC_PhyWrite start\n");

#ifndef NO_USE_BOOT0
  if (!(mmc->part_support & PART_SUPPORT))
#endif
  {
    MMCDBG("Card doesn't support part_switch\n");
    ret = mmc_bwrite(card_no, start_sector, nsector, buf);
    if(!ret)
      ret = -1;
    if(ret==-1)
      MMCINFO("phy write failed\n");    
    MMCDBG("SDMMC_PhyWrite end\n");
    return ret;
  }

  if(mmc->part_num!=BOOT1_PART){
    MMCINFO("SDMMC_PhyWrite part_num%d should be %d\n",mmc->part_num,BOOT1_PART);
    return -1;//
  }

  //eMMC boot from boot0 0 sector
  if(start_sector<(EMMC_BOOT_START_BLOCK+SD_BOOT_START_BLOCK)){
    MMCINFO("SDMMC_PhyWrite wrong start_sector%d\n",start_sector);
    return -1;//
  }

  ret = mmc_bwrite(card_no, start_sector-SD_BOOT_START_BLOCK, nsector, buf);
  if(!ret)
    ret = -1; 
  if(ret==-1)
    MMCINFO("phy write failed\n");    
  MMCDBG("SDMMC_PhyWrite end\n");
  return ret;
}

INT32 SDMMC_PhyDiskSize(UINT32 card_no)
{
  struct mmc *mmc = find_mmc_device(card_no);
  MMCDBG("SDMMC_PhyDiskSize start\n");

#ifndef NO_USE_BOOT0
  if (!(mmc->part_support & PART_SUPPORT))
#endif
  {
    MMCDBG("Card doesn't support part_switch\n");
    mmc->lba = mmc->user_lba;
    MMCDBG("SDMMC_PhyDiskSize end\n");
    return mmc->lba;;
  }

  if(mmc->part_num!=BOOT1_PART){
    MMCINFO("SDMMC_PhyDiskSize part_num%d should be %d\n",mmc->part_num,BOOT1_PART);
    return -1;//
  }

  mmc->lba = mmc->boot1_lba;
  MMCDBG("SDMMC_PhyDiskSize end\n");
  return mmc->lba;
}

INT32 SDMMC_PhyErase(UINT32 block, UINT32 nblock, UINT32 card_no)
{
  struct mmc *mmc = find_mmc_device(card_no);
  int ret = 0;
  MMCDBG("card%d SDMMC_PhyErase start\n",card_no);

  if(mmc==NULL){
    MMCINFO("SDMMC_PhyErase mmc should not be NULL\n");
    return -1;
  }

#ifndef NO_USE_BOOT0
  if (!(mmc->part_support & PART_SUPPORT))
#endif
  {
    MMCDBG("Card doesn't support part_switch\n");
    ret = mmc_berase(card_no, block, nblock);
    if(!ret)
      ret = -1; 
    if(ret==-1)
      MMCINFO("phy Erase failed\n");    
    MMCDBG("SDMMC_PhyErase end\n");
    return ret;
  }

  if(mmc->part_num!=BOOT1_PART){
    MMCINFO("SDMMC_PhyErase part_num%d should be %d",mmc->part_num,BOOT1_PART);
    return -1;//
  }

  //eMMC boot from boot0 0 sector
  if(block<(EMMC_BOOT_START_BLOCK+SD_BOOT_START_BLOCK)){
    MMCINFO("SDMMC_PhyErase wrong start_sector%d\n",block);
    return -1;//
  }

  ret = mmc_berase(card_no, block-SD_BOOT_START_BLOCK, nblock);
  if(!ret)
    ret = -1;
  if(ret==-1)
    MMCINFO("phy Erase failed\n");    
  MMCDBG("card%d SDMMC_PhyErase end\n",card_no);
  return ret;
}

INT32 SDMMC_LogicalInit(UINT32 card_no, UINT32 card_offset)
{
  INT32 ret = 0;
  struct mmc *mmc_dev = NULL;
  //struct mmc *mmc = find_mmc_device(card_no);
  MMCINFO("gch SDMMC_LogicalInit start\n");

  bootcard_offset = card_offset;

  ret = sunxi_mmc_init(card_no);
  if ( ret < 0) {
    MMCINFO("SDMMC_LogicalInit:platform init failed !\n");
    return -1;
  }
  mmc_dev = find_mmc_device(card_no);
  if (mmc_init(mmc_dev)) {
    MMCINFO("SDMMC_LogicalInit:Init SD/MMC card failed \n");
    return  -1;
  }

  MMCDBG("SDMMC_LogicalInit:mmc switch access part user succeed\n");
  MMCDBG("SDMMC_LogicalInit end\n");
  return 0;
}

INT32 SDMMC_LogicalExit(UINT32 card_no)
{
  int ret = 0;
  bootcard_offset = 0;
  MMCDBG("SDMMC_LogicalExit start\n");
  ret = SDMMC_PhyExit(card_no);
  if(ret)
    MMCINFO("LogicalExit failed\n");
  MMCDBG("SDMMC_LogicalExit end\n");
  return ret;
}

lbaint_t SDMMC_LogicalLBA(UINT32 card_no)
{
  struct mmc *mmc_dev = NULL;
  mmc_dev = find_mmc_device(card_no);
  return mmc_dev->block_dev.lba;
}

INT32 SDMMC_LogicalRead(UINT32 start_sector, UINT32 nsector, void *buf, UINT32 card_no)
{
  struct mmc *mmc = find_mmc_device(card_no);
  int ret = 0;
  MMCDBG("SDMMC_LogicalRead start\n");
  if(mmc->part_num!=USER_PART){
    MMCINFO("SDMMC_LogicalRead part_num%d should be %d",mmc->part_num,USER_PART);
    return -1;//
  }
  ret = mmc_bread(card_no, start_sector + bootcard_offset, nsector, buf);
  if(!ret)
    ret = -1;
  if(ret==-1)
    MMCINFO("LogicalRead failed\n");  
  MMCDBG("SDMMC_LogicalRead end\n");
  return ret;
}

INT32 SDMMC_LogicalWrite(UINT32 start_sector, UINT32 nsector, void *buf, UINT32 card_no)
{
  struct mmc *mmc = find_mmc_device(card_no);
  int ret = 0;
  MMCDBG("SDMMC_LogicalWrite start\n");
  if(mmc->part_num!=USER_PART){
    MMCINFO("SDMMC_LogicalWrite part_num%d should be %d",mmc->part_num,USER_PART);
    return -1;//
  }
  ret = mmc_bwrite(card_no, start_sector + bootcard_offset, nsector, buf);
  if(!ret)
    ret = -1; 
  if(ret==-1)
    MMCINFO("LogicalWrite failed\n");
  
  MMCDBG("SDMMC_LogicalWrite end\n");
  return ret;
}

INT32 SDMMC_LogicalDiskSize(UINT32 card_no)
{
  struct mmc *mmc = find_mmc_device(card_no);
  MMCDBG("SDMMC_LogicalDiskSize start\n");
  if(mmc->part_num!=USER_PART){
    MMCINFO("SDMMC_LogicalDiskSize part_num%d should be %d",mmc->part_num,USER_PART);
    return -1;//
  }

  mmc->lba = mmc->user_lba;
  MMCDBG("SDMMC_LogicalDiskSize end\n");
  return mmc->lba-bootcard_offset;
}

INT32 SDMMC_LogicaErase(UINT32 block, UINT32 nblock, UINT32 card_no)
{
  struct mmc *mmc = find_mmc_device(card_no);
  int ret = 0;
  MMCDBG("card%d SDMMC_LogicaErase start\n",card_no);
  if(mmc->part_num!=USER_PART){
    MMCINFO("SDMMC_LogicaErase part_num%d should be %d",mmc->part_num,USER_PART);
    return -1;//
  }
  ret = mmc_berase(card_no, block + bootcard_offset, nblock);
  if(!ret)
    ret = -1; 
  if(ret==-1)
    MMCINFO("LogicaErase failed\n");  
  MMCDBG("card%d SDMMC_LogicaErase end\n",card_no);
  return ret;
}

INT32 SDMMC_BootFileWrite(UINT32 start_sector, UINT32 nsector, void *buf, UINT32 card_no)
{
  struct mmc *mmc = find_mmc_device(card_no);
  int ret = 0;
  MMCDBG("card%d SDMMC_BootFileWrite start\n",card_no);

  ret = mmc->block_dev.block_write_mass_pro(card_no,start_sector,nsector,buf);
  if(!ret)
    ret = -1; 
  if(ret==-1)
    MMCINFO("SDMMC_BootFileWrite failed\n");  
  MMCDBG("card%d SDMMC_BootFileWrite end\n",card_no);
  return ret;
}
