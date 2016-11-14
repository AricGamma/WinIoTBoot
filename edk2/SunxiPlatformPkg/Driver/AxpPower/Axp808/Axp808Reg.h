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

#ifndef   __Axp808_REGS_H__
#define   __Axp808_REGS_H__

#define   RSB_RTSADDR_AXP808             (0x3a)
#define   AXP808_ADDR RSB_RTSADDR_AXP808 

//define Axp808 REGISTER
#define   BOOT_POWER808_STARUP_SRC                  (0x00)
#define   BOOT_POWER808_VERSION                   (0x03)
#define   BOOT_POWER808_DATA_BUFFER0              (0x04)
#define   BOOT_POWER808_DATA_BUFFER1              (0x05)
#define   BOOT_POWER808_DATA_BUFFER2              (0x06)
#define   BOOT_POWER808_DATA_BUFFER3              (0x07)

#define   BOOT_POWER808_OUTPUT_CTL1               (0x10)
#define   BOOT_POWER808_OUTPUT_CTL2               (0x11)

#define   BOOT_POWER808_DCAOUT_VOL                    (0x12)
#define   BOOT_POWER808_DCBOUT_VOL                (0x13)
#define   BOOT_POWER808_DCCOUT_VOL                (0x14)
#define   BOOT_POWER808_DCDOUT_VOL                (0x15)
#define   BOOT_POWER808_DCEOUT_VOL                (0x16)
#define   BOOT_POWER808_ALDO1OUT_VOL                    (0x17)
#define   BOOT_POWER808_ALDO2OUT_VOL                    (0x18)
#define   BOOT_POWER808_ALDO3OUT_VOL                    (0x19)
#define   BOOT_POWER808_DCMOD_CTL1                    (0x1A)
#define   BOOT_POWER808_DCMOD_CTL2                    (0x1B)
#define   BOOT_POWER808_DCFREQ_SET                    (0x1C)
#define   BOOT_POWER808_DCMONITOR_CTL                   (0x1D)

#define   BOOT_POWER808_BLDO1OUT_VOL                    (0x20)
#define   BOOT_POWER808_BLDO2OUT_VOL                    (0x21)
#define   BOOT_POWER808_BLDO3OUT_VOL                    (0x22)
#define   BOOT_POWER808_BLDO4OUT_VOL                    (0x23)

#define   BOOT_POWER808_CLDO1OUT_VOL                    (0x24)
#define   BOOT_POWER808_CLDO2OUT_VOL                    (0x25)
#define   BOOT_POWER808_CLDO3OUT_VOL                    (0x26)

#define   BOOT_POWER808_VOFF_SETTING                    (0x31)
#define   BOOT_POWER808_POK_SETTING                     (0x36)

#define   BOOT_POWER808_INTEN1                    (0x40)
#define   BOOT_POWER808_INTEN2                    (0x41)
#define   BOOT_POWER808_INTSTS1                   (0x48)
#define   BOOT_POWER808_INTSTS2                   (0x49)



#endif /* __AXP221_REGS_H__ */
