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
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/IoLib.h>
#include <Library/PrintLib.h>
#include <Library/SysConfigLib.h>
#include <Library/SunxiKeyLib.h>
#include <platform.h>
#include <ccmu.h>
#include <key.h>

#include <Guid/SunxiVariableHob.h>
#include <Guid/SunxiBootInfoHob.h>

#include<Library/SunxiTwiLib.h>
#include<Library/TimerLib.h>
#include <Interinc/sunxi_uefi.h>

#define readl(addr)         (*((volatile UINT32 *)(addr)))          /* word input */
#define writel(value,addr)  (*((volatile UINT32 *)(addr))  = (value))   /* word output */

extern UINT32 SunxiLibGetBootInfo(SUNXI_BOOTINFO_HOB  **Hob);

static BOOLEAN KeyEnable = FALSE;

BOOLEAN EFIAPI SunxiKeyInit(VOID)
{
  struct sunxi_lradc *sunxi_key_base = (struct sunxi_lradc *)SUNXI_KEYADC_BASE;
  UINT32 reg_val = 0;
  INT32 keyen_flag = 1;

  reg_val = sunxi_key_base->ctrl;
  reg_val &= ~((7<<1) | (0xffU << 24));
  reg_val |=  LRADC_HOLD_EN;
  reg_val |=  LRADC_EN;
  sunxi_key_base->ctrl = reg_val;

  /* disable all key irq */
  sunxi_key_base->intc = 0;
  sunxi_key_base->ints = 0x1f1f;

  if( !script_parser_fetch("key_detect_en","keyen_flag",&keyen_flag,1) )
  {
    KeyEnable = keyen_flag? TRUE:FALSE;
    AsciiPrint("[key_detect_en] keyen_flag = %d\n",keyen_flag);
    AsciiPrint("Key enable flag :%a\n", KeyEnable?"Enable":"Disable"); 
  }

  return TRUE;
}

VOID    EFIAPI SunxiKeyExit(VOID)
{
  struct sunxi_lradc *sunxi_key_base = (struct sunxi_lradc *)SUNXI_KEYADC_BASE;

  sunxi_key_base->ctrl = 0;
  /* disable all key irq */
  sunxi_key_base->intc = 0;
  sunxi_key_base->ints = 0x1f1f;
}

INT32    EFIAPI SunxiKeyRead(VOID)
{
  UINT32 ints;
  INT32 key = 0;
  struct sunxi_lradc *sunxi_key_base = (struct sunxi_lradc *)SUNXI_KEYADC_BASE;

  if(!KeyEnable)
  {
    return 0;
  }
  ints = sunxi_key_base->ints;
  /* clear the pending data */
  sunxi_key_base->ints |= (ints & 0x1f);
  /* if there is already data pending, read it */
  if( ints & ADC0_KEYDOWN_PENDING)
  {
    if(ints & ADC0_DATA_PENDING)
    {
      key = sunxi_key_base->data0 & 0x3f;
    }
  }
  else if(ints & ADC0_DATA_PENDING)
  {
    key = sunxi_key_base->data0 & 0x3f;
  }

  if(key > 0)
    AsciiPrint("key pressed value=0x%x\n", key);
    
  return key;
}

EFI_STATUS EFIAPI I2cKeyRead(UINT8 *Value)
{
  EFI_STATUS Status = EFI_SUCCESS;
  UINTN Speed = 400;
  UINTN BusNum = 0x2;
  UINTN DevAddr = 0x11;
  UINTN RegAddr = 0x25;
  UINTN AddrLen = 1;
  UINTN DataLen = 1;

  TwiInit(BusNum, Speed);
  MicroSecondDelay(500);
  
  Status = TwiRead(BusNum, DevAddr,RegAddr, AddrLen, Value, DataLen);
  if(Status != EFI_SUCCESS) {
    AsciiPrint("Read I2C Key Value Failure\n");
  }

  AsciiPrint("I2C key Value is 0x%x\n", *Value);
  
  return Status;
}

EFI_STATUS EFIAPI I2cKeyCheck(VOID)
{
  EFI_STATUS Status;
  UINTN times = 20;
  UINT8 KeyValue = 0;
  CHAR8 BootCmd[64] = {0};

  INT32 KeyCheck = 0;
  SUNXI_BOOTINFO_HOB *Hob = NULL;

  Status = SunxiLibGetBootInfo(&Hob);
  if(Status != EFI_SUCCESS){
    AsciiPrint("Get Boot Info Hob Failure\n");
    return Status;
  }

  if(Hob->WorkMode != WORK_MODE_BOOT){
    return EFI_SUCCESS;
  }

  script_parser_fetch("user_def","i2c_key_check", &KeyCheck, 1);
  if(KeyCheck == 0){
    return EFI_SUCCESS;
  }

  while(times--)
  {  
    gBS->Stall (10000);
    Status = I2cKeyRead(&KeyValue);
    if(Status == EFI_SUCCESS){
      if(KeyValue & 0x1)
        break;
    }
  }

  if(KeyValue & 0x1){
    AsciiPrint("Access Vol+ Key Success\n");
    AsciiStrCpy(BootCmd, "boot-mass");
    Status = gRT->SetVariable (
        L"SunxiBootCmd",
        &gSunxiVariableGuid,
        (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS),
        AsciiStrLen(BootCmd)+1, //add '\0' in the end
        BootCmd);
    if(EFI_ERROR(Status))
    {
      AsciiPrint("Set Sunxi BootCmd Variable Error\n");
      return Status;
    }
    AsciiPrint("Sunxi BootCmd = %a\n", BootCmd);
  }
  
  return Status;
}

