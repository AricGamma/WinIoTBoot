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
#ifndef  __SUNXI_SPRITE_H__
#define  __SUNXI_SPRITE_H__

#include <Sunxi_type/Sunxi_type.h>

int sunxi_sprite_init(int stage);

int sunxi_sprite_read (uint start_block, uint nblock, void *buffer);

int sunxi_sprite_write(uint start_block, uint nblock, void *buffer);

int sunxi_sprite_erase(int erase, void *mbr_buffer);

int sunxi_sprite_exit(int force);


int sunxi_sprite_download_mbr(void *buffer, uint buffer_size);

int sunxi_sprite_download_uboot(void *buffer);

int sunxi_sprite_download_boot0(void *buffer);

int sunxi_sprite_erase_flash(void *MbrBuffer);


//--------------------------
int sunxi_flash_read (uint start_block, uint nblock, void *buffer);
int sunxi_flash_write(uint start_block, uint nblock, void *buffer);
uint sunxi_flash_size(void);

#endif