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
  File Name     : SunxiTwiLib.h
  Version       : Initial Draft
  Author        : tangmanliang
  Created       : 2016/9/11
  Last Modified :
  Description   : twi lib head file
  Function List :
  History       :
  1.Date        : 2016/9/11
    Author      : tangmanliang
    Modification: Created file

******************************************************************************/

#ifndef _SUNXI_TWI_LIB_H_
#define _SUNXI_TWI_LIB_H_

#include <Library/TimerLib.h>

#define TWI_WRITE     0
#define TWI_READ      1

#define TWI_OK        0
#define TWI_NOK       1
#define TWI_NACK      2
#define TWI_NOK_LA    3 /* Lost arbitration */
#define TWI_NOK_TOUT  4 /* time out */

#define TWI_START_TRANSMIT     0x08
#define TWI_RESTART_TRANSMIT   0x10
#define TWI_ADDRWRITE_ACK      0x18
#define TWI_ADDRREAD_ACK       0x40
#define TWI_DATAWRITE_ACK      0x28
#define TWI_READY              0xf8
#define TWI_DATAREAD_NACK      0x58
#define TWI_DATAREAD_ACK       0x50 

#define CFG_SW_TWI_MAX         0x4
#define CFG_SW_TWI_HOST0       SUNXI_TWI0_BASE               
#define CFG_SW_TWI_HOST1     SUNXI_TWI1_BASE             
#define CFG_SW_TWI_HOST2       SUNXI_TWI2_BASE               
#define CFG_SW_TWI_HOSTR       SUNXI_RTWI_BASE                

typedef struct  SUNXI_TWI_REG
{
  volatile unsigned int addr;        /* slave address     */
  volatile unsigned int xaddr;       /* extend address    */
  volatile unsigned int data;        /* data              */
  volatile unsigned int ctl;         /* control           */
  volatile unsigned int status;      /* status            */
  volatile unsigned int clk;         /* clock             */
  volatile unsigned int srst;        /* soft reset        */
  volatile unsigned int eft;         /* enhanced future   */
  volatile unsigned int lcr;         /* line control      */
  volatile unsigned int dvfs;        /* dvfs control      */
}*TWI_REG;


/* Function define */
#define mdelay(ms) MicroSecondDelay((ms)*1000)
#define udelay(us) MicroSecondDelay(us)

extern EFI_STATUS
EFIAPI
TwiInit(
  IN UINTN BusNum,
  IN UINTN Speed
);

EFI_STATUS
EFIAPI
TwiWrite (
  IN UINTN    BusNum,
  IN UINTN    Chip,
  IN UINTN    Addr,
  IN UINTN    AddrLen,
  IN UINT8      *Buffer,
  IN UINTN      Length
);

EFI_STATUS
EFIAPI
TwiRead (
  IN  UINTN     BusNum,
  IN  UINTN   Chip,
  IN  UINTN   Addr,
  IN  UINTN   AddrLen,
  OUT UINT8     *Buffer,
  IN  UINTN     Length
);


#endif /* _SUNXI_TWI_LIB_H_ */
