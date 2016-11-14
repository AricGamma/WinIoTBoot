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

#ifndef   __AXP22X_H__
#define   __AXP22X_H__

#include <Driver/AxpPower/Axp22x/Axp22xReg.h>

extern EFI_STATUS Axp22xProbe(VOID);
extern EFI_STATUS Axp22xSetChargerOnOff(IN UINTN OnOff);
extern EFI_STATUS Axp22xProbeBatteryRatio(OUT UINTN *Ratio);
extern EFI_STATUS Axp22xProbePowerBusExistance(OUT UINTN *Status);
extern EFI_STATUS Axp22xProbeBatteryExistance(OUT UINTN *Status);
extern EFI_STATUS Axp22xProbeBatteryVoltage(OUT UINTN *Voltage);
extern EFI_STATUS Axp22xProbePowerKey(OUT UINTN *Pressed);
extern EFI_STATUS Axp22xProbePreSysMode(OUT UINTN *Status);
extern EFI_STATUS Axp22xSetNextSysMode(IN UINTN Status);
extern EFI_STATUS Axp22xProbeThisPowerOnCause(IN UINTN *Status);
extern EFI_STATUS Axp22xSetPowerOff(VOID);
extern EFI_STATUS Axp22xSetPowerOnoffVoltage(IN UINTN Voltage,IN UINTN Stage);
extern EFI_STATUS Axp22xSetChargeCurrent(IN UINTN Current);
extern EFI_STATUS Axp22xProbeChargeCurrent(OUT UINTN *Current);
extern EFI_STATUS Axp22xSetVbusCurrentLimit(IN UINTN Current);
extern EFI_STATUS Axp22xSetVbusVoltagelimit(IN UINTN Voltage);
extern EFI_STATUS Axp22xProbeIntPending(OUT UINT64 *IntMask);
extern EFI_STATUS Axp22xProbeIntEnable(OUT UINT64 *IntMask);
extern EFI_STATUS Axp22xSetIntEnable(IN UINT64 IntMask);
extern EFI_STATUS Axp22xSetIntDisable(IN UINT64 IntMask);
extern EFI_STATUS Axp22xSetSupplyStatus(IN UINTN VoltageIndex, IN UINTN Voltage, IN INTN  OnOff);
extern EFI_STATUS Axp22xSetSupplyStatusByName(IN CHAR8 *VoltageName,IN UINTN  Voltage, IN INTN OnOff);
extern EFI_STATUS Axp22xProbeSupplyStatus(IN UINTN VoltageIndex,IN UINTN  *Voltage, IN INTN *OnOff);

extern AXP_PM_BUS_OPS Axp22xPmBusOps; 

STATIC inline EFI_STATUS AxpPmBusRead(UINT8 chip, UINT8 addr, UINT8 *buffer)
{
  return Axp22xPmBusOps.AxpPmBusRead(addr,buffer);
}

STATIC inline EFI_STATUS AxpPmBusWrite(UINT8 chip, UINT8 addr, UINT8 data)
{
  return Axp22xPmBusOps.AxpPmBusWrite(addr,data);
}


#endif
