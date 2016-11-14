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

#ifndef   __AXP81X_H__
#define   __AXP81X_H__

#include <Driver/AxpPower/Axp81X/Axp81XReg.h>

extern EFI_STATUS Axp81XProbe(VOID);
extern EFI_STATUS Axp81XSetChargerOnOff(IN UINTN OnOff);
extern EFI_STATUS Axp81XProbeBatteryRatio(OUT UINTN *Ratio);
extern EFI_STATUS Axp81XProbePowerBusExistance(OUT UINTN *Status);
extern EFI_STATUS Axp81XProbeBatteryExistance(OUT UINTN *Status);
extern EFI_STATUS Axp81XProbeBatteryVoltage(OUT UINTN *Voltage);
extern EFI_STATUS Axp81XProbePowerKey(OUT UINTN *Pressed);
extern EFI_STATUS Axp81XProbePreSysMode(OUT UINTN *Status);
extern EFI_STATUS Axp81XSetNextSysMode(IN UINTN Status);
extern EFI_STATUS Axp81XProbeThisPowerOnCause(IN UINTN *Status);
extern EFI_STATUS Axp81XSetPowerOff(VOID);
extern EFI_STATUS Axp81XSetPowerOnoffVoltage(IN UINTN Voltage,IN UINTN Stage);
extern EFI_STATUS Axp81XSetChargeCurrent(IN UINTN Current);
extern EFI_STATUS Axp81XProbeChargeCurrent(OUT UINTN *Current);
extern EFI_STATUS Axp81XSetVbusCurrentLimit(IN UINTN Current);
extern EFI_STATUS Axp81XProbeVbusCurrentLimit(IN UINTN *Current);
extern EFI_STATUS Axp81XSetVbusVoltagelimit(IN UINTN Voltage);
extern EFI_STATUS Axp81XProbeIntPending(OUT UINT64 *IntMask);
extern EFI_STATUS Axp81XProbeIntEnable(OUT UINT64 *IntMask);
extern EFI_STATUS Axp81XSetIntEnable(IN UINT64 IntMask);
extern EFI_STATUS Axp81XSetIntDisable(IN UINT64 IntMask);
extern EFI_STATUS Axp81XSetSupplyStatus(IN UINTN VoltageIndex, IN UINTN Voltage, IN INTN  OnOff);
extern EFI_STATUS Axp81XSetSupplyStatusByName(IN CHAR8 *VoltageName,IN UINTN  Voltage, IN INTN OnOff);
extern EFI_STATUS Axp81XProbeSupplyStatus(IN UINTN VoltageIndex,IN UINTN  *Voltage, IN INTN *OnOff);
extern EFI_STATUS Axp81XSetCoulombmeterOnOff(IN UINTN OnOff);
extern EFI_STATUS Axp81XProbeSupplyStatusByName(IN CHAR8 *vol_name, OUT UINTN *Vol);


extern AXP_PM_BUS_OPS Axp81XPmBusOps; 
extern EFI_STATUS AxpPmBusRead(UINT8 chip, UINT8 addr, UINT8 *buffer);
extern EFI_STATUS AxpPmBusWrite(UINT8 chip, UINT8 addr, UINT8 data);

#endif
