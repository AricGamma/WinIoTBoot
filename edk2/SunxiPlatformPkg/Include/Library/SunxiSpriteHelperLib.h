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

#ifndef  __SPRITE_PRIVATEDATA_H__
#define __SPRITE_PRIVATEDATA_H__

BOOLEAN SunxiSpriteStorePrivateData(void *mbr);
BOOLEAN SunxiSpriteRestorePrivateData(void *mbr);
BOOLEAN SunxiSpriteProbePrivatePartition(void  *mbr);
BOOLEAN SunxiSpritePartRawDataVerify(UINT32 BaseStart, UINT64 BaseBytes);
BOOLEAN SunxiSpriteVerifyMbr(void *buffer);


#endif




