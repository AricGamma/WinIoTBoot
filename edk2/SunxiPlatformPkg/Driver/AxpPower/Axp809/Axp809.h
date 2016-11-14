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

#ifndef   __AXP22X_H__
#define   __AXP22X_H__

#include <Driver/AxpPower/Axp809/Axp809Reg.h>

extern EFI_STATUS Axp809Probe(VOID);
extern EFI_STATUS Axp809SetChargerOnOff(IN UINTN OnOff);
extern EFI_STATUS Axp809ProbeBatteryRatio(OUT UINTN *Ratio);
extern EFI_STATUS Axp809ProbePowerBusExistance(OUT UINTN *Status);
extern EFI_STATUS Axp809ProbeBatteryExistance(OUT UINTN *Status);
extern EFI_STATUS Axp809ProbeBatteryVoltage(OUT UINTN *Voltage);
extern EFI_STATUS Axp809ProbePowerKey(OUT UINTN *Pressed);
extern EFI_STATUS Axp809ProbePreSysMode(OUT UINTN *Status);
extern EFI_STATUS Axp809SetNextSysMode(IN UINTN Status);
extern EFI_STATUS Axp809ProbeThisPowerOnCause(IN UINTN *Status);
extern EFI_STATUS Axp809SetPowerOff(VOID);
extern EFI_STATUS Axp809SetPowerOnoffVoltage(IN UINTN Voltage,IN UINTN Stage);
extern EFI_STATUS Axp809SetChargeCurrent(IN UINTN Current);
extern EFI_STATUS Axp809ProbeChargeCurrent(OUT UINTN *Current);
extern EFI_STATUS Axp809SetVbusCurrentLimit(IN UINTN Current);
extern EFI_STATUS Axp809ProbeVbusCurrentLimit(IN UINTN *Current);
extern EFI_STATUS Axp809SetVbusVoltagelimit(IN UINTN Voltage);
extern EFI_STATUS Axp809ProbeIntPending(OUT UINT64 *IntMask);
extern EFI_STATUS Axp809ProbeIntEnable(OUT UINT64 *IntMask);
extern EFI_STATUS Axp809SetIntEnable(IN UINT64 IntMask);
extern EFI_STATUS Axp809SetIntDisable(IN UINT64 IntMask);
extern EFI_STATUS Axp809SetSupplyStatus(IN UINTN VoltageIndex, IN UINTN Voltage, IN INTN  OnOff);
extern EFI_STATUS Axp809SetSupplyStatusByName(IN CHAR8 *VoltageName,IN UINTN  Voltage, IN INTN OnOff);
extern EFI_STATUS Axp809ProbeSupplyStatus(IN UINTN VoltageIndex,IN UINTN  *Voltage, IN INTN *OnOff);
extern EFI_STATUS Axp809SetCoulombmeterOnOff(IN UINTN OnOff);
extern EFI_STATUS Axp809ProbeSupplyStatusByName(IN CHAR8 *vol_name, OUT UINTN *Vol);


extern AXP_PM_BUS_OPS Axp809PmBusOps; 
extern EFI_STATUS AxpPmBusRead(UINT8 chip, UINT8 addr, UINT8 *buffer);
extern EFI_STATUS AxpPmBusWrite(UINT8 chip, UINT8 addr, UINT8 data);

#endif
