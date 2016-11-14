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
  File Name     : LedLib.c
  Version       : Initial Draft
  Author        : tangmanliang
  Created       : 2016/9/11
  Last Modified :
  Description   : Led Control Function
  Function List :
              LedCtrlAll
              PlatformLedCtrl
  History       :
  1.Date        : 2016/9/11
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
#include <Library/PcdLib.h>
#include <Library/SysConfigLib.h>

#include <Library/SunxiTwiLib.h>
#include <Library/LedLib.h>

/*****************************************************************************
 Prototype    : LedRgbCtrlAll
 Description  : All LED Color Control
 Input        : IN VOID  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2016/9/9
    Author       : tangmanliang
    Modification : Created function

*****************************************************************************/
VOID
EFIAPI
LedCtrlAll(
  IN UINT8 Red,
  IN UINT8 Green,
  IN UINT8 Blue
)
{
  UINTN BusNum = 1;
  UINTN LedAddr = 0x3c;
  UINTN LedPwmCtrlReg = 0x01;
  UINTN LedPwmUpdateCtrlReg = 0x25; 
  //UINTN LedNum = 0x24; 
  UINTN LedNum = 0x3;
  UINTN Index = 0;
  UINT8 Data;

  Data = Red;
  for (Index = 2; Index < LedNum; Index = Index + 3)
  {
    TwiWrite(BusNum, LedAddr, LedPwmCtrlReg + Index, 1, &Data, 1);    
    MicroSecondDelay(20);
  }

  Data = Green;
  for (Index = 1; Index < LedNum; Index = Index + 3)
  {
    TwiWrite(BusNum, LedAddr, LedPwmCtrlReg + Index, 1, &Data, 1);  
    MicroSecondDelay(20);
  }

  Data = Blue;
  for (Index = 0; Index < LedNum; Index = Index + 3)
  {
    TwiWrite(BusNum, LedAddr, LedPwmCtrlReg + Index, 1, &Data, 1);  
    MicroSecondDelay(20);
  }

  Data = 0;
  TwiWrite(BusNum, LedAddr, LedPwmUpdateCtrlReg, 1, &Data, 1);    

   
}

/*****************************************************************************
 Prototype    : LedCtrl
 Description  : Led Control
 Input        : IN VOID  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2016/9/9
    Author       : tangmanliang
    Modification : Created function

*****************************************************************************/

INT32
EFIAPI
PlatformLedCtrl(
  IN UINT8 Red,
  IN UINT8 Green,
  IN UINT8 Blue

)
{
  UINTN Speed = 400;
  UINTN BusNum = 1;
  UINTN LedAddr = 0x3c;
  UINTN LedShutDownReg = 0x0;
  UINTN LedGlobalCtrlReg = 0x4a;
  UINTN LedResetCtrlReg = 0x4f;
  UINTN LedCtrlReg = 0x26;
  UINTN LedNum = 0x24;

  UINT8 Data = 0;
  UINTN i;

  INT32 LedCtrl = 0;

  script_parser_fetch("user_def","led_ctrl", &LedCtrl, 1);
  if(LedCtrl == 0){
    return EFI_SUCCESS;
  }

  TwiInit(BusNum,Speed);
  
  /* reset */
  TwiWrite(BusNum, LedAddr, LedResetCtrlReg, 1, &Data, 1);  
  MicroSecondDelay(20);
  /* global */
  TwiWrite(BusNum, LedAddr, LedGlobalCtrlReg, 1, &Data, 1);  
  MicroSecondDelay(20);
  /* shutdown */
  Data = 0x1;
  TwiWrite(BusNum, LedAddr, LedShutDownReg, 1, &Data, 1);  
  /* control register */
  for ( i = 0  ; i < LedNum ; i++ )
  {
    TwiWrite(BusNum, LedAddr, LedCtrlReg + i, 1, &Data, 1);
    MicroSecondDelay(20);
  }
  /* color */
  LedCtrlAll(Red, Green, Blue); 

  return EFI_SUCCESS;
}
 

