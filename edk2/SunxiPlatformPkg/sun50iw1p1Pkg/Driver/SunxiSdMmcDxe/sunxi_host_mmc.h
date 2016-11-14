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

#ifndef __SUNXI_HOST_MMC_H__
#define __SUNXI_HOST_MMC_H__

#ifndef CONFIG_ARCH_SUN7I
#define MMC_REG_FIFO_OS   (0x200)
#define MMC2_REG_FIFO_OS  (0X20)
#else
#define MMC_REG_FIFO_OS   (0x100)
#endif

int sunxi_sun50iw1p1_mmc_init(int sdc_no);
int sunxi_sun50iw1p1_mmc_exit(int sdc_no);



#endif /*  __SUNXI_HOST_MMC_H__ */




