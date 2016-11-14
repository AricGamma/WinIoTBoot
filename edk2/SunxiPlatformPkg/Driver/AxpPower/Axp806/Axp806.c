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

#include <Uefi.h>
#include <Library/DebugLib.h>

#include <Protocol/AxpPower.h>
#include <Axp806.h>


 inline EFI_STATUS Axp806PmBusRead(UINT8 chip, UINT8 addr, UINT8 *buffer)
{
  return Axp806PmBusOps.AxpPmBusRead(addr,buffer);
}

 inline EFI_STATUS Axp806PmBusWrite(UINT8 chip, UINT8 addr, UINT8 data)
{
  return Axp806PmBusOps.AxpPmBusWrite(addr,data);
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
EFI_STATUS __Axp806SetIntEnable(OUT UINT8 *IntEnable)
{
  UINTN   i;

  for(i=0;i<2;i++)
  {
    if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_INTEN1 + i, IntEnable[i]))
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
STATIC EFI_STATUS __Axp806ProbeIntEnable(OUT UINT8 *IntEnable)
{
  UINTN   i;

  for(i=0;i<2;i++)
  {
    if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_INTEN1 + i, IntEnable + i))
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
STATIC EFI_STATUS __Axp806ProbeIntPending(OUT UINT8 *IntStatus)
{
  UINTN   i;

  for(i=0;i<2;i++)
  {
    if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_INTSTS1 + i, IntStatus + i))
    {
      return EFI_DEVICE_ERROR;
    }
  }

  for(i=0;i<2;i++)
  {
    if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_INTSTS1 + i, 0xff))
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
EFI_STATUS Axp806Probe(VOID)
{
  UINT8    pmu_type;
  INTN i;
  UINT8 IntReg[8];
  EFI_STATUS Status;
  UINT8 data;
  
  for(i=0;i<sizeof(IntReg);i++)
  {
    IntReg[i] = 0;      
  }
  if(Axp806PmBusWrite(AXP806_ADDR, 0xff, 0x10))
  {
    return EFI_DEVICE_ERROR;
  }
  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_VERSION, &pmu_type))
  {
    DEBUG((EFI_D_ERROR,"axp read error\n"));
    return EFI_DEVICE_ERROR;
  }
  pmu_type &= 0xCf;
  if(pmu_type == 0x40)
  {
    /* pmu type AXP806 */
    DEBUG((EFI_D_INIT,"PMU: Axp806\n"));
    /*disable all the interrupt */
    Status = __Axp806SetIntEnable(IntReg);
    if(Status) return Status;
    /*clean all the interrupt pendding */
    Status = __Axp806ProbeIntPending(IntReg);
    if(Status) return Status;

    if(Axp806PmBusRead(AXP806_ADDR,BOOT_POWER806_DCFREQ_SET,&data))
    {
      DEBUG((EFI_D_INIT,"axp 806 read DCFREQ error\n"));
      return EFI_DEVICE_ERROR;
    }
    data &= (~(0x3 << 4));
    data |= (0x2 << 4);

    if(Axp806PmBusWrite(AXP806_ADDR,BOOT_POWER806_DCFREQ_SET,data))
    {
      DEBUG((EFI_D_INIT,"axp 806 read DCFREQ error\n"));
      return EFI_DEVICE_ERROR;
    }
    //DEBUG((EFI_D_INIT,"PMU : data is %x\n",data));
    return 0;   
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
EFI_STATUS Axp806SetChargerOnOff(IN UINTN OnOff)
{
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
EFI_STATUS Axp806ProbeBatteryRatio(OUT UINTN *Ratio)
{
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
EFI_STATUS Axp806ProbePowerBusExistance(OUT UINTN *Status)
{
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
EFI_STATUS Axp806ProbeBatteryExistance(OUT UINTN *Status)
{  
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
EFI_STATUS Axp806ProbeBatteryVoltage(OUT UINTN *Voltage)
{ 
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
EFI_STATUS Axp806ProbePowerKey(OUT UINTN *Pressed)
{
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
EFI_STATUS Axp806ProbePreSysMode(OUT UINTN *Status)
{
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
EFI_STATUS Axp806SetNextSysMode(IN UINTN Status)
{  
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
EFI_STATUS Axp806ProbeThisPowerOnCause(IN UINTN *Status)
{ 
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
EFI_STATUS Axp806SetPowerOff(VOID)
{
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
EFI_STATUS Axp806SetPowerOnoffVoltage(IN UINTN Voltage,IN UINTN Stage)
{ 
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
EFI_STATUS Axp806SetChargeCurrent(IN UINTN Current)
{
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
EFI_STATUS Axp806ProbeChargeCurrent(OUT UINTN *Current)
{
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
EFI_STATUS Axp806SetVbusCurrentLimit(IN UINTN Current)
{
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
EFI_STATUS Axp806SetVbusVoltagelimit(IN UINTN Voltage)
{
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
EFI_STATUS Axp806ProbeIntPending(OUT UINT64 *IntMask)
{
  UINT8 IntValue[8];
  EFI_STATUS Status;
  
  *IntMask=0;
  
  Status = __Axp806ProbeIntPending(IntValue);
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
EFI_STATUS Axp806ProbeIntEnable(UINT64 *IntMask)
{
  UINT8 IntEnable[8];
  UINTN i;
  EFI_STATUS Status;
  
  for(i=0;i<sizeof(IntEnable);i++)
  {
    IntEnable[i] = 0x00;
  }

  Status = __Axp806ProbeIntEnable(IntEnable);
  if(Status)
    return Status;
  
  if(IntEnable[0] &= 1<<5)    *IntMask |=AXP_INT_MASK_AC_REMOVE;
  if(IntEnable[0] &= 1<<6)    *IntMask |=AXP_INT_MASK_AC_INSERT;
  if(IntEnable[0] &= 1<<2)    *IntMask |=AXP_INT_MASK_VBUS_REMOVE;
  if(IntEnable[0] &= 1<<3)    *IntMask |=AXP_INT_MASK_VBUS_INSERT;
  if(IntEnable[1] &= 1<<2)    *IntMask |=AXP_INT_MASK_CHARGE_DONE;
  
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
EFI_STATUS Axp806SetIntEnable(IN UINT64 IntMask)
{
  UINT8 IntEnable[8];
  UINTN i;
  EFI_STATUS Status;
  
  for(i=0;i<sizeof(IntEnable);i++)
  {
    IntEnable[i] = 0x00;
  }
  Status = __Axp806ProbeIntEnable(IntEnable);
  if(Status)
    return Status;
      
  if(IntMask & AXP_INT_MASK_AC_REMOVE)       IntEnable[0] |= 1<<5;
  if(IntMask & AXP_INT_MASK_AC_INSERT)       IntEnable[0] |= 1<<6;
  if(IntMask & AXP_INT_MASK_VBUS_REMOVE)     IntEnable[0] |= 1<<2;
  if(IntMask & AXP_INT_MASK_VBUS_INSERT)     IntEnable[0] |= 1<<3;
  if(IntMask & AXP_INT_MASK_CHARGE_DONE)     IntEnable[1] |= 1<<2;
     
  Status = __Axp806SetIntEnable(IntEnable);

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
EFI_STATUS Axp806SetIntDisable(IN UINT64 IntMask)
{
  UINT8 IntEnable[8];
  UINTN i;
  EFI_STATUS Status;
  
  for(i=0;i<sizeof(IntEnable);i++)
  {
    IntEnable[i] = 0x00;
  }
  Status = __Axp806ProbeIntEnable(IntEnable);
  if(Status)
    return Status;
      
  if(IntMask & AXP_INT_MASK_AC_REMOVE)       IntEnable[0] &= ~1<<5;
  if(IntMask & AXP_INT_MASK_AC_INSERT)       IntEnable[0] &= ~1<<6;
  if(IntMask & AXP_INT_MASK_VBUS_REMOVE)     IntEnable[0] &= ~1<<2;
  if(IntMask & AXP_INT_MASK_VBUS_INSERT)     IntEnable[0] &= ~1<<3;
  if(IntMask & AXP_INT_MASK_CHARGE_DONE)     IntEnable[1] &= ~1<<2;
  
  Status = __Axp806SetIntEnable(IntEnable);

  return Status;
}
