/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  tangmanliang <tangmanliang@allwinnertech.com>
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



#include <Base.h>

#include <Library/BaseLib.h>
#include <Library/TimerLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Sun50iW1P1/timer.h>
#include <Sun50iW1P1/platform.h>
#include <Sun50iW1P1/ccmu.h>
#include <Library/DebugLib.h>

/* init timer register */
RETURN_STATUS
EFIAPI
TimerConstructor (
  VOID
  )
{
  UINT32 val;
  //DEBUG((DEBUG_INFO, "--%a: run begin\n",__func__));
  struct sunxi_timer_reg *timer_reg = (struct sunxi_timer_reg *)SUNXI_TIMER_BASE;
  //struct sunxi_ccm_reg *ccm_reg = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
  val = MmioRead32(CCMU_AVS_CLK_REG);
  val |= (1U << 31);
  MmioWrite32(CCMU_AVS_CLK_REG, val);

  timer_reg->tirqen  = 0;
  timer_reg->tirqsta |= 0x03;
  /* start avs as counter */
  //ccm_reg->avs_clk_cfg |= (1 << 31);
  timer_reg->avs.ctl  = 3; //enable avs cnt0 and cnt1,source is 24M
  /* div cnt0 12000 to 2000hz, high 32 bit means 1000hz.*/
  /* div cnt 1 12 to 2000000hz ,high 32 bit means 1000000hz */
  timer_reg->avs.div  |= 0xc0000;
  //timer_reg->avs.cnt0 = 0;
  //DEBUG((DEBUG_INFO, "--%a: run end\n",__func__));

  return RETURN_SUCCESS;
}

/**
  Stalls the CPU for at least the given number of microseconds.

  Stalls the CPU for the number of microseconds specified by MicroSeconds.

  @param  MicroSeconds  The minimum number of microseconds to delay.

  @return The value of MicroSeconds inputted.

**/
UINTN
EFIAPI
uSecondDelay (
  IN  UINTN uSeconds
  )
{
  UINT32 t1, t2;
  struct sunxi_timer_reg *timer_reg = (struct sunxi_timer_reg *)SUNXI_TIMER_BASE;

  t1 = timer_reg->avs.cnt0;
  t2 = t1 + uSeconds;
  do
  {
    t1 = timer_reg->avs.cnt0;
  }
  while(t2 >= t1);

  return uSeconds;
}

/**
  Stalls the CPU for at least the given number of useconds.

  Stalls the CPU for the number of nanoseconds specified by NanoSeconds.

  @param  NanoSeconds The minimum number of nanoseconds to delay.

  @return The value of NanoSeconds inputted.

**/
UINTN
EFIAPI
MicroSecondDelay (
  IN  UINTN MicroSeconds
  )
{
  UINT32 t1, t2;
  //  UINT64 TimerTicks64;

  // Calculate counter ticks that can represent requested delay
  //      TimerTicks64 = MultU64x32 (MicroSeconds, 24000000/1000000U);
  
  struct sunxi_timer_reg *timer_reg = (struct sunxi_timer_reg *)SUNXI_TIMER_BASE;
  timer_reg->avs.cnt1 = 0;

  t1 = timer_reg->avs.cnt1;
  //t2 = t1 + TimerTicks64;
  t2 = t1 + MicroSeconds;
  do
  {
    t1 = timer_reg->avs.cnt1;
  }
  while(t2 >= t1);

  return MicroSeconds;
}


/**
  Stalls the CPU for at least the given number of nanoseconds.

  Stalls the CPU for the number of nanoseconds specified by NanoSeconds.

  @param  NanoSeconds The minimum number of nanoseconds to delay.

  @return The value of NanoSeconds inputted.

**/
UINTN
EFIAPI
NanoSecondDelay (
  IN  UINTN NanoSeconds
  )
{
  UINTN  MicroSeconds;

  MicroSeconds = NanoSeconds / 1000;
  MicroSeconds += ((NanoSeconds % 1000) == 0) ? 0 : 1;

  MicroSecondDelay(MicroSeconds);

  return NanoSeconds;
}

/**
  Retrieves the current value of a 64-bit free running performance counter.

  The counter can either count up by 1 or count down by 1. If the physical
  performance counter counts by a larger increment, then the counter values
  must be translated. The properties of the counter can be retrieved from
  GetPerformanceCounterProperties().

  @return The current value of the free running performance counter.

**/
UINT64
EFIAPI
GetPerformanceCounter (
  VOID
  )
{
  UINT64 count;
  struct sunxi_timer_reg *timer_reg = (struct sunxi_timer_reg *)SUNXI_TIMER_BASE;
  count = (UINT64)timer_reg->avs.cnt0;
  //        DEBUG((DEBUG_INFO, "--%a: current timer is 0x%x\n",__func__, count));
  /* fix overflow issue, cnt1 register is 32 bit */
  count = count * 1000; 
  count = count * (PcdGet32 (PcdArmArchTimerFreqInHz)/1000000);
  return count;
}


/**
  Retrieves the 64-bit frequency in Hz and the range of performance counter
  values.

  If StartValue is not NULL, then the value that the performance counter starts
  with immediately after is it rolls over is returned in StartValue. If
  EndValue is not NULL, then the value that the performance counter end with
  immediately before it rolls over is returned in EndValue. The 64-bit
  frequency of the performance counter in Hz is always returned. If StartValue
  is less than EndValue, then the performance counter counts up. If StartValue
  is greater than EndValue, then the performance counter counts down. For
  example, a 64-bit free running counter that counts up would have a StartValue
  of 0 and an EndValue of 0xFFFFFFFFFFFFFFFF. A 24-bit free running counter
  that counts down would have a StartValue of 0xFFFFFF and an EndValue of 0.

  @param  StartValue  The value the performance counter starts with when it
                      rolls over.
  @param  EndValue    The value that the performance counter ends with before
                      it rolls over.

  @return The frequency in Hz.

**/
UINT64
EFIAPI
GetPerformanceCounterProperties (
  OUT UINT64  *StartValue,  OPTIONAL
  OUT UINT64  *EndValue     OPTIONAL
  )
{
  if (StartValue != NULL) {
    // Timer starts with the reload value
    *StartValue = (UINT64)0ULL;
  }

  if (EndValue != NULL) {
    // Timer counts up to 0xFFFFFFFF
    *EndValue = 0xFFFFFFFF;
  }

  return (UINT64)(PcdGet32 (PcdArmArchTimerFreqInHz));
}

