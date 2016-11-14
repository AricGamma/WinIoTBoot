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

#include "disp_private.h"

extern void disp_delay_us(u32 us);
extern void disp_delay_ms(u32 ms);


s32 bsp_disp_delay_ms(u32 ms)
{
#if defined(__LINUX_PLAT__)
  u32 timeout = ms*HZ/1000;

  set_current_state(TASK_INTERRUPTIBLE);
  schedule_timeout(timeout);
#endif
#if 0
#ifdef __BOOT_OSAL__
  wBoot_timer_delay(ms);//assume cpu runs at 1000Mhz,10 clock one cycle
#endif
#endif
#ifdef __UBOOT_OSAL__
    disp_delay_ms(ms);
#endif
  return 0;
}


s32 bsp_disp_delay_us(u32 us)
{
#if defined(__LINUX_PLAT__)
  udelay(us);
#endif
#ifdef __BOOT_OSAL__
  volatile u32 time;

  for(time = 0; time < (us*700/10);time++);//assume cpu runs at 700Mhz,10 clock one cycle
#endif
#ifdef __UBOOT_OSAL__
    disp_delay_us(us);
#endif
  return 0;
}

struct disp_notifier_block disp_notifier_list;

s32 disp_notifier_init(void)
{
  INIT_LIST_HEAD(&disp_notifier_list.list);
  return 0;
}

s32 disp_notifier_register(struct disp_notifier_block *nb)
{
  if((NULL == nb)) {
    DE_WRN("hdl is NULL\n");
    return -1;
  }
  list_add_tail(&(nb->list), &(disp_notifier_list.list));
  return 0;
}

s32 disp_notifier_unregister(struct disp_notifier_block *nb)
{
  struct disp_notifier_block *ptr;
  if((NULL == nb)) {
    DE_WRN("hdl is NULL\n");
    return -1;
  }
  list_for_each_entry(ptr, &disp_notifier_list.list, list) {
    if(ptr == nb) {
      list_del(&ptr->list);
      return 0;
    }
  }
  return -1;
}

s32 disp_notifier_call_chain(u32 event, u32 sel, void *v)
{
  struct disp_notifier_block *ptr;
  list_for_each_entry(ptr, &disp_notifier_list.list, list) {
    if(ptr->notifier_call)
      ptr->notifier_call(ptr, event, sel, v);
  }

  return 0;
}

