/** @file
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  Martin.Zheng <martinzheng@allwinnertech.com>
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

#ifndef __SUNXI_DMA_LIB_H__
#define __SUNXI_DMA_LIB_H__


//================================
//======    DMA 配置     =========
//================================

/* DMA 基础配置  */
#define DMAC_CFG_CONTINUOUS_ENABLE              (0x01)
#define DMAC_CFG_CONTINUOUS_DISABLE             (0x00)

/* DMA 传输目的端 配置 */
/* DMA 目的端 传输宽度 */
#define DMAC_CFG_DEST_DATA_WIDTH_8BIT     (0x00)
#define DMAC_CFG_DEST_DATA_WIDTH_16BIT      (0x01)
#define DMAC_CFG_DEST_DATA_WIDTH_32BIT      (0x02)

/* DMA 目的端 突发传输模式 */
#define DMAC_CFG_DEST_1_BURST             (0x00)
#define DMAC_CFG_DEST_4_BURST           (0x01)
#define DMAC_CFG_DEST_8_BURST         (0x02)

/* DMA 目的端 地址变化模式 */
#define DMAC_CFG_DEST_ADDR_TYPE_LINEAR_MODE   (0x00)
#define DMAC_CFG_DEST_ADDR_TYPE_IO_MODE     (0x01)


/* DMA 传输源端 配置 */
/* DMA 源端 传输宽度 */
#define DMAC_CFG_SRC_DATA_WIDTH_8BIT      (0x00)
#define DMAC_CFG_SRC_DATA_WIDTH_16BIT     (0x01)
#define DMAC_CFG_SRC_DATA_WIDTH_32BIT     (0x02)

/* DMA 源端 突发传输模式 */
#define DMAC_CFG_SRC_1_BURST              (0x00)
#define DMAC_CFG_SRC_4_BURST            (0x01)
#define DMAC_CFG_SRC_8_BURST            (0x02)

/* DMA 源端 地址变化模式 */
#define DMAC_CFG_SRC_ADDR_TYPE_LINEAR_MODE    (0x00)
#define DMAC_CFG_SRC_ADDR_TYPE_IO_MODE      (0x01)


/* DMA 传输目的端 配置 */
#define DMAC_CFG_DEST_TYPE_SRAM         (0x00)
#define DMAC_CFG_DEST_TYPE_DRAM           (0x01)

#define DMAC_CFG_DEST_TYPE_CODEC          (15)

#define DMAC_CFG_DEST_TYPE_OTG_EP1          (17)
#define DMAC_CFG_DEST_TYPE_OTG_EP2          (18)
#define DMAC_CFG_DEST_TYPE_OTG_EP3          (19)
#define DMAC_CFG_DEST_TYPE_OTG_EP4          (20)
#define DMAC_CFG_DEST_TYPE_OTG_EP5          (21)
/* DMA 传输源端 配置 */
#define DMAC_CFG_SRC_TYPE_SRAM          (0x00)
#define DMAC_CFG_SRC_TYPE_DRAM              (0x01)

#define DMAC_CFG_SRC_TYPE_CODEC           (15)

#define DMAC_CFG_SRC_TYPE_OTG_EP1         (17)
#define DMAC_CFG_SRC_TYPE_OTG_EP2         (18)
#define DMAC_CFG_SRC_TYPE_OTG_EP3         (19)
#define DMAC_CFG_SRC_TYPE_OTG_EP4         (20)
#define DMAC_CFG_SRC_TYPE_OTG_EP5         (21)


typedef struct
{
  UINT32 config;
  UINT32 source_addr;
  UINT32 dest_addr;
  UINT32 byte_count;
  UINT32 commit_para;
  UINT32 link;
  UINT32 reserved[2];
}
sunxi_dma_start_t;


typedef struct
{
  UINT32      src_drq_type     : 5;            //源地址存储类型，如DRAM, SPI,NAND等，参见  __ndma_drq_type_t
  UINT32      src_addr_mode    : 2;            //原地址类型 0:递增模式  1:保持不变
  UINT32      src_burst_length : 2;            //发起一次burst宽度 0:1   1:4   2:8
  UINT32      src_data_width   : 2;            //数据传输宽度，0:一次传输8bit，1:一次传输16bit，2:一次传输32bit，3:保留
  UINT32      reserved0        : 5;
  UINT32      dst_drq_type     : 5;            //目的地址存储类型，如DRAM, SPI,NAND等
  UINT32      dst_addr_mode    : 2;            //目的地址类型，如递增，或者不变  0:递增模式  1:保持不变
  UINT32      dst_burst_length : 2;            //发起一次burst宽度 填0对应于1，填1对应于4,
  UINT32      dst_data_width   : 2;            //数据传输宽度，0:一次传输8bit，1:一次传输16bit，2:一次传输32bit，3:保留
  UINT32      reserved1        : 5;
}
sunxi_dma_channal_config;

//for user request
typedef struct
{
  sunxi_dma_channal_config  cfg;
  UINT32  loop_mode;
  UINT32  data_block_size;
  UINT32  wait_cyc;
}
sunxi_dma_setting_t;

typedef VOID (*sunxi_dma_int_hander_t)(VOID *);

extern    EFI_STATUS EFIAPI sunxi_dma_init(VOID);
extern    EFI_STATUS EFIAPI sunxi_dma_exit(VOID);

extern    UINT32    EFIAPI  sunxi_dma_request (UINT32 dmatype);
extern    EFI_STATUS    EFIAPI  sunxi_dma_release     (UINT32 hdma);
extern    EFI_STATUS    EFIAPI  sunxi_dma_setting     (UINT32 hdma, sunxi_dma_setting_t *cfg);
extern    EFI_STATUS    EFIAPI  sunxi_dma_start         (UINT32 hdma, UINT32 saddr, UINT32 daddr, UINT32 bytes);
extern    EFI_STATUS    EFIAPI  sunxi_dma_stop          (UINT32 hdma);
extern    INT32     EFIAPI  sunxi_dma_querystatus   (UINT32 hdma);

extern    EFI_STATUS    EFIAPI  sunxi_dma_install_int(UINT32 hdma, sunxi_dma_int_hander_t dma_INT32_func, VOID *p);
extern    EFI_STATUS    EFIAPI  sunxi_dma_disable_int(UINT32 hdma);

extern    EFI_STATUS    EFIAPI  sunxi_dma_enable_int(UINT32 hdma);
extern    EFI_STATUS    EFIAPI  sunxi_dma_free_int(UINT32 hdma);

#endif  //_DMA_H_

/* end of _DMA_H_ */

