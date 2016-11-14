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

#ifndef _CCMU_H_
#define _CCMU_H_

#include  "hd_scrpt.h"

/* Offset */
#define CCMU_REG_o_PLL1_CTRL            0x00
#define CCMU_REG_o_PLL2_CTRL            0x08
#define CCMU_REG_o_PLL3_CTRL            0x10
#define CCMU_REG_o_PLL4_CTRL            0x18
#define CCMU_REG_o_PLL5_CTRL            0x20
#define CCMU_REG_o_PLL6_CTRL            0x28
#define CCMU_REG_o_PLL7_CTRL            0x30
#define CCMU_REG_o_PLL8_CTRL            0x38
#define CCMU_REG_o_MIPI_PLL_CTRL        0x40
#define CCMU_REG_o_PLL9_CTRL            0x44
#define CCMU_REG_o_PLL10_CTRL           0x48

#define CCMU_REG_o_AXI_MOD        0x50
#define CCMU_REG_o_AHB1_APB1      0x54
#define CCMU_REG_o_APB2_DIV       0x58
#define CCMU_REG_o_AXI_GATING     0x5C
#define CCMU_REG_o_AHB1_GATING0     0x60
#define CCMU_REG_o_AHB1_GATING1     0x64
#define CCMU_REG_o_APB1_GATING      0x68
#define CCMU_REG_o_APB2_GATING      0x6C

#define CCMU_REG_o_NAND0                0x80
#define CCMU_REG_o_NAND1                0x84
#define CCMU_REG_o_SD_MMC0              0x88
#define CCMU_REG_o_SD_MMC2              0x90

#define CCMU_REG_o_AVS                  0x144

#define CCMU_REG_o_MBUS0                0x15c
#define CCMU_REG_o_MBUS1                0x160


#define CCMU_REG_o_AHB1_RESET0      0x2C0
#define CCMU_REG_o_AHB1_RESET1      0x2C4
#define CCMU_REG_o_APB1_RESET     0x2D0
#define CCMU_REG_o_APB2_RESET     0x2D4

/* registers */
#define CCMU_REG_PLL1_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL1_CTRL   )
#define CCMU_REG_PLL2_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL2_CTRL   )
#define CCMU_REG_PLL3_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL3_CTRL   )
#define CCMU_REG_PLL4_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL4_CTRL   )
#define CCMU_REG_PLL5_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL5_CTRL   )
#define CCMU_REG_PLL6_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL6_CTRL   )
#define CCMU_REG_PLL8_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL8_CTRL   )
#define CCMU_REG_PLL9_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL9_CTRL   )
#define CCMU_REG_PLL10_CTRL             ( CCMU_REGS_BASE + CCMU_REG_o_PLL10_CTRL  )
#define CCMU_REG_MIPI_PLL_CTRL          ( CCMU_REGS_BASE + CCMU_REG_o_MIPI_PLL_CTRL  )


#define CCMU_REG_AXI_MOD                ( CCMU_REGS_BASE + CCMU_REG_o_AXI_MOD     )
#define CCMU_REG_AHB1_APB1        ( CCMU_REGS_BASE + CCMU_REG_o_AHB1_APB1   )
#define CCMU_REG_APB2_DIV               ( CCMU_REGS_BASE + CCMU_REG_o_APB2_DIV    )
#define CCMU_REG_AHB1_GATING0     ( CCMU_REGS_BASE + CCMU_REG_o_AHB1_GATING0)
#define CCMU_REG_AHB1_GATING1     ( CCMU_REGS_BASE + CCMU_REG_o_AHB1_GATING1)
#define CCMU_REG_APB1_GATING      ( CCMU_REGS_BASE + CCMU_REG_o_APB1_GATING )
#define CCMU_REG_APB2_GATING      ( CCMU_REGS_BASE + CCMU_REG_o_APB2_GATING )


#define CCMU_REG_NAND                   ( CCMU_REGS_BASE + CCMU_REG_o_NAND        )
#define CCMU_REG_SD_MMC0                ( CCMU_REGS_BASE + CCMU_REG_o_SD_MMC0     )
#define CCMU_REG_SD_MMC2                ( CCMU_REGS_BASE + CCMU_REG_o_SD_MMC2     )
#define CCMU_REG_SPI0                   ( CCMU_REGS_BASE + CCMU_REG_o_SPI0        )
#define CCMU_REG_SPI1                   ( CCMU_REGS_BASE + CCMU_REG_o_SPI1        )

#define CCMU_REG_MBUS0                  ( CCMU_REGS_BASE + CCMU_REG_o_MBUS0       )
#define CCMU_REG_MBUS1                  ( CCMU_REGS_BASE + CCMU_REG_o_MBUS1       )

#define CCMU_REG_AVS          ( CCMU_REGS_BASE + CCMU_REG_o_AVS         )


#define CCMU_REG_AHB1_RESET0      ( CCMU_REGS_BASE + CCMU_REG_o_AHB1_RESET0 )
#define CCMU_REG_AHB1_RESET1      ( CCMU_REGS_BASE + CCMU_REG_o_AHB1_RESET1 )
#define CCMU_REG_APB1_RESET         ( CCMU_REGS_BASE + CCMU_REG_o_APB1_RESET  )
#define CCMU_REG_APB2_RESET       ( CCMU_REGS_BASE + CCMU_REG_o_APB2_RESET  )

#endif    // #ifndef _CCMU_H_
