/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 *
 *  This program and the accompanying materials                          
 *  are licensed and made available under the terms and conditions of the BSD License         
 *  which accompanies this distribution.  The full text of the license may be found at        
 *  http://opensource.org/licenses/bsd-license.php                                            
 *
 *  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
 *  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
 *
 */

#ifndef _SUNXI_CPU_H
#define _SUNXI_CPU_H

#include "platform.h"

#ifndef __ASSEMBLY__
/* boot type */
typedef enum {
  SUNXI_BOOT_TYPE_NULL = -1,
  SUNXI_BOOT_TYPE_NAND = 0,
  SUNXI_BOOT_TYPE_MMC0 = 1,
  SUNXI_BOOT_TYPE_MMC2 = 2,
  SUNXI_BOOT_TYPE_SPI  = 3
} sunxi_boot_type_t;

sunxi_boot_type_t get_boot_type(void);
#endif /* __ASSEMBLY__ */

#define SUNXI_GET_BITS(value, start_bit, bits_num) ( (value >> start_bit) & \
                          ((1 << bits_num) - 1) )

#endif /* _CPU_H */
