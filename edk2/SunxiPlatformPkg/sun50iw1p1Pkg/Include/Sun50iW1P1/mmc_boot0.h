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

#ifndef _SUNXI_MMC_BOOT0_H
#define _SUNXI_MMC_BOOT0_H

void set_mmc_para(int smc_no,void *sdly_addr );
unsigned long mmc_bread(int dev_num, unsigned long start, unsigned blkcnt, void *dst);
int sunxi_mmc_init(int sdc_no, unsigned bus_width, const normal_gpio_cfg *gpio_info, int offset ,void *extra_data);
int sunxi_mmc_exit(int sdc_no, const normal_gpio_cfg *gpio_info, int offset);

#endif /* _SUNXI_MMC_BOOT0_H */
