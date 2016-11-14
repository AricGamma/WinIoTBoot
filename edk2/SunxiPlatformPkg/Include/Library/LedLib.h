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
  File Name     : LedLib.h
  Version       : Initial Draft
  Author        : tangmanliang
  Created       : 2016/9/11
  Last Modified :
  Description   : Led lib head file
  Function List :
  History       :
  1.Date        : 2016/9/11
    Author      : tangmanliang
    Modification: Created file

******************************************************************************/

#ifndef _LED_LIB_H_
#define _LED_LIB_H_


/* Function define */
#define mdelay(ms) MicroSecondDelay((ms)*1000)
#define udelay(us) MicroSecondDelay(us)

extern INT32
EFIAPI
PlatformLedCtrl(
  IN UINT8 Red,
  IN UINT8 Green,
  IN UINT8 Blue

);


#endif /* _LED_LIB_H_ */
