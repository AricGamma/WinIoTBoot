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
#include "string.h"

#include "mmc_def.h"

#include "mmc.h"

/* Set block count limit because of 16 bit register limit on some hardware*/
#ifndef CONFIG_SYS_MMC_MAX_BLK_COUNT
#define CONFIG_SYS_MMC_MAX_BLK_COUNT 65535
#endif

static struct mmc* mmc_devices[MAX_MMC_NUM];

unsigned long mmc_bread(int dev_num, unsigned long start, unsigned long blkcnt, void *dst);

//LIST_HEAD(mmc_devices);
//static LIST_ENTRY mmc_devices;

int __board_mmc_getcd(u8 *cd, struct mmc *mmc) {
  return -1;
}

int board_mmc_getcd(u8 *cd, struct mmc *mmc)__attribute__((weak,
  alias("__board_mmc_getcd")));

int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
#ifdef CONFIG_MMC_TRACE
  int ret;
  int i;
  u8 *ptr;

  MMCDBG("CMD_SEND:%d\n", cmd->cmdidx);
  MMCDBG("\t\tARG\t\t\t 0x%08X\n", cmd->cmdarg);
  MMCDBG("\t\tFLAG\t\t\t %d\n", cmd->flags);
  ret = mmc->send_cmd(mmc, cmd, data);
  switch (cmd->resp_type) {
    case MMC_RSP_NONE:
      MMCDBG("\t\tMMC_RSP_NONE\n");
      break;
    case MMC_RSP_R1:
      MMCDBG("\t\tMMC_RSP_R1,5,6,7 \t 0x%08X \n",
        cmd->response[0]);
      break;
    case MMC_RSP_R1b:
      MMCDBG("\t\tMMC_RSP_R1b\t\t 0x%08X \n",
        cmd->response[0]);
      break;
    case MMC_RSP_R2:
      MMCDBG("\t\tMMC_RSP_R2\t\t 0x%08X \n",
        cmd->response[0]);
      MMCDBG("\t\t          \t\t 0x%08X \n",
        cmd->response[1]);
      MMCDBG("\t\t          \t\t 0x%08X \n",
        cmd->response[2]);
      MMCDBG("\t\t          \t\t 0x%08X \n",
        cmd->response[3]);
      MMCMSG("\n");
      MMCDBG("\t\t\t\t\tDUMPING DATA\n");
      for (i = 0; i < 4; i++) {
        int j;
        MMCMSG("\t\t\t\t\t%03d - ", i*4);
        ptr = (u8 *)(&cmd->response[i]);
        ptr += 3;
        for (j = 0; j < 4; j++)
          MMCMSG("%02X ", *ptr--);
        MMCMSG("\n");
      }
      break;
    case MMC_RSP_R3:
      MMCDBG("\t\tMMC_RSP_R3,4\t\t 0x%08X \n",
        cmd->response[0]);
      break;
    default:
      MMCDBG("\t\tERROR MMC rsp not supported\n");
      break;
  }
  return ret;
#else
  return mmc->send_cmd(mmc, cmd, data);
#endif
}

int mmc_send_status(struct mmc *mmc, int timeout)
{
  struct mmc_cmd cmd;
  int err;
#ifdef CONFIG_MMC_TRACE
  int status;
#endif

  cmd.cmdidx = MMC_CMD_SEND_STATUS;
  cmd.resp_type = MMC_RSP_R1;
  if (!mmc_host_is_spi(mmc))
    cmd.cmdarg = mmc->rca << 16;
  cmd.flags = 0;

  do {
    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err){
      MMCINFO("Send status failed\n");
      return err;
    }
    else if (cmd.response[0] & MMC_STATUS_RDY_FOR_DATA)
      break;

    MicroSecondDelay(1000);

    if (cmd.response[0] & MMC_STATUS_MASK) {
      MMCINFO("Status Error: 0x%08X\n", cmd.response[0]);
      return COMM_ERR;
    }
  } while (timeout--);

#ifdef CONFIG_MMC_TRACE
  status = (cmd.response[0] & MMC_STATUS_CURR_STATE) >> 9;
  MMCDBG("CURR STATE:%d\n", status);
#endif
  if (timeout < 0) {
    MMCINFO("Timeout waiting card ready\n");
    return TIMEOUT;
  }

  return 0;
}

int mmc_set_blocklen(struct mmc *mmc, int len)
{
  struct mmc_cmd cmd;

  cmd.cmdidx = MMC_CMD_SET_BLOCKLEN;
  cmd.resp_type = MMC_RSP_R1;
  cmd.cmdarg = len;
  cmd.flags = 0;

  return mmc_send_cmd(mmc, &cmd, NULL);
}

struct mmc *find_mmc_device(int dev_num)
{

  if (mmc_devices[dev_num] != NULL)
    return mmc_devices[dev_num];
 
  
  MMCINFO("MMC Device %d not found\n", dev_num);

  return NULL;
}
unsigned long mmc_bwrite(int dev_num, unsigned long start, unsigned long blkcnt, const void*src);

 unsigned long
mmc_berase(int dev_num, unsigned long start, unsigned long blkcnt)
{
  int err = 0;
  struct mmc *mmc = find_mmc_device(dev_num);

  if (!mmc){
    MMCINFO("Can not find mmc dev\n");
    return -1;
  }
  #if 0
    if(0==blkcnt * mmc->write_bl_len){
    return 0;
  }
  #endif

  void* src = AllocateZeroPool(blkcnt * mmc->write_bl_len);
  if(src == NULL){
    MMCINFO("erase malloc failed\n");
    return -1;
  }


  memset(src, 0, mmc->write_bl_len*blkcnt);
  MMCINFO("erase blk %ld ~ %ld\n", start, start + blkcnt - 1);
  err = mmc_bwrite(dev_num, start, blkcnt, src);
  //mmcinfo("erase flag%d",err);
  if(!err){
    MMCINFO("erase failed\n");
  }

  FreePool(src);
  return err;
}

static unsigned long
mmc_write_blocks(struct mmc *mmc, unsigned long start, unsigned long blkcnt, const void*src)
{
  struct mmc_cmd cmd;
  struct mmc_data data;
  int timeout = 1000;

  if ((start + blkcnt) > mmc->block_dev.lba) {
    MMCINFO("MMC: block number 0x%lx exceeds max(0x%lx)\n",
      start + blkcnt, mmc->block_dev.lba);
    return 0;
  }

  if (blkcnt > 1)
    cmd.cmdidx = MMC_CMD_WRITE_MULTIPLE_BLOCK;
  else
    cmd.cmdidx = MMC_CMD_WRITE_SINGLE_BLOCK;

  if (mmc->high_capacity)
    cmd.cmdarg = start;
  else
    cmd.cmdarg = start * mmc->write_bl_len;

  cmd.resp_type = MMC_RSP_R1;
  cmd.flags = 0;

  data.src = src;
  data.blocks = blkcnt;
  data.blocksize = mmc->write_bl_len;
  data.flags = MMC_DATA_WRITE;

  if (mmc_send_cmd(mmc, &cmd, &data)) {
    MMCINFO("mmc write failed\n");
    return 0;
  }

  /* SPI multiblock writes terminate using a special
   * token, not a STOP_TRANSMISSION request.
   */
  if (!mmc_host_is_spi(mmc) && blkcnt > 1) {
    cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
    cmd.cmdarg = 0;
    cmd.resp_type = MMC_RSP_R1b;
    cmd.flags = 0;
    if (mmc_send_cmd(mmc, &cmd, NULL)) {
      MMCINFO("mmc fail to send stop cmd\n");
      return 0;
    }
  }
  /* Waiting for the ready status */
  mmc_send_status(mmc, timeout);

  return blkcnt;
}

unsigned long
mmc_bwrite(int dev_num, unsigned long start, unsigned long blkcnt, const void*src)
{
  unsigned long cur, blocks_todo = blkcnt;

  struct mmc *mmc = find_mmc_device(dev_num);

  if (blkcnt == 0){
    MMCINFO("blkcnt should not be 0\n");
    return 0;
  }
  if (!mmc){
    MMCINFO("Can not found device\n");
    return 0;
  }

  if (mmc_set_blocklen(mmc, mmc->write_bl_len)){
    MMCINFO("set block len failed\n");

    return 0;
  }

  do {
    cur = (blocks_todo > mmc->b_max) ?  mmc->b_max : blocks_todo;
    if(mmc_write_blocks(mmc, start, cur, src) != cur){
      MMCINFO("write block failed\n");
      return 0;
    }
    blocks_todo -= cur;
    start += cur;
//    src += cur * mmc->write_bl_len;
    src = (char*)src + cur * mmc->write_bl_len;
  } while (blocks_todo > 0);

  return blkcnt;
}


static unsigned long
mmc_bwrite_mass_pro(int dev_num, unsigned long start, unsigned long blkcnt, const void*src)
{
  unsigned long blocks_do = 0;
  unsigned long start_todo = start;

  signed int err = 0;
  struct mmc *mmc = find_mmc_device(dev_num);
#ifdef USE_EMMC_BOOT_PART
  MMCINFO("mmc start mass pro boot part...\n");
  do{
    if((start != BOOT0_SDMMC_START_ADDR)&&(start != UEFI_SDMMC_START_ADDR)) {
      MMCINFO("error boot0/UEFI start address %d\n", start);
      break;
    }

    if (!(mmc->boot_support)) //!(mmc->part_support & PART_SUPPORT)
    {
      MMCINFO("Card doesn't support boot part\n");
      break;
    }


    err = mmc_switch_boot_bus_cond(dev_num, MMC_SWITCH_BOOT_SDR_NORMAL, MMC_SWITCH_BOOT_RST_BUS_COND, MMC_SWITCH_BOOT_BUS_SDRx4_DDRx4);
    if (err) {
      MMCINFO("mmc switch boot bus condition failed\n");
      return err;
    }
    MMCDBG("mmc switch boot bus condition succeed\n");

    err = mmc_switch_boot_part(dev_num, MMC_SWITCH_PART_BOOT_ACK_ENB, (start==BOOT0_SDMMC_START_ADDR) ?MMC_SWITCH_PART_BOOT_PART_1 : MMC_SWITCH_PART_BOOT_PART_2);
    if (err) {
      MMCINFO("mmc switch boot part failed\n");
      return err;
    }
    MMCDBG("mmc switch boot part  succeed\n");

  }while(0);
#else
  do{
    if((start != BOOT0_SDMMC_START_ADDR)&&(start != UEFI_SDMMC_START_ADDR)) {
      MMCINFO("error boot start address %d\n", start);
      break;
    }

    if (!(mmc->boot_support)) //!(mmc->part_support & PART_SUPPORT)
    {
      MMCINFO("Card doesn't support boot part\n");
      break;
    }

    err = mmc_switch_boot_part(dev_num, 0, MMC_SWITCH_PART_BOOT_PART_NONE); //disable boot mode
    if (err) {
      MMCINFO("mmc disable boot mode failed\n");
      return err;
    }
    MMCDBG("mmc disable boot mode succeed\n");
  }while(0);

#endif


#ifdef USE_EMMC_BOOT_PART

  //write boot0.bin to boot1 partition
  if ( (mmc->boot_support) && (start == BOOT0_SDMMC_START_ADDR) )//(mmc->part_support & PART_SUPPORT)
  {
    if (mmc_switch_part(dev_num, MMC_SWITCH_PART_BOOT_PART_1)) {
      MMCINFO("switch to boot1 partition failed\n");
      return 0;
    }
    //In eMMC boot partition,boot0.bin start from 0 sector,so we must change start form 16 to 0;
    start_todo= 0;
  }
  else
  //write uefi.bin to boot2 partition
  if ( (mmc->boot_support) && (start == UEFI_SDMMC_START_ADDR) )//(mmc->part_support & PART_SUPPORT)
  {
    if (mmc_switch_part(dev_num, MMC_SWITCH_PART_BOOT_PART_2)) {
      MMCINFO("switch to boot2 partition failed\n");
      return 0;
    }
    //In eMMC boot partition,boot0.bin start from 0 sector,so we must change start form 16 to 0;
    start_todo= 0;
  }
#endif

  blocks_do = mmc_bwrite(dev_num, start_todo, blkcnt, src);
  //check boot0
  if(start == BOOT0_SDMMC_START_ADDR||start == UEFI_SDMMC_START_ADDR){
    u32  rblk = 0;
    s32 err = 0;
    void *dst = AllocateZeroPool(blkcnt * mmc->write_bl_len);
    MMCDBG("try to check boot0\n");
    if(dst == NULL){
      MMCINFO("malloc mem failed\n");
      return 0;
    }
    rblk = mmc_bread(dev_num, start_todo, blkcnt, dst);
    if(rblk){
      if(CompareMem(src,dst,blkcnt * mmc->write_bl_len)){
        err = -1;
      }
    }else{
      err = -1;
    }
    FreePool(dst);
    if(err){
      MMCINFO("check boot0 failded\n");
      return 0;
    }
    MMCDBG("check boot0 ok\n");

  }

#ifdef USE_EMMC_BOOT_PART
  //
  if ((mmc->boot_support)  && ((start == BOOT0_SDMMC_START_ADDR) || (start == UEFI_SDMMC_START_ADDR)))//(mmc->part_support & PART_SUPPORT)
  {
    //switch back to user partiton
    if (mmc_switch_part(dev_num,  0)) {
      MMCINFO("switch to boot1 partition failed\n");
      return 0;
    }
#ifdef USE_EMMC_USER_WHEN_USE_BOOT_PART//use eMMC boot and user part at the same time
    MMCINFO("mmc start write user part after write boot part...\n");
    blocks_do = mmc_bwrite(dev_num, start, blkcnt, src);
    //check boot0
    {
      u32  rblk = 0;
      s32 err = 0;
      void *dst = malloc(blkcnt * mmc->write_bl_len);
      MMCDBG("try to check boot0 in user part when use eMMC boot and user part at the same time\n");
      if(dst == NULL){
        MMCINFO("malloc mem failed\n");
        return 0;
      }
      rblk = mmc_bread(dev_num, start, blkcnt, dst);
      if(rblk){
        if(memcmp(src,dst,blkcnt * mmc->write_bl_len)){
          err = -1;
      }
      }else{
        err = -1;
      }
      free(dst);
      if(err){
        MMCINFO("check boot0 in user part failded when use eMMC boot and user part at the same time\n");
        return 0;
      }
      MMCDBG("check boot0 in user part ok when use eMMC boot and user part at the same time\n");
  }


#endif
  }
#endif
  return blocks_do;

}




int mmc_read_blocks(struct mmc *mmc, void *dst, unsigned long start, unsigned long blkcnt)
{
  struct mmc_cmd cmd;
  struct mmc_data data;
  int timeout = 1000;

  if (blkcnt > 1)
    cmd.cmdidx = MMC_CMD_READ_MULTIPLE_BLOCK;
  else
    cmd.cmdidx = MMC_CMD_READ_SINGLE_BLOCK;
  if (mmc->high_capacity)
    cmd.cmdarg = start;
  else
    cmd.cmdarg = start * mmc->read_bl_len;
  cmd.resp_type = MMC_RSP_R1;
  cmd.flags = 0;

  data.dest = dst;
  data.blocks = blkcnt;
  data.blocksize = mmc->read_bl_len;
  data.flags = MMC_DATA_READ;

  if (mmc_send_cmd(mmc, &cmd, &data)){
    MMCINFO(" read block failed\n");
    return 0;
  }
  if (blkcnt > 1) {
    cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
    cmd.cmdarg = 0;
    cmd.resp_type = MMC_RSP_R1b;
    cmd.flags = 0;
    if (mmc_send_cmd(mmc, &cmd, NULL)) {
      MMCINFO("mmc fail to send stop cmd\n");
      return 0;
    }

    /* Waiting for the ready status */
    mmc_send_status(mmc, timeout);
  }

  return blkcnt;
}

 unsigned long mmc_bread(int dev_num, unsigned long start, unsigned long blkcnt, void *dst)
{
  unsigned long cur, blocks_todo = blkcnt;

  if (blkcnt == 0){
    MMCINFO("blkcnt should not be 0\n");
    return 0;
  }
  struct mmc *mmc = find_mmc_device(dev_num);
  if (!mmc){
    MMCINFO("Can not find mmc dev\n");
    return 0;
  }
  if ((start + blkcnt) > mmc->block_dev.lba) {
    MMCINFO("MMC: block number 0x%lx exceeds max(0x%lx)\n",
      start + blkcnt, mmc->block_dev.lba);
    return 0;
  }
  if (mmc_set_blocklen(mmc, mmc->read_bl_len)){
    MMCINFO("Set block len failed\n");
    return 0;
  }

  do {
    cur = (blocks_todo > mmc->b_max) ?  mmc->b_max : blocks_todo;
    if(mmc_read_blocks(mmc, dst, start, cur) != cur){
      MMCINFO("block read failed\n");
      return 0;
    }
    blocks_todo -= cur;
    start += cur;
//    dst += cur * mmc->read_bl_len;
    dst = (char*)dst + cur * mmc->read_bl_len;
  } while (blocks_todo > 0);

  return blkcnt;
}



 unsigned long mmc_bread_mass_pro(int dev_num, unsigned long start, unsigned long blkcnt, void *dst)
{
  unsigned long blocks_do = 0;
  unsigned long start_todo = start;

#ifdef USE_EMMC_BOOT_PART
  struct mmc *mmc = find_mmc_device(dev_num);
  //write boot0.bin to boot partition
  if ((mmc->boot_support)  && (start == BOOT0_SDMMC_START_ADDR) )//(mmc->part_support & PART_SUPPORT)
  {
    if (mmc_switch_part(dev_num, MMC_SWITCH_PART_BOOT_PART_1))
    {
      MMCINFO("switch to boot1 partition failed\n");
      return 0;
    }
    //In eMMC boot partition,boot0.bin start from 0 sector,so we must change start form 16 to 0;
    start_todo= 0;
  }
#endif

  blocks_do = mmc_bread(dev_num, start_todo, blkcnt, dst);

#ifdef USE_EMMC_BOOT_PART
  //
  if ( (mmc->boot_support) && (start == BOOT0_SDMMC_START_ADDR) )//(mmc->part_support & PART_SUPPORT)
  {
    //switch back to user partiton
    if (mmc_switch_part(dev_num,  0)) {
      MMCINFO("switch to boot1 partition failed\n");
      return 0;
    }
  }
#endif

  return blocks_do;
}

int mmc_go_idle(struct mmc* mmc)
{
  struct mmc_cmd cmd;
  int err;

  MicroSecondDelay(1000);

  cmd.cmdidx = MMC_CMD_GO_IDLE_STATE;
  cmd.cmdarg = 0;
  cmd.resp_type = MMC_RSP_NONE;
  cmd.flags = 0;

  err = mmc_send_cmd(mmc, &cmd, NULL);

  if (err){
    MMCINFO("go idle failed\n");
    return err;
  }

  MicroSecondDelay(2000);

  return 0;
}

int
sd_send_op_cond(struct mmc *mmc)
{
  int timeout = 1000;
  int err;
  struct mmc_cmd cmd;

  do {
    cmd.cmdidx = MMC_CMD_APP_CMD;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = 0;
    cmd.flags = 0;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err){
      MMCINFO("send app cmd failed\n");
      return err;
    }

    cmd.cmdidx = SD_CMD_APP_SEND_OP_COND;
    cmd.resp_type = MMC_RSP_R3;

    /*
     * Most cards do not answer if some reserved bits
     * in the ocr are set. However, Some controller
     * can set bit 7 (reserved for low voltages), but
     * how to manage low voltages SD card is not yet
     * specified.
     */
    cmd.cmdarg = mmc_host_is_spi(mmc) ? 0 :
      (mmc->voltages & 0xff8000);

    if (mmc->version == SD_VERSION_2)
      cmd.cmdarg |= OCR_HCS;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err){
      MMCINFO("send cmd41 failed\n");
      return err;
    }

    MicroSecondDelay(2000);
  } while ((!(cmd.response[0] & OCR_BUSY)) && timeout--);

  if (timeout < 0){
    MMCINFO("wait card init failed\n");
    return UNUSABLE_ERR;
  }

  if (mmc->version != SD_VERSION_2)
    mmc->version = SD_VERSION_1_0;

  if (mmc_host_is_spi(mmc)) { /* read OCR for spi */
    cmd.cmdidx = MMC_CMD_SPI_READ_OCR;
    cmd.resp_type = MMC_RSP_R3;
    cmd.cmdarg = 0;
    cmd.flags = 0;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err){
      MMCINFO("spi read ocr failed\n");
      return err;
    }
  }

  mmc->ocr = cmd.response[0];

  mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
  mmc->rca = 0;

  return 0;
}

int mmc_send_op_cond(struct mmc *mmc)
{
  int timeout = 10000;
  struct mmc_cmd cmd;
  int err;

  /* Some cards seem to need this */
  mmc_go_idle(mmc);

  /* Asking to the card its capabilities */
  cmd.cmdidx = MMC_CMD_SEND_OP_COND;
  cmd.resp_type = MMC_RSP_R3;
  cmd.cmdarg = 0x40ff8000;//foresee
  cmd.flags = 0;

  err = mmc_send_cmd(mmc, &cmd, NULL);

  if (err){
    MMCINFO("mmc send op cond failed\n");
    return err;
  }

  MicroSecondDelay(1000);

  do {
    cmd.cmdidx = MMC_CMD_SEND_OP_COND;
    cmd.resp_type = MMC_RSP_R3;
    cmd.cmdarg = (mmc_host_is_spi(mmc) ? 0 :
        (mmc->voltages &
        (cmd.response[0] & OCR_VOLTAGE_MASK)) |
        (cmd.response[0] & OCR_ACCESS_MODE));

    if (mmc->host_caps & MMC_MODE_HC)
      cmd.cmdarg |= OCR_HCS;

    cmd.flags = 0;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err){
      MMCINFO("mmc send op cond failed\n");
      return err;
    }

    MicroSecondDelay(1000);
  } while (!(cmd.response[0] & OCR_BUSY) && timeout--);

  if (timeout < 0){
    MMCINFO("wait for mmc init failed\n");
    return UNUSABLE_ERR;
  }

  if (mmc_host_is_spi(mmc)) { /* read OCR for spi */
    cmd.cmdidx = MMC_CMD_SPI_READ_OCR;
    cmd.resp_type = MMC_RSP_R3;
    cmd.cmdarg = 0;
    cmd.flags = 0;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err){
      MMCINFO("spi read ocr failed\n");
      return err;
    }
  }

  mmc->version = MMC_VERSION_UNKNOWN;
  mmc->ocr = cmd.response[0];

  mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
  mmc->rca = 1;

  return 0;
}


int mmc_send_ext_csd(struct mmc *mmc, char *ext_csd)
{
  struct mmc_cmd cmd;
  struct mmc_data data;
  int err;

  /* Get the Card Status Register */
  cmd.cmdidx = MMC_CMD_SEND_EXT_CSD;
  cmd.resp_type = MMC_RSP_R1;
  cmd.cmdarg = 0;
  cmd.flags = 0;

  data.dest = ext_csd;
  data.blocks = 1;
  data.blocksize = 512;
  data.flags = MMC_DATA_READ;

  err = mmc_send_cmd(mmc, &cmd, &data);
  if(err)
    MMCINFO("mmc send ext csd failed\n");

  return err;
}


int mmc_switch(struct mmc *mmc, u8 set, u8 index, u8 value)
{
  struct mmc_cmd cmd;
  int timeout = 1000;
  int ret;

  cmd.cmdidx = MMC_CMD_SWITCH;
  cmd.resp_type = MMC_RSP_R1b;
  cmd.cmdarg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
         (index << 16) |
         (value << 8);
  cmd.flags = 0;

  ret = mmc_send_cmd(mmc, &cmd, NULL);
  if(ret){
    MMCINFO("mmc switch failed\n");
  }


  /* Waiting for the ready status */
  mmc_send_status(mmc, timeout);

  return ret;

}

int mmc_change_freq(struct mmc *mmc)
{
  char ext_csd[512];
  char cardtype;
  int err;
  int retry = 5;

  mmc->card_caps = 0;

  if (mmc_host_is_spi(mmc))
    return 0;

  /* Only version 4 supports high-speed */
  if (mmc->version < MMC_VERSION_4)
    return 0;

  mmc->card_caps |= MMC_MODE_4BIT;

  err = mmc_send_ext_csd(mmc, ext_csd);

  if (err){
    MMCINFO("mmc get ext csd failed\n");
    return err;
  }

  cardtype = ext_csd[196] & 0xf;

  //retry for Toshiba emmc;for the first time Toshiba emmc change to HS
  //it will return response crc err,so retry
  do{
    err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, 1);
    if(!err){
      break;
    }
    MMCINFO("retry mmc switch(cmd6)\n");
  }while(retry--);

  if (err){
    MMCINFO("mmc change to hs failed\n");
    return err;
  }

  /* Now check to see that it worked */
  err = mmc_send_ext_csd(mmc, ext_csd);

  if (err){
    MMCINFO("send ext csd faild\n");
    return err;
  }

  /* No high-speed support */
  if (!ext_csd[185])
    return 0;

  /* High Speed is set, there are two types: 52MHz and 26MHz */
  if (cardtype & MMC_HS_52MHZ)
    mmc->card_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
  else
    mmc->card_caps |= MMC_MODE_HS;

  return 0;
}


int mmc_switch_boot_bus_cond(int dev_num, u32 boot_mode, u32 rst_bus_cond, u32 bus_width)
{
  char ext_csd[512]={0};
  unsigned char boot_bus_cond = 0;
  int ret = 0;
  struct mmc *mmc = find_mmc_device(dev_num);
  if (!mmc){
    MMCINFO("can not find mmc device\n");
    return -1;
  }


  boot_bus_cond = (mmc->boot_bus_cond &
          (~MMC_SWITCH_BOOT_MODE_MASK) & (~MMC_SWITCH_BOOT_RST_BUS_COND_MASK) & (~MMC_SWITCH_BOOT_BUS_WIDTH_MASK))
        | ((boot_mode << 3) & MMC_SWITCH_BOOT_MODE_MASK)
        | ((rst_bus_cond << 2) & MMC_SWITCH_BOOT_RST_BUS_COND_MASK)
        | ((bus_width) & MMC_SWITCH_BOOT_BUS_WIDTH_MASK);

  ret = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_BOOT_BUS_COND,boot_bus_cond);
  if(ret){
    MMCINFO("switch bus cond failed\n");
    return -1;
  }
  ret = mmc_send_ext_csd(mmc, ext_csd);
  if(ret){
    MMCINFO("send ext csd failed\n");
    return -1;
  }
  MMCDBG("boot bus cond:0x%x\n",ext_csd[EXT_CSD_BOOT_BUS_COND]);
  if(boot_bus_cond!=ext_csd[EXT_CSD_BOOT_BUS_COND]) {
    MMCINFO("Set boot bus cond failed,now bus con is 0x%x\n",ext_csd[EXT_CSD_BOOT_BUS_COND]);
    return -1;
  }
  mmc->boot_bus_cond = boot_bus_cond;
  return ret;
}

int mmc_switch_boot_part(int dev_num, u32 boot_ack, u32 boot_part)
{
  char ext_csd[512]={0};
  unsigned char part_config = 0;
  int ret = 0;
  struct mmc *mmc = find_mmc_device(dev_num);
  if (!mmc){
    MMCINFO("can not find mmc device\n");
    return -1;
  }


  part_config = (mmc->part_config & (~MMC_SWITCH_PART_BOOT_PART_MASK) & (~MMC_SWITCH_PART_BOOT_ACK_MASK))
                | ((boot_part << 3) & MMC_SWITCH_PART_BOOT_PART_MASK) | (boot_ack << 6);
  ret = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_PART_CONF,part_config);

  if(ret){
    MMCINFO("switch boot partd failed\n");
    return -1;
  }
  ret = mmc_send_ext_csd(mmc, ext_csd);
  if(ret){
    MMCINFO("send ext csd failed\n");
    return -1;
  }

  MMCDBG("part conf:0x%x\n",ext_csd[EXT_CSD_PART_CONF]);
  if(part_config!=ext_csd[EXT_CSD_PART_CONF]) {
    MMCINFO("switch boot part failed,now part conf is 0x%x\n",ext_csd[EXT_CSD_PART_CONF]);
    return -1;
  }
  mmc->part_config = part_config;
  return ret;
}



int mmc_switch_part(int dev_num, unsigned int part_num)
{
  char ext_csd[512]={0};
  unsigned char part_config = 0;
  int ret = 0;
  struct mmc *mmc = find_mmc_device(dev_num);

  MMCDBG("Try to switch part \n");
  if (!mmc){
    MMCINFO("can not find mmc device\n");
    return -1;
  }


  part_config = (mmc->part_config & ~PART_ACCESS_MASK)
                  | (part_num & PART_ACCESS_MASK);
  ret = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_PART_CONF,part_config);
  if(ret){
    MMCINFO("mmc switch part failed\n");
    return -1;
  }
  ret = mmc_send_ext_csd(mmc, ext_csd);
  if(ret){
    MMCINFO("send ext csd failed\n");
    return -1;
  }
  MMCDBG("part conf:0x%x\n",ext_csd[EXT_CSD_PART_CONF]);
  if(part_config!=ext_csd[EXT_CSD_PART_CONF]) {
    MMCINFO("switch boot part failed,now bus con is 0x%x\n",ext_csd[EXT_CSD_PART_CONF]);
    return -1;
  }
  mmc->part_config = part_config;

  MMCDBG("switch part succeed\n");
  return ret;

}

int sd_switch(struct mmc *mmc, int mode, int group, u8 value, u8 *resp)
{
  struct mmc_cmd cmd;
  struct mmc_data data;

  /* Switch the frequency */
  cmd.cmdidx = SD_CMD_SWITCH_FUNC;
  cmd.resp_type = MMC_RSP_R1;
  cmd.cmdarg = (mode << 31) | 0xffffff;
  cmd.cmdarg &= ~(0xf << (group * 4));
  cmd.cmdarg |= value << (group * 4);
  cmd.flags = 0;

  data.dest = (char *)resp;
  data.blocksize = 64;
  data.blocks = 1;
  data.flags = MMC_DATA_READ;

  return mmc_send_cmd(mmc, &cmd, &data);
}


int sd_change_freq(struct mmc *mmc)
{
  int err;
  struct mmc_cmd cmd;
  uint scr[2];
  uint switch_status[16];
  struct mmc_data data;
  int timeout;

  mmc->card_caps = 0;

  if (mmc_host_is_spi(mmc))
    return 0;

  /* Read the SCR to find out if this card supports higher speeds */
  cmd.cmdidx = MMC_CMD_APP_CMD;
  cmd.resp_type = MMC_RSP_R1;
  cmd.cmdarg = mmc->rca << 16;
  cmd.flags = 0;

  err = mmc_send_cmd(mmc, &cmd, NULL);

  if (err){
    MMCINFO("Send app cmd failed\n");
    return err;
  }

  cmd.cmdidx = SD_CMD_APP_SEND_SCR;
  cmd.resp_type = MMC_RSP_R1;
  cmd.cmdarg = 0;
  cmd.flags = 0;

  timeout = 3;

retry_scr:
  data.dest = (char *)&scr;
  data.blocksize = 8;
  data.blocks = 1;
  data.flags = MMC_DATA_READ;

  err = mmc_send_cmd(mmc, &cmd, &data);

  if (err) {
    if (timeout--)
      goto retry_scr;

    MMCINFO("Send scr failed\n");
    return err;
  }

  mmc->scr[0] = __be32_to_cpu(scr[0]);
  mmc->scr[1] = __be32_to_cpu(scr[1]);

  switch ((mmc->scr[0] >> 24) & 0xf) {
    case 0:
      mmc->version = SD_VERSION_1_0;
      break;
    case 1:
      mmc->version = SD_VERSION_1_10;
      break;
    case 2:
      mmc->version = SD_VERSION_2;
      break;
    default:
      mmc->version = SD_VERSION_1_0;
      break;
  }

  if (mmc->scr[0] & SD_DATA_4BIT)
    mmc->card_caps |= MMC_MODE_4BIT;

  /* Version 1.0 doesn't support switching */
  if (mmc->version == SD_VERSION_1_0)
    return 0;

  timeout = 4;
  while (timeout--) {
    err = sd_switch(mmc, SD_SWITCH_CHECK, 0, 1,
        (u8 *)&switch_status);

    if (err){
      MMCINFO("Check high speed status faild\n");
      return err;
    }

    /* The high-speed function is busy.  Try again */
    if (!(__be32_to_cpu(switch_status[7]) & SD_HIGHSPEED_BUSY))
      break;
  }

  /* If high-speed isn't supported, we return */
  if (!(__be32_to_cpu(switch_status[3]) & SD_HIGHSPEED_SUPPORTED))
    return 0;

  err = sd_switch(mmc, SD_SWITCH_SWITCH, 0, 1, (u8 *)&switch_status);

  if (err){
    MMCINFO("switch to high speed failed\n");
    return err;
  }

  if ((__be32_to_cpu(switch_status[4]) & 0x0f000000) == 0x01000000)
    mmc->card_caps |= MMC_MODE_HS;

  return 0;
}

/* frequency bases */
/* divided by 10 to be nice to platforms without floating point */
static const int fbase[] = {
  10000,
  100000,
  1000000,
  10000000,
};

/* Multiplier values for TRAN_SPEED.  Multiplied by 10 to be nice
 * to platforms without floating point.
 */
static const int multipliers[] = {
  0,  /* reserved */
  10,
  12,
  13,
  15,
  20,
  25,
  30,
  35,
  40,
  45,
  50,
  55,
  60,
  70,
  80,
};

void mmc_set_ios(struct mmc *mmc)
{
  mmc->set_ios(mmc);
}

void mmc_set_clock(struct mmc *mmc, uint clock)
{
  if (clock > mmc->f_max)
    clock = mmc->f_max;

  if (clock < mmc->f_min)
    clock = mmc->f_min;

  mmc->clock = clock;

  mmc_set_ios(mmc);
}

void mmc_set_bus_width(struct mmc *mmc, uint width)
{
  mmc->bus_width = width;

  mmc_set_ios(mmc);
}

int mmc_startup(struct mmc *mmc)
{
  int err;
  uint mult, freq;
  u64 cmult, csize, capacity;
  struct mmc_cmd cmd;
  char ext_csd[512];
  int timeout = 1000;

#ifdef CONFIG_MMC_SPI_CRC_ON
  if (mmc_host_is_spi(mmc)) { /* enable CRC check for spi */
    cmd.cmdidx = MMC_CMD_SPI_CRC_ON_OFF;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = 1;
    cmd.flags = 0;
    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err){
      MMCINFO("Spi crc on off failed\n");
      return err;
    }
  }
#endif

  /* Put the Card in Identify Mode */
  cmd.cmdidx = mmc_host_is_spi(mmc) ? MMC_CMD_SEND_CID :
    MMC_CMD_ALL_SEND_CID; /* cmd not supported in spi */
  cmd.resp_type = MMC_RSP_R2;
  cmd.cmdarg = 0;
  cmd.flags = 0;

  err = mmc_send_cmd(mmc, &cmd, NULL);

  if (err){
    MMCINFO("Put the Card in Identify Mode failed\n");
    return err;
  }

  memcpy(mmc->cid, cmd.response, 16);

  /*
   * For MMC cards, set the Relative Address.
   * For SD cards, get the Relatvie Address.
   * This also puts the cards into Standby State
   */
  if (!mmc_host_is_spi(mmc)) { /* cmd not supported in spi */
    cmd.cmdidx = SD_CMD_SEND_RELATIVE_ADDR;
    cmd.cmdarg = mmc->rca << 16;
    cmd.resp_type = MMC_RSP_R6;
    cmd.flags = 0;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err){
      MMCINFO("send rca failed\n");
      return err;
    }

    if (IS_SD(mmc))
      mmc->rca = (cmd.response[0] >> 16) & 0xffff;
  }

  /* Get the Card-Specific Data */
  cmd.cmdidx = MMC_CMD_SEND_CSD;
  cmd.resp_type = MMC_RSP_R2;
  cmd.cmdarg = mmc->rca << 16;
  cmd.flags = 0;

  err = mmc_send_cmd(mmc, &cmd, NULL);

  /* Waiting for the ready status */
  mmc_send_status(mmc, timeout);

  if (err){
    MMCINFO("get csd failed\n");
    return err;
  }

  mmc->csd[0] = cmd.response[0];
  mmc->csd[1] = cmd.response[1];
  mmc->csd[2] = cmd.response[2];
  mmc->csd[3] = cmd.response[3];

  if (mmc->version == MMC_VERSION_UNKNOWN) {
    int version = (cmd.response[0] >> 26) & 0xf;

    switch (version) {
      case 0:
        mmc->version = MMC_VERSION_1_2;
        break;
      case 1:
        mmc->version = MMC_VERSION_1_4;
        break;
      case 2:
        mmc->version = MMC_VERSION_2_2;
        break;
      case 3:
        mmc->version = MMC_VERSION_3;
        break;
      case 4:
        mmc->version = MMC_VERSION_4;
        break;
      default:
        mmc->version = MMC_VERSION_1_2;
        break;
    }
  }

  /* divide frequency by 10, since the mults are 10x bigger */
  freq = fbase[(cmd.response[0] & 0x7)];
  mult = multipliers[((cmd.response[0] >> 3) & 0xf)];

  mmc->tran_speed = freq * mult;

  mmc->read_bl_len = 1 << ((cmd.response[1] >> 16) & 0xf);

  if (IS_SD(mmc))
    mmc->write_bl_len = mmc->read_bl_len;
  else
    mmc->write_bl_len = 1 << ((cmd.response[3] >> 22) & 0xf);

  if (mmc->high_capacity) {
    csize = (mmc->csd[1] & 0x3f) << 16
      | (mmc->csd[2] & 0xffff0000) >> 16;
    cmult = 8;
  } else {
    csize = (mmc->csd[1] & 0x3ff) << 2
      | (mmc->csd[2] & 0xc0000000) >> 30;
    cmult = (mmc->csd[2] & 0x00038000) >> 15;
  }

  mmc->capacity = (csize + 1) << (cmult + 2);
  mmc->capacity *= mmc->read_bl_len;

  if (mmc->read_bl_len > 512)
    mmc->read_bl_len = 512;

  if (mmc->write_bl_len > 512)
    mmc->write_bl_len = 512;

  /* Select the card, and put it into Transfer Mode */
  if (!mmc_host_is_spi(mmc)) { /* cmd not supported in spi */
    cmd.cmdidx = MMC_CMD_SELECT_CARD;
    cmd.resp_type = MMC_RSP_R1b;
    cmd.cmdarg = mmc->rca << 16;
    cmd.flags = 0;
    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err){
      MMCINFO("Select the card failed\n");
      return err;
    }
  }

  /*
   * For SD, its erase group is always one sector
   */
  mmc->erase_grp_size = 1;
  mmc->part_config = MMCPART_NOAVAILABLE;
  if (!IS_SD(mmc) && (mmc->version >= MMC_VERSION_4)) {
    /* check  ext_csd version and capacity */
    err = mmc_send_ext_csd(mmc, ext_csd);

    if(!err){
        /* update mmc version */
      switch (ext_csd[192]) {
        case 0:
          mmc->version = MMC_VERSION_4;
          break;
        case 1:
          mmc->version = MMC_VERSION_4_1;
          break;
        case 2:
          mmc->version = MMC_VERSION_4_2;
          break;
        case 3:
          mmc->version = MMC_VERSION_4_3;
          break;
        case 5:
          mmc->version = MMC_VERSION_4_41;
          break;
        case 6:
          mmc->version = MMC_VERSION_4_5;
          break;
        case 7:
          mmc->version = MMC_VERSION_5_0;
          break;
        default:
          MMCINFO("Invalid ext_csd revision %d\n", ext_csd[192]);
          break;
      }
    }


    if (!err & (ext_csd[192] >= 2)) {
      /*
       * According to the JEDEC Standard, the value of
       * ext_csd's capacity is valid if the value is more
       * than 2GB
       */
      capacity = (unsigned char)(ext_csd[212]) << 0 | (unsigned char)(ext_csd[213]) << 8 |
           (unsigned char)(ext_csd[214]) << 16 | (unsigned char)(ext_csd[215]) << 24;
      capacity *= 512;
      if ((capacity >> 20) > 2 * 1024)
        mmc->capacity = capacity;
    }

    /*
     * Check whether GROUP_DEF is set, if yes, read out
     * group size from ext_csd directly, or calculate
     * the group size from the csd value.
     */
    if (ext_csd[175])
      mmc->erase_grp_size = ext_csd[224] * 512 * 1024;
    else {
      int erase_gsz, erase_gmul;
      erase_gsz = (mmc->csd[2] & 0x00007c00) >> 10;
      erase_gmul = (mmc->csd[2] & 0x000003e0) >> 5;
      mmc->erase_grp_size = (erase_gsz + 1)
        * (erase_gmul + 1);
    }

    /* store the partition info of emmc */
    if (ext_csd[226])//ext_csd[160] & PART_SUPPORT
    {
      mmc->boot_support = 1;//lwj
      mmc->part_support = ext_csd[160];
      mmc->part_config = ext_csd[179];
      mmc->boot1_lba  = 128000*ext_csd[226]/512;
      mmc->boot2_lba  = 128000*ext_csd[226]/512;
      mmc->boot_bus_cond = ext_csd[177];

      MMCDBG("PART_SUPPORT mmc->boot1_lba = %d\n",mmc->boot1_lba);
    }
    else
    {
      MMCDBG("not PART_SUPPORT ext_csd[226] = %d\n",ext_csd[226]);
    }
  }


  mmc_set_clock(mmc,25000000);

  if (IS_SD(mmc))
    err = sd_change_freq(mmc);
  else
    err = mmc_change_freq(mmc);

  if (err){
    MMCINFO("Change speed mode failed\n");
    return err;
  }

  /* Restrict card's capabilities by what the host can do */
  mmc->card_caps &= mmc->host_caps;

  if (IS_SD(mmc)) {
    if (mmc->card_caps & MMC_MODE_4BIT) {
      cmd.cmdidx = MMC_CMD_APP_CMD;
      cmd.resp_type = MMC_RSP_R1;
      cmd.cmdarg = mmc->rca << 16;
      cmd.flags = 0;

      err = mmc_send_cmd(mmc, &cmd, NULL);
      if (err){
        MMCINFO("send app cmd failed\n");
        return err;
      }

      cmd.cmdidx = SD_CMD_APP_SET_BUS_WIDTH;
      cmd.resp_type = MMC_RSP_R1;
      cmd.cmdarg = 2;
      cmd.flags = 0;
      err = mmc_send_cmd(mmc, &cmd, NULL);
      if (err){
        MMCINFO("sd set bus width failed\n");
        return err;
      }

      mmc_set_bus_width(mmc, 4);
    }

    if (mmc->card_caps & MMC_MODE_HS)
      mmc_set_clock(mmc, 50000000);
    else
      mmc_set_clock(mmc, 25000000);
  } else {
    if (mmc->card_caps & MMC_MODE_4BIT) {
      /* Set the card to use 4 bit*/
      err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
          EXT_CSD_BUS_WIDTH,
          EXT_CSD_BUS_WIDTH_4);

      if (err){
        MMCINFO("mmc switch bus width failed\n");
        return err;
      }

      mmc_set_bus_width(mmc, 4);
    } else if (mmc->card_caps & MMC_MODE_8BIT) {
      /* Set the card to use 8 bit*/
      err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
          EXT_CSD_BUS_WIDTH,
          EXT_CSD_BUS_WIDTH_8);

      if (err){
        MMCINFO("mmc switch bus width8 failed\n");
        return err;
      }

      mmc_set_bus_width(mmc, 8);
    }

    if (mmc->card_caps & MMC_MODE_HS) {
      if (mmc->card_caps & MMC_MODE_HS_52MHz)
        mmc_set_clock(mmc, 52000000);
      else
        mmc_set_clock(mmc, 26000000);
    } else
      mmc_set_clock(mmc, 20000000);
  }

  /* fill in device description */
  mmc->block_dev.lun = 0;
  mmc->block_dev.type = 0;
  mmc->block_dev.blksz = mmc->read_bl_len;
  mmc->block_dev.lba = mmc->capacity/mmc->read_bl_len;
  if (IS_SD(mmc)){
    AsciiSPrint(mmc->block_dev.vendor,sizeof(mmc->block_dev.vendor),"MID %06x PSN %08x",
      mmc->cid[0] >> 24, (mmc->cid[2] << 8) | (mmc->cid[3] >> 24));
    AsciiSPrint(mmc->block_dev.product,sizeof(mmc->block_dev.product), "PNM %c%c%c%c%c", mmc->cid[0] & 0xff,
      (mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
      (mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff);
    AsciiSPrint(mmc->block_dev.revision,sizeof(mmc->block_dev.revision),"PRV %d.%d", mmc->cid[2] >> 28,
      (mmc->cid[2] >> 24) & 0xf);
  } else {
    AsciiSPrint(mmc->block_dev.vendor,sizeof(mmc->block_dev.vendor), "MID %06x PSN %04x%04x",
      mmc->cid[0] >> 24, (mmc->cid[2] & 0xffff),
      (mmc->cid[3] >> 16) & 0xffff);
    AsciiSPrint(mmc->block_dev.product,sizeof(mmc->block_dev.product), "PNM %c%c%c%c%c%c", mmc->cid[0] & 0xff,
      (mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
      (mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff,
      (mmc->cid[2] >> 24) & 0xff);
    AsciiSPrint(mmc->block_dev.revision,sizeof(mmc->block_dev.revision), "PRV %d.%d", (mmc->cid[2] >> 20) & 0xf,
      (mmc->cid[2] >> 16) & 0xf);
  }

  MMCINFO("%a\n", mmc->block_dev.vendor);
  MMCINFO("%a -- 0x%02x-%02x-%02x-%02x-%02x-%02x\n", mmc->block_dev.product,
      mmc->cid[0] & 0xff, (mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
      (mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff);
  MMCINFO("%a\n", mmc->block_dev.revision);

  if (IS_SD(mmc)){
    MMCINFO("MDT m-%d y-%d\n", ((mmc->cid[3] >> 8) & 0xF), (((mmc->cid[3] >> 12) & 0xFF) + 2000));
  } else {
    if (ext_csd[192] > 4) {
      MMCINFO("MDT m-%d y-%d\n", ((mmc->cid[3] >> 12) & 0xF),
        (((mmc->cid[3] >> 8) & 0xF) < 13) ? (((mmc->cid[3] >> 8) & 0xF) + 2013) : (((mmc->cid[3] >> 8) & 0xF) + 1997));
    } else {
      MMCINFO("MDT m-%d y-%d\n", ((mmc->cid[3] >> 12) & 0xF), (((mmc->cid[3] >> 8) & 0xF) + 1997));
    }
  }

    /*printf("---------------ext_csd[192] = %d--------------\n", ext_csd[192]);*/
  if(!IS_SD(mmc)){
    switch(mmc->version)
    {
      case MMC_VERSION_1_2:
        MMCINFO("MMC ver 1.2\n");
        break;
      case MMC_VERSION_1_4:
        MMCINFO("MMC ver 1.4\n");
        break;
      case MMC_VERSION_2_2:
        MMCINFO("MMC ver 2.2\n");
        break;
      case MMC_VERSION_3:
        MMCINFO("MMC ver 3.0\n");
        break;
      case MMC_VERSION_4:
        MMCINFO("MMC ver 4.0\n");
        break;
      case MMC_VERSION_4_1:
        MMCINFO("MMC ver 4.1\n");
        break;
      case MMC_VERSION_4_2:
        MMCINFO("MMC ver 4.2\n");
        break;
      case MMC_VERSION_4_3:
        MMCINFO("MMC ver 4.3\n");
        break;
      case MMC_VERSION_4_41:
        MMCINFO("MMC ver 4.41\n");
        break;
      case MMC_VERSION_4_5:
        MMCINFO("MMC ver 4.5\n");
        break;
      case MMC_VERSION_5_0:
        MMCINFO("MMC ver 5.0\n");
        break;
      default:
        MMCINFO("Unknow MMC ver\n");
        break;
    }
  }

  MMCINFO("---------------mmc->clock %d-----------\n", mmc->clock);
  MMCINFO("---------------mmc->bus_width %d--------------\n", mmc->bus_width);
  MMCINFO("SD/MMC Card: %dbit, capacity: %dMB\n",mmc->card_caps & MMC_MODE_4BIT ? 4 : 1, mmc->block_dev.lba>>11);
  MMCINFO("boot0 capacity: %dKB,boot1 capacity: %dKB\n"
            ,mmc->boot1_lba*512/1024,mmc->boot2_lba*512/1024);
  MMCINFO("************SD/MMC %d init OK!!!************\n",mmc->control_num);
  //init_part(&mmc->block_dev);

  return 0;
}

int mmc_send_if_cond(struct mmc *mmc)
{
  struct mmc_cmd cmd;
  int err;

  cmd.cmdidx = SD_CMD_SEND_IF_COND;
  /* We set the bit if the host supports voltages between 2.7 and 3.6 V */
  cmd.cmdarg = ((mmc->voltages & 0xff8000) != 0) << 8 | 0xaa;
  cmd.resp_type = MMC_RSP_R7;
  cmd.flags = 0;

  err = mmc_send_cmd(mmc, &cmd, NULL);

  if (err){
    MMCINFO("mmc send if cond failed\n");
    return err;
  }

  if ((cmd.response[0] & 0xff) != 0xaa)
    return UNUSABLE_ERR;
  else
    mmc->version = SD_VERSION_2;

  return 0;
}

#ifdef CONFIG_PARTITIONS
block_dev_desc_t *mmc_get_dev(int dev)
{
  struct mmc *mmc = find_mmc_device(dev);

  return mmc ? &mmc->block_dev : NULL;
}
#endif

int mmc_init(struct mmc *mmc)
{
  int err;

  if (mmc->has_init){
    MMCINFO("Has init\n");
    return 0;
  }

  err = mmc->init(mmc);

  if (err){
    MMCINFO("mmc->init error\n");
    return err;
  }
  mmc_set_bus_width(mmc, 1);
  mmc_set_clock(mmc, 1);

  /* Reset the Card */
  err = mmc_go_idle(mmc);

  if (err){
    MMCINFO("mmc go idle error\n");
    return err;
  }
  /* The internal partition reset to user partition(0) at every CMD0*/
  mmc->part_num = 0;

  MMCINFO("************Try SD card %d************\n",mmc->control_num);
  /* Test for SD version 2 */
  err = mmc_send_if_cond(mmc);

    /* Now try to get the SD card's operating condition */
  err = sd_send_op_cond(mmc);

  /* If the command timed out, we check for an MMC card */
  if (err == -1){
    MMCINFO("************Try MMC card %d************\n",mmc->control_num);
    err = mmc_send_op_cond(mmc);

    if (err) {
      MMCINFO("Card did not respond to voltage select!\n");
      MMCINFO("************SD/MMC %d init error!!!************\n",mmc->control_num);
      return UNUSABLE_ERR;
    }
  }

  err = mmc_startup(mmc);
  if (err){
    MMCINFO("************SD/MMC %d init error!!!************\n",mmc->control_num);
    mmc->has_init = 0;
  }
  else{
    mmc->has_init = 1;
    //MMCINFO("************SD/MMC %d init OK!!!************\n",mmc->control_num);
  }

  return err;

}

int mmc_register(int dev_num, struct mmc *mmc)
{
  mmc_devices[dev_num] = mmc;
  /* Setup the universal parts of the block interface just once */
  mmc->block_dev.if_type = IF_TYPE_MMC;
  //mmc->block_dev.dev = cur_dev_num++;
  mmc->block_dev.dev = mmc->control_num;
  mmc->block_dev.removable = 1;
  mmc->block_dev.block_read = mmc_bread;
  mmc->block_dev.block_write = mmc_bwrite;
  mmc->block_dev.block_erase = mmc_berase;
  mmc->block_dev.block_read_mass_pro  = mmc_bread_mass_pro;
  mmc->block_dev.block_write_mass_pro = mmc_bwrite_mass_pro;

  if (!mmc->b_max)
    mmc->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;
   
   return mmc_init(mmc);

}

int mmc_unregister(int dev_num)
{
  mmc_devices[dev_num] = NULL;
  MMCDBG("mmc%d unregister\n",dev_num);
  return 0;
}
