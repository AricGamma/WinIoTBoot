/** @file
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  wangwei <wangwei@allwinnertech.com>
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

#ifndef __SUNXI_KEY_LIB_H__
#define __SUNXI_KEY_LIB_H__

BOOLEAN EFIAPI SunxiKeyInit(VOID);

VOID    EFIAPI SunxiKeyExit(VOID);

INT32   EFIAPI SunxiKeyRead(VOID);

EFI_STATUS EFIAPI I2cKeyRead(UINT8 *KeyValue);
EFI_STATUS EFIAPI I2cKeyCheck(VOID);

#endif  


