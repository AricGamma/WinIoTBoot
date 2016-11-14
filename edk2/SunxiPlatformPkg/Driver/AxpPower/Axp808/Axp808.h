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

#ifndef   __AXP808_H__
#define   __AXP808_H__

#include <Driver/AxpPower/Axp808/Axp808Reg.h>

extern EFI_STATUS Axp808Probe(VOID);
extern EFI_STATUS Axp808SetChargerOnOff(IN UINTN OnOff);
extern EFI_STATUS Axp808ProbeBatteryRatio(OUT UINTN *Ratio);
extern EFI_STATUS Axp808ProbePowerBusExistance(OUT UINTN *Status);
extern EFI_STATUS Axp808ProbeBatteryExistance(OUT UINTN *Status);
extern EFI_STATUS Axp808ProbeBatteryVoltage(OUT UINTN *Voltage);
extern EFI_STATUS Axp808ProbePowerKey(OUT UINTN *Pressed);
extern EFI_STATUS Axp808ProbePreSysMode(OUT UINTN *Status);
extern EFI_STATUS Axp808SetNextSysMode(IN UINTN Status);
extern EFI_STATUS Axp808ProbeThisPowerOnCause(IN UINTN *Status);
extern EFI_STATUS Axp808SetPowerOff(VOID);
extern EFI_STATUS Axp808SetPowerOnoffVoltage(IN UINTN Voltage,IN UINTN Stage);
extern EFI_STATUS Axp808SetChargeCurrent(IN UINTN Current);
extern EFI_STATUS Axp808ProbeChargeCurrent(OUT UINTN *Current);
extern EFI_STATUS Axp808SetVbusCurrentLimit(IN UINTN Current);
extern EFI_STATUS Axp808SetVbusVoltagelimit(IN UINTN Voltage);
extern EFI_STATUS Axp808ProbeIntPending(OUT UINT64 *IntMask);
extern EFI_STATUS Axp808ProbeIntEnable(OUT UINT64 *IntMask);
extern EFI_STATUS Axp808SetIntEnable(IN UINT64 IntMask);
extern EFI_STATUS Axp808SetIntDisable(IN UINT64 IntMask);
extern EFI_STATUS Axp808SetSupplyStatus(IN UINTN VoltageIndex, IN UINTN Voltage, IN INTN  OnOff);
extern EFI_STATUS Axp808SetSupplyStatusByName(IN CHAR8 *VoltageName,IN UINTN  Voltage, IN INTN OnOff);
extern EFI_STATUS Axp808ProbeSupplyStatus(IN UINTN VoltageIndex,IN UINTN  *Voltage, IN INTN *OnOff);
extern EFI_STATUS Axp808ProbeSupplyStatusByName(IN CHAR8  *vol_name, OUT UINTN *Vol);

extern AXP_PM_BUS_OPS Axp808PmBusOps; 
extern EFI_STATUS Axp808PmBusRead(UINT8 chip, UINT8 addr, UINT8 *buffer);
extern EFI_STATUS Axp808PmBusWrite(UINT8 chip, UINT8 addr, UINT8 data);


#endif
