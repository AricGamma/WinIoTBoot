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

#ifndef  __SUNXI_SPRITE_CARD_H__
#define  __SUNXI_SPRITE_CARD_H__

#include <Sunxi_type/Sunxi_type.h>
#include <Library/SunxiMbr.h>
extern uint sprite_card_firmware_start(void);

extern int sprite_card_firmware_probe(char *name);

extern int sprite_card_fetch_download_map(sunxi_download_info  *dl_map);

extern int sprite_card_fetch_mbr(void  *img_mbr);

extern int sunxi_sprite_deal_part(sunxi_download_info *dl_map);

extern int sunxi_sprite_deal_uboot(void);

extern int sunxi_sprite_deal_boot0(void);

extern int card_download_uboot(uint length, void *buffer);

extern int card_download_boot0(uint length, void *buffer);

extern int card_download_standard_mbr(void *buffer);

#endif

