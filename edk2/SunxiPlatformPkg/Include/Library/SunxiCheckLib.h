/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  sunny <sunny@allwinnertech.com>
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

#ifndef __CAL_CRC_H__
#define __CAL_CRC_H__

UINT32 SunxiCrc32(const void * buffer, UINT32 length);
UINT32 SunxiAddSum(void *buffer, UINT32 length);
UINT32 SunxiSpriteGenerateChecksum(void *buffer, UINT32 length, UINT32 src_sum);
INT32 SunxiSpriteVerifyChecksum(void *buffer, UINT32 length, UINT32 src_sum);


#endif	/* __CAL_CRC_H__ */
