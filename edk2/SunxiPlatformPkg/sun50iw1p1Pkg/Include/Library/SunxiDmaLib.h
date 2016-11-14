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
//======    DMA ����     =========
//================================

/* DMA ��������  */
#define DMAC_CFG_CONTINUOUS_ENABLE              (0x01)
#define DMAC_CFG_CONTINUOUS_DISABLE             (0x00)

/* DMA ����Ŀ�Ķ� ���� */
/* DMA Ŀ�Ķ� ������ */
#define DMAC_CFG_DEST_DATA_WIDTH_8BIT     (0x00)
#define DMAC_CFG_DEST_DATA_WIDTH_16BIT      (0x01)
#define DMAC_CFG_DEST_DATA_WIDTH_32BIT      (0x02)

/* DMA Ŀ�Ķ� ͻ������ģʽ */
#define DMAC_CFG_DEST_1_BURST             (0x00)
#define DMAC_CFG_DEST_4_BURST           (0x01)
#define DMAC_CFG_DEST_8_BURST         (0x02)

/* DMA Ŀ�Ķ� ��ַ�仯ģʽ */
#define DMAC_CFG_DEST_ADDR_TYPE_LINEAR_MODE   (0x00)
#define DMAC_CFG_DEST_ADDR_TYPE_IO_MODE     (0x01)


/* DMA ����Դ�� ���� */
/* DMA Դ�� ������ */
#define DMAC_CFG_SRC_DATA_WIDTH_8BIT      (0x00)
#define DMAC_CFG_SRC_DATA_WIDTH_16BIT     (0x01)
#define DMAC_CFG_SRC_DATA_WIDTH_32BIT     (0x02)

/* DMA Դ�� ͻ������ģʽ */
#define DMAC_CFG_SRC_1_BURST              (0x00)
#define DMAC_CFG_SRC_4_BURST            (0x01)
#define DMAC_CFG_SRC_8_BURST            (0x02)

/* DMA Դ�� ��ַ�仯ģʽ */
#define DMAC_CFG_SRC_ADDR_TYPE_LINEAR_MODE    (0x00)
#define DMAC_CFG_SRC_ADDR_TYPE_IO_MODE      (0x01)


/* DMA ����Ŀ�Ķ� ���� */
#define DMAC_CFG_DEST_TYPE_SRAM         (0x00)
#define DMAC_CFG_DEST_TYPE_DRAM           (0x01)

#define DMAC_CFG_DEST_TYPE_CODEC          (15)

#define DMAC_CFG_DEST_TYPE_OTG_EP1          (17)
#define DMAC_CFG_DEST_TYPE_OTG_EP2          (18)
#define DMAC_CFG_DEST_TYPE_OTG_EP3          (19)
#define DMAC_CFG_DEST_TYPE_OTG_EP4          (20)
#define DMAC_CFG_DEST_TYPE_OTG_EP5          (21)
/* DMA ����Դ�� ���� */
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
  UINT32      src_drq_type     : 5;            //Դ��ַ�洢���ͣ���DRAM, SPI,NAND�ȣ��μ�  __ndma_drq_type_t
  UINT32      src_addr_mode    : 2;            //ԭ��ַ���� 0:����ģʽ  1:���ֲ���
  UINT32      src_burst_length : 2;            //����һ��burst��� 0:1   1:4   2:8
  UINT32      src_data_width   : 2;            //���ݴ����ȣ�0:һ�δ���8bit��1:һ�δ���16bit��2:һ�δ���32bit��3:����
  UINT32      reserved0        : 5;
  UINT32      dst_drq_type     : 5;            //Ŀ�ĵ�ַ�洢���ͣ���DRAM, SPI,NAND��
  UINT32      dst_addr_mode    : 2;            //Ŀ�ĵ�ַ���ͣ�����������߲���  0:����ģʽ  1:���ֲ���
  UINT32      dst_burst_length : 2;            //����һ��burst��� ��0��Ӧ��1����1��Ӧ��4,
  UINT32      dst_data_width   : 2;            //���ݴ����ȣ�0:һ�δ���8bit��1:һ�δ���16bit��2:һ�δ���32bit��3:����
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

