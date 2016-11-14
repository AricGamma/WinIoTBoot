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
#include <Library/BaseLib.h>

#include <Protocol/AxpPower.h>
#include <Axp809.h>

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
static int axp809_set_dc1sw(int onoff)
{
  UINT8   reg_value;

  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL2, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  if(onoff)
  {
    reg_value |= (1 << 7);
  }
  else
  {
    reg_value &= ~(1 << 7);
  }
  if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dc1sw\n"));
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
static int axp809_set_dc5ldo(int onoff)
{
  UINT8   reg_value;

  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL1, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  if(onoff)
  {
    reg_value |= (1 << 0);
  }
  else
  {
    reg_value &= ~(1 << 0);
  }
  if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dc5ldo\n"));
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
static int axp809_set_dcdc1(int set_vol, int onoff)
{
  UINT8   reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 1600)
    {
      set_vol = 1600;
    }
    else if(set_vol > 3400)
    {
      set_vol = 3400;
    }
    if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_DC1OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 1600)/100);
    if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_DC1OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dcdc1\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL1, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 1);
  }
  else
  {
    reg_value |=  (1 << 1);
  }
  if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff dcdc1\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp809_probe_dcdc1(void)
{
  UINT8  reg_value;

  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_DC1OUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x1f;

  return 1600 + 100 * reg_value;
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
static int axp809_set_dcdc2(int set_vol, int onoff)
{
  UINT8   reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 600)
    {
      set_vol = 600;
    }
    else if(set_vol > 1540)
    {
      set_vol = 1540;
    }
    if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_DC2OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= ~0x3f;
    reg_value |= (set_vol - 600)/20;
    if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_DC2OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dcdc2\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL1, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 2);
  }
  else
  {
    reg_value |=  (1 << 2);
  }
  if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff dcdc2\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}


static int axp809_probe_dcdc2(void)
{
  UINT8  reg_value;

  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_DC2OUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x3f;

  return 600 + 20 * reg_value;
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
static int axp809_set_dcdc3(int set_vol, int onoff)
{
  UINT8   reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 600)
    {
      set_vol = 600;
    }
    else if(set_vol > 1860)
    {
      set_vol = 1860;
    }
    if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_DC3OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xC0;
    reg_value = ((set_vol - 600)/20);
    if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_DC3OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dcdc3\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL1, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 3);
  }
  else
  {
    reg_value |=  (1 << 3);
  }
  if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set onoff dcdc3\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp809_probe_dcdc3(void)
{
  UINT8  reg_value;

  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_DC3OUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x3f;

  return 600 + 20 * reg_value;
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
static int axp809_set_dcdc4(int set_vol, int onoff)
{
  UINT8   reg_value;
  UINT8   tmp_step;

  if(set_vol > 0)
  {
    if(set_vol <= 1540)
    {
      if(set_vol < 600)
      {
        set_vol = 600;
      }
      tmp_step = (set_vol - 600)/20;
    }
    else
    {
      if(set_vol < 1800)
      {
        set_vol = 1800;
      }
      else if(set_vol > 2600)
      {
        set_vol = 2600;
      }

      tmp_step = (set_vol - 1800)/100 + 48;
    }
    if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_DC4OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xC0;
    reg_value |= tmp_step;
  
    if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_DC4OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dcdc4\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL1, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 4);
  }
  else
  {
    reg_value |=  (1 << 4);
  }
  if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set onoff dcdc4\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp809_probe_dcdc4(void)
{
  int vol;
  UINT8  reg_value;

  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_DC4OUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x3f;
  if(reg_value < 48)
  {
    vol = 600 + 20 * reg_value;
  }
  else
  {
    vol = 1800 + 100 * (reg_value - 48);
  }

  return vol;
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
static int axp809_set_dcdc5(int set_vol, int onoff)
{
  UINT8   reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 1000)
    {
      set_vol = 1000;
    }
    else if(set_vol > 2550)
    {
      set_vol = 2550;
    }
    if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_DC5OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xC0;
    reg_value |= ((set_vol - 1000)/50);
    if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_DC5OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dcdc5\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL1, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 5);
  }
  else
  {
    reg_value |=  (1 << 5);
  }
  if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set onoff dcdc5\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp809_probe_dcdc5(void)
{
  UINT8  reg_value;

  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_DC5OUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x1f;

  return 1000 + 50 * reg_value;
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
static int axp809_set_aldo1(int set_vol, int onoff)
{
  UINT8 reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 700)
    {
      set_vol = 700;
    }
    else if(set_vol > 3300)
    {
      set_vol = 3300;
    }
    if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_ALDO1OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 700)/100);
    if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_ALDO1OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set aldo1\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL1, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 6);
  }
  else
  {
    reg_value |=  (1 << 6);
  }
  if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff aldo1\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}


static int axp809_probe_aldo1(void)
{
  UINT8  reg_value;

  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_ALDO1OUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x1f;

  return 700 + 100 * reg_value;
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
static int axp809_set_aldo2(int set_vol, int onoff)
{
  UINT8 reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 700)
    {
      set_vol = 700;
    }
    else if(set_vol > 3300)
    {
      set_vol = 3300;
    }
    if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_ALDO2OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 700)/100);
    if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_ALDO2OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set aldo2\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL1, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 7);
  }
  else
  {
    reg_value |=  (1 << 7);
  }
  if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff aldo2\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp809_probe_aldo2(void)
{
  UINT8  reg_value;

  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_ALDO2OUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x1f;

  return 700 + 100 * reg_value;
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
static int axp809_set_aldo3(int set_vol, int onoff)
{
  UINT8 reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 700)
    {
      set_vol = 700;
    }
    else if(set_vol > 3300)
    {
      set_vol = 3300;
    }
    if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_ALDO3OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 700)/100);
    if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_ALDO3OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set aldo3\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL2, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 5);
  }
  else
  {
    reg_value |=  (1 << 5);
  }
  if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff aldo3\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp809_probe_aldo3(void)
{
  UINT8  reg_value;

  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_ALDO3OUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x1f;

  return 700 + 100 * reg_value;
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
static int axp809_set_dldo1(int set_vol, int onoff)
{
  UINT8 reg_value;
  UINT8 tmp_step;

  if(set_vol > 0)
  {
    if(set_vol < 3500)
    {
      if(set_vol < 700)
      {
        set_vol = 700;
      }
      tmp_step = (set_vol - 700)/100;
    }
    else
    {
      if(set_vol > 4200)
      {
        set_vol = 4200;
      }
      tmp_step = (set_vol - 3400)/200 + 27;
    }
    
    if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_DLDO1_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xE0;
    reg_value |= tmp_step;
    if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_DLDO1_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dldo1\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL2, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 3);
  }
  else
  {
    reg_value |=  (1 << 3);
  }
  if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff dldo1\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp809_probe_dldo1(void)
{
  int vol;
  UINT8  reg_value;

  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_DLDO1_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x1f;
  if(reg_value < 27)
  {
    vol = 700 + 100 * reg_value;
  }
  else
  {
    vol = 3400 + 200 * (reg_value - 27);
  }

  return vol;
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
static int axp809_set_dldo2(int set_vol, int onoff)
{
  UINT8 reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 700)
    {
      set_vol = 700;
    }
    else if(set_vol > 3300)
    {
      set_vol = 3300;
    }
    if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_DLDO2_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 700)/100);
    if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_DLDO2_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dldo2\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL2, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 4);
  }
  else
  {
    reg_value |=  (1 << 4);
  }
  if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff dldo2\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp809_probe_dldo2(void)
{
  UINT8  reg_value;

  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_DLDO2_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x1f;

  return 700 + 100 * reg_value;
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
static int axp809_set_eldo1(int set_vol, int onoff)
{
  UINT8 reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 700)
    {
      set_vol = 700;
    }
    else if(set_vol > 3300)
    {
      set_vol = 3300;
    }
    if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_ELDO1_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 700)/100);
    if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_ELDO1_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set eldo1\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL2, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 0);
  }
  else
  {
    reg_value |=  (1 << 0);
  }
  if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff eldo1\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp809_probe_eldo1(void)
{
  UINT8  reg_value;

  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_ELDO1_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x1f;

  return 700 + 100 * reg_value;
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
static int axp809_set_eldo2(int set_vol, int onoff)
{
  UINT8 reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 700)
    {
      set_vol = 700;
    }
    else if(set_vol > 3300)
    {
      set_vol = 3300;
    }
    if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_ELDO2_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 700)/100);
    if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_ELDO2_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set eldo2\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL2, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 1);
  }
  else
  {
    reg_value |=  (1 << 1);
  }
  if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff eldo2\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp809_probe_eldo2(void)
{
  UINT8  reg_value;

  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_ELDO2_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x1f;

  return 700 + 100 * reg_value;
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
static int axp809_set_eldo3(int set_vol, int onoff)
{
  UINT8 reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 700)
    {
      set_vol = 700;
    }
    else if(set_vol > 3300)
    {
      set_vol = 3300;
    }
    if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_ELDO3_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 700)/100);
    if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_ELDO3_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set eldo3\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL2, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 2);
  }
  else
  {
    reg_value |=  (1 << 2);
  }
  if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff edlo3\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp809_probe_eldo3(void)
{
  UINT8  reg_value;

  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_ELDO3_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x1f;

  return 700 + 100 * reg_value;
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
static int axp809_set_gpio0ldo(int set_vol, int onoff)
{
  UINT8 reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 700)
    {
      set_vol = 700;
    }
    else if(set_vol > 3300)
    {
      set_vol = 3300;
    }
    if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_GPIO0_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 700)/100);
    if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_GPIO0_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set gpio0ldo\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_GPIO0_CTL, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  if(onoff == 0)
  {
    reg_value &= ~(7 << 0);
    reg_value |=  (4 << 0);
  }
  else
  {
    reg_value &= ~(7 << 0);
    reg_value |=  (3 << 0);
  }
  if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_GPIO0_CTL, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff gpio0ldo\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp809_probe_gpio0ldo(void)
{
  UINT8  reg_value;

  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_GPIO0_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x1f;
  return 700 + 100 * reg_value;
}


static int axp809_set_gpio1ldo(int set_vol, int onoff)
{
  UINT8 reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 700)
    {
      set_vol = 700;
    }
    else if(set_vol > 3300)
    {
      set_vol = 3300;
    }
    if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_GPIO1_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 700)/100);
    if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_GPIO1_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set gpio1ldo\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_GPIO1_CTL, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  if(onoff == 0)
  {
    reg_value &= ~(7 << 0);
    reg_value |=  (4 << 0);
  }
  else
  {
    reg_value &= ~(7 << 0);
    reg_value |=  (3 << 0);
  }
  if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_GPIO1_CTL, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff gpio1ldo\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp809_probe_gpio1ldo(void)
{
  UINT8  reg_value;

  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_GPIO0_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x1f;
  return 700 + 100 * reg_value;
}

static int axp809_set_gpio4ldo(int set_vol, int onoff)
{
  UINT8 reg_value;

  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_HOTOVER_CTL, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  reg_value &= ~0x10;
  if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_HOTOVER_CTL, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set gpio4\n"));
    return EFI_DEVICE_ERROR;
  }

  if(AxpPmBusRead(AXP809_ADDR, BOOT_POWER809_VBUS_SET, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  if(onoff)
  {
    reg_value |=0x4;
  }
  else
  {
    reg_value &=~0x4;
  }
  if(AxpPmBusWrite(AXP809_ADDR, BOOT_POWER809_VBUS_SET, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff gpio4\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static int axp809_set_dcdc_output(int sppply_index, int vol_value, int onoff)
{
  switch(sppply_index)
  {
    case 1:
      return axp809_set_dcdc1(vol_value, onoff);
    case 2:
      return axp809_set_dcdc2(vol_value, onoff);
    case 3:
      return axp809_set_dcdc3(vol_value, onoff);
    case 4:
      return axp809_set_dcdc4(vol_value, onoff);
    case 5:
      return axp809_set_dcdc5(vol_value, onoff);
  }

  return EFI_UNSUPPORTED;
}

static int axp809_set_aldo_output(int sppply_index, int vol_value, int onoff)
{
  switch(sppply_index)
  {
    case 1:
      return axp809_set_aldo1(vol_value, onoff);
    case 2:
      return axp809_set_aldo2(vol_value, onoff);
    case 3:
      return axp809_set_aldo3(vol_value, onoff);
  }

  return EFI_UNSUPPORTED;
}

static int axp809_set_dldo_output(int sppply_index, int vol_value, int onoff)
{
  switch(sppply_index)
  {
    case 1:
      return axp809_set_dldo1(vol_value, onoff);
    case 2:
      return axp809_set_dldo2(vol_value, onoff);
  }

  return EFI_UNSUPPORTED;
}

static int axp809_set_eldo_output(int sppply_index, int vol_value, int onoff)
{
  switch(sppply_index)
  {
    case 1:
      return axp809_set_eldo1(vol_value, onoff);
    case 2:
      return axp809_set_eldo2(vol_value, onoff);
    case 3:
      return axp809_set_eldo3(vol_value, onoff);
  }

  return EFI_UNSUPPORTED;
}

static int axp809_set_gpioldo_output(int sppply_index, int vol_value, int onoff)
{
  switch(sppply_index)
  {
    case 1:
      return axp809_set_gpio0ldo(vol_value, onoff);
    case 2:
      return axp809_set_gpio1ldo(vol_value, onoff);
    case 4:
      return axp809_set_gpio4ldo(vol_value, onoff);
  }

  return EFI_UNSUPPORTED;
}

static int axp809_set_misc_output(int sppply_index, int vol_value, int onoff)
{
  switch(sppply_index)
  {
    case PMU_SUPPLY_DC5LDO:
      return axp809_set_dc5ldo(onoff);
    case PMU_SUPPLY_DC1SW:
      return axp809_set_dc1sw(onoff);
  }

  return EFI_UNSUPPORTED;
}


EFI_STATUS Axp809SetSupplyStatus(IN UINTN VoltageIndex, IN UINTN Voltage, IN INTN OnOff)
{
  int supply_type;
  int sppply_index;

  supply_type  = VoltageIndex & 0xffff0000;
  sppply_index = VoltageIndex & 0x0000ffff;

  switch(supply_type)
  {
    case PMU_SUPPLY_DCDC_TYPE:
      return axp809_set_dcdc_output(sppply_index, Voltage, OnOff);

    case PMU_SUPPLY_ADLO_TYPE:
      return axp809_set_aldo_output(sppply_index, Voltage, OnOff);

    case PMU_SUPPLY_EDLO_TYPE:
      return axp809_set_eldo_output(sppply_index, Voltage, OnOff);

    case PMU_SUPPLY_DDLO_TYPE:
      return axp809_set_dldo_output(sppply_index, Voltage, OnOff);

    case PMU_SUPPLY_GPIOLDO_TYPE:
      return axp809_set_gpioldo_output(sppply_index, Voltage, OnOff);

    case PMU_SUPPLY_MISC_TYPE:
      return axp809_set_misc_output(VoltageIndex, Voltage, OnOff);

    default:
      return EFI_UNSUPPORTED;
  }
}

EFI_STATUS Axp809SetSupplyStatusByName(IN CHAR8 *VoltageName,IN UINTN Voltage, IN INTN OnOff)
{
  int sppply_index;

  if(!AsciiStrnCmp(VoltageName, "dcdc", 4))
  {
    sppply_index = AsciiStrDecimalToUintn(VoltageName + 4);
    return axp809_set_dcdc_output(sppply_index, Voltage, OnOff);
  }
  else if(!AsciiStrnCmp(VoltageName, "aldo", 4))
  {
    sppply_index = AsciiStrDecimalToUintn(VoltageName + 4);
    return axp809_set_aldo_output(sppply_index, Voltage, OnOff);
  }
  else if(!AsciiStrnCmp(VoltageName, "eldo", 4))
  {
    sppply_index = AsciiStrDecimalToUintn(VoltageName + 4);
    return axp809_set_eldo_output(sppply_index, Voltage, OnOff);
  }
  else if(!AsciiStrnCmp(VoltageName, "dldo", 4))
  {
    sppply_index = AsciiStrDecimalToUintn(VoltageName + 4);
    return axp809_set_dldo_output(sppply_index, Voltage, OnOff);
  }
  else if(!AsciiStrnCmp(VoltageName, "gpio", 4))
  {
    sppply_index = AsciiStrDecimalToUintn(VoltageName + 4);
    return axp809_set_gpioldo_output(sppply_index, Voltage, OnOff);
  }
  else if(!AsciiStrnCmp(VoltageName, "dc5ldo", 6))
  {
    return axp809_set_dc5ldo(OnOff);
  }
  else if(!AsciiStrnCmp(VoltageName, "dc1sw", 5))
  {
    return axp809_set_dc1sw(OnOff);
  }

  return EFI_UNSUPPORTED;
}

EFI_STATUS Axp809ProbeSupplyStatus(IN UINTN VoltageIndex,IN UINTN  *Voltage, IN INTN *OnOff)
{
  return EFI_UNSUPPORTED;
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static int axp809_probe_dcdc_output(int sppply_index)
{
  switch(sppply_index)
  {
    case 1:
      return axp809_probe_dcdc1();
    case 2:
      return axp809_probe_dcdc2();
    case 3:
      return axp809_probe_dcdc3();
    case 4:
      return axp809_probe_dcdc4();
    case 5:
      return axp809_probe_dcdc5();
  }

  return EFI_UNSUPPORTED;
}

static int axp809_probe_aldo_output(int sppply_index)
{
  switch(sppply_index)
  {
    case 1:
      return axp809_probe_aldo1();
    case 2:
      return axp809_probe_aldo2();
    case 3:
      return axp809_probe_aldo3();
  }

  return EFI_UNSUPPORTED;
}

static int axp809_probe_dldo_output(int sppply_index)
{
  switch(sppply_index)
  {
    case 1:
      return axp809_probe_dldo1();
    case 2:
      return axp809_probe_dldo2();
  }

  return -1;
}

static int axp809_probe_eldo_output(int sppply_index)
{
  switch(sppply_index)
  {
    case 1:
      return axp809_probe_eldo1();
    case 2:
      return axp809_probe_eldo2();
    case 3:
      return axp809_probe_eldo3();
  }

  return -1;
}


static int axp809_probe_gpioldo_output(int sppply_index)
{
  switch(sppply_index)
  {
    case 0:
      return axp809_probe_gpio0ldo();
    case 1:
      return axp809_probe_gpio1ldo();
  }

  return -1;
}


EFI_STATUS Axp809ProbeSupplyStatusByName(IN CHAR8 *vol_name, UINTN *Vol)
{
  int sppply_index;
  int ret = -1;

  *Vol = 0;
  sppply_index = 1 + vol_name[4] - '1';

  if(!AsciiStrnCmp(vol_name, "dcdc", 4))
  {
    ret = axp809_probe_dcdc_output(sppply_index);
  }
  else if(!AsciiStrnCmp(vol_name, "aldo", 4))
  {
    ret = axp809_probe_aldo_output(sppply_index);
  }
  else if(!AsciiStrnCmp(vol_name, "dldo", 4))
  {
    ret = axp809_probe_dldo_output(sppply_index);
  }
  else if(!AsciiStrnCmp(vol_name, "eldo", 4))
  {
    ret = axp809_probe_eldo_output(sppply_index);
  }
  else if(!AsciiStrnCmp(vol_name, "gpio", 4))
  {
    ret = axp809_probe_gpioldo_output(sppply_index);
  }
  if(ret >= 0) 
    *Vol = ret;
  
  return ret == (-1) ? EFI_DEVICE_ERROR : EFI_SUCCESS;
}



