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


#ifndef __BSP_NAND_H__
#define __BSP_NAND_H__

int nand_uboot_init(int boot_mode);

int nand_uboot_exit(void);

uint nand_uboot_read(uint start, uint sectors, void *buffer);

uint nand_uboot_write(uint start, uint sectors, void *buffer);

int nand_download_boot0(uint length, void *buffer);

int nand_download_uboot(uint length, void *buffer);

int nand_uboot_erase(int user_erase);

uint nand_uboot_get_flash_info(void *buffer, uint length);

uint nand_uboot_set_flash_info(void *buffer, uint length);

uint nand_uboot_get_flash_size(void);


#endif  //ifndef __NAND_LOGIC_H__



