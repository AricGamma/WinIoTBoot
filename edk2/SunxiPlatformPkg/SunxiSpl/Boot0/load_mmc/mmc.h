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

#include <Interinc/spare_head.h>

extern void set_mmc_para(int smc_no, void *addr);
extern int SunxiMmcInit(int sdc_no, unsigned bus_width, const normal_gpio_cfg *gpio_info, int offset ,void *extra_data);
extern int SunxiMmcExit(int sdc_no, const normal_gpio_cfg *gpio_info, int offset);
extern unsigned long MmcBread(int dev_num, unsigned long start, unsigned blkcnt, void *dst);
