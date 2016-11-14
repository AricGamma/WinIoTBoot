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

#ifndef _EFI_SUNXI_USB2_PHY_H_
#define _EFI_SUNXI_USB2_PHY_H_

//0:EHCI0, 1:EHCI1
#define EHCI_PORT            0
#define USB_EHCI_HCCAPBASE   (USB_EHCI0_HCCAPBASE + EHCI_PORT * 0x1000)
#define USB_OHCI_HCCAPBASE   (USB_EHCI0_HCCAPBASE + EHCI_PORT * 0x1000 + 0x400)
#define USB_EHCI0_HCCAPBASE        (0x01c1a000) //usb_hci0
#define USB_EHCI1_HCCAPBASE        (0x01c1b000) //usb_hci1
#define USB_OHCI2_HCCAPBASE        (USB_EHCI1_HCCAPBASE + 0x1000) //usb_ohci2
#define USB_OTG_HCCAPBASE          (0x01c19000)
#define USB_PHY_CSR                (USB_OTG_HCCAPBASE + 0x404)
#define SUNXI_CCU_REG_BASE         (0x01c20000)
#define SUNXI_CCU_AHB1_GATE0       (SUNXI_CCU_REG_BASE + 0x60)
#define SUNXI_CCU_USB_CLK          (SUNXI_CCU_REG_BASE + 0xCC)
#define SUNXI_CCU_MOD_RST          (SUNXI_CCU_REG_BASE + 0x2C0)
#define CMU_ECHI_AHB1_GATE(n)      (0x1 << (26 + n))
#define CMU_OCHI_AHB1_GATE(n)      (0x1 << (29 + n))
#define CMU_ECHI_AHB1_RST(n)       (0x1 << (26 + n))
#define CMU_OCHI_AHB1_RST(n)       (0x1 << (29 + n))
#define CMU_USB_OHCI_PHY_GATE(n)   (0x1 << (16 + n))
#define CMU_USB_PHY_GATE(n)        (0x1 << (9 + n))
#define CMU_USB_PHY_RST(n)         (0x1 << (1 + n))
#define SW_USB_PMU_IRQ_ENABLE      (0x800)
#define AHB_INCR8_ENABLE           (0x1 << 10)
#define AHB_BURST_INCR4_ENABLE     (0x1 << 9)
#define AHB_INCRRX_ENABLE          (0x1 << 8)
#define ULPI_BYPASS_ENABLE         (0x1 << 0)
#define USBC_Readl(x)       MmioRead32(x)
#define USBC_Writel(v,a)      MmioWrite32(a,v)

//-------------------------------------------------------
// Functions
//-------------------------------------------------------

VOID SunxiUsb20Init(VOID);

#endif


