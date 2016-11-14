/** @file
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

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>

#include <Library/SysConfigLib.h>
#include <Library/SunxiBootInfoLib.h>
#include <Library/SunxiBootInfoLib.h>
#include <Guid/SunxiBootInfoHob.h>

#include <Protocol/AxpPower.h>

#include <Interinc/sunxi_uefi.h>
#include <Sunxi_type/Sunxi_type.h>
#include <Sun50iW1P1/platform.h>
#include <Sun50iW1P1/ccmu.h>

#include "gmac_sys.h"
#include "gmac_reg.h"

#define AXP_81X_NAME  ("axp81X")

extern UINT32 SunxiLibGetBootInfo(SUNXI_BOOTINFO_HOB  **Hob);

EFI_STATUS
GetAxpInstanceByName(char* AxpName,  AXP_POWER_PROTOCOL  **Instance)
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    *HandleBuffer;
  UINTN                         NumberOfHandles;
  UINTN                         Index;
  AXP_POWER_ID                  AxpID;
  AXP_POWER_ID                  AxpID_Need;
  AXP_POWER_PROTOCOL  *AxpInstance = NULL;

  if(!AsciiStrCmp(AxpName,AXP_81X_NAME))
  {
    AxpID_Need = AXP_POWER_ID_AXP81X;
  }
  else
  {
    return EFI_DEVICE_ERROR;
  }

  //
  // Locate protocol.
  //
  Status = gBS->LocateHandleBuffer (
                   ByProtocol,
                   &gAxpPowerProtocolGuid,
                   NULL,
                   &NumberOfHandles,
                   &HandleBuffer
                   );
  if (EFI_ERROR (Status)) {
    //
    // Defined errors at this time are not found and out of resources.
    //
    return Status;
  }

  DEBUG((DEBUG_INFO,"AXP NumberOfHandles = %d\n",NumberOfHandles));
  AsciiPrint("AXP NumberOfHandles = %d\n",NumberOfHandles);
  for (Index = 0; Index < NumberOfHandles; Index++) {
    //
    // Get the protocol on this handle
    // This should not fail because of LocateHandleBuffer
    //
    Status = gBS->HandleProtocol (
                     HandleBuffer[Index],
                     &gAxpPowerProtocolGuid,
                     (VOID**) &AxpInstance
                     );
    ASSERT_EFI_ERROR (Status);

    AxpID = AxpInstance->AxpPowerId;
    if(AxpID == AxpID_Need)
    {
      *Instance = AxpInstance;
      break;
    }

  }

  //
  // Our exit status is determined by the success of the previous operations
  // If the protocol was found, Instance already points to it.
  //

  //
  // Free any allocated buffers
  //
  gBS->FreePool (HandleBuffer);

  return Status;
}

void gdma_soft_reset(void)
{
  //assert soft-reset
  MmioWrite32(BASIC_CTL_1, 0x1);

  //wait for phy to be ready
  //while(MmioRead32(BASIC_CTL_1) & 0x1)
  //{
  //  AsciiPrint("Waiting for PHY!\n");
  //  MicroSecondDelay(10*1000);
  //}
}

EFI_STATUS gmac_set_phy_inf_ctrl(PHY_INF phy_inf_sel, TXC_SRC txc_src_sel)
{
  AW_STATUS   status = EFI_SUCCESS;
  u32     val = 0;

  val = MmioRead32(SUNXI_SYSCRL_BASE+0x30);

  val &= (PHY_INF_SEL_MASK ^ REG_MASK_32B);
  val |= PHY_INF_SEL_SET(phy_inf_sel);

  val &= (PHY_TXC_SEL_MASK ^ REG_MASK_32B);
  val |= PHY_TXC_SEL_SET(txc_src_sel);

  MmioWrite32(SUNXI_SYSCRL_BASE+0x30, val);

  MicroSecondDelay(10);
  if (val != MmioRead32(SUNXI_SYSCRL_BASE+0x30))
  {
    status = EFI_NOT_READY;
  }

  return status;
}

EFI_STATUS gmac_txclk_inv_onoff(u32 on)
{
  AW_STATUS   status = EFI_SUCCESS;
  u32 reg_val;

  reg_val = MmioRead32(SUNXI_SYSCRL_BASE+0x30);
  reg_val &= (PHY_TXC_EN_INV_MASK ^ REG_MASK_32B);
  reg_val |= PHY_TXC_EN_INV_SET(on&0x1);
  MmioWrite32(SUNXI_SYSCRL_BASE+0x30, reg_val);

  MicroSecondDelay(10);
  if (reg_val != MmioRead32(SUNXI_SYSCRL_BASE+0x30))
  {
    status = EFI_NOT_READY;
  }

  return status;
}

EFI_STATUS gmac_set_tx_delay_chain(u32 tx_delay)
{
  AW_STATUS   status = EFI_SUCCESS;
  u32     reg_val = 0;

  reg_val = MmioRead32(SUNXI_SYSCRL_BASE+0x30);
  reg_val &= (PHY_TXC_DELAY_MASK ^ REG_MASK_32B);
  reg_val |= PHY_TXC_DELAY_SET(tx_delay);
  MmioWrite32(SUNXI_SYSCRL_BASE+0x30, reg_val);

  MicroSecondDelay(10);
  if (reg_val != MmioRead32(SUNXI_SYSCRL_BASE+0x30))
  {
    status = EFI_NOT_READY;
  }
  return status;
}

EFI_STATUS gmac_set_rx_delay_chain(u32 rx_delay)
{
  AW_STATUS   status = EFI_SUCCESS;
  u32     reg_val = 0;

  reg_val = MmioRead32(SUNXI_SYSCRL_BASE+0x30);
  reg_val &= (PHY_RXC_DELAY_MASK ^ REG_MASK_32B);
  reg_val |= PHY_RXC_DELAY_SET(rx_delay);
  MmioWrite32(SUNXI_SYSCRL_BASE+0x30, reg_val);

  MicroSecondDelay(10);
  if (reg_val != MmioRead32(SUNXI_SYSCRL_BASE+0x30))
  {
    status = EFI_NOT_READY;
  }
  return status;
}

EFI_STATUS gmac_mii_select(PHY_INF phy_inf_sel, TXC_SRC txc_src_sel)
{
  AW_STATUS   status = EFI_SUCCESS;
  u32     val = 0;

  val = MmioRead32(SUNXI_SYSCRL_BASE+0x30);

  val &= (PHY_INF_SEL_MASK ^ REG_MASK_32B);
  val |= PHY_INF_SEL_SET(phy_inf_sel);

  val &= (PHY_TXC_SEL_MASK ^ REG_MASK_32B);
  val |= PHY_TXC_SEL_SET(txc_src_sel);

  MmioWrite32(SUNXI_SYSCRL_BASE+0x30, val);

  MicroSecondDelay(10);
  if (val != MmioRead32(SUNXI_SYSCRL_BASE+0x30))
  {
    status = EFI_NOT_READY;
  }

  return status;
}

EFI_STATUS gmac_rmii_select(u32 rmii_enable, TXC_SRC rmii_clk_sel)
{
  AW_STATUS   status = EFI_SUCCESS;
  u32     val = 0;

  val = MmioRead32(SUNXI_SYSCRL_BASE+0x30);

  val &= (PHY_INF_SEL_MASK ^ REG_MASK_32B);
  val |= (((rmii_enable) << (13)) & (0x00002000));

  val &= (PHY_TXC_SEL_MASK ^ REG_MASK_32B);
  val |= PHY_TXC_SEL_SET(rmii_clk_sel);

  MmioWrite32(SUNXI_SYSCRL_BASE+0x30, val);

  MicroSecondDelay(10);
  if (val != MmioRead32(SUNXI_SYSCRL_BASE+0x30))
  {
    AsciiPrint("gmac rmii select fail!\n");
    status = EFI_NOT_READY;
  }
  
  return status;
}

EFI_STATUS gmac_rgmii_select()
{
  AW_STATUS   status = EFI_SUCCESS;

  //rgmii phy interface
  gmac_set_phy_inf_ctrl(RGMII, RXC_IN);

  gmac_txclk_inv_onoff(0);
  gmac_set_tx_delay_chain(0x1);
  
  gmac_set_tx_delay_chain(0);
  gmac_set_rx_delay_chain(0);

  return status;
}

EFI_STATUS
EFIAPI gmac_phy_power(UINT32 PowerId, INT32 Voltage, UINT32 OnOff)
{
  EFI_STATUS Status = EFI_SUCCESS;
  UINTN PowerBusStatus = 0;
  AXP_POWER_PROTOCOL  *AxpPowerProtocol = NULL;
  
  DEBUG ((EFI_D_INFO, "%a START $$$\n",__FUNCTION__));
  Status = GetAxpInstanceByName(AXP_81X_NAME,&AxpPowerProtocol);
  if (EFI_ERROR (Status))
  {
    DEBUG((EFI_D_ERROR,"GetAxpInstanceByName fail: pmu name is %a\n", AXP_81X_NAME));
    return Status;
  }
  
  AxpPowerProtocol->ProbePowerBusExistance(&PowerBusStatus);
  
  Status = AxpPowerProtocol->SetSupplyStatus(PowerId, Voltage, OnOff);

  return Status;
}


EFI_STATUS
EFIAPI
SunxiEthDxeEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS        Status = EFI_SUCCESS;
  char PhyMode[32];
  char PhyPower[32];
  //INT32       Voltage;
  INT32      Ret;

  UINT32 BootMode;
  SUNXI_BOOTINFO_HOB *Hob = NULL;

  Status = SunxiLibGetBootInfo(&Hob);
  if(Status != EFI_SUCCESS){
    DEBUG((DEBUG_ERROR, "Get Boot Info Hob Failure\n"));
    return Status;
  }
  BootMode = Hob->WorkMode;
  if(BootMode != WORK_MODE_BOOT){
    return RETURN_SUCCESS;
  }
  
  Ret = script_parser_fetch("gmac0", "gmac_power1", (INT32 *)PhyPower, 32/sizeof(INT32));
  if(!Ret)
  {
    if(!AsciiStrCmp(PhyPower, "axp81x_dc1sw"))
    {
      gmac_phy_power(PMU_SUPPLY_DC1SW, 0, 0);
      MicroSecondDelay(10*1000);
      gmac_phy_power(PMU_SUPPLY_DC1SW, 0, 1);
    }
  }
  #if 0
  Ret = script_parser_fetch("gmac0", "gmac_power2", (INT32 *)PhyPower, 32/sizeof(INT32));
  if(!Ret)
  {
    if(!AsciiStrCmp(PhyPower, "gmac-2v5"))
    {
      Ret = script_parser_fetch("power_sply", "dldo3_vol", &Voltage, 1);
      if(Ret)
        Voltage = 2500;
      
      DEBUG((EFI_D_INIT, "gmac-2v5 voltage = %d\n", Voltage));
      gmac_phy_power(PMU_SUPPLY_DLDO3, Voltage, 1);
    }
  }
  #endif 
  MicroSecondDelay(10*1000);
  
#if 0
  //config init
  gmac_init_config();
  
  //0:loopback mode   1:normal mode
  gmac_conf.basic_conf.phy_conf.set_loopback_mode = BIT_SET;
  gmac_conf.basic_conf.loopback_mode              = BIT_SET;
  gmac_conf.basic_conf.phy_conf.en_AN             = BIT_SET;
  gmac_conf.basic_conf.select_gmii_mii            = BIT_SET;
  gmac_conf.basic_conf.phy_conf.sel_phy_speed     = PHY_10MBPS;
  //gmac core init
  //gmac_init(&gmac_conf, &gdma_conf);
#endif 

  //gmac clock and reset
  MmioAnd32(CCMU_BUS_SOFT_RST_REG0,~(1<<17));
  MicroSecondDelay(10*1000);
  MmioOr32(CCMU_BUS_SOFT_RST_REG0,(1<<17));
  MicroSecondDelay(10*1000);
  MmioOr32(CCMU_BUS_CLK_GATING_REG0,(1<<17));
  MicroSecondDelay(10*1000);

  // mii/rmii/rgmii select
  Ret = script_parser_fetch("gmac0", "phy-mode", (INT32 *)PhyMode, 32/sizeof(INT32));
  if(Ret)
  {
    DEBUG ((EFI_D_ERROR, "parser fetch gmac0 phy-mode failed,set to device by default\n"));
    AsciiStrCpy(PhyMode, "rgmii");
  }
  DEBUG ((EFI_D_INIT, "%a: phy mode = %a\n",__FUNCTION__, PhyMode));

  if(!AsciiStrCmp(PhyMode, "mii"))
  {
    gmac_mii_select(GMII_MII, 0);
  }
  else if(!AsciiStrCmp(PhyMode, "rmii"))
  {
    gmac_rmii_select(1, CLK125);  
  }
  else
  {
    DEBUG ((EFI_D_INIT, "gmac_rgmii_select\n"));
    gmac_rgmii_select();
  }

  //Soft reset
  MicroSecondDelay(10*1000);
  gdma_soft_reset();

  return Status;
}



