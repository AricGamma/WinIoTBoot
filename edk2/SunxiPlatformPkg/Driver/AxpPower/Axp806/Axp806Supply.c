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
#include <Axp806.h>



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
static int axp806_set_dcdc1(int set_vol, int onoff)
{
  UINT8   reg_value;
  UINT8   tmp_step; 

  if(set_vol > 0)
  {
    if(set_vol <= 1110)
    {
      if(set_vol < 600)
      {
        set_vol = 600;
      }
      tmp_step = (set_vol - 600)/10;
    }
    else
    {
      if(set_vol < 1120)
      {
        set_vol = 1120;
      }
      else if(set_vol > 1520)
      {
        set_vol = 1520;
      }

      tmp_step = (set_vol - 1120)/20 + 51;
    }
    if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_DCAOUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0x80;
    reg_value |= tmp_step;
    if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_DCAOUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dcdc1\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL1, &reg_value))
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
  if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff dcdc1\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp806_probe_dcdc1(void)
{
  int vol;
  UINT8  reg_value;

  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_DCAOUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x7f;
  if(reg_value < 51)
  {
    vol = 600 + 10 * reg_value;
  }
  else
  {
    vol = 1120 + 20 * (reg_value - 51);
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
static int axp806_set_dcdc2(int set_vol, int onoff)
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
    if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_DCBOUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= ~0x1f;
    reg_value |= (set_vol - 1000)/50;
    if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_DCBOUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dcdc2\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL1, &reg_value))
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
  if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff dcdc2\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp806_probe_dcdc2(void)
{
  UINT8  reg_value;

  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_DCBOUT_VOL, &reg_value))
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
static int axp806_set_dcdc3(int set_vol, int onoff)
{
  UINT8   reg_value;
  UINT8   tmp_step;

  if(set_vol > 0)
  {
    if(set_vol <= 1110)
    {
      if(set_vol < 600)
      {
        set_vol = 600;
      }
      tmp_step = (set_vol - 600)/10;
    }
    else
    {
      if(set_vol < 1120)
      {
        set_vol = 1120;
      }
      else if(set_vol > 1520)
      {
        set_vol = 1520;
      }

      tmp_step = (set_vol - 1120)/20 + 51;
    }
    if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_DCCOUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0x80;
    reg_value |= tmp_step;
    if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_DCCOUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dcdc3\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL1, &reg_value))
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
  if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set onoff dcdc3\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp806_probe_dcdc3(void)
{
  int vol;
  UINT8  reg_value;

  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_DCCOUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x7f;
  if(reg_value < 51)
  {
    vol = 600 + 10 * reg_value;
  }
  else
  {
    vol = 1120 + 20 * (reg_value - 51);
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
static int axp806_set_dcdc4(int set_vol, int onoff)
{
  UINT8   reg_value;
  UINT8   tmp_step;

  if(set_vol > 0)
  {
    if(set_vol <= 1500)
    {
      if(set_vol < 600)
      {
        set_vol = 600;
      }
      tmp_step = (set_vol - 600)/20;
    }
    else
    {
      if(set_vol < 1600)
      {
        set_vol = 1600;
      }
      else if(set_vol > 3300)
      {
        set_vol = 3300;
      }

      tmp_step = (set_vol - 1600)/100 + 47;
    }
    if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_DCDOUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xC0;
    reg_value |= tmp_step;
  
    if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_DCDOUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dcdc4\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL1, &reg_value))
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
  if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set onoff dcdc4\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp806_probe_dcdc4(void)
{
  int vol;
  UINT8  reg_value;

  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_DCDOUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x3f;
  if(reg_value < 46)
  {
    vol = 600 + 20 * reg_value;
  }
  else
  {
    vol = 1600 + 100 * (reg_value - 46);
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
static int axp806_set_dcdc5(int set_vol, int onoff)
{
  UINT8   reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 1100)
    {
      set_vol = 1100;
    }
    else if(set_vol > 3400)
    {
      set_vol = 3400;
    }
    if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_DCEOUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xC0;
    reg_value |= ((set_vol - 1100)/100);
    if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_DCEOUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dcdc5\n"));

      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL1, &reg_value))
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
  if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set onoff dcdc5\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp806_probe_dcdc5(void)
{
  UINT8  reg_value;

  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_DCEOUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x1f;

  return 1100 + 100 * reg_value;
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
static int axp806_set_aldo1(int set_vol, int onoff)
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
    if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_ALDO1OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 700)/100);
    if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_ALDO1OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set aldo1\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL1, &reg_value))
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
  if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff aldo1\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp806_probe_aldo1(void)
{
  UINT8  reg_value;

  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_ALDO1OUT_VOL, &reg_value))
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
static int axp806_set_aldo2(int set_vol, int onoff)
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
    if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_ALDO2OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 700)/100);
    if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_ALDO2OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set aldo2\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL1, &reg_value))
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
  if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff aldo2\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp806_probe_aldo2(void)
{
  UINT8  reg_value;

  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_ALDO2OUT_VOL, &reg_value))
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
static int axp806_set_aldo3(int set_vol, int onoff)
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
    if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_ALDO3OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 700)/100);
    if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_ALDO3OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set aldo3\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL1, &reg_value))
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
  if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff aldo3\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp806_probe_aldo3(void)
{
  UINT8  reg_value;

  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_ALDO3OUT_VOL, &reg_value))
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
static int axp806_set_bldo1(int set_vol, int onoff)
{
  UINT8 reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 700)
    {
      set_vol = 700;
    }
    else if(set_vol > 1900)
    {
      set_vol = 1900;
    }
    if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_BLDO1OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xf0;
    reg_value |= ((set_vol - 700)/100);
    if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_BLDO1OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set bldo1\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL2, &reg_value))
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
  if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff bldo1\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp806_probe_bldo1(void)
{
  UINT8  reg_value;

  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_BLDO1OUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0xf;

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
static int axp806_set_bldo2(int set_vol, int onoff)
{
  UINT8 reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 700)
    {
      set_vol = 700;
    }
    else if(set_vol > 1900)
    {
      set_vol = 1900;
    }
    if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_BLDO2OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xf0;
    reg_value |= ((set_vol - 700)/100);
    if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_BLDO2OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set bldo1\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL2, &reg_value))
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
  if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff bldo1\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp806_probe_bldo2(void)
{
  UINT8  reg_value;

  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_BLDO2OUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0xf;

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
static int axp806_set_bldo3(int set_vol, int onoff)
{
  UINT8 reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 700)
    {
      set_vol = 700;
    }
    else if(set_vol > 1900)
    {
      set_vol = 1900;
    }
    if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_BLDO3OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xf0;
    reg_value |= ((set_vol - 700)/100);
    if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_BLDO3OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set bldo1\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL2, &reg_value))
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
  if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff bldo1\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp806_probe_bldo3(void)
{
  UINT8  reg_value;

  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_BLDO3OUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0xf;

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
static int axp806_set_bldo4(int set_vol, int onoff)
{
  UINT8 reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 700)
    {
      set_vol = 700;
    }
    else if(set_vol > 1900)
    {
      set_vol = 1900;
    }
    if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_BLDO4OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xf0;
    reg_value |= ((set_vol - 700)/100);
    if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_BLDO4OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set bldo1\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL2, &reg_value))
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
  if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff bldo1\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp806_probe_bldo4(void)
{
  UINT8  reg_value;

  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_BLDO4OUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0xf;

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
static int axp806_set_cldo1(int set_vol, int onoff)
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
    if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_CLDO1OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xf0;
    reg_value |= ((set_vol - 700)/100);
    if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_CLDO1OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set bldo1\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL2, &reg_value))
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
  if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff bldo1\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp806_probe_cldo1(void)
{
  UINT8  reg_value;

  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_CLDO1OUT_VOL, &reg_value))
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
static int axp806_set_cldo2(int set_vol, int onoff)
{
  UINT8 reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 700)
    {
      set_vol = 700;
    }
    else if(set_vol > 1900)
    {
      set_vol = 1900;
    }
    if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_CLDO2OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xf0;
    reg_value |= ((set_vol - 700)/100);
    if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_CLDO2OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set bldo1\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL2, &reg_value))
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
  if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff bldo1\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp806_probe_cldo2(void)
{
  int vol;
  UINT8  reg_value;

  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_CLDO2OUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x1f;
  if(reg_value < 28)
  {
    vol = 700 + 100 * reg_value;
  }
  else
  {
    vol = 3600 + 200 * (reg_value - 28);
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
static int axp806_set_cldo3(int set_vol, int onoff)
{
  UINT8 reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 700)
    {
      set_vol = 700;
    }
    else if(set_vol > 1900)
    {
      set_vol = 1900;
    }
    if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_CLDO3OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xf0;
    reg_value |= ((set_vol - 700)/100);
    if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_CLDO3OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set bldo1\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL2, &reg_value))
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
  if(Axp806PmBusWrite(AXP806_ADDR, BOOT_POWER806_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff bldo1\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp806_probe_cldo3(void)
{
  UINT8  reg_value;

  if(Axp806PmBusRead(AXP806_ADDR, BOOT_POWER806_CLDO3OUT_VOL, &reg_value))
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
static int axp806_set_dcdc_output(int sppply_index, int vol_value, int onoff)
{
  switch(sppply_index)
  {
    case 1:
      return axp806_set_dcdc1(vol_value, onoff);
    case 2:
      return axp806_set_dcdc2(vol_value, onoff);
    case 3:
      return axp806_set_dcdc3(vol_value, onoff);
    case 4:
      return axp806_set_dcdc4(vol_value, onoff);
    case 5:
      return axp806_set_dcdc5(vol_value, onoff);
  }

  return EFI_UNSUPPORTED;
}

static int axp806_set_aldo_output(int sppply_index, int vol_value, int onoff)
{
  switch(sppply_index)
  {
    case 1:
      return axp806_set_aldo1(vol_value, onoff);
    case 2:
      return axp806_set_aldo2(vol_value, onoff);
    case 3:
      return axp806_set_aldo3(vol_value, onoff);
  }

  return EFI_UNSUPPORTED;
}


static int axp806_set_bldo_output(int sppply_index, int vol_value, int onoff)
{
  switch(sppply_index)
  {
    case 1:
      return axp806_set_bldo1(vol_value, onoff);
    case 2:
      return axp806_set_bldo2(vol_value, onoff);
    case 3:
      return axp806_set_bldo3(vol_value, onoff);
    case 4:
      return axp806_set_bldo4(vol_value, onoff);
  }

  return EFI_UNSUPPORTED;
}

static int axp806_set_cldo_output(int sppply_index, int vol_value, int onoff)
{
  switch(sppply_index)
  {
    case 1:
      return axp806_set_cldo1(vol_value, onoff);
    case 2:
      return axp806_set_cldo2(vol_value, onoff);
    case 3:
      return axp806_set_cldo3(vol_value, onoff);
  }

  return EFI_UNSUPPORTED;
}


EFI_STATUS Axp806SetSupplyStatus(IN UINTN VoltageIndex, IN UINTN Voltage, IN INTN OnOff)
{
  int supply_type;
  int sppply_index;

  supply_type  = VoltageIndex & 0xffff0000;
  sppply_index = VoltageIndex & 0x0000ffff;

  switch(supply_type)
  {
    case PMU_SUPPLY_DCDC_TYPE:
      return axp806_set_dcdc_output(sppply_index, Voltage, OnOff);

    case PMU_SUPPLY_ADLO_TYPE:
      return axp806_set_aldo_output(sppply_index, Voltage, OnOff);

    case PMU_SUPPLY_BDLO_TYPE:
      return axp806_set_bldo_output(sppply_index, Voltage, OnOff);

    case PMU_SUPPLY_CDLO_TYPE:
      return axp806_set_cldo_output(sppply_index, Voltage, OnOff);

    default:
      return EFI_UNSUPPORTED;
  }
}

EFI_STATUS Axp806SetSupplyStatusByName(IN CHAR8 *VoltageName,IN UINTN Voltage, IN INTN OnOff)
{
  int sppply_index;

  if(!AsciiStrnCmp(VoltageName, "dcdc", 4))
  {
    sppply_index = 1 + VoltageName[4] - 'a';
    return axp806_set_dcdc_output(sppply_index, Voltage, OnOff);
  }
  else if(!AsciiStrnCmp(VoltageName, "aldo", 4))
  {
    sppply_index = AsciiStrDecimalToUintn(VoltageName + 4);
    return axp806_set_aldo_output(sppply_index, Voltage, OnOff);
  }
  else if(!AsciiStrnCmp(VoltageName, "bldo", 4))
  {
    sppply_index = AsciiStrDecimalToUintn(VoltageName + 4);
    return axp806_set_bldo_output(sppply_index, Voltage, OnOff);
  }
  else if(!AsciiStrnCmp(VoltageName, "cldo", 4))
  {
    sppply_index = AsciiStrDecimalToUintn(VoltageName + 4);
    return axp806_set_cldo_output(sppply_index, Voltage, OnOff);
  }

  return EFI_UNSUPPORTED;
}


EFI_STATUS Axp806ProbeSupplyStatus(IN UINTN VoltageIndex,IN UINTN  *Voltage, IN INTN *OnOff)
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
static int axp806_probe_dcdc_output(int sppply_index)
{
  switch(sppply_index)
  {
    case 1:
      return axp806_probe_dcdc1();
    case 2:
      return axp806_probe_dcdc2();
    case 3:
      return axp806_probe_dcdc3();
    case 4:
      return axp806_probe_dcdc4();
    case 5:
      return axp806_probe_dcdc5();
  }

  return -1;
}

static int axp806_probe_aldo_output(int sppply_index)
{
  switch(sppply_index)
  {
    case 1:
      return axp806_probe_aldo1();
    case 2:
      return axp806_probe_aldo2();
    case 3:
      return axp806_probe_aldo3();
  }

  return -1;
}

static int axp806_probe_bldo_output(int sppply_index)
{
  switch(sppply_index)
  {
    case 1:
      return axp806_probe_bldo1();
    case 2:
      return axp806_probe_bldo2();
    case 3:
      return axp806_probe_bldo3();
    case 4:
      return axp806_probe_bldo4();
  }

  return -1;
}

static int axp806_probe_cldo_output(int sppply_index)
{
  switch(sppply_index)
  {
    case 1:
      return axp806_probe_cldo1();
    case 2:
      return axp806_probe_cldo2();
    case 3:
      return axp806_probe_cldo3();
  }

  return -1;
}


EFI_STATUS Axp806ProbeSupplyStatusByName(IN CHAR8  *vol_name, UINTN *Vol)
{
  int sppply_index;
  int ret = -1;

  *Vol = 0;
  if(!AsciiStrnCmp(vol_name, "dcdc", 4))
  {
    sppply_index = 1 + vol_name[4] - 'a';
    ret =  axp806_probe_dcdc_output(sppply_index);
  }
  else if(!AsciiStrnCmp(vol_name, "aldo", 4))
  {
    sppply_index = 1 + vol_name[4] - '1';
    ret = axp806_probe_aldo_output(sppply_index);
  }
  else if(!AsciiStrnCmp(vol_name, "bldo", 4))
  {
    sppply_index = 1 + vol_name[4] - '1';
    ret =  axp806_probe_bldo_output(sppply_index);
  }
  else if(!AsciiStrnCmp(vol_name, "cldo", 4))
  {
    sppply_index = 1 + vol_name[4] - '1';
    ret= axp806_probe_cldo_output(sppply_index);
  }
  if(ret >=0 ) 
    *Vol = ret;

  return ret == (-1) ? EFI_DEVICE_ERROR : EFI_SUCCESS;
}
