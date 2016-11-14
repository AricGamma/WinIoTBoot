/** @file
*
*  Copyright (c) 2007-2015, Allwinner Technology Co., Ltd. All rights reserved.
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

#include <Protocol/AxpPower.h>
#include <Axp81X.h>

 inline EFI_STATUS AxpPmBusRead(UINT8 chip, UINT8 addr, UINT8 *buffer)
{
  return Axp81XPmBusOps.AxpPmBusRead(addr,buffer);
}

 inline EFI_STATUS AxpPmBusWrite(UINT8 chip, UINT8 addr, UINT8 data)
{
  return Axp81XPmBusOps.AxpPmBusWrite(addr,data);
}


/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS __Axp81XSetIntEnable(OUT UINT8 *IntEnable)
{
  UINTN   i;

  for(i=0;i<5;i++)
  {
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_INTEN1 + i, IntEnable[i]))
    {
      return EFI_DEVICE_ERROR;
    }
  }
  
  return EFI_SUCCESS;
}

/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
STATIC EFI_STATUS __Axp81XProbeIntEnable(OUT UINT8 *IntEnable)
{
  UINTN   i;

  for(i=0;i<5;i++)
  {
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_INTEN1 + i, IntEnable + i))
    {
      return EFI_DEVICE_ERROR;
    }
  }
  
  return EFI_SUCCESS;
}

/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
STATIC EFI_STATUS __Axp81XProbeIntPending(OUT UINT8 *IntStatus)
{
  UINTN   i;

  for(i=0;i<5;i++)
  {
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_INTSTS1 + i, IntStatus + i))
    {
      return EFI_DEVICE_ERROR;
    }
  }

  for(i=0;i<5;i++)
  {
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_INTSTS1 + i, 0xff))
    {
      return EFI_DEVICE_ERROR;
    }
  }

  return EFI_SUCCESS;
}


/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS Axp81XProbe(VOID)
{
  UINT8    pmu_type;
  INTN i;
  UINT8 IntReg[8];
  EFI_STATUS Status;
  
  for(i=0;i<sizeof(IntReg);i++)
  {
    IntReg[i] = 0;      
  }
  
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_VERSION, &pmu_type))
  {
    DEBUG((EFI_D_ERROR,"axp read error\n"));
    return EFI_DEVICE_ERROR;
  }
  pmu_type &= 0xCf;
  if(pmu_type == 0x41)
  {
    /* pmu type AXP81X */
    DEBUG((EFI_D_INIT,"PMU: AXP81X\n"));
    /*disable all the interrupt */
    Status = __Axp81XSetIntEnable(IntReg);
    if(Status) return Status;
    /*clean all the interrupt pendding */
    return __Axp81XProbeIntPending(IntReg);
  }

  return EFI_DEVICE_ERROR;
}

/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS Axp81XSetChargerOnOff(IN UINTN OnOff)
{
  UINT8 RegValue;
  //disable ts adc, enable all other adc
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ADC_EN, &RegValue))
  {
    return EFI_DEVICE_ERROR;
  }
  RegValue |= 0xC0;
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_ADC_EN, RegValue))
  {
    return EFI_DEVICE_ERROR;
  }

    //enable charge
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_CHARGE1, &RegValue))
  {
    return EFI_DEVICE_ERROR;
  }
  
  if(OnOff)
    RegValue |= 0x80;
  else
    RegValue &=~0x80;
  
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_CHARGE1, RegValue))
  {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS Axp81XProbeBatteryRatio(OUT UINTN *Ratio)
{
  UINT8 RegValue;

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_BAT_PERCEN_CAL, &RegValue))
  {
    return EFI_DEVICE_ERROR;
  }

  *Ratio = RegValue & 0x7f;
  return EFI_SUCCESS;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS Axp81XProbePowerBusExistance(OUT UINTN *Status)
{
  UINT8 RegValue;

  *Status = 0;
  
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_STATUS, &RegValue))
  {
    return EFI_DEVICE_ERROR;
  }
  if(RegValue & 0x10)   //vbus exist
  {
    *Status |= AXP_POWER_VBUS_EXIST;
  }
  if(RegValue & 0x40)   //dc in exist
  {
    *Status |= AXP_POWER_AC_BUS_EXIST;
  }
  return EFI_SUCCESS;
}

/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS Axp81XProbeBatteryExistance(OUT UINTN *Status)
{
  UINT8 RegValue;
  *Status = 0;
  
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_MODE_CHGSTATUS, &RegValue))
  {
    return EFI_DEVICE_ERROR;
  }

  if(RegValue & 0x10)
  {
    *Status = (RegValue&0x20);
    return EFI_SUCCESS;
  }
  else
  {
    return EFI_DEVICE_ERROR;
  }
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS Axp81XProbeBatteryVoltage(OUT UINTN *Voltage)
{
  UINT8  RegValue_h, RegValue_l;
  UINTN  Value;

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_BAT_AVERVOL_H8, &RegValue_h))
  {
    return EFI_DEVICE_ERROR;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_BAT_AVERVOL_L4, &RegValue_l))
  {
    return EFI_DEVICE_ERROR;
  }
   Value = (RegValue_h << 4) | RegValue_l;
  *Voltage = Value * 11;
  *Voltage /= 10;

  return EFI_SUCCESS;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS Axp81XProbePowerKey(OUT UINTN *Pressed)
{
  UINT8  RegValue;

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_INTSTS5, &RegValue))
  {
    return EFI_DEVICE_ERROR;
  }
  RegValue &= (0x03<<3);
  if(RegValue)
  {
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_INTSTS5, RegValue))
    {
      return EFI_DEVICE_ERROR;
    }
  }
  *Pressed = (RegValue>>3)&3;
  return EFI_SUCCESS;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS Axp81XProbePreSysMode(OUT UINTN *Status)
{
  UINT8  RegValue;

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DATA_BUFFER11, &RegValue))
  {
    return EFI_DEVICE_ERROR;
  }
  *Status = RegValue;
  
  return EFI_SUCCESS;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS Axp81XSetNextSysMode(IN UINTN Status)
{
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_DATA_BUFFER11, (UINT8)Status))
  {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS Axp81XProbeThisPowerOnCause(IN UINTN *Status)
{
  UINT8   RegValue;

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_STATUS, &RegValue))
  {
    return EFI_DEVICE_ERROR;
  }

  *Status = RegValue & 0x01;
  
  return EFI_SUCCESS;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS Axp81XSetPowerOff(VOID)
{
  UINT8 RegValue;

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OFF_CTL, &RegValue))
  {
    return EFI_DEVICE_ERROR;
  }
  RegValue |= (1 << 7);
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_OFF_CTL, RegValue))
  {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS Axp81XSetPowerOnoffVoltage(IN UINTN Voltage,IN UINTN Stage)
{
  UINT8 RegValue;

  if(!Voltage)
  {
    if(!Stage)
    {
      Voltage = 3300;
    }
    else
    {
      Voltage = 2900;
    }
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_VOFF_SET, &RegValue))
  {
    return EFI_DEVICE_ERROR;
  }
  RegValue &= 0xf8;
  if(Voltage >= 2600 && Voltage <= 3300)
  {
    RegValue |= (Voltage - 2600)/100;
  }
  else if(Voltage <= 2600)
  {
    RegValue |= 0x00;
  }
  else
  {
    RegValue |= 0x07;
  }
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_VOFF_SET, RegValue))
  {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS Axp81XSetChargeCurrent(IN UINTN Current)
{
  UINT8   RegValue;
  UINTN  step;

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_CHARGE1, &RegValue))
  {
    return EFI_DEVICE_ERROR;
  }
  RegValue &= ~0x0f;
  if(Current > 2800)
  {
    Current = 2800;
  }
  else if(Current < 200)
  {
    Current = 200;
  }
  step       = (Current/200) - 1;//0-13------>200 to 2800
  RegValue |= (step & 0x0f);

  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_CHARGE1, RegValue))
  {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS Axp81XProbeChargeCurrent(OUT UINTN *Current)
{
  UINT8  RegValue;

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_CHARGE1, &RegValue))
  {
    return EFI_DEVICE_ERROR;
  }
  RegValue &= 0x0f;
  *Current = (RegValue + 1) * 200;

  return EFI_SUCCESS;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS Axp81XSetVbusCurrentLimit(IN UINTN Current)
{
  UINT8 RegValue;

  //set bus current limit off
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_VBUS_SET, &RegValue))
  {
    return EFI_DEVICE_ERROR;
  }
  RegValue &= 0xfC;
  if(!Current)
  {
    RegValue |= 0x03;
  }
  else if(Current <= 500)   //limit to 500
  {
    RegValue |= 0x01;
  }
  else            //limit to 900
  {
    RegValue |= 0;
  }
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_VBUS_SET, RegValue))
  {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}
EFI_STATUS Axp81XProbeVbusCurrentLimit(IN UINTN *Current)
{
  UINT8 RegValue;

  //set bus current limit off
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_VBUS_SET, &RegValue))
  {
    return EFI_DEVICE_ERROR;
  }
  RegValue &= 0x3;
  if(RegValue == 0)
  {
    *Current  = 900;
  }
  else if(RegValue == 0x1)    //limit to 500
  {
    *Current  = 500;
  }
  else            //limit to 900
  {
    *Current = 0;
  }
  
  return EFI_SUCCESS;
}

/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS Axp81XSetVbusVoltagelimit(IN UINTN Voltage)
{
  UINT8 RegValue;

  //set bus vol limit off
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_VBUS_SET, &RegValue))
  {
    return EFI_DEVICE_ERROR;
  }
  RegValue &= ~(7 << 3);
  if(!Voltage)
  {
    RegValue &= ~(1 << 6);
  }
  else
  {
    if(Voltage < 4000)
    {
      Voltage = 4000;
    }
    else if(Voltage > 4700)
    {
      Voltage = 4700;
    }
    RegValue |= ((Voltage-4000)/100) << 3;
  }
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_VBUS_SET, RegValue))
  {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS Axp81XProbeIntPending(OUT UINT64 *IntMask)
{
  UINT8 IntValue[8];
  EFI_STATUS Status;
  
  *IntMask=0;
  
  Status = __Axp81XProbeIntPending(IntValue);
  if(Status)
    return Status;
  
  if(IntValue[0] &= 1<<5)    *IntMask |=AXP_INT_MASK_AC_REMOVE;
  if(IntValue[0] &= 1<<6)    *IntMask |=AXP_INT_MASK_AC_INSERT;
  if(IntValue[0] &= 1<<2)    *IntMask |=AXP_INT_MASK_VBUS_REMOVE;
  if(IntValue[0] &= 1<<3)    *IntMask |=AXP_INT_MASK_VBUS_INSERT;
  if(IntValue[1] &= 1<<2)    *IntMask |=AXP_INT_MASK_CHARGE_DONE;
  if(IntValue[4] &= 1<<3)    *IntMask |=AXP_INT_MASK_LONG_KEY_PRESS;
  if(IntValue[4] &= 1<<4)    *IntMask |=AXP_INT_MASK_SHORT_KEY_PRESS;

  return EFI_SUCCESS;
}


/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS Axp81XProbeIntEnable(UINT64 *IntMask)
{
  UINT8 IntEnable[8];
  UINTN i;
  EFI_STATUS Status;
  
  for(i=0;i<sizeof(IntEnable);i++)
  {
    IntEnable[i] = 0x00;
  }

  Status = __Axp81XProbeIntEnable(IntEnable);
  if(Status)
    return Status;
  
  if(IntEnable[0] &= 1<<5)    *IntMask |=AXP_INT_MASK_AC_REMOVE;
  if(IntEnable[0] &= 1<<6)    *IntMask |=AXP_INT_MASK_AC_INSERT;
  if(IntEnable[0] &= 1<<2)    *IntMask |=AXP_INT_MASK_VBUS_REMOVE;
  if(IntEnable[0] &= 1<<3)    *IntMask |=AXP_INT_MASK_VBUS_INSERT;
  if(IntEnable[1] &= 1<<2)    *IntMask |=AXP_INT_MASK_CHARGE_DONE;
  if(IntEnable[4] &= 1<<3)    *IntMask |=AXP_INT_MASK_LONG_KEY_PRESS;
  if(IntEnable[4] &= 1<<4)    *IntMask |=AXP_INT_MASK_SHORT_KEY_PRESS;

  return EFI_SUCCESS;
}

/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS Axp81XSetIntEnable(IN UINT64 IntMask)
{
  UINT8 IntEnable[8];
  UINTN i;
  EFI_STATUS Status;
  
  for(i=0;i<sizeof(IntEnable);i++)
  {
    IntEnable[i] = 0x00;
  }
  Status = __Axp81XProbeIntEnable(IntEnable);
  if(Status)
    return Status;
      
  if(IntMask & AXP_INT_MASK_AC_REMOVE)       IntEnable[0] |= 1<<5;
  if(IntMask & AXP_INT_MASK_AC_INSERT)       IntEnable[0] |= 1<<6;
  if(IntMask & AXP_INT_MASK_VBUS_REMOVE)     IntEnable[0] |= 1<<2;
  if(IntMask & AXP_INT_MASK_VBUS_INSERT)     IntEnable[0] |= 1<<3;
  if(IntMask & AXP_INT_MASK_CHARGE_DONE)     IntEnable[1] |= 1<<2;
  if(IntMask & AXP_INT_MASK_LONG_KEY_PRESS)  IntEnable[4] |= 1<<3;
  if(IntMask & AXP_INT_MASK_SHORT_KEY_PRESS) IntEnable[4] |= 1<<4;
  
  Status = __Axp81XSetIntEnable(IntEnable);

  return Status;
}

/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
EFI_STATUS Axp81XSetIntDisable(IN UINT64 IntMask)
{
  UINT8 IntEnable[8];
  UINTN i;
  EFI_STATUS Status;
  
  for(i=0;i<sizeof(IntEnable);i++)
  {
    IntEnable[i] = 0x00;
  }
  Status = __Axp81XProbeIntEnable(IntEnable);
  if(Status)
    return Status;
    
  if(IntMask & AXP_INT_MASK_AC_REMOVE)       IntEnable[0] &= ~1<<5;
  if(IntMask & AXP_INT_MASK_AC_INSERT)       IntEnable[0] &= ~1<<6;
  if(IntMask & AXP_INT_MASK_VBUS_REMOVE)     IntEnable[0] &= ~1<<2;
  if(IntMask & AXP_INT_MASK_VBUS_INSERT)     IntEnable[0] &= ~1<<3;
  if(IntMask & AXP_INT_MASK_CHARGE_DONE)     IntEnable[1] &= ~1<<2;
  if(IntMask & AXP_INT_MASK_LONG_KEY_PRESS)  IntEnable[4] &= ~1<<3;
  if(IntMask & AXP_INT_MASK_SHORT_KEY_PRESS) IntEnable[4] &= ~1<<4;
  
  Status = __Axp81XSetIntEnable(IntEnable);

  return Status;
}


EFI_STATUS Axp81XSetCoulombmeterOnOff(IN UINTN OnOff)
{
  UINT8 RegValue;
  if(AxpPmBusRead(AXP81X_ADDR,BOOT_POWER81X_FUEL_GAUGE_CTL, &RegValue))
  {
    return EFI_DEVICE_ERROR;
  }
  if(!OnOff)
    RegValue &= ~(0x01 << 7);
  else
    RegValue |= (0x01 << 7);

  if(AxpPmBusWrite(AXP81X_ADDR,BOOT_POWER81X_FUEL_GAUGE_CTL,RegValue))
  {
    return EFI_DEVICE_ERROR;
  }
  return 0;
}

