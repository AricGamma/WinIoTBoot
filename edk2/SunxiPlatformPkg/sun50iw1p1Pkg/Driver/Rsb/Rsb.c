/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  WangWei <wangwei@allwinnertech.com>
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
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/SunxiSmcLib.h>
#include <platform.h>
#include <rsb.h>


typedef struct  {
  volatile UINT32 rsb_flag;
  volatile UINT32 rsb_busy;
  volatile UINT32 rsb_load_busy;
}rsb_info;

typedef  struct {
  UINT8 *m_slave_name;
  UINT32 m_slave_addr;
  UINT32 m_rtaddr;
}rsb_slave_set;

typedef struct 
{
  UINT8 *m_slave_name;
  UINT32 m_slave_addr;
  UINT32 m_rtaddr;
  UINT32 chip_id;
}sunxi_rsb_slave_set;

#define SUNXI_RSB_SLAVE_MAX      13

#define CD_HIGH    0x3
#define CD_LOW     0x1
#define CK_HIGH    (0x3<<2)
#define CK_LOW     (0x1<<2)

static UINT32 gSecureAccess= 1;

static  rsb_info rsbc = {1, 1, 1};
//static  sunxi_rsb_slave_set rsb_slave[SUNXI_RSB_SLAVE_MAX]={{NULL, 1, 1, 1}};
//static int    sunxi_rsb_rtsaddr[16] = {0x2d, 0x3a, 0x4e, 0x59, 0x63, 0x74, 0x8b, 0x9c, 0xa6, 0xb1, 0xc5, 0xd2, 0xe8, 0xff};

static void rsb_cfg_io(void)
{
  UINT32 reg,val;
  //if r_pio gating not open
  reg = SUNXI_RPRCM_BASE + 0x28;
  val = MmioRead32(reg);
  if( !(MmioRead32(reg)&0x1))
  {
    val |= 0x1; //open r_pio
    MmioWrite32(reg,val);
  }

  //PL0,PL1 cfg 2
  reg = SUNXI_RPIO_BASE;
  val = MmioRead32(reg)  & (~0xff);
  val |= 0x22;
  MmioWrite32(reg,val);
  //PL0,PL1 pull up 1
  reg = SUNXI_RPIO_BASE+0x1c;
  val = MmioRead32(reg)  & (~0xf);
  val |= 0x5;
  MmioWrite32(reg,val);
  //PL0,PL1 drv 2
  reg = SUNXI_RPIO_BASE+0x14;
  val = MmioRead32(reg)  & (~0xf);
  val |= 0xa;
  MmioWrite32(reg,val);
}


static void rsb_module_reset(void)
{
  //  r_prcm_module_reset(R_RSB_CKID);
  MmioWrite32(SUNXI_RPRCM_BASE + 0xb0, MmioRead32(SUNXI_RPRCM_BASE + 0xb0)& ~(0x1U << 3));
  MmioWrite32(SUNXI_RPRCM_BASE + 0xb0, MmioRead32(SUNXI_RPRCM_BASE + 0xb0)|(0x1U << 3));
}

static void rsb_clock_enable(void)
{
  MmioWrite32(SUNXI_RPRCM_BASE + 0x28, MmioRead32(SUNXI_RPRCM_BASE + 0x28)|(0x1U << 3));
}

static void rsb_set_clk(UINT32 sck)
{
  UINT32 src_clk = 0;
  UINT32 div = 0;
  UINT32 cd_odly = 0;
  UINT32 rval = 0;

  src_clk = 24000000;

  div = src_clk/sck/2;
  if(0==div){
    div = 1;
    DEBUG((EFI_D_ERROR,"Source clock is too low\n"));
  }else if(div>256){
    div = 256;
    DEBUG((EFI_D_ERROR,"Source clock is too high\n"));
  }
  div--;
  cd_odly = div >> 1;
  //cd_odly = 1;
  if(!cd_odly)
    cd_odly = 1;
  rval = div | (cd_odly << 8);
  MmioWrite32(RSB_REG_CCR, rval);
}

/*  RSB function  */
#ifdef RSB_USE_INT
static void rsb_irq_handler(void)
{
  UINT32 istat = MmioRead32(RSB_REG_STAT);

  if(istat & RSB_LBSY_INT){
    rsbc.rsb_load_busy = 1;
  }

  if (istat & RSB_TERR_INT) {
    rsbc.rsb_flag = (istat >> 8) & 0xffff;
  }

  if (istat & RSB_TOVER_INT) {
    rsbc.rsb_busy = 0;
  }

  MmioWrite32( RSB_REG_STAT, ista);
}
#endif

static void rsb_init(void)
{
  rsbc.rsb_flag = 0;
  rsbc.rsb_busy = 0;
  rsbc.rsb_load_busy  = 0;

  rsb_cfg_io();
  rsb_module_reset();
  rsb_clock_enable();

  MmioWrite32(RSB_REG_CTRL, RSB_SOFT_RST);
  rsb_set_clk(RSB_SCK);
#ifdef RSB_USE_INT
  MmioWrite32(RSB_REG_CTRL, RSB_GLB_INTEN);
  MmioWrite32(RSB_REG_INTE,RSB_TOVER_INT|RSB_TERR_INT|RSB_LBSY_INT);
  irq_request(GIC_SRC_RRSB, rsb_irq_handler);
  irq_enable(GIC_SRC_RRSB);
#endif
}

//
static INT32 rsb_send_initseq(UINT32 slave_addr, UINT32 reg, UINT32 data)
{
  while(MmioRead32(RSB_REG_STAT)&(RSB_LBSY_INT|RSB_TERR_INT|RSB_TOVER_INT))
  {
    DEBUG((EFI_D_ERROR,"status err\n"));
  }

  rsbc.rsb_busy = 1;
  rsbc.rsb_flag = 0;
  rsbc.rsb_load_busy = 0;
  MmioWrite32(RSB_REG_PMCR, RSB_PMU_INIT|(slave_addr << 1)        \
          |(reg << PMU_MOD_REG_ADDR_SHIFT)      \
          |(data << PMU_INIT_DAT_SHIFT));
  while(MmioRead32(RSB_REG_PMCR) & RSB_PMU_INIT){
  }

  while(rsbc.rsb_busy){
#ifndef RSB_USE_INT
    //istat will be optimize?
    UINT32 istat = MmioRead32(RSB_REG_STAT);

    if(istat & RSB_LBSY_INT){
      rsbc.rsb_load_busy = 1;
      MmioWrite32(RSB_REG_STAT, istat);
      break;
    }

    if (istat & RSB_TERR_INT) {
      rsbc.rsb_flag = (istat >> 8) & 0xffff;
      MmioWrite32(RSB_REG_STAT, istat);
      break;
    }

    if (istat & RSB_TOVER_INT) {
      rsbc.rsb_busy = 0;
      MmioWrite32(RSB_REG_STAT, istat);
    }
#endif
  }

  if(rsbc.rsb_load_busy){
    DEBUG((EFI_D_ERROR,"Load busy\n"));
    return RET_FAIL;
  }

  if (rsbc.rsb_flag) {
    DEBUG((EFI_D_ERROR, "rsb write failed, flag 0x%x:%a%a%a%a%a !!\n",
          rsbc.rsb_flag,
          rsbc.rsb_flag & ERR_TRANS_1ST_BYTE  ? " 1STE "  : "",
          rsbc.rsb_flag & ERR_TRANS_2ND_BYTE  ? " 2NDE "  : "",
          rsbc.rsb_flag & ERR_TRANS_3RD_BYTE  ? " 3RDE "  : "",
          rsbc.rsb_flag & ERR_TRANS_4TH_BYTE  ? " 4THE "  : "",
          rsbc.rsb_flag & ERR_TRANS_RT_NO_ACK ? " NOACK " : ""
          ));
    return -rsbc.rsb_flag;
  }

  return 0;
}


static INT32 set_run_time_addr(UINT32 saddr,UINT32 rtsaddr)
{

  while(MmioRead32(RSB_REG_STAT)&(RSB_LBSY_INT|RSB_TERR_INT|RSB_TOVER_INT))
  {
    DEBUG((EFI_D_ERROR,"status err\n"));
  }

  rsbc.rsb_busy = 1;
  rsbc.rsb_flag = 0;
  rsbc.rsb_load_busy = 0;
  MmioWrite32(RSB_REG_SADDR, (saddr<<RSB_SADDR_SHIFT) |(rtsaddr<<RSB_RTSADDR_SHIFT));
  MmioWrite32(RSB_REG_CMD, RSB_CMD_SET_RTSADDR);
  MmioWrite32(RSB_REG_CTRL, MmioRead32(RSB_REG_CTRL)|RSB_START_TRANS);

  while(rsbc.rsb_busy){
#ifndef RSB_USE_INT
    //istat will be optimize?
    UINT32 istat = MmioRead32(RSB_REG_STAT);

    if(istat & RSB_LBSY_INT){
      rsbc.rsb_load_busy = 1;
      MmioWrite32(RSB_REG_STAT, istat);
      break;
    }

    if (istat & RSB_TERR_INT) {
      rsbc.rsb_flag = (istat >> 8) & 0xffff;
      MmioWrite32(RSB_REG_STAT, istat);
      break;
    }

    if (istat & RSB_TOVER_INT) {
      rsbc.rsb_busy = 0;
      MmioWrite32(RSB_REG_STAT, istat);
    }
#endif
  }

  if(rsbc.rsb_load_busy){
    DEBUG((EFI_D_ERROR,"Load busy\n"));
    return RET_FAIL;
  }

  if (rsbc.rsb_flag) {
    DEBUG((EFI_D_ERROR, "rsb set run time address failed, flag 0x%x:%a%a%a%a%a !!\n",
          rsbc.rsb_flag,
          rsbc.rsb_flag & ERR_TRANS_1ST_BYTE  ? " 1STE "  : "",
          rsbc.rsb_flag & ERR_TRANS_2ND_BYTE  ? " 2NDE "  : "",
          rsbc.rsb_flag & ERR_TRANS_3RD_BYTE  ? " 3RDE "  : "",
          rsbc.rsb_flag & ERR_TRANS_4TH_BYTE  ? " 4THE "  : "",
          rsbc.rsb_flag & ERR_TRANS_RT_NO_ACK ? " NOACK " : ""
          ));
    return -rsbc.rsb_flag;
  }
  return 0;
}


//INT32 rsb_write(UINT32 rtsaddr,struct rsb_ad *ad, UINT32 len)
static INT32 rsb_write(UINT32 rtsaddr,UINT32 daddr, UINT8 *data,UINT32 len)
{
  UINT32 cmd = 0;
  UINT32 dat = 0;
  INT32 i = 0;
  if (len > 4 || len==0||len==3) {
    DEBUG((EFI_D_ERROR,"error length %d\n", len));
    return -1;
  }
  if(NULL==data){
    DEBUG((EFI_D_ERROR,"data should not be NULL\n"));
    return -1;
  }

  while(MmioRead32(RSB_REG_STAT)&(RSB_LBSY_INT|RSB_TERR_INT|RSB_TOVER_INT))
  {
    DEBUG((EFI_D_ERROR,"status err\n"));
  }

  rsbc.rsb_flag = 0;
  rsbc.rsb_busy = 1;
  rsbc.rsb_load_busy  = 0;

  MmioWrite32(RSB_REG_SADDR, rtsaddr<<RSB_RTSADDR_SHIFT);
  MmioWrite32(RSB_REG_DADDR0, daddr);

  for(i=0;i<len;i++){
    dat |= data[i]<<(i*8);
  }

  MmioWrite32(RSB_REG_DATA0, dat);

  switch(len) {
  case 1:
    cmd = RSB_CMD_BYTE_WRITE;
    break;
  case 2:
    cmd = RSB_CMD_HWORD_WRITE;
    break;
  case 4:
    cmd = RSB_CMD_WORD_WRITE;
    break;
  default:
    break;
  }
  MmioWrite32(RSB_REG_CMD, cmd);

  MmioWrite32(RSB_REG_CTRL, MmioRead32(RSB_REG_CTRL)|RSB_START_TRANS);
  while(rsbc.rsb_busy){
#ifndef RSB_USE_INT
    //istat will be optimize?
    UINT32 istat = MmioRead32(RSB_REG_STAT);

    if(istat & RSB_LBSY_INT){
      rsbc.rsb_load_busy = 1;
      MmioWrite32(RSB_REG_STAT, istat);
      break;
    }

    if (istat & RSB_TERR_INT) {
      rsbc.rsb_flag = (istat >> 8) & 0xffff;
      MmioWrite32(RSB_REG_STAT, istat);
      break;
    }

    if (istat & RSB_TOVER_INT) {
      rsbc.rsb_busy = 0;
      MmioWrite32(RSB_REG_STAT, istat);
    }
#endif
  }

  if(rsbc.rsb_load_busy){
    DEBUG((EFI_D_ERROR,"Load busy\n"));
    return RET_FAIL;
  }

  if (rsbc.rsb_flag) {
    DEBUG((EFI_D_ERROR, "rsb write failed, flag 0x%x:%a%a%a%a%a !!\n",
          rsbc.rsb_flag,
          rsbc.rsb_flag & ERR_TRANS_1ST_BYTE  ? " 1STE "  : "",
          rsbc.rsb_flag & ERR_TRANS_2ND_BYTE  ? " 2NDE "  : "",
          rsbc.rsb_flag & ERR_TRANS_3RD_BYTE  ? " 3RDE "  : "",
          rsbc.rsb_flag & ERR_TRANS_4TH_BYTE  ? " 4THE "  : "",
          rsbc.rsb_flag & ERR_TRANS_RT_NO_ACK ? " NOACK " : ""
          ));
    return -rsbc.rsb_flag;
  }

  return 0;
}


//INT32 rsb_read(UINT32 rtsaddr,struct rsb_ad *ad, UINT32 len)
static INT32 rsb_read(UINT32 rtsaddr,UINT32 daddr, UINT8 *data, UINT32 len)
{
  UINT32 cmd = 0;
  UINT32 dat = 0;
  INT32 i = 0;
  if (len > 4 || len==0||len==3) {
    DEBUG((EFI_D_ERROR,"error length %d\n", len));
    return -1;
  }
  if(NULL==data){
    DEBUG((EFI_D_ERROR,"data should not be NULL\n"));
    return -1;
  }

  while(MmioRead32(RSB_REG_STAT)&(RSB_LBSY_INT|RSB_TERR_INT|RSB_TOVER_INT))
  {
    DEBUG((EFI_D_ERROR,"status err\n"));
  }

  rsbc.rsb_flag = 0;
  rsbc.rsb_busy = 1;
  rsbc.rsb_load_busy  = 0;

  MmioWrite32(RSB_REG_SADDR, rtsaddr<<RSB_RTSADDR_SHIFT);
  MmioWrite32(RSB_REG_DADDR0, daddr);
//  MmioWrite32((len-1)|RSB_READ_FLAG, RSB_REG_DLEN);

  switch(len){
  case 1:
    cmd = RSB_CMD_BYTE_READ;
    break;
  case 2:
    cmd = RSB_CMD_HWORD_READ;
    break;
  case 4:
    cmd = RSB_CMD_WORD_READ;
    break;
  default:
    break;
  }
  MmioWrite32(RSB_REG_CMD,cmd);

  MmioWrite32(RSB_REG_CTRL,MmioRead32(RSB_REG_CTRL)|RSB_START_TRANS);
  while(rsbc.rsb_busy){
#ifndef RSB_USE_INT
    //istat will be optimize?
    UINT32 istat = MmioRead32(RSB_REG_STAT);

    if(istat & RSB_LBSY_INT){
      rsbc.rsb_load_busy = 1;
      MmioWrite32(RSB_REG_STAT, istat);
      break;
    }

    if (istat & RSB_TERR_INT) {
      rsbc.rsb_flag = (istat >> 8) & 0xffff;
      MmioWrite32(RSB_REG_STAT, istat);
      break;
    }

    if (istat & RSB_TOVER_INT) {
      rsbc.rsb_busy = 0;
      MmioWrite32(RSB_REG_STAT, istat);
    }
#endif
  }

  if(rsbc.rsb_load_busy){
    DEBUG((EFI_D_ERROR,"Load busy\n"));
    return RET_FAIL;
  }

  if (rsbc.rsb_flag) {
    DEBUG((EFI_D_ERROR, "rsb read failed, flag 0x%x:%a%a%a%a%a !!\n",
          rsbc.rsb_flag,
          rsbc.rsb_flag & ERR_TRANS_1ST_BYTE  ? " 1STE "  : "",
          rsbc.rsb_flag & ERR_TRANS_2ND_BYTE  ? " 2NDE "  : "",
          rsbc.rsb_flag & ERR_TRANS_3RD_BYTE  ? " 3RDE "  : "",
          rsbc.rsb_flag & ERR_TRANS_4TH_BYTE  ? " 4THE "  : "",
          rsbc.rsb_flag & ERR_TRANS_RT_NO_ACK ? " NOACK " : ""
          ));
    return -rsbc.rsb_flag;
  }


  dat = MmioRead32(RSB_REG_DATA0);
  for(i=0;i<len;i++){
    data[i]=(dat>>(i*8))&0xff;
  }

  return 0;
}


EFI_STATUS SunxiRsbInit(VOID)
{
  
  INT32 ret = 0;
  if(gSecureAccess)
  {
    return EFI_SUCCESS;
  }
  rsb_init();
  ret = rsb_send_initseq(0x00, 0x3e, 0x7c);
  if(ret != 0 )
  {
    return EFI_DEVICE_ERROR;;
  }
  
   return EFI_SUCCESS;
}


EFI_STATUS SunxiRsbConfig(UINT32 saddr, UINT32 rtsaddr)
{
  INT32 ret = 0;
  if(gSecureAccess)
  {
    return EFI_SUCCESS;
  }
  ret = set_run_time_addr(saddr, rtsaddr);
  if(ret != 0 )
  {
    return EFI_DEVICE_ERROR;;
  }

  return EFI_SUCCESS;
}
static INT32 SunxiRsbReadSecureOS(UINT32 slave_id, UINT32 daddr, UINT8 *data, UINT32 len)
{
  INT32 ret = 0;
  ret = (UINT8)(arm_svc_arisc_read_pmu((UINT32)daddr));
  if(ret < 0 )
  {
    return EFI_DEVICE_ERROR;
  }
  *data = (UINT8)(ret&0xff);
  return EFI_SUCCESS;
}

static INT32 SunxiRsbWriteSecureOS(UINT32 slave_id, UINT32 daddr, UINT8 *data, UINT8 len)
{
  return arm_svc_arisc_write_pmu((UINT32)daddr,(UINT32)(*data));
}



EFI_STATUS SunxiRsbRead(UINT32 rtsaddr, UINT32 daddr, UINT8 *data, UINT32 len)
{
  if(gSecureAccess)
  {
    return SunxiRsbReadSecureOS(rtsaddr,daddr,data,len);
  }
  if(0 != rsb_read(rtsaddr,daddr, data,len))
  {
    return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;
}


EFI_STATUS SunxiRsbWrite(UINT32 rtsaddr, UINT32 daddr, UINT8 *data, UINT32 len)
{
  if(gSecureAccess)
  {
    return SunxiRsbWriteSecureOS(rtsaddr,daddr,data,len);
  }
  if(0 != rsb_write(rtsaddr,daddr, data,len))
  {
    return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;
}

