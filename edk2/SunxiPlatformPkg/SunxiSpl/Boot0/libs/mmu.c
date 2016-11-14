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

#include <boot0_include/config.h>
#include <boot0_include/boot0_helper.h>
#include <boot0_include/archdef.h>

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
void MmuSetup(UINT32 dram_size)
{
  UINT32 mmu_base;
  //use dram high 16M
  UINT32* mmu_base_addr = (UINT32 *)(PLAT_SDRAM_BASE +((dram_size-16)<<20));
  UINT32* page_table = mmu_base_addr;

  int i;
  UINT32 reg;

  page_table[0] = (3 << 10) | (15 << 5) | (1 << 3) | (0 << 2) | 0x2;
  /* the front 1G of memory(treated as 4G for all) is set up as none cacheable */
  for (i = 1; i < (PLAT_SDRAM_BASE>>20); i++)
    page_table[i] = (i << 20) | (3 << 10) | (15 << 5) | (0 << 3) | 0x2;
  /* Set up as write through and buffered(not write back) for other 3GB, rw for everyone */
  for (i = (PLAT_SDRAM_BASE>>20); i < 4096; i++)
    page_table[i] = (i << 20) | (3 << 10) | (15 << 5) | (1 << 3) | (0 << 2) | 0x2;
  /* flush tlb */
  asm volatile("mcr p15, 0, %0, c8, c7, 0" : : "r" (0));
  /* Copy the page table address to cp15 */

  mmu_base = (UINT32)mmu_base_addr;
  mmu_base |= (1 << 0) | (1 << 1) | (2 << 3);
  asm volatile("mcr p15, 0, %0, c2, c0, 0"
         : : "r" (mmu_base) : "memory");
  asm volatile("mcr p15, 0, %0, c2, c0, 1"
         : : "r" (mmu_base) : "memory");
  /* Set the access control to all-supervisor */
  asm volatile("mcr p15, 0, %0, c3, c0, 0"
         : : "r" (0x55555555));     //modified, origin value is (~0)
  asm volatile("isb");
  /* and enable the mmu */
  asm volatile("mrc p15, 0, %0, c1, c0, 0 @ get CR" : "=r" (reg) : : "cc");

  __usdelay(100);
  reg |= 1;    //enable mmu
  asm volatile("mcr p15, 0, %0, c1, c0, 0 @ set CR" : : "r" (reg) : "cc");
  asm volatile("isb");
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
void  MmuTurnOff( void )
{
  UINT32 reg;
  /* and disable the mmu */
  asm volatile("mrc p15, 0, %0, c1, c0, 0 @ get CR" : "=r" (reg) : : "cc");
  __usdelay(100);
  reg &= ~((7<<0)|(1<<12));    //disable mmu
  asm volatile("mcr p15, 0, %0, c1, c0, 0 @ set CR" : : "r" (reg) : "cc");
  ARCHISB;
  /*
   * Invalidate all instruction caches to PoU.
   * Also flushes branch target cache.
   */
  asm volatile ("mcr p15, 0, %0, c7, c5, 0" : : "r" (0));
  /* Invalidate entire branch predictor array */
  asm volatile ("mcr p15, 0, %0, c7, c5, 6" : : "r" (0));
  /* Full system DSB - make sure that the invalidation is complete */
  ARCHDSB;
  /* ISB - make sure the instruction stream sees it */
  ARCHISB;
}
