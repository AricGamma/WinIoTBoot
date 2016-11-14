/******************************************************************************

  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
  http://www.allwinnertech.com

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN 'AS IS' BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
 ******************************************************************************
  File Name     : SunxiTwiLib.c
  Version       : Initial Draft
  Author        : tangmanliang
  Created       : 2016/9/10
  Last Modified :
  Description   : 
  Function List :
              InitializeSunxiTwi
              SunxiTwiGetData
              SunxiTwiGpioInit
              SunxiTwiInit
              SunxiTwiRead
              SunxiTwiSendByteAddr
              SunxiTwiSendData
              SunxiTwiSendRestart
              SunxiTwiSendSlaveAddr
              SunxiTwiSendStart
              SunxiTwiSendStop
              SunxiTwiSetClock
              SunxiTwiSetCpuxClock
              SunxiTwiWrite
              TwiRead
              TwiWrite
  History       :
  1.Date        : 2016/9/10
    Author      : tangmanliang
    Modification: Created file

******************************************************************************/

#include <Uefi.h>
#include <Base.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <platform.h>
#include <Library/SysConfigLib.h>
#include <ccmu.h>
#include <Interinc/sunxi_uefi.h>
#include <Library/PcdLib.h>

#include <Library/SunxiTwiLib.h>


/*
* Global Statement
*/
static TWI_REG gTwiController[CFG_SW_TWI_MAX] = {
  (TWI_REG)CFG_SW_TWI_HOST0,
  (TWI_REG)CFG_SW_TWI_HOST1,
  (TWI_REG)CFG_SW_TWI_HOST2,
  (TWI_REG)CFG_SW_TWI_HOSTR
};


/*
* Public Functions.
*/

UINTN 
EFIAPI
SunxiTwiGpioInit(
  IN UINTN BusNum
)
{
  CHAR8 DevName[5];

  if(BusNum == 1){
    AsciiStrCpy(DevName,"twi1");
  }
  else if(BusNum == 2){
    AsciiStrCpy(DevName,"twi2");
  }
  else{
    AsciiStrCpy(DevName,"twi0");
  }
  gpio_request_simple(DevName, NULL);
  
  return RETURN_SUCCESS;
}


UINTN
EFIAPI
SunxiTwiSendStart (
  IN UINTN BusNum
)
{
  INTN   time = 0xfffff;
  UINTN  tmp_val;

  gTwiController[BusNum]->eft  = 0;
  gTwiController[BusNum]->srst = 1;
  gTwiController[BusNum]->ctl |= 0x20;

  while((time--)&&(!(gTwiController[BusNum]->ctl & 0x08)));
  if(time <= 0)
  {
    return -TWI_NOK_TOUT;
  }

  tmp_val = gTwiController[BusNum]->status;
  if(tmp_val != TWI_START_TRANSMIT)
  {
    return -TWI_START_TRANSMIT;
  }
  
  return TWI_OK;
}

UINTN
EFIAPI
SunxiTwiSendRestart (
  IN UINTN  BusNum
)
{
  INT32   time = 0xffff;
  UINT32  tmp_val;
  
  //tmp_val = gTwiController[BusNum]->ctl & 0xC0;
  tmp_val = gTwiController[BusNum]->ctl;
  tmp_val |= 0x20;
  gTwiController[BusNum]->ctl = tmp_val;
  
  while( (time--) && (!(gTwiController[BusNum]->ctl & 0x08)) );
  if(time <= 0)
  {
    return -TWI_NOK_TOUT;
  }
  
  tmp_val = gTwiController[BusNum]->status;
  if(tmp_val != TWI_RESTART_TRANSMIT)
  {
    return -TWI_RESTART_TRANSMIT;
  }
  
  return TWI_OK;
}


UINTN
EFIAPI
SunxiTwiSendSlaveAddr (
  IN UINTN    BusNum,
  IN UINTN    SlaveAddr,
  IN UINTN    RW
)
{
  INT32   time = 0xffff;
  UINT32  tmp_val;
  
  RW &= 1;
  gTwiController[BusNum]->data = ((SlaveAddr & 0xff) << 1)| RW;

  gTwiController[BusNum]->ctl |=  (0x01<<3); /* write 0 to clean int flag */
  while(( time-- ) && (!( gTwiController[BusNum]->ctl & 0x08 )));
  if(time <= 0)
  {
    DEBUG((EFI_D_ERROR,"Send addr error\n"));
    return -TWI_NOK_TOUT;
  }
  
  tmp_val = gTwiController[BusNum]->status;
  if(RW == TWI_WRITE) /* +write */
  {
    if(tmp_val != TWI_ADDRWRITE_ACK)
    {
      DEBUG((EFI_D_ERROR,"TWI_ADDRWRITE_ACK\n"));
      return -TWI_ADDRWRITE_ACK;
    }
  }
  else /* +read */
  {
    if(tmp_val != TWI_ADDRREAD_ACK)
    {
      DEBUG((EFI_D_ERROR,"TWI_ADDRREAD_ACK\n"));
      return -TWI_ADDRREAD_ACK;
    }
  }
  
  return TWI_OK;
}


UINTN
EFIAPI
SunxiTwiSendByteAddr (
  IN UINTN      BusNum,
  IN UINTN    ByteAddr                    
)
{
  INTN   time = 0xffff;
  UINTN  tmp_val;

  gTwiController[BusNum]->data = ByteAddr & 0xff;
//  gTwiController[BusNum]->ctl &= 0xF7;   /* write 0 to clean int flag */
  gTwiController[BusNum]->ctl |= (0x01<<3);   /* write 0 to clean int flag */

  while( (time--) && (!(gTwiController[BusNum]->ctl & 0x08)) );
  if(time <= 0)
  {
    return -TWI_NOK_TOUT;
  }

  tmp_val = gTwiController[BusNum]->status;  
  if(tmp_val != TWI_DATAWRITE_ACK)
  {
    return -TWI_DATAWRITE_ACK;
  }

  return TWI_OK;
}

UINTN
EFIAPI
SunxiTwiGetData (
  IN UINTN      BusNum,
  OUT UINT8   *Data,
  IN UINTN    Count                    
)
{
  INT32   time = 0xffff;
  UINT32  tmp_val;
  UINT32  i;
  
  if(Count == 1)
  {
    //gTwiController[BusNum]->ctl &= 0xF7;
    gTwiController[BusNum]->ctl |= (0x01<<3);
    while( (time--) && (!(gTwiController[BusNum]->ctl & 0x08)) );
    if(time <= 0)
    {
      return -TWI_NOK_TOUT;
    }
    for(time=0;time<100;time++);
    *Data = gTwiController[BusNum]->data;
  
    tmp_val = gTwiController[BusNum]->status;
    if(tmp_val != TWI_DATAREAD_NACK)
    {
      return -TWI_DATAREAD_NACK;
    }
  }
  else
  {
    for(i=0; i< Count - 1; i++)
    {
      time = 0xffff;
      tmp_val = gTwiController[BusNum]->ctl | (0x01<<2);
      tmp_val = gTwiController[BusNum]->ctl & (0xf7);

      tmp_val |= 0x04;
      gTwiController[BusNum]->ctl = tmp_val;
  
      while( (time--) && (!(gTwiController[BusNum]->ctl & 0x08)) );
      if(time <= 0)
      {
        return -TWI_NOK_TOUT;
      }
      for(time=0;time<100;time++);
      time = 0xffff;
      Data[i] = gTwiController[BusNum]->data;
      while( (time--) && (gTwiController[BusNum]->status != TWI_DATAREAD_ACK) );
      if(time <= 0)
      {
        return -TWI_NOK_TOUT;
      }
    }
  
    time = 0xffff;
    gTwiController[BusNum]->ctl &= 0xFb;

    gTwiController[BusNum]->ctl &= 0xf7;

    while( (time--) && (!(gTwiController[BusNum]->ctl & 0x08)) );
    if(time <= 0)
    {
      return -TWI_NOK_TOUT;
    }
    for(time=0;time<100;time++);
    Data[Count - 1] = gTwiController[BusNum]->data;
    while( (time--) && (gTwiController[BusNum]->status != TWI_DATAREAD_NACK) );
    if(time <= 0)
    {
      return -TWI_NOK_TOUT;
    }
  }
  
  return TWI_OK;
}

UINTN
EFIAPI
SunxiTwiSendData (
  IN UINTN      BusNum,
  IN UINT8    *Data,
  IN UINTN    Count                     
)
{
  INTN   time = 0xffff;
  UINTN  i;
  
  for(i=0; i < Count; i++)
  {
    time = 0xffff;
    gTwiController[BusNum]->data = Data[i];
    //gTwiController[BusNum]->ctl &= 0xF7;
    gTwiController[BusNum]->ctl |= (0x01<<3);

    while( (time--) && (!(gTwiController[BusNum]->ctl & 0x08)) );
    if(time <= 0)
    {
      return -TWI_NOK_TOUT;
    }
    time = 0xffff;
    while( (time--) && (gTwiController[BusNum]->status != TWI_DATAWRITE_ACK) );
    if(time <= 0)
    {
      return -TWI_NOK_TOUT;
    }
  }
  
  return TWI_OK;
}

UINTN
EFIAPI
SunxiTwiSendStop (
  IN UINTN BusNum            
)
{
  INTN   time = 0xffff;
  UINTN  tmp_val;
  
  gTwiController[BusNum]->ctl |= (0x01 << 4);
  gTwiController[BusNum]->ctl &= 0xf7;
  while( (time--) && (gTwiController[BusNum]->ctl & 0x10) );
  if(time <= 0)
  {
    return -TWI_NOK_TOUT;
  }
  time = 0xffff;
  while( (time--) && (gTwiController[BusNum]->status != TWI_READY) );
  tmp_val = gTwiController[BusNum]->status;
  if(tmp_val != TWI_READY)
  {
    return -TWI_NOK_TOUT;
  }
  
  return TWI_OK;
}


UINTN
EFIAPI
SunxiTwiSetCpuxClock (
  IN  UINTN BusNum              
)
{
  UINTN regValue = 0;

  /* twi bus software reset  */
  regValue = MmioRead32(CCMU_BUS_SOFT_RST_REG4);
  regValue |= 0x01 << BusNum;
  MmioWrite32(CCMU_BUS_SOFT_RST_REG4, regValue);
  mdelay(1);

  /* twi bus clock gating */
  regValue = MmioRead32(CCMU_BUS_CLK_GATING_REG3);
  regValue &= ~(1 << BusNum);
  MmioWrite32(CCMU_BUS_CLK_GATING_REG3, regValue);
  mdelay(1);
  regValue |= (1 << BusNum);
  MmioWrite32(CCMU_BUS_CLK_GATING_REG3, regValue);

  return RETURN_SUCCESS;
}

UINTN
EFIAPI
SunxiTwiSetClock (
  IN  UINTN  BusNum,
  IN  UINTN  Speed             
)
{
  UINTN i;
  UINTN clk_n;
  UINTN clk_m;
  
  /* reset twi control  */
  i = 0xffff;
  gTwiController[BusNum]->srst = 1;
  while((gTwiController[BusNum]->srst) && (i))
  {
    i --;
  }
  if((gTwiController[BusNum]->lcr & 0x30) != 0x30 )
  {
    /* toggle twi SCL until bus idle */
    gTwiController[BusNum]->lcr = 0x05;
    udelay(500);
    i = 10;
    while ((i > 0) && ((gTwiController[BusNum]->lcr & 0x02) != 2))
    {
      gTwiController[BusNum]->lcr |= 0x08;
      udelay(1000);
      gTwiController[BusNum]->lcr &= ~0x08;
      udelay(1000);
      i--;
    }
    gTwiController[BusNum]->lcr = 0x0;
    udelay(500);
  }

  if(Speed < 100)
  {
    Speed = 100;
  }
  else if(Speed > 400)
  {
    Speed = 400;
  }
  clk_n = 1;
  clk_m = (24000/10)/((2^clk_n) * Speed) - 1;

  gTwiController[BusNum]->clk = (clk_m<<3) | clk_n;
  gTwiController[BusNum]->ctl = 0x40;
  gTwiController[BusNum]->eft = 0;
  
  return RETURN_SUCCESS;
}

EFI_STATUS
EFIAPI
TwiRead (
  IN  UINTN     BusNum,
  IN  UINTN   Chip,
  IN  UINTN   Addr,
  IN  UINTN   AddrLen,
  OUT UINT8     *Buffer,
  IN  UINTN     Length
)
{
  INTN   i;
  INTN   ret;
  INTN   status;
  INTN   addrSize;
  INT8  *slaveReg;
  
  status = -1;
  ret = SunxiTwiSendStart(BusNum);
  if(ret)
  {
    DEBUG((EFI_D_ERROR,"Send Start error\n"));
    goto TwiReadErrOccur;
  }
  
  ret = SunxiTwiSendSlaveAddr(BusNum, Chip, TWI_WRITE);
  if(ret)
  {
    DEBUG((EFI_D_ERROR,"Send Slave Addr error\n"));
    goto TwiReadErrOccur;
  }
  /* send byte address */
  if(AddrLen >= 3)
  {
    addrSize = 2;
  }
  else if(AddrLen <= 1)
  {
    addrSize = 0;
  }
  else
  {
    addrSize = 1;
  }
  slaveReg = (INT8 *)&Addr;
  for (i = addrSize; i>=0; i--)
  {
    ret = SunxiTwiSendByteAddr(BusNum, slaveReg[i] & 0xff);
    if(ret)
    {
      DEBUG((EFI_D_ERROR,"Send Byte Addr error\n"));    
      goto TwiReadErrOccur;
    }
  }
  ret = SunxiTwiSendRestart(BusNum);
  if(ret)
  {
    DEBUG((EFI_D_ERROR,"Send Restart error\n"));  
    goto TwiReadErrOccur;
  }
  ret = SunxiTwiSendSlaveAddr(BusNum, Chip, TWI_READ);
  if(ret)
  {
    DEBUG((EFI_D_ERROR,"Send Slave Addr error\n"));
    goto TwiReadErrOccur;
  }
  /* get data */
  ret = SunxiTwiGetData(BusNum,Buffer, Length);
  if(ret)
  {
    DEBUG((EFI_D_ERROR,"Get Data error\n"));
    goto TwiReadErrOccur;
  }
  status = 0;
  
TwiReadErrOccur:
  SunxiTwiSendStop(BusNum);
  
  return status;
}

EFI_STATUS
EFIAPI
TwiWrite (
  IN UINTN    BusNum,
  IN UINTN    Chip,
  IN UINTN    Addr,
  IN UINTN    AddrLen,
  IN UINT8      *Buffer,
  IN UINTN      Length
)
{
  INTN   i;
  INTN   ret;
  INTN   status;
  INTN   addrSize;
  INT8  *slaveReg;
  
  status = -1;
  ret = SunxiTwiSendStart(BusNum);
  if(ret)
  {
    DEBUG((EFI_D_ERROR,"Send Start error\n"));
    goto TwiWriteErrOccur;
  }
  
  ret = SunxiTwiSendSlaveAddr(BusNum, Chip, TWI_WRITE);
  if(ret)
  { 
    DEBUG((EFI_D_ERROR,"Send Slave Addr error\n"));
    goto TwiWriteErrOccur;
  }
  /* send byte address */
  if(AddrLen >= 3)
  {
    addrSize = 2;
  }
  else if(AddrLen <= 1)
  {
    addrSize = 0;
  }
  else
  {
    addrSize = 1;
  }
  slaveReg = (INT8 *)&Addr;
  for (i = addrSize; i>=0; i--)
  {
    ret = SunxiTwiSendByteAddr(BusNum, slaveReg[i] & 0xff);
    if(ret)
    {
      DEBUG((EFI_D_ERROR,"Send Byte Addr error\n"));
      goto TwiWriteErrOccur;
    }
  }
  
  ret = SunxiTwiSendData(BusNum,Buffer, Length);
  if(ret)
  {
    DEBUG((EFI_D_ERROR,"Send Send Data error\n"));
    goto TwiWriteErrOccur;
  }
  status = 0;
  
TwiWriteErrOccur:
  SunxiTwiSendStop(BusNum);
  
  return status;  
}

EFI_STATUS
EFIAPI
TwiInit(
  IN UINTN BusNum,
  IN UINTN Speed
)
{
  EFI_STATUS Status;

  /* cpux twi clock init */
  Status = SunxiTwiSetCpuxClock(BusNum);
  /* gpio pin init for twi */
  Status = SunxiTwiGpioInit(BusNum);
  /* twi set clock */
  Status = SunxiTwiSetClock(BusNum,Speed);

  return Status;
}

