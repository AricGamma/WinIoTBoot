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

#ifndef __SUNXI_COMMON_LIB_H__
#define __SUNXI_COMMON_LIB_H__

//set fel flag
VOID SunxiSetFelKey(VOID);

//reset
VOID SunxiWatchDogReset(VOID);

//get core pll
UINT32 SunxiGetCoreClock(VOID);

void BootDramUpdateFlagSet(UINT32 *dram_para);
void DumpDramPara(void* dram, UINT32 size);

#endif  


