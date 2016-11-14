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

#ifndef   __AXP806_H__
#define   __AXP806_H__

#include <Driver/AxpPower/Axp806/Axp806Reg.h>

extern EFI_STATUS Axp806Probe(VOID);
extern EFI_STATUS Axp806SetChargerOnOff(IN UINTN OnOff);
extern EFI_STATUS Axp806ProbeBatteryRatio(OUT UINTN *Ratio);
extern EFI_STATUS Axp806ProbePowerBusExistance(OUT UINTN *Status);
extern EFI_STATUS Axp806ProbeBatteryExistance(OUT UINTN *Status);
extern EFI_STATUS Axp806ProbeBatteryVoltage(OUT UINTN *Voltage);
extern EFI_STATUS Axp806ProbePowerKey(OUT UINTN *Pressed);
extern EFI_STATUS Axp806ProbePreSysMode(OUT UINTN *Status);
extern EFI_STATUS Axp806SetNextSysMode(IN UINTN Status);
extern EFI_STATUS Axp806ProbeThisPowerOnCause(IN UINTN *Status);
extern EFI_STATUS Axp806SetPowerOff(VOID);
extern EFI_STATUS Axp806SetPowerOnoffVoltage(IN UINTN Voltage,IN UINTN Stage);
extern EFI_STATUS Axp806SetChargeCurrent(IN UINTN Current);
extern EFI_STATUS Axp806ProbeChargeCurrent(OUT UINTN *Current);
extern EFI_STATUS Axp806SetVbusCurrentLimit(IN UINTN Current);
extern EFI_STATUS Axp806SetVbusVoltagelimit(IN UINTN Voltage);
extern EFI_STATUS Axp806ProbeIntPending(OUT UINT64 *IntMask);
extern EFI_STATUS Axp806ProbeIntEnable(OUT UINT64 *IntMask);
extern EFI_STATUS Axp806SetIntEnable(IN UINT64 IntMask);
extern EFI_STATUS Axp806SetIntDisable(IN UINT64 IntMask);
extern EFI_STATUS Axp806SetSupplyStatus(IN UINTN VoltageIndex, IN UINTN Voltage, IN INTN  OnOff);
extern EFI_STATUS Axp806SetSupplyStatusByName(IN CHAR8 *VoltageName,IN UINTN  Voltage, IN INTN OnOff);
extern EFI_STATUS Axp806ProbeSupplyStatus(IN UINTN VoltageIndex,IN UINTN  *Voltage, IN INTN *OnOff);
extern EFI_STATUS Axp806ProbeSupplyStatusByName(IN CHAR8  *vol_name, OUT UINTN *Vol);

extern AXP_PM_BUS_OPS Axp806PmBusOps; 
extern EFI_STATUS Axp806PmBusRead(UINT8 chip, UINT8 addr, UINT8 *buffer);
extern EFI_STATUS Axp806PmBusWrite(UINT8 chip, UINT8 addr, UINT8 data);


#endif
