/** @file
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

#include <Protocol/AxpPower.h>

#include <Interinc/sunxi_uefi.h>
#include <Sunxi_type/Sunxi_type.h>
#include <Sun50iW1P1/platform.h>
#include <Sun50iW1P1/cpu.h>
#include <Sun50iW1P1/ccmu.h>
#include <Sun50iW1P1/clock.h>
//#include <Sun50iW1P1/usb.h>
#include "Sunxi_USB2Phy.h"

#define AXP_81X_NAME  ("axp81X")

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


VOID
SunxiUsb0PowerInit(
 UINT32 PortType
 )
{
  EFI_STATUS Status;
  UINTN PowerBusStatus = 0;
  AXP_POWER_PROTOCOL  *AxpPowerProtocol = NULL;
  
  DEBUG ((EFI_D_INFO, "%a START $$$\n",__FUNCTION__));
  Status = GetAxpInstanceByName(AXP_81X_NAME,&AxpPowerProtocol);
  if (EFI_ERROR (Status))
  {
    DEBUG((EFI_D_ERROR,"GetAxpInstanceByName fail: pmu name is %a\n", AXP_81X_NAME));
    return;
  }
  
  AxpPowerProtocol->ProbePowerBusExistance(&PowerBusStatus);
    
  switch(PortType)
  {
    case 0://device only
      Status = AxpPowerProtocol->SetSupplyStatus(PMU_SUPPLY_VBUSEN, 0, 0);
      break;
    case 1://host only
      Status = AxpPowerProtocol->SetSupplyStatus(PMU_SUPPLY_VBUSEN, 0, 1);
      break;
    case 2://otg
      if(PowerBusStatus&AXP_POWER_VBUS_EXIST){
        DEBUG((EFI_D_ERROR,"Axp Vbus exist!\n"));
        Status = AxpPowerProtocol->SetSupplyStatus(PMU_SUPPLY_VBUSEN, 0, 0);
      }
      else
      {
        Status = AxpPowerProtocol->SetSupplyStatus(PMU_SUPPLY_VBUSEN, 0, 1);   
      }
    default:
      DEBUG((EFI_D_ERROR,"wrong usbc0 port type,please check the sysconfig\n"));
      break;
  }
  
  if (EFI_ERROR (Status))
  {
    DEBUG((EFI_D_ERROR,"set usbc0 vbus power failed\n"));
  } 
}

static void UsbOpenOtgClock(void)
{
  //Enable module clock for USB phy0
  MmioOr32(SUNXI_CCU_USB_CLK,(1 << 0) | (1 << 8));
  //delay some time
  MicroSecondDelay(10*1000);

  //Gating AHB clock for USB_phy0
  MmioOr32(CCMU_BUS_CLK_GATING_REG0,(1 << 23));

  //delay to wati SIE stable
  MicroSecondDelay(10*1000);

  MmioOr32(CCMU_BUS_SOFT_RST_REG0,(1 << 23));
  MicroSecondDelay(10*1000);

  MmioOr32(SUNXI_USBOTG_BASE+0x420,(0x01 << 0));
  MicroSecondDelay(10*1000);

  MmioAnd32(SUNXI_USBOTG_BASE+0x410,~(0x01 << 1));
  MicroSecondDelay(10*1000);
}

VOID 
UsbHostAhbClkInit(
  VOID
  )
{
  DEBUG ((EFI_D_ERROR, "%a\n",__FUNCTION__));
  
  //USB HOST AHB clk gate
  MmioOr32(CCMU_BUS_CLK_GATING_REG0,(1<<24));
  MmioOr32(CCMU_BUS_CLK_GATING_REG0,(1<<25));
  //ohci clk
  MmioOr32(CCMU_BUS_CLK_GATING_REG0,(1<<28));
  MmioOr32(CCMU_BUS_CLK_GATING_REG0,(1<<29));

  MicroSecondDelay(10*1000);
  //de-assert ehci clk
  MmioOr32(CCMU_BUS_SOFT_RST_REG0,(1<<24));
  MmioOr32(CCMU_BUS_SOFT_RST_REG0,(1<<25));
  //de-assert ohci clk
  MmioOr32(CCMU_BUS_SOFT_RST_REG0,(1<<28));
  MmioOr32(CCMU_BUS_SOFT_RST_REG0,(1<<29));

  MicroSecondDelay(10*1000);
  //open usb otg clock.
  UsbOpenOtgClock();
  
  return;
}

static int UsbPhy20Init(UINT32 HostNumber, UINT32 ohci)
{

  int reg_value = 0;
  DEBUG((EFI_D_ERROR,"open hci clock usbc_no:%d, is_ohci:%d\n",HostNumber, ohci));

  switch (HostNumber){
    case 0:
      //switch to host role.
      reg_value = USBC_Readl(USB_OTG_HCCAPBASE+0x420);
      reg_value &= ~(0x01<<0);
      USBC_Writel(reg_value,USB_OTG_HCCAPBASE+0x420);
    
      /*PHY*/
      reg_value = USBC_Readl(SUNXI_CCU_USB_CLK);
      reg_value |= (0x01<<8);// enable SCLK_GATING_USB0_PHY
    
      if(ohci){
        reg_value |= (0x1<<16);//enable USB OHCI0 Special Clock(12M and 48M) Gating
      }

      USBC_Writel(reg_value, SUNXI_CCU_USB_CLK);

      //configure phy itself
      //De-assert USBPHY0
      reg_value = USBC_Readl(SUNXI_CCU_USB_CLK);
      reg_value |=(1<<0);
      USBC_Writel(reg_value,SUNXI_CCU_USB_CLK);

      MicroSecondDelay(10*1000);

      reg_value = USBC_Readl(USB_EHCI0_HCCAPBASE+0x800);
      reg_value |= (0x03<<8);
      reg_value &= (~(0x01<<1));
      reg_value |= (0x01<<0);
      USBC_Writel(reg_value,USB_EHCI0_HCCAPBASE+0x800);
    
      /*Phy Clock Open*/
      reg_value = USBC_Readl(USB_EHCI0_HCCAPBASE+0x810);
      reg_value &= ~(0x01<<1);//Clean SIDDP
      USBC_Writel(reg_value,USB_EHCI0_HCCAPBASE+0x810);
      break;
    
    case 1:
      /*PHY*/
      reg_value = USBC_Readl(SUNXI_CCU_USB_CLK);
      reg_value |= (0x01<<9);// enable SCLK_GATING_USB1_PHY
    
      if(ohci){
        reg_value |= (0x1<<17);//enable USB OHCI1 Special Clock(12M and 48M) Gating
      }

      USBC_Writel(reg_value, SUNXI_CCU_USB_CLK);

      //configure phy itself
      //De-assert USBPHY0
      reg_value = USBC_Readl(SUNXI_CCU_USB_CLK);
      reg_value |=(1<<1);
      USBC_Writel(reg_value,SUNXI_CCU_USB_CLK);

      MicroSecondDelay(10*1000);

      reg_value = USBC_Readl(USB_EHCI1_HCCAPBASE+0x800);
      reg_value |= (0x03<<8);
      reg_value &= (~(0x01<<1));
      reg_value |= (0x01<<0);
      USBC_Writel(reg_value,USB_EHCI1_HCCAPBASE+0x800);
    
      /*Phy Clock Open*/
      reg_value = USBC_Readl(USB_EHCI1_HCCAPBASE+0x810);
      reg_value &= ~(0x01<<1);//Clean SIDDP
      USBC_Writel(reg_value,USB_EHCI1_HCCAPBASE+0x810);
      break;

    default:
      return 0;

  }
  return 0;
}

#if 1
static VOID EhciPassBy(UINT32 HostNumber, UINT32 Enable)
{
  unsigned long reg_value = 0;
  UINT32 UsbVbase = USB_EHCI0_HCCAPBASE+(HostNumber)*0x1000;
  /*enable passby*/
  reg_value = USBC_Readl(UsbVbase+ SW_USB_PMU_IRQ_ENABLE);
  if(Enable){
    reg_value |= (1 << 10);   /* AHB Master interface INCR8 enable */
    reg_value |= (1 << 9);    /* AHB Master interface burst type INCR4 enable */
    reg_value |= (1 << 8);    /* AHB Master interface INCRX align enable */
    reg_value |= (1 << 0);    /* enable UTMI, disable ULPI */
  }else if(!Enable){
    reg_value &= ~(1 << 10);  /* AHB Master interface INCR8 disable */
    reg_value &= ~(1 << 9);   /* AHB Master interface burst type INCR4 disable */
    reg_value &= ~(1 << 8);   /* AHB Master interface INCRX align disable */
    reg_value &= ~(1 << 0);   /* ULPI bypass disable */
  }
  USBC_Writel(reg_value, (UsbVbase + SW_USB_PMU_IRQ_ENABLE));

  return;
}
#endif

VOID
SunxiUsb20Init(
 VOID
 )
{
  user_gpio_set_t     usb_gpio;
  INT32 Ret;
  UINT32 GpioHd;
  INT32 PortType;
  DEBUG ((EFI_D_ERROR, "%a START $$$\n",__FUNCTION__));
  UsbHostAhbClkInit();

  Ret = script_parser_fetch("usbc0", "usb_port_type", &PortType, 1);
  if(Ret)
  {
    DEBUG ((EFI_D_ERROR, "parser fetch usbc0 usb_port_type failed,set to device by default\n"));
    PortType  = 0;
  }

  SunxiUsb0PowerInit(PortType);
  if(PortType)
  {
    UsbPhy20Init(0,1);
    EhciPassBy(0,1);
  }
  
  UsbPhy20Init(1,1);
  EhciPassBy(1,1);
      
  Ret = script_parser_fetch("usbc1", "usb_drv_vbus_gpio", (int *)&usb_gpio, sizeof(user_gpio_set_t)/4);
  if(Ret)
  {
    DEBUG ((EFI_D_ERROR, "parser fetch usbc1 usb_drv_vbus_gpio failed\n"));
    return;
  }

  usb_gpio.data =1;
  GpioHd = gpio_request(&usb_gpio, 1);
  
  if(!GpioHd)
  {
    DEBUG ((EFI_D_ERROR, "Request usbc1 usb_drv_vbus_gpio failed\n"));
    return;
  }

  Ret=gpio_release(GpioHd,2);//release the gpio,but keep the hw state.
  if(Ret)
  {
    DEBUG ((EFI_D_ERROR, "Release usbc1 usb_drv_vbus_gpio failed\n"));
    return;
  }

  DEBUG ((EFI_D_ERROR, "%a END $$$\n",__FUNCTION__));
}

VOID 
SunxiUsb20Off(
  VOID
  )
{

}


