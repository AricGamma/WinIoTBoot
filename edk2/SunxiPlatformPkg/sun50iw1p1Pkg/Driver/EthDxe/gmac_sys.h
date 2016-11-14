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

#ifndef _GMAC_SYS_H_
#define _GMAC_SYS_H_



#ifdef GMAC_AW1633
/*reg for bist test*/
//aw1633 gmac fifo map control bit is BIST_TEST_REG[16];
#define SRAM_CTRL_BASE_ADDR   (0x01C00000)
#define SRAM_CTRL_REG0      (SRAM_CTRL_BASE_ADDR + 0x0)
#define SRAM_CTRL_REG1      (SRAM_CTRL_BASE_ADDR + 0x4)
#define BIST_CTRL_REG     (SRAM_CTRL_BASE_ADDR + 0x14)
#define BIST_START_ADDR_REG   (SRAM_CTRL_BASE_ADDR + 0x18)
#define BIST_END_ADDR_REG   (SRAM_CTRL_BASE_ADDR + 0x1C)
#define BIST_DATA_MASK_REG    (SRAM_CTRL_BASE_ADDR + 0x20)

#define BIST_TEST_REG     (SRAM_CTRL_BASE_ADDR + 0x48)

#define SRAM_CFG        SRAM_CTRL_REG0
#define SRAM_CTL                SRAM_CTRL_REG1
#define SRAM_BISTC              BIST_CTRL_REG
#define SRAM_BISTSA             BIST_START_ADDR_REG
#define SRAM_BISTEA             BIST_END_ADDR_REG
#endif /*#ifdef GMAC_AW1633*/

#ifdef GMAC_AW1651
//aw1651 gmac fifo map control bit is SRAM_CTRL_REG1[20];
#define SYS_CTRL_BASE_ADDR    (0x01C00000)

#define SRAM_CTRL_REG0      (SYS_CTRL_BASE_ADDR + 0x0)
#define SRAM_CTRL_REG1      (SYS_CTRL_BASE_ADDR + 0x4)
#define BIST_CTRL_REG     (SYS_CTRL_BASE_ADDR + 0x14)
#define BIST_START_ADDR_REG   (SYS_CTRL_BASE_ADDR + 0x18)
#define BIST_END_ADDR_REG   (SYS_CTRL_BASE_ADDR + 0x1C)
#define BIST_DATA_MASK_REG    (SYS_CTRL_BASE_ADDR + 0x20)

#define SRAM_CFG        SRAM_CTRL_REG0
#define SRAM_CTL                SRAM_CTRL_REG1
#define SRAM_BISTC              BIST_CTRL_REG
#define SRAM_BISTSA             BIST_START_ADDR_REG
#define SRAM_BISTEA             BIST_END_ADDR_REG
#endif /*#ifdef GMAC_AW1651*/

#ifdef GMAC_AW1639
//aw1639 gmac fifo map control bit is SRAM_TEST_REG[16]
//----------------------------------------------------------
#define SYS_CTRL_BASE_ADDR    (SYSCTRL_BASE)

#define SRAM_CTRL_REG0      (SYS_CTRL_BASE_ADDR + 0x0) //SRAM_C1_MAP
#define SRAM_CTRL_REG1      (SYS_CTRL_BASE_ADDR + 0x4) //BIST_DMA_CTRL_SEL
#define VER_REG         (SYS_CTRL_BASE_ADDR + 0x24)

//#define GMAC_CLK_REG      (SYS_CTRL_BASE_ADDR + 0x30) //EMAC_PHY_INF_CTRL_REG
//#define DISP_MUX_CTRL_REG   (SYS_CTRL_BASE_ADDR + 0x38) //

#define BIST0_CTRL_REG      (SYS_CTRL_BASE_ADDR + 0x40)
#define BIST_START_ADDR_REG   (SYS_CTRL_BASE_ADDR + 0x44)
#define BIST_END_ADDR_REG   (SYS_CTRL_BASE_ADDR + 0x48)
#define BIST_DATA_MASK_REG    (SYS_CTRL_BASE_ADDR + 0x4C)
#define BIST1_CTRL_REG      (SYS_CTRL_BASE_ADDR + 0x60)
#define BIST2_CTRL_REG      (SYS_CTRL_BASE_ADDR + 0x64)
#define BIST3_CTRL_REG      (SYS_CTRL_BASE_ADDR + 0x68)

#define TEST_DBG_REG0     (SYS_CTRL_BASE_ADDR + 0x90)
#define TEST_DBG_REG1     (SYS_CTRL_BASE_ADDR + 0x94)

//#define SRAM_CONFIG_REG     (SYS_CTRL_BASE_ADDR + 0xC0)
#define SRAM_TEST_REG     (SYS_CTRL_BASE_ADDR + 0xC8)
//#define SRAM_ADDR_TWIST_REG   (SYS_CTRL_BASE_ADDR + 0xE0)
//----------------------------------------------------------

#define SRAM_BISTC              BIST1_CTRL_REG
#endif /*#ifdef GMAC_AW1639*/

#ifdef GMAC_AW1673 //or GMAC_AW1671
//aw1639 gmac fifo map control bit is SRAM_TEST_REG[16]
//----------------------------------------------------------
#define SYS_CTRL_BASE_ADDR    (SYSCTRL_BASE)

#define SRAM_CTRL_REG0      (SYS_CTRL_BASE_ADDR + 0x0) //SRAM_C1_MAP
#define SRAM_CTRL_REG1      (SYS_CTRL_BASE_ADDR + 0x4) //BIST_DMA_CTRL_SEL
#define VER_REG         (SYS_CTRL_BASE_ADDR + 0x24)

//#define GMAC_CLK_REG      (SYS_CTRL_BASE_ADDR + 0x30) //EMAC_PHY_INF_CTRL_REG
//#define DISP_MUX_CTRL_REG   (SYS_CTRL_BASE_ADDR + 0x38) //

#define BIST_CTRL_REG     (SYS_CTRL_BASE_ADDR + 0x14)
#define BIST_START_ADDR_REG   (SYS_CTRL_BASE_ADDR + 0x18)
#define BIST_END_ADDR_REG   (SYS_CTRL_BASE_ADDR + 0x1c)
#define BIST_DATA_MASK_REG    (SYS_CTRL_BASE_ADDR + 0x20)


#define TEST_DBG_REG0     (SYS_CTRL_BASE_ADDR + 0x90)
#define TEST_DBG_REG1     (SYS_CTRL_BASE_ADDR + 0x94)

//#define SRAM_CONFIG_REG     (SYS_CTRL_BASE_ADDR + 0xC0)
#define SRAM_TEST_REG     (SYS_CTRL_BASE_ADDR + 0x48)
//#define SRAM_ADDR_TWIST_REG   (SYS_CTRL_BASE_ADDR + 0xE0)
//----------------------------------------------------------

#define SRAM_BISTC              BIST_CTRL_REG

#endif /*#ifdef GMAC_AW1671*/


#ifndef NULL
  #define NULL 0
#endif

typedef enum {
  AW_ERR = -1,
  AW_OK = 0,
  AW_UNKNOWN_DMA_DESC_TYPE,
  AW_UNKNOWN_DMA_DESC_LIST_ARCH,
  AW_ERR_NO_ENOUGH_DMA_DESC,
  AW_ERR_NEXT_DESC_HAVE_NO_SOF,
  AW_ERR_NO_FREE_FRM_SPACE,
  AW_STATUS_NUM_MAX
} AW_STATUS;

typedef enum _BIT_VAL {
  BIT_RESET = 0,
  BIT_SET = 1,
  BIT_VAL_NUM_MAX
}BIT_VAL;


#define AW_SUCCESS(x)        (x == A_OK)
#define AW_FAILED(x)         (!A_SUCCESS(x))


/**
 * GMAC Interface Config
 * -- MAC Interface and Tx Clk selection
 */
/**
 * GMAC Interface Config
 * -- MAC Interface and Tx Clk selection
 */
#ifdef GMAC_AW1633
#define EMAC_PHY_INF_CTRL_REG   (CCM_BASE + 0xD0) /*CCM_BASE: 0x01C20000*/
#endif
#ifdef GMAC_AW1651
#define EMAC_PHY_INF_CTRL_REG   (CCM_BASE + 0x0164) /*CCM_BASE: 0x01C20000*/
#endif
#ifdef GMAC_AW1639
#define EMAC_PHY_INF_CTRL_REG (SYSCTRL_BASE + 0x30)
#endif
#ifdef GMAC_AW1671
#define EMAC_PHY_INF_CTRL_REG (SYSCTRL_BASE + 0x30)
#endif
#ifdef GMAC_AW1673
#define EMAC_PHY_INF_CTRL_REG (SYSCTRL_BASE + 0x30)
#endif


#define PHY_TXC_DELAY_MSB   (12)
#define PHY_TXC_DELAY_LSB   (10)
#define PHY_TXC_DELAY_MASK    (0x00001C00)
#define PHY_TXC_DELAY_SET(x)  (((x) << PHY_TXC_DELAY_LSB) & PHY_TXC_DELAY_MASK)
#define PHY_TXC_DELAY_GET(x)  (((x) & PHY_TXC_DELAY_MASK) >> PHY_TXC_DELAY_LSB)

#define PHY_RXC_DELAY_MSB   (9)
#define PHY_RXC_DELAY_LSB   (5)
#define PHY_RXC_DELAY_MASK    (0x000003E0)
#define PHY_RXC_DELAY_SET(x)  (((x) << PHY_RXC_DELAY_LSB) & PHY_RXC_DELAY_MASK)
#define PHY_RXC_DELAY_GET(x)  (((x) & PHY_RXC_DELAY_MASK) >> PHY_RXC_DELAY_LSB)

#define PHY_RXC_EN_INV_MSB    (4)
#define PHY_RXC_EN_INV_LSB    (4)
#define PHY_RXC_EN_INV_MASK   (0x00000010)
#define PHY_RXC_EN_INV_SET(x)   (((x) << PHY_RXC_EN_INV_LSB) & PHY_RXC_EN_INV_MASK)
#define PHY_RXC_EN_INV_GET(x)   (((x) & PHY_RXC_EN_INV_MASK) >> PHY_RXC_EN_INV_LSB)

#define PHY_TXC_EN_INV_MSB    (3)
#define PHY_TXC_EN_INV_LSB    (3)
#define PHY_TXC_EN_INV_MASK   (0x00000008)
#define PHY_TXC_EN_INV_SET(x)   (((x) << PHY_TXC_EN_INV_LSB) & PHY_TXC_EN_INV_MASK)
#define PHY_TXC_EN_INV_GET(x)   (((x) & PHY_TXC_EN_INV_MASK) >> PHY_TXC_EN_INV_LSB)

#define PHY_INF_SEL_MSB     (2)
#define PHY_INF_SEL_LSB     (2)
#define PHY_INF_SEL_MASK    (0x00000004)
#define PHY_INF_SEL_SET(x)    (((x) << PHY_INF_SEL_LSB) & PHY_INF_SEL_MASK)
#define PHY_INF_SEL_GET(x)    (((x) & PHY_INF_SEL_MASK) >> PHY_INF_SEL_LSB)

#define PHY_TXC_SEL_MSB     (1)
#define PHY_TXC_SEL_LSB     (0)
#define PHY_TXC_SEL_MASK    (0x00000003)
#define PHY_TXC_SEL_SET(x)    (((x) << PHY_TXC_SEL_LSB) & PHY_TXC_SEL_MASK)
#define PHY_TXC_SEL_GET(x)    (((x) & PHY_TXC_SEL_MASK) >> PHY_TXC_SEL_LSB)


#ifdef GMAC_AW1673
#define EMAC_AHB2_CFG     (0x01c20000 + 0x5C)
#endif


typedef enum _phy_inf_type{
  GMII_MII = 0,
  RGMII = 1,
  PHY_INF_TYPE_MAX_NUM
} PHY_INF;

typedef enum _tx_clk_src{
  TXC_IN   = 0, /*only for MII*/
  CLK125   = 1,
  RXC_IN   = 2,
  RESERVED = 3  /*Reserved*/
} TXC_SRC;

typedef struct _phy_inf_ctrl{
  PHY_INF     phy_inf_sel;
  TXC_SRC     txc_src_sel;
} PHY_INF_CTRL;


void gmac_sys_init (void);
void gmac_sys_exit (void);
//AW_STATUS gmac_set_phy_inf_ctrl(PHY_INF phy_inf_sel, TXC_SRC txc_src_sel);
//AW_STATUS gmac_set_rx_delay_chain(u32 rx_delay);
//AW_STATUS gmac_set_tx_delay_chain(u32 tx_delay);
//AW_STATUS gmac_txclk_inv_onoff(u32 on);
//AW_STATUS gmac_rxclk_inv_onoff(u32 on);

#endif /*_GMAC_SYS_H_*/
