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

#ifndef  __SBORM_LIBS_H__
#define  __SBORM_LIBS_H__


extern void mmu_setup(void);
extern void mmu_turn_off(void);

extern int create_heap(unsigned int pHeapHead, unsigned int nHeapSize);

extern unsigned int go_exec (unsigned int run_addr, unsigned int para_addr, int out_secure);

void boot0_jump(unsigned int addr);

#endif

