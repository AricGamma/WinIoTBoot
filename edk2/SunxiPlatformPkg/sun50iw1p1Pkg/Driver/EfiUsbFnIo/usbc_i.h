/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  Jerry Wang <wangflord@allwinnertech.com>
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


#ifndef  __USBC_I_H__
#define  __USBC_I_H__

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UncachedMemoryAllocationLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/TimerLib.h>

#include <Sunxi_type/Sunxi_type.h>
#include <Sunxi_type/PrivateDef.h>
#include <Sun50iW1P1/platform.h>
#include <Sun50iW1P1/cpu.h>
#include <Sun50iW1P1/ccmu.h>
#include <Sun50iW1P1/clock.h>
#include <Sun50iW1P1/usb.h>

#define  USBC_MAX_OPEN_NUM    8

/* 记录USB的公共信息 */
typedef struct __fifo_info{
  uint port0_fifo_addr;
  uint port0_fifo_size;

  uint port1_fifo_addr;
  uint port1_fifo_size;

  uint port2_fifo_addr;
  uint port2_fifo_size;
}__fifo_info_t;

/* 记录当前USB port所有的硬件信息 */
typedef struct __usbc_otg{
  uint  port_num;
  ulong base_addr;        /* usb base address     */

  uint used;             /* 是否正在被使用       */
  uint no;               /* 在管理数组中的位置     */
}__usbc_otg_t;

#endif   //__USBC_I_H__

