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

#ifndef __SUNXI_SPRITE_SPARSE_H__
#define __SUNXI_SPRITE_SPARSE_H__


#define   ANDROID_FORMAT_UNKNOW    (0)
#define   ANDROID_FORMAT_BAD       (-1)
#define   ANDROID_FORMAT_DETECT    (1)


extern int  unsparse_probe(char *source, unsigned int length, unsigned int flash_start);
extern int  unsparse_direct_write(void *pbuf, unsigned int length);
extern unsigned int unsparse_checksum(void);


#endif /* __SUNXI_SPRITE_SPARSE_H__ */
