/** @file
*
*  Copyright (c) 2007-2015, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  Martin.Zheng <martinzheng@allwinnertech.com>
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


#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/RealTimeClockLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/RealTimeClock.h>
#include <Guid/EventGroup.h>
#include <Guid/GlobalVariable.h>
#include <Library/PcdLib.h>
#include <Library/TimerLib.h>
#include <Sun50iW1P1/platform.h>
#include <Sun50iW1P1/rtc.h>


#define RTC_YEAR_DATUM 2000

BOOLEAN isRunTimeMode;

EFI_EVENT ExitBootServiceEvent;

INT32 rtc_hw_init(void)
{
  UINT32 tmp_data;
  DEBUG((EFI_D_INFO, "++%s:%d\n", __func__, __LINE__));

  /*
   * select rtc clock source
   * on fpga board, internal 32k clk src is the default, and can not be changed
   */ 
  tmp_data  = MmioRead32(SUNXI_RTC_BASE + SUNXI_LOSC_CTRL_REG);
  tmp_data &= (~REG_CLK32K_AUTO_SWT_EN);                //disable auto switch,bit-14
  tmp_data |= (RTC_SOURCE_EXTERNAL | REG_LOSCCTRL_MAGIC); //external     32768hz osc
  tmp_data |= (EXT_LOSC_GSM);                             //external 32768hz osc gsm
  MmioWrite32(SUNXI_RTC_BASE + SUNXI_LOSC_CTRL_REG,tmp_data);
  
  MicroSecondDelay(100);

  /*step2: check set result,查询是否设置成功*/
  tmp_data = MmioRead32(SUNXI_RTC_BASE + SUNXI_LOSC_CTRL_REG);
  if(!(tmp_data & RTC_SOURCE_EXTERNAL)){    
    DEBUG((EFI_D_ERROR,"[RTC] WARNING: Rtc time will be wrong!!\n"));
    return -1;
  }
  
  /*clean the alarm count value*/
  MmioWrite32(SUNXI_RTC_BASE + SUNXI_RTC_ALARM_COUNTER_REG,0x00000000);//0x0020
  /*clean the alarm current value*/
  MmioWrite32(SUNXI_RTC_BASE + SUNXI_RTC_ALARM_CURRENT_REG,0x00000000);//0x0024
  /*disable the alarm0 when init*/
  MmioWrite32(SUNXI_RTC_BASE + SUNXI_ALARM_EN_REG,0x00000000);//0x0028
  /*disable the alarm0 irq*/
  MmioWrite32(SUNXI_RTC_BASE + SUNXI_ALARM_INT_CTRL_REG,0x00000000);//0x002c
  /*Clear pending count alarm*/
  MmioWrite32(SUNXI_RTC_BASE + SUNXI_ALARM_INT_STATUS_REG,0x00000001);//0x0030
  
  MmioWrite32(SUNXI_RTC_BASE + SUNXI_ALARM_CONFIG,0x00000000);

  return 0;

}
/**
  Returns the current time and date information, and the time-keeping capabilities
  of the hardware platform.

  @param  Time                  A pointer to storage to receive a snapshot of the current time.
  @param  Capabilities          An optional pointer to a buffer to receive the real time clock
                                device's capabilities.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_INVALID_PARAMETER Time is NULL.
  @retval EFI_DEVICE_ERROR      The time could not be retrieved due to hardware error.

**/
EFI_STATUS
EFIAPI
LibGetTime (
  OUT EFI_TIME                *Time,
  OUT  EFI_TIME_CAPABILITIES  *Capabilities
  )
{
  /*
   * Kimoon added on 2011.12.08
   */
  UINT32 date_tmp = 0;
  UINT32 time_tmp = 0;
  BOOLEAN Retried = FALSE;

  // DEBUG((EFI_D_INFO, "++%a:%d\n", __func__, __LINE__));

  /*
   * Check set time
   */
  if (Time == NULL)
  goto cleanUp;

  if (isRunTimeMode) {
    DEBUG((EFI_D_ERROR, "RTCGetTime in RunTime Services Mode, Return\n"));
    return EFI_UNSUPPORTED;
  }
  /*
   * 2. Read registers
   */


RetryGetTime:

   /*first to get the date, then time, because the sec turn to 0 will effect the date;*/
  date_tmp = MmioRead32(SUNXI_RTC_BASE + SUNXI_RTC_DATE_REG);
  time_tmp = MmioRead32(SUNXI_RTC_BASE + SUNXI_RTC_TIME_REG);
  
  Time->Second = TIME_GET_SEC_VALUE(time_tmp);
  Time->Minute = TIME_GET_MIN_VALUE(time_tmp);
  Time->Hour = TIME_GET_HOUR_VALUE(time_tmp);
  Time->Day = DATE_GET_DAY_VALUE(date_tmp);
  Time->Month = DATE_GET_MON_VALUE(date_tmp);
  Time->Year = DATE_GET_YEAR_VALUE(date_tmp) +RTC_YEAR_DATUM;
  
  Time->Pad1 = 0;
  Time->Nanosecond = 0; // we do not get data at this resolution.

  // Time->Daylight = UEFI_Daylight;

  Time->Pad2 = 0;
  /*
   * 3. if second value is 0, try to read registers to escape errors.
   */
  if (Time->Second == 0 && !Retried) {
  Retried = TRUE;
  goto RetryGetTime;
  }

  // Update the Capabilities info
  if (Capabilities != NULL) {
    // rtc runs at frequency 1Hz
    Capabilities->Resolution  = 1;
    // Accuracy in ppm multiplied by 1,000,000, e.g. for 50ppm set 50,000,000
    Capabilities->Accuracy    = 200000000;
    // FALSE: Setting the time does not clear the values below the resolution level
    Capabilities->SetsToZero  = FALSE;
  }

  //  DEBUG((EFI_D_ERROR, "--%a:%d (%d/%d/%d %d:%d:%d)\n",
  //                  __func__, __LINE__,
  //                  Time->Year, Time->Month, Time->Day, Time->Hour, Time->Minute, Time->Second));
  return EFI_SUCCESS;

cleanUp:
  DEBUG((EFI_D_ERROR, "ERROR: %a:%d\n", __func__, __LINE__));
  return EFI_DEVICE_ERROR;

}


/**
  Sets the current local time and date information.

  @param  Time                  A pointer to the current time.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_INVALID_PARAMETER A time field is out of range.
  @retval EFI_DEVICE_ERROR      The time could not be set due due to hardware error.

**/
EFI_STATUS
EFIAPI
LibSetTime (
  IN EFI_TIME                *Time
  )
{

  UINT32 date_tmp = 0;
  UINT32 time_tmp = 0;
  UINT32 crystal_data;
  UINT32 timeout;
  UINT32 leap_year;
  /*
   * Check set time
   */
  if (Time == NULL)
  goto cleanUp;

  DEBUG((EFI_D_INFO, "++%a:%d (%d/%d/%d %d:%d:%d)\n", 
                  __func__, __LINE__,
                  Time->Year, Time->Month, Time->Day, Time->Hour, Time->Minute, Time->Second));
                  
  if (isRunTimeMode) {
    DEBUG ((EFI_D_ERROR, "RTC Lib in RunTime Services Mode. Return\n"));
    return EFI_UNSUPPORTED;
  }
  /*
   * The RTC will only support a year value of 0 - 63.  The year datum is
   * 2000, so any dates greater than 2063 will fail unless the datum is
   * adjusted.
   */
  if ((Time->Year < RTC_YEAR_DATUM) || (Time->Year - RTC_YEAR_DATUM > 63)) {
  DEBUG((EFI_D_ERROR, "RTC cannot support a year greater than %d or less than %d (value %d)\n",
                    (RTC_YEAR_DATUM + 63), RTC_YEAR_DATUM, Time->Year));
    goto cleanUp;
  }
  
  crystal_data = MmioRead32(SUNXI_RTC_BASE + SUNXI_LOSC_CTRL_REG);

  /*Any bit of [9:7] is set, The time and date
  * register can`t be written, we re-try the entried read
  */
  {
    /*check at most 3 times.*/
    int times = 3;
    while((crystal_data & 0x380) && (times--)){
      DEBUG((EFI_D_ERROR,"[RTC]canot change rtc now!\n"));
      MicroSecondDelay(100);
      crystal_data = MmioRead32(SUNXI_RTC_BASE + SUNXI_LOSC_CTRL_REG);
    }
  }

  date_tmp = (DATE_SET_DAY_VALUE(Time->Day)|DATE_SET_MON_VALUE(Time->Month)
                    |DATE_SET_YEAR_VALUE(Time->Year-RTC_YEAR_DATUM));

  time_tmp = (TIME_SET_SEC_VALUE(Time->Second)|TIME_SET_MIN_VALUE(Time->Minute)
                    |TIME_SET_HOUR_VALUE(Time->Hour));


  MmioWrite32(SUNXI_RTC_BASE + SUNXI_RTC_TIME_REG,time_tmp);
  timeout = 0xffff;
  
  while((MmioRead32(SUNXI_RTC_BASE + SUNXI_LOSC_CTRL_REG)&(RTC_HHMMSS_ACCESS))&&(--timeout))
  if (timeout == 0) {
    DEBUG((EFI_D_ERROR, "fail to set rtc time.\n"));        
    goto cleanUp;
  }
  
  leap_year = Time->Year;
  if((leap_year%400==0) || ((leap_year%100!=0) && (leap_year%4==0))) {
    /*Set Leap Year bit*/
    date_tmp |= LEAP_SET_VALUE(1);
  }

  MmioWrite32(SUNXI_RTC_BASE + SUNXI_RTC_DATE_REG,date_tmp);
  timeout = 0xffff;
  while((MmioRead32(SUNXI_RTC_BASE + SUNXI_LOSC_CTRL_REG)&(RTC_YYMMDD_ACCESS))&&(--timeout))
  if (timeout == 0) {
    DEBUG((EFI_D_ERROR, "fail to set rtc date.\n"));
    goto cleanUp;
  }

  DEBUG((EFI_D_INFO, "--%a:%d\n", __func__, __LINE__));
  return EFI_SUCCESS;

cleanUp:
  DEBUG((EFI_D_ERROR, "ERROR: %a:%d\n", __func__, __LINE__));
  return EFI_DEVICE_ERROR;

}


/**
  Returns the current wakeup alarm clock setting.

  @param  Enabled               Indicates if the alarm is currently enabled or disabled.
  @param  Pending               Indicates if the alarm signal is pending and requires acknowledgement.
  @param  Time                  The current alarm setting.

  @retval EFI_SUCCESS           The alarm settings were returned.
  @retval EFI_INVALID_PARAMETER Any parameter is NULL.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be retrieved due to a hardware error.

**/
EFI_STATUS
EFIAPI
LibGetWakeupTime (
  OUT BOOLEAN     *Enabled,
  OUT BOOLEAN     *Pending,
  OUT EFI_TIME    *Time
  )
{
  // Not a required feature
  return EFI_UNSUPPORTED;
}


/**
  Sets the system wakeup alarm clock time.

  @param  Enabled               Enable or disable the wakeup alarm.
  @param  Time                  If Enable is TRUE, the time to set the wakeup alarm for.

  @retval EFI_SUCCESS           If Enable is TRUE, then the wakeup alarm was enabled. If
                                Enable is FALSE, then the wakeup alarm was disabled.
  @retval EFI_INVALID_PARAMETER A time field is out of range.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be set due to a hardware error.
  @retval EFI_UNSUPPORTED       A wakeup timer is not supported on this platform.

**/
EFI_STATUS
EFIAPI
LibSetWakeupTime (
  IN BOOLEAN      Enabled,
  OUT EFI_TIME    *Time
  )
{
  // Not a required feature
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
RTCExitBootService (
  EFI_EVENT                      Event,
  VOID                           *Context
  )
{
  isRunTimeMode = TRUE;
  return EFI_SUCCESS;
}


/**
  This is the declaration of an EFI image entry point. This can be the entry point to an application
  written to this specification, an EFI boot service driver, or an EFI runtime driver.

  @param  ImageHandle           Handle that identifies the loaded image.
  @param  SystemTable           System Table for this image.

  @retval EFI_SUCCESS           The operation completed successfully.

**/
EFI_STATUS
EFIAPI
LibRtcInitialize (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
{
  EFI_HANDLE    Handle;
  EFI_TIME      Time;
  EFI_STATUS Status;
  
  DEBUG((EFI_D_INFO, "++%a:%d\n", __func__, __LINE__));

  if(rtc_hw_init())
  {
    DEBUG((EFI_D_ERROR, "ERROR: %a:%d\n", __func__, __LINE__));
    return EFI_DEVICE_ERROR;
  }


  // Setup the setters and getters
  gRT->GetTime       = LibGetTime;
  gRT->SetTime       = LibSetTime;
  gRT->GetWakeupTime = LibGetWakeupTime;
  gRT->SetWakeupTime = LibSetWakeupTime;

  Time.Second = 0;
  Time.Minute = 0;
  Time.Hour   = 14;
  Time.Day    = 10;
  Time.Month  = 9;
  Time.Year   = 2014;

  LibSetTime(&Time);

  // Install the protocol
  Handle = NULL;
  gBS->InstallMultipleProtocolInterfaces (
                 &Handle,
                 &gEfiRealTimeClockArchProtocolGuid,  NULL,
                 NULL
                );
                
  isRunTimeMode = FALSE;
  Status = gBS->CreateEventEx (
                EVT_NOTIFY_SIGNAL,
                TPL_CALLBACK,
                (EFI_EVENT_NOTIFY)RTCExitBootService,
                NULL,
                &gEfiEventExitBootServicesGuid,
                &ExitBootServiceEvent
                );
  if (Status != EFI_SUCCESS) {
    DEBUG((EFI_D_WARN, "RTC Could not register for RTCExitBootService!!!, Status = 0x%x\n", Status));
    return Status;
  }

  DEBUG((EFI_D_INFO, "--%a:%d\n", __func__, __LINE__));
  return EFI_SUCCESS;
}


/**
  Fixup internal data so that EFI can be call in virtual mode.
  Call the passed in Child Notify event and convert any pointers in
  lib to virtual mode.

  @param[in]    Event   The Event that is being processed
  @param[in]    Context Event Context
**/
VOID
EFIAPI
LibRtcVirtualNotifyEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  //
  // Only needed if you are going to support the OS calling RTC functions in virtual mode.
  // You will need to call EfiConvertPointer (). To convert any stored physical addresses 
  // to virtual address. After the OS transistions to calling in virtual mode, all future
  // runtime calls will be made in virtual mode.
  //
  return;
}



