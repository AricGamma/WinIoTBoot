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
#include <Axp81X.h>

/*
************************************************************************************************************
*
*                                             function
*
*    oˉêy??3?￡o
*
*    2?êyáD±í￡o
*
*    ·μ???μ  ￡o
*
*    ?μ?÷    ￡o
*
*
************************************************************************************************************
*/
static int axp81X_set_dc1sw(int onoff)
{
  UINT8   reg_value;

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, &reg_value))
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
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, reg_value))
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
*    oˉêy??3?￡o
*
*    2?êyáD±í￡o
*
*    ·μ???μ  ￡o
*
*    ?μ?÷    ￡o
*
*
************************************************************************************************************
*/
static int axp81X_set_dc5ldo(int onoff)
{
  UINT8   reg_value;

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL1, &reg_value))
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
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL1, reg_value))
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
*    oˉêy??3?￡o
*
*    2?êyáD±í￡o
*
*    ·μ???μ  ￡o
*
*    ?μ?÷    ￡o
*
*
************************************************************************************************************
*/
static int axp81X_set_dcdc1(int set_vol, int onoff)
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
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DC1OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 1600)/100);
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_DC1OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dcdc1\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL1, &reg_value))
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
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff dcdc1\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp81X_probe_dcdc1(void)
{
  UINT8  reg_value;
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL1, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  if(!(reg_value & (0x01 << 0)))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DC1OUT_VOL, &reg_value))
  {
    return EFI_DEVICE_ERROR;
  }
  reg_value &= 0x1f;

  return 1600 + 100 * reg_value;
}
/*
************************************************************************************************************
*
*                                             function
*
*    oˉêy??3?￡o
*
*    2?êyáD±í￡o
*
*    ·μ???μ  ￡o
*
*    ?μ?÷    ￡o
*
*
************************************************************************************************************
*/
static int axp81X_set_dcdc2(int set_vol, int onoff)
{
  UINT8   reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 500)
    {
      set_vol = 500;
    }
    else if(set_vol > 1300)
    {
      set_vol = 1300;
    }
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DC2OUT_VOL, &reg_value))
    {
      return -1;
    }
    reg_value &= ~0x7f;
    //dcdc2: 0.5v-1.2v  10mv/step   1.22v-1.3v  20mv/step
    if(set_vol > 1200)
    {
      reg_value |= (70+(set_vol - 1200)/20);
    }
    else
    {
      reg_value |= (set_vol - 500)/10;
    }
   
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_DC2OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dcdc2\n"));
      return -1;
    }

  }

  if(onoff < 0)
  {
    return 0;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL1, &reg_value))
  {
    return -1;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 1);
  }
  else
  {
    reg_value |=  (1 << 1);
  }
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff dcdc2\n"));
    return -1;
  }

  return EFI_SUCCESS;
}

static int axp81X_probe_dcdc2(void)
{
  UINT8  reg_value;
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL1, &reg_value))
  {
    return -1;
  }
  if(!(reg_value & (0x01 << 1)))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DC2OUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x7f;
   //dcdc2: 0.5v-1.2v  10mv/step   1.22v-1.3v  20mv/step
  if(reg_value > 70)
  {
    return 1200 + 20 * (reg_value-70);
  }
  else
  {
    return 500 + 10 * reg_value;
  }

}

/*
************************************************************************************************************
*
*                                             function
*
*    oˉêy??3?￡o
*
*    2?êyáD±í￡o
*
*    ·μ???μ  ￡o
*
*    ?μ?÷    ￡o
*
*
************************************************************************************************************
*/
static int axp81X_set_dcdc3(int set_vol, int onoff)
{
  UINT8   reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 500)
    {
      set_vol = 500;
    }
    else if(set_vol > 1300)
    {
      set_vol = 1300;
    }
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DC3OUT_VOL, &reg_value))
    {
      DEBUG((EFI_D_ERROR,"%d\n", __LINE__));
      return -1;
    }
    reg_value &= (~0x7f);
    //dcdc3: 0.5v-1.2v  10mv/step   1.22v-1.3v  20mv/step
    if(set_vol > 1200)
    {
      reg_value |= (70+(set_vol - 1200)/20);
    }
    else
    {
      reg_value |= (set_vol - 500)/10;
    }
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_DC3OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dcdc3\n"));
      return -1;
    }
#if 0
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DC3OUT_VOL, &reg_value))
    {
      DEBUG((EFI_D_ERROR,"%d\n", __LINE__));
      return -1;
    }
    DEBUG((EFI_D_ERROR,"BOOT_POWER81X_DC3OUT_VOL=%d\n", reg_value));
#endif
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL1, &reg_value))
  {
    return -1;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 2);
  }
  else
  {
    reg_value |=  (1 << 2);
  }
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff dcdc3\n"));
    return -1;
  }

  return EFI_SUCCESS;
}

static int axp81X_probe_dcdc3(void)
{
  UINT8  reg_value;
  
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL1, &reg_value))
  {
    return -1;
  }
  if(!(reg_value & (0x01 << 2)))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DC3OUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x7f;
  //dcdc3: 0.5v-1.2v  10mv/step   1.22v-1.3v  20mv/step
  if(reg_value > 70)
  {
    return 1200 + 20 * (reg_value-70);
  }
  else
  {
    return 500 + 10 * reg_value;
  }

}

/*
************************************************************************************************************
*
*                                             function
*
*    oˉêy??3?￡o
*
*    2?êyáD±í￡o
*
*    ·μ???μ  ￡o
*
*    ?μ?÷    ￡o
*
*
************************************************************************************************************
*/
static int axp81X_set_dcdc4(int set_vol, int onoff)
{
  UINT8   reg_value;
 
  if(set_vol > 0)
  {
    if(set_vol < 500)
    {
      set_vol = 500;
    }
    else if(set_vol > 1300)
    {
      set_vol = 1300;
    }

    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DC4OUT_VOL, &reg_value))
    {
      DEBUG((EFI_D_ERROR,"%d\n", __LINE__));
      return -1;
    }
    reg_value &= (~0x7f);
    //dcdc4: 0.5v-1.2v  10mv/step   1.22v-1.3v  20mv/step
    if(set_vol > 1200)
    {
      reg_value |= (70+(set_vol - 1200)/20);
    }
    else
    {
      reg_value |= (set_vol - 500)/10;
    }
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_DC4OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dcdc4\n"));
      return -1;
    }
#if 0
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DC4OUT_VOL, &reg_value))
    {
      DEBUG((EFI_D_ERROR,"%d\n", __LINE__));
      return -1;
    }
    DEBUG((EFI_D_ERROR,"BOOT_POWER81X_DC4OUT_VOL=%d\n", reg_value));
#endif
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL1, &reg_value))
  {
    return -1;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 3);
  }
  else
  {
    reg_value |=  (1 << 3);
  }
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff dcdc4\n"));
    return -1;
  }

  return EFI_SUCCESS;
}

static int axp81X_probe_dcdc4(void)
{
  int vol;
  UINT8  reg_value;
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL1, &reg_value))
  {
    return -1;
  }
  if(!(reg_value & (0x01 << 3)))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DC4OUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x7f;
  //dcdc4: 0.5v-1.2v  10mv/step   1.22v-1.3v  20mv/step
  if(reg_value > 70)
  {
    return 1200 + 20 * (reg_value-70);
  }
  else
  {
    return 500 + 10 * reg_value;
  }

  return vol;

}
/*
************************************************************************************************************
*
*                                             function
*
*    oˉêy??3?￡o
*
*    2?êyáD±í￡o
*
*    ·μ???μ  ￡o
*
*    ?μ?÷    ￡o
*
*
************************************************************************************************************
*/
static int axp81X_set_dcdc5(int set_vol, int onoff)
{
  UINT8   reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 800)
    {
      set_vol = 800;
    }
    else if(set_vol > 1840)
    {
      set_vol = 1840;
    }
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DC5OUT_VOL, &reg_value))
    {
      DEBUG((EFI_D_ERROR,"%d\n", __LINE__));
      return -1;
    }
    reg_value &= (~0x7f);
    //dcdc5: 0.8v-1.12v  10mv/step   1.12v-1.84v  20mv/step
    if(set_vol > 1120)
    {
      reg_value |= (32+(set_vol - 1120)/20);
    }
    else
    {
      reg_value |= (set_vol - 800)/10;
    }
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_DC5OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dcdc5\n"));
      return -1;
    }
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL1, &reg_value))
  {
    return -1;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 4);
  }
  else
  {
    reg_value |=  (1 << 4);
  }
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff dcdc5\n"));
    return -1;
  }

  return EFI_SUCCESS;
}

static int axp81X_probe_dcdc5(void)
{
  UINT8  reg_value;
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL1, &reg_value))
  {
    return -1;
  }
  if(!(reg_value & (0x01 << 4)))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DC5OUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x7f;
  //dcdc5: 0.8v-1.12v  10mv/step   1.12v-1.84v  20mv/step
  if(reg_value > 32)
  {
    return 1120 + 20 * (reg_value-32);
  }
  else
  {
    return 800 + 10 * reg_value;
  }

}

static int axp81X_set_dcdc6(int set_vol, int onoff)
{
  UINT8   reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 600)
    {
      set_vol = 600;
    }
    else if(set_vol > 1520)
    {
      set_vol = 1520;
    }
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DC6OUT_VOL, &reg_value))
    {
      DEBUG((EFI_D_ERROR,"%d\n", __LINE__));
      return -1;
    }
    reg_value &= (~0x7f);
    //dcdc6: 0.6v-1.1v  10mv/step   1.12v-1.52v  20mv/step
    if(set_vol > 1100)
    {
      reg_value |= (50+(set_vol - 1100)/20);
    }
    else
    {
      reg_value |= (set_vol - 600)/10;
    }
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_DC6OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dcdc5\n"));
      return -1;
    }
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL1, &reg_value))
  {
    return -1;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 5);
  }
  else
  {
    reg_value |=  (1 << 5);
  }
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL1, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff dcdc5\n"));
    return -1;
  }

  return 0;
}

static int axp81X_probe_dcdc6(void)
{
  UINT8  reg_value;
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL1, &reg_value))
  {
    return -1;
  }
  if(!(reg_value & (0x05 << 0)))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DC6OUT_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x7f;
  //dcdc6: 0.6v-1.1v  10mv/step   1.12v-1.52v  20mv/step
  if(reg_value > 50)
  {
    return 1100 + 20 * (reg_value-50);
  }
  else
  {
    return 600 + 10 * reg_value;
  }
}
/*
************************************************************************************************************
*
*                                             function
*
*    oˉêy??3?￡o
*
*    2?êyáD±í￡o
*
*    ·μ???μ  ￡o
*
*    ?μ?÷    ￡o
*
*
************************************************************************************************************
*/
static int axp81X_set_aldo1(int set_vol, int onoff)
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
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ALDO1OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 700)/100);
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_ALDO1OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set aldo1\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ALDO_CTL, &reg_value))
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
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_ALDO_CTL, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff aldo1\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}


static int axp81X_probe_aldo1(void)
{
  UINT8  reg_value;
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ALDO_CTL, &reg_value))
  {
    return -1;
  }
  if(!(reg_value & (0x01 << 5)))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ALDO1OUT_VOL, &reg_value))
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
*    oˉêy??3?￡o
*
*    2?êyáD±í￡o
*
*    ·μ???μ  ￡o
*
*    ?μ?÷    ￡o
*
*
************************************************************************************************************
*/
static int axp81X_set_aldo2(int set_vol, int onoff)
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
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ALDO2OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 700)/100);
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_ALDO2OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set aldo2\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ALDO_CTL, &reg_value))
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
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_ALDO_CTL, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff aldo2\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp81X_probe_aldo2(void)
{
  UINT8  reg_value;
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ALDO_CTL, &reg_value))
  {
    return -1;
  }
  if(!(reg_value & (0x01 << 6)))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ALDO2OUT_VOL, &reg_value))
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
*    oˉêy??3?￡o
*
*    2?êyáD±í￡o
*
*    ·μ???μ  ￡o
*
*    ?μ?÷    ￡o
*
*
************************************************************************************************************
*/
static int axp81X_set_aldo3(int set_vol, int onoff)
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
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ALDO3OUT_VOL, &reg_value))
    {
      return EFI_DEVICE_ERROR;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 700)/100);
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_ALDO3OUT_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set aldo3\n"));
      return EFI_DEVICE_ERROR;
    }
  }

  if(onoff < 0)
  {
    return EFI_SUCCESS;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ALDO_CTL, &reg_value))
  {
    return -1;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 7);
  }
  else
  {
    reg_value |=  (1 << 7);
  }
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_ALDO_CTL, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff aldo3\n"));
    return -1;
  }

  return 0;
}

static int axp81X_probe_aldo3(void)
{
  UINT8  reg_value;
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ALDO_CTL, &reg_value))
  {
    return -1;
  }
  if(!(reg_value & (0x01 << 7)))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ALDO3OUT_VOL, &reg_value))
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
static int axp81X_set_dldo1(int set_vol, int onoff)
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
        
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DLDO1_VOL, &reg_value))
    {
      return -1;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol-700)/100);
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_DLDO1_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dldo1\n"));
      return -1;
    }
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, &reg_value))
  {
    return -1;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 3);
  }
  else
  {
    reg_value |=  (1 << 3);
  }
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff dldo1\n"));
    return -1;
  }

  return 0;
}

static int axp81X_probe_dldo1(void)
{
  int vol;
  UINT8  reg_value;
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, &reg_value))
  {
    return -1;
  }
  if(!(reg_value & (0x01 << 3)))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DLDO1_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x1f;
  vol = 700 + 100 * reg_value;

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
static int axp81X_set_dldo2(int set_vol, int onoff)
{
  UINT8 reg_value;

  if(set_vol > 0)
  { 
    if(set_vol < 700)
    {
      set_vol = 700;
    }
    else if(set_vol > 4200)
    {
      set_vol = 4200;
    }
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DLDO2_VOL, &reg_value))
    {
      return -1;
    }
    reg_value &= 0xE0;
    //dldo： 0.7v-3.4v  100mv/step   3.4v-4.2v  200mv/step
    if(set_vol > 3400)
    {
      reg_value |= (27 + ((set_vol - 3400)/200));  //(3400-700)/100
    }
    else
    {
      reg_value |= ((set_vol - 700)/100);
    }
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_DLDO2_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dldo2\n"));
      return -1;
    }
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, &reg_value))
  {
    return -1;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 4);
  }
  else
  {
    reg_value |=  (1 << 4);
  }
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff dldo2\n"));
    return -1;
  }

  return 0;
}

static int axp81X_probe_dldo2(void)
{
  UINT8  reg_value;
  int vol;
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, &reg_value))
  {
    return -1;
  }
  if(!(reg_value & (0x01 << 4)))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DLDO2_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x1f;

  if(reg_value > 27)
    vol = 3400+(reg_value-27)*200;
  else 
    vol = 700 + 100 * reg_value;

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
static int axp81X_set_dldo3(int set_vol, int onoff)
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

    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DLDO3_VOL, &reg_value))
    {
      return -1;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol-700)/100);
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_DLDO3_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dldo1\n"));
      return -1;
    }
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, &reg_value))
  {
    return -1;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 5);
  }
  else
  {
    reg_value |=  (1 << 5);
  }
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff aldo3\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

static int axp81X_probe_dldo3(void)
{

  UINT8  reg_value;
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, &reg_value))
  {
    return -1;
  }
  if(!(reg_value & (0x01 << 5)))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DLDO3_VOL, &reg_value))
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
*    oˉêy??3?￡o
*
*    2?êyáD±í￡o
*
*    ·μ???μ  ￡o
*
*    ?μ?÷    ￡o
*
*
************************************************************************************************************
*/
static int axp81X_set_dldo4(int set_vol, int onoff)
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

    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DLDO4_VOL, &reg_value))
    {
      return -1;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol-700)/100);
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_DLDO4_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set dldo1\n"));
      return -1;
    }
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, &reg_value))
  {
    return -1;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 6);
  }
  else
  {
    reg_value |=  (1 << 6);
  }
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff dldo1\n"));
    return -1;
  }

  return 0;
}

static int axp81X_probe_dldo4(void)
{
  int vol;
  UINT8  reg_value;
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, &reg_value))
  {
    return -1;
  }
  if(!(reg_value & (0x01 << 6)))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_DLDO4_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x1f;
  vol = 700 + 100 * reg_value;

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
static int axp81X_set_eldo1(int set_vol, int onoff)
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
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ELDO1_VOL, &reg_value))
    {
      return -1;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 700)/50);
       
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_ELDO1_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set eldo1\n"));
      return -1;
    }
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, &reg_value))
  {
    return -1;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 0);
  }
  else
  {
    reg_value |=  (1 << 0);
  }
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff eldo1\n"));
    return -1;
  }

  return 0;
}

static int axp81X_probe_eldo1(void)
{
  UINT8  reg_value;
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, &reg_value))
  {
    return -1;
  }
  if(!(reg_value & (0x01 << 0)))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ELDO1_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x1f;

  return 700 + 50 * reg_value;
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
static int axp81X_set_eldo2(int set_vol, int onoff)
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
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ELDO2_VOL, &reg_value))
    {
      return -1;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 700)/50);
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_ELDO2_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set eldo2\n"));
      return -1;
    }
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, &reg_value))
  {
    return -1;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 1);
  }
  else
  {
    reg_value |=  (1 << 1);
  }
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff eldo2\n"));
    return -1;
  }

  return 0;
}

static int axp81X_probe_eldo2(void)
{
  UINT8  reg_value;
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, &reg_value))
  {
    return -1;
  }
  if(!(reg_value & (0x01 << 1)))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ELDO2_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x1f;

  return 700 + 50 * reg_value;
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
static int axp81X_set_eldo3(int set_vol, int onoff)
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
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ELDO3_VOL, &reg_value))
    {
      return -1;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 700)/50);
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_ELDO3_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set eldo3\n"));
      return -1;
    }
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, &reg_value))
  {
    return -1;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 2);
  }
  else
  {
    reg_value |=  (1 << 2);
  }
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff eldo3\n"));
    return -1;
  }

  return 0;
}

static int axp81X_probe_eldo3(void)
{
  UINT8  reg_value;
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_OUTPUT_CTL2, &reg_value))
  {
    return -1;
  }
  if(!(reg_value & (0x01 << 2)))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ELDO3_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x1f;

  return 700 + 50 * reg_value;
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
static int axp81X_set_fldo1(int set_vol, int onoff)
{
  UINT8 reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 700)
    {
      set_vol = 700;
    }
    else if(set_vol > 1450)
    {
      set_vol = 1450;
    }
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_FLDO1_VOL, &reg_value))
    {
      return -1;
    }
    reg_value &= 0xF0;
    reg_value |= ((set_vol - 700)/50);
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_FLDO1_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set fldo1\n"));
      return -1;
    }
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ALDO_CTL, &reg_value))
  {
    return -1;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 2);
  }
  else
  {
    reg_value |=  (1 << 2);
  }
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_ALDO_CTL, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff fldo1\n"));
    return -1;
  }

  return 0;
}
static int axp81X_probe_fldo1(void)
{
  UINT8  reg_value;
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ALDO_CTL, &reg_value))
  {
    return -1;
  }
  if(!(reg_value & (0x01 << 2)))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_FLDO1_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x0f;

  return 700 + 50 * reg_value;
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
static int axp81X_set_fldo2(int set_vol, int onoff)
{
  UINT8 reg_value;

  if(set_vol > 0)
  {
    if(set_vol < 700)
    {
      set_vol = 700;
    }
    else if(set_vol > 1450)
    {
      set_vol = 1450;
    }
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_FLDO2_VOL, &reg_value))
    {
      return -1;
    }
    reg_value &= 0xF0;
    reg_value |= ((set_vol - 700)/50);
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_FLDO2_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set fldo2\n"));
      return -1;
    }
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ALDO_CTL, &reg_value))
  {
    return -1;
  }
  if(onoff == 0)
  {
    reg_value &= ~(1 << 3);
  }
  else
  {
    reg_value |=  (1 << 3);
  }
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_ALDO_CTL, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff fldo2\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}
static int axp81X_probe_fldo2(void)
{
  UINT8  reg_value;
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_ALDO_CTL, &reg_value))
  {
    return -1;
  }
  if(!(reg_value & (0x01 << 3)))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_FLDO2_VOL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x0f;

  return 700 + 50 * reg_value;
}

/*
************************************************************************************************************
*
*                                             function
*
*    oˉêy??3?￡o
*
*    2?êyáD±í￡o
*
*    ·μ???μ  ￡o
*
*    ?μ?÷    ￡o
*
*
************************************************************************************************************
*/
static int axp81X_set_gpio0ldo(int set_vol, int onoff)
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
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_GPIO0_VOL, &reg_value))
    {
      return -1;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 700)/100);
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_GPIO0_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set gpio0ldo\n"));
      return -1;
    }
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_GPIO0_CTL, &reg_value))
  {
    return -1;
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
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_GPIO0_CTL, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff gpio0ldo\n"));
    return -1;
  }

  return 0;
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
static int axp81X_set_gpio1ldo(int set_vol, int onoff)
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
    if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_GPIO1_VOL, &reg_value))
    {
      return -1;
    }
    reg_value &= 0xE0;
    reg_value |= ((set_vol - 700)/100);
    if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_GPIO1_VOL, reg_value))
    {
      DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to set gpio1ldo\n"));
      return -1;
    }
  }

  if(onoff < 0)
  {
    return 0;
  }
  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_GPIO1_CTL, &reg_value))
  {
    return -1;
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
  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_GPIO1_CTL, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to onoff gpio1ldo\n"));
    return -1;
  }

  return 0;
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
static int axp81X_probe_gpio0ldo(void)
{
  UINT8  reg_value;

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_GPIO0_VOL, &reg_value))
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
static int axp81X_probe_gpio1ldo(void)
{
  UINT8  reg_value;

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_GPIO1_VOL, &reg_value))
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
static int axp81X_set_gpio0(int level)
{
  UINT8 reg_value;

  if((level < 0) || (level > 1))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_GPIO0_CTL, &reg_value))
  {
    return -1;
  }

  if(level == 0)//drive low
  {
    reg_value &= ~(7 << 0);
  }
  else    //drive high
  {
    reg_value &= ~(7 << 0);
    reg_value |=  (1 << 0);
  }

  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_GPIO0_CTL, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to level gpio0\n"));
    return -1;
  }

  return 0;
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
static int axp81X_set_gpio1(int level)
{
  UINT8 reg_value;

  if((level < 0) || (level > 1))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_GPIO1_CTL, &reg_value))
  {
    return -1;
  }

  if(level == 0)//drive low
  {
    reg_value &= ~(7 << 0);
  }
  else    //drive high
  {
    reg_value &= ~(7 << 0);
    reg_value |=  (1 << 0);
  }

  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_GPIO1_CTL, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to level gpio0\n"));
    return -1;
  }

  return 0;
}

//map the drv_vbus pin to gpio2
static int axp81X_set_vbusen(int level)
{
  UINT8 reg_value;

  if((level < 0) || (level > 1))
  {
    return 0;
  }

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_VBUS_SET, &reg_value))
  {
    return -1;
  }

  if(level == 0)//drive low
  {
    reg_value &= ~(1 << 2);
  }
  else    //drive high
  {
    reg_value |=  (1 << 2);
  }

  if(AxpPmBusWrite(AXP81X_ADDR, BOOT_POWER81X_VBUS_SET, reg_value))
  {
    DEBUG((EFI_D_ERROR,"sunxi pmu error : unable to level gpio2\n"));
    return -1;
  }

  return 0;
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
static int axp81X_probe_gpio0(void)
{
  UINT8  reg_value;

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_GPIO0_CTL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x07;
  if((reg_value == 0) || (reg_value == 1))
  {
    return reg_value;
  }

  return -1;
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
static int axp81X_probe_gpio1(void)
{
  UINT8  reg_value;

  if(AxpPmBusRead(AXP81X_ADDR, BOOT_POWER81X_GPIO1_CTL, &reg_value))
  {
    return -1;
  }
  reg_value &= 0x07;
  if((reg_value == 0) || (reg_value == 1))
  {
    return reg_value;
  }

  return -1;
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
static int axp81X_set_dcdc_output(int sppply_index, int vol_value, int onoff)
{
  switch(sppply_index)
  {
    case 1:
      return axp81X_set_dcdc1(vol_value, onoff);
    case 2:
      return axp81X_set_dcdc2(vol_value, onoff);
    case 3:
      return axp81X_set_dcdc3(vol_value, onoff);
    case 4:
      return axp81X_set_dcdc4(vol_value, onoff);
    case 5:
      return axp81X_set_dcdc5(vol_value, onoff);
    case 6:
      return axp81X_set_dcdc6(vol_value, onoff);
  }

  return EFI_UNSUPPORTED;
}

static int axp81X_set_aldo_output(int sppply_index, int vol_value, int onoff)
{
  switch(sppply_index)
  {
    case 1:
      return axp81X_set_aldo1(vol_value, onoff);
    case 2:
      return axp81X_set_aldo2(vol_value, onoff);
    case 3:
      return axp81X_set_aldo3(vol_value, onoff);
  }

  return EFI_UNSUPPORTED;
}

static int axp81X_set_dldo_output(int sppply_index, int vol_value, int onoff)
{
  switch(sppply_index)
  {
    case 1:
      return axp81X_set_dldo1(vol_value, onoff);
    case 2:
      return axp81X_set_dldo2(vol_value, onoff);
    case 3:
      return axp81X_set_dldo3(vol_value, onoff);
    case 4:
      return axp81X_set_dldo4(vol_value, onoff);
  }

  return EFI_UNSUPPORTED;
}

static int axp81X_set_eldo_output(int sppply_index, int vol_value, int onoff)
{
  switch(sppply_index)
  {
    case 1:
      return axp81X_set_eldo1(vol_value, onoff);
    case 2:
      return axp81X_set_eldo2(vol_value, onoff);
    case 3:
      return axp81X_set_eldo3(vol_value, onoff);
  }

  return EFI_UNSUPPORTED;
}

static int axp81X_set_fldo_output(int sppply_index, int vol_value, int onoff)
{
  switch(sppply_index)
  {
    case 1:
      return axp81X_set_fldo1(vol_value, onoff);
    case 2:
      return axp81X_set_fldo2(vol_value, onoff);
  }

  return -1;
}

static int axp81X_set_gpioldo_output(int sppply_index, int vol_value, int onoff)
{
  switch(sppply_index)
  {
    case 0:
      return axp81X_set_gpio0ldo(vol_value, onoff);
    case 1:
      return axp81X_set_gpio1ldo(vol_value, onoff);
  }

  return -1;
}

static int axp81X_set_gpio_output(int sppply_index, int level)
{
  switch(sppply_index)
  {
    case 0:
      return axp81X_set_gpio0(level);
    case 1:
      return axp81X_set_gpio1(level);

  }
  return -1;
}

static int axp81X_set_misc_output(int sppply_index, int vol_value, int onoff)
{
  switch(sppply_index)
  {
    case PMU_SUPPLY_DC5LDO:
      return axp81X_set_dc5ldo(onoff);
    case PMU_SUPPLY_DC1SW:
      return axp81X_set_dc1sw(onoff);
    case PMU_SUPPLY_VBUSEN:
      return axp81X_set_vbusen(onoff);
      
  }

  return EFI_UNSUPPORTED;
}


EFI_STATUS Axp81XSetSupplyStatus(IN UINTN VoltageIndex, IN UINTN Voltage, IN INTN OnOff)
{
  int supply_type;
  int sppply_index;

  supply_type  = VoltageIndex & 0xffff0000;
  sppply_index = VoltageIndex & 0x0000ffff;

  switch(supply_type)
  {
    case PMU_SUPPLY_DCDC_TYPE:
      return axp81X_set_dcdc_output(sppply_index, Voltage, OnOff);

    case PMU_SUPPLY_ALDO_TYPE:
      return axp81X_set_aldo_output(sppply_index, Voltage, OnOff);

    case PMU_SUPPLY_ELDO_TYPE:
      return axp81X_set_eldo_output(sppply_index, Voltage, OnOff);

    case PMU_SUPPLY_DLDO_TYPE:
      return axp81X_set_dldo_output(sppply_index, Voltage, OnOff);

    case PMU_SUPPLY_GPIOLDO_TYPE:
      return axp81X_set_gpioldo_output(sppply_index, Voltage, OnOff);

    case PMU_SUPPLY_MISC_TYPE:
      return axp81X_set_misc_output(VoltageIndex, Voltage, OnOff);

    case PMU_SUPPLY_GPIO_TYPE:
      return axp81X_set_gpio_output(sppply_index, OnOff);

    default:
      return EFI_UNSUPPORTED;
  }
}

EFI_STATUS Axp81XSetSupplyStatusByName(IN CHAR8 *VoltageName,IN UINTN Voltage, IN INTN OnOff)
{
  int sppply_index;

  if(!AsciiStrnCmp(VoltageName, "dcdc", 4))
  {
    sppply_index = AsciiStrDecimalToUintn(VoltageName + 4);

    return axp81X_set_dcdc_output(sppply_index, Voltage, OnOff);
  }
  else if(!AsciiStrnCmp(VoltageName, "aldo", 4))
  {
    sppply_index = AsciiStrDecimalToUintn(VoltageName + 4);

    return axp81X_set_aldo_output(sppply_index, Voltage, OnOff);
  }
  else if(!AsciiStrnCmp(VoltageName, "eldo", 4))
  {
    sppply_index = AsciiStrDecimalToUintn(VoltageName + 4);

    return axp81X_set_eldo_output(sppply_index, Voltage, OnOff);
  }else if(!AsciiStrnCmp(VoltageName, "fldo", 4))
  {
    sppply_index = AsciiStrDecimalToUintn(VoltageName + 4);

    return axp81X_set_fldo_output(sppply_index, Voltage, OnOff);
  }
  else if(!AsciiStrnCmp(VoltageName, "dldo", 4))
  {
    sppply_index = AsciiStrDecimalToUintn(VoltageName + 4);

    return axp81X_set_dldo_output(sppply_index, Voltage, OnOff);
  }
  else if(!AsciiStrnCmp(VoltageName, "gpio", 4))
  {
    sppply_index = AsciiStrDecimalToUintn(VoltageName + 4);

    return axp81X_set_gpioldo_output(sppply_index, Voltage, OnOff);
  }
    else if(!AsciiStrnCmp(VoltageName, "dc5ldo", 6))
  {
    return axp81X_set_dc5ldo(OnOff);
  }
  else if(!AsciiStrnCmp(VoltageName, "dc1sw", 5))
  {
    return axp81X_set_dc1sw(OnOff);
  }else if (!AsciiStrnCmp(VoltageName, "power", 5)) //axp gpio used
  {
    sppply_index = AsciiStrDecimalToUintn(VoltageName + 5);

    return axp81X_set_gpio_output(sppply_index, OnOff);
  }

  return EFI_UNSUPPORTED;
}

EFI_STATUS Axp81XProbeSupplyStatus(IN UINTN VoltageIndex,IN UINTN  *Voltage, IN INTN *OnOff)
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
static int axp81X_probe_dcdc_output(int sppply_index)
{
  switch(sppply_index)
  {
    case 1:
      return axp81X_probe_dcdc1();
    case 2:
      return axp81X_probe_dcdc2();
    case 3:
      return axp81X_probe_dcdc3();
    case 4:
      return axp81X_probe_dcdc4();
    case 5:
      return axp81X_probe_dcdc5();
    case 6:
      return axp81X_probe_dcdc6();
  }

  return EFI_UNSUPPORTED;
}

static int axp81X_probe_aldo_output(int sppply_index)
{
  switch(sppply_index)
  {
    case 1:
      return axp81X_probe_aldo1();
    case 2:
      return axp81X_probe_aldo2();
    case 3:
      return axp81X_probe_aldo3();
  }

  return EFI_UNSUPPORTED;
}

static int axp81X_probe_dldo_output(int sppply_index)
{
  switch(sppply_index)
  {
    case 1:
      return axp81X_probe_dldo1();
    case 2:
      return axp81X_probe_dldo2();
    case 3:
      return axp81X_probe_dldo3();
    case 4:
      return axp81X_probe_dldo4();
  }

  return -1;
}

static int axp81X_probe_eldo_output(int sppply_index)
{
  switch(sppply_index)
  {
    case 1:
      return axp81X_probe_eldo1();
    case 2:
      return axp81X_probe_eldo2();
    case 3:
      return axp81X_probe_eldo3();
  }

  return -1;
}
static int axp81X_probe_fldo_output(int sppply_index)
{
  switch(sppply_index)
  {
    case 1:
      return axp81X_probe_fldo1();
    case 2:
      return axp81X_probe_fldo2();
  }

  return -1;
}


static int axp81X_probe_gpioldo_output(int sppply_index)
{
  switch(sppply_index)
  {
    case 0:
      return axp81X_probe_gpio0ldo();
    case 1:
      return axp81X_probe_gpio1ldo();
  }

  return -1;
}

static int axp81X_probe_gpio_output(int sppply_index)
{
  switch(sppply_index)
  {
    case 0:
      return axp81X_probe_gpio0();
    case 1:
      return axp81X_probe_gpio1();
  }

  return -1;
}

EFI_STATUS Axp81XProbeSupplyStatusByName(IN CHAR8 *vol_name, UINTN *Vol)
{
  INT32 ret=EFI_SUCCESS;
  int sppply_index;

  *Vol = 0;
  sppply_index = 1 + vol_name[4] - '1';

  if(!AsciiStrnCmp(vol_name, "dcdc", 4))
  {
    ret = axp81X_probe_dcdc_output(sppply_index);
  }
  else if(!AsciiStrnCmp(vol_name, "aldo", 4))
  {
    ret = axp81X_probe_aldo_output(sppply_index);
  }
  else if(!AsciiStrnCmp(vol_name, "dldo", 4))
  {
    ret = axp81X_probe_dldo_output(sppply_index);
  }
  else if(!AsciiStrnCmp(vol_name, "eldo", 4))
  {
    ret = axp81X_probe_eldo_output(sppply_index);
  }
  else if(!AsciiStrnCmp(vol_name, "fldo", 4))
  {
    return axp81X_probe_fldo_output(sppply_index);
  }
  else if(!AsciiStrnCmp(vol_name, "gpio", 4))
  {
    ret = axp81X_probe_gpioldo_output(sppply_index);
  }
  else if (!AsciiStrnCmp(vol_name, "power", 5))
  {
    sppply_index = AsciiStrDecimalToUintn(vol_name + 5);
    return axp81X_probe_gpio_output(sppply_index);
  }

  return ret;
}
