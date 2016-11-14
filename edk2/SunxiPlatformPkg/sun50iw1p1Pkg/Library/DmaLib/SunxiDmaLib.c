/** @file
*
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


#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UncachedMemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Sun8iW1/cpu.h>
#include <Sun8iW1/intc.h>
#include <Library/SunxiDmaLib.h>

#include <Protocol/HardwareInterrupt.h>


#define SUNXI_DMA_MAX     16


#define SUNXI_DMA_CHANNAL_BASE    (SUNXI_DMA_BASE + 0x100)
#define SUNXI_DMA_CHANANL_SIZE    (0x40)

#define SUNXI_DMA_LINK_NULL       (0xfffff800)


struct dma_irq_handler
{
  VOID                *m_data;
  VOID (*m_func)( VOID * data);
};


typedef struct
{
  UINT32 irq_en0;
  UINT32 irq_en1;
  UINT32 reserved0[2];
  UINT32 irq_pending0;
  UINT32 irq_pending1;
  UINT32 reserved1[2];
  UINT32 reserved2[4];
  UINT32 status;
}
sunxi_dma_int_set;

typedef struct sunxi_dma_channal_set_t
{
  volatile UINT32 enable;
  volatile UINT32 pause;
  volatile UINT32 start_addr;   //起始地址
  volatile UINT32 config;
  volatile UINT32 cur_src_addr;   //当前传输地址
  volatile UINT32 cur_dst_addr;
  volatile UINT32 left_bytes;   //剩余未传字节数
  volatile UINT32 parameters;   //参数
}
sunxi_dma_channal_set;



typedef struct sunxi_dma_source_t
{
  UINT32          used;
  UINT32            channal_count;
  sunxi_dma_channal_set *channal;
  UINT32      reserved;
  sunxi_dma_start_t       *config;
  struct dma_irq_handler  dma_func;
}
sunxi_dma_source;

#define  DMA_PKG_HALF_INT   (1<<0)
#define  DMA_PKG_END_INT    (1<<1)
#define  DMA_QUEUE_END_INT  (1<<2)

static INT32    dma_int_count = 0;
static sunxi_dma_source   dma_channal_source[SUNXI_DMA_MAX];

static EFI_HARDWARE_INTERRUPT_PROTOCOL *gInterrupt = NULL;

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
VOID
EFIAPI
sunxi_dma_int_func (
  IN  HARDWARE_INTERRUPT_SOURCE   Source,
  IN  EFI_SYSTEM_CONTEXT          SystemContext       
  )
{
  INT32 i;
  UINT32 pending;
  sunxi_dma_int_set *dma_int = (sunxi_dma_int_set *)SUNXI_DMA_BASE;

  for(i=0; i<8; i++)
  {
    if(dma_channal_source[i].dma_func.m_func)
    {
      pending = (DMA_PKG_END_INT << (i * 4));
      if(dma_int->irq_pending0 & pending)
      {
        dma_int->irq_pending0 = pending;
        dma_channal_source[i].dma_func.m_func(dma_channal_source[i].dma_func.m_data);
      }
    }
  }
  for(i=8;i<15;i++)
  {
    if(dma_channal_source[i].dma_func.m_func)
    {
      pending = (DMA_PKG_END_INT << (i * 4));
      if(dma_int->irq_pending1 & pending)
      {
        dma_int->irq_pending1 = pending;
        dma_channal_source[i].dma_func.m_func(dma_channal_source[i].dma_func.m_data);
      }
    }
  }

  gInterrupt->EndOfInterrupt (gInterrupt, Source);
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
EFI_STATUS
EFIAPI
sunxi_dma_init(VOID)
{
  INT32 i;
  EFI_STATUS              Status;

  sunxi_dma_int_set *dma_int = (sunxi_dma_int_set *)SUNXI_DMA_BASE;

  dma_int->irq_en0 = 0;
  dma_int->irq_en1 = 0;

  dma_int->irq_pending0 = 0xffffffff;
  dma_int->irq_pending1 = 0xffffffff;

  ZeroMem((VOID *)dma_channal_source,SUNXI_DMA_MAX * sizeof(struct sunxi_dma_source_t));

  for(i=0;i<SUNXI_DMA_MAX;i++)
  {
    dma_channal_source[i].used = 0;
    dma_channal_source[i].channal = (struct sunxi_dma_channal_set_t *)(SUNXI_DMA_BASE + i * SUNXI_DMA_CHANANL_SIZE + 0x100);
    DEBUG((DEBUG_INFO,"++%a :allocate %d\n", __func__,i));
    dma_channal_source[i].config  = (sunxi_dma_start_t *)UncachedAllocatePool(sizeof(sunxi_dma_start_t));
    DEBUG((DEBUG_INFO,"++%a :allocate %d done=%x\n", __func__,i,(void*)dma_channal_source[i].config));
    ASSERT(dma_channal_source[i].config);
  }

  dma_int_count = 0;
  
  Status = gInterrupt->RegisterInterruptSource (gInterrupt, AW_IRQ_DMA, sunxi_dma_int_func);
  gInterrupt->DisableInterruptSource(gInterrupt,AW_IRQ_DMA);
  ASSERT_EFI_ERROR (Status);
  
  return EFI_SUCCESS;
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
EFI_STATUS
EFIAPI 
sunxi_dma_exit(VOID)
{
  INT32 i;

  sunxi_dma_int_set *dma_int = (sunxi_dma_int_set *)SUNXI_DMA_BASE;

  dma_int->irq_en0 = 0;
  dma_int->irq_en1 = 0;

  dma_int->irq_pending0 = 0xffffffff;
  dma_int->irq_pending1 = 0xffffffff;

  gInterrupt->DisableInterruptSource(gInterrupt,AW_IRQ_DMA);

  for(i=0;i<SUNXI_DMA_MAX;i++)
  {
    UncachedFreePool(dma_channal_source[i].config);
  }

  return EFI_SUCCESS;
}
/*
****************************************************************************************************
*
*             DMAC_RequestDma
*
*  Description:
*       request dma
*
*  Parameters:
*   type  0: normal dma
*       1: dedicate dma
*  Return value:
*   dma handler
*   if 0, fail
****************************************************************************************************
*/
UINT32 
EFIAPI
sunxi_dma_request(UINT32 dmatype)
{
  INT32   i;

  for(i=0;i<SUNXI_DMA_MAX;i++)
  {
    if(dma_channal_source[i].used == 0)
    {
      dma_channal_source[i].used = 1;
      dma_channal_source[i].channal_count = i;

      return (UINT32)&dma_channal_source[i];
    }
  }

  return 0;
}
/*
****************************************************************************************************
*
*             DMAC_ReleaseDma
*
*  Description:
*       release dma
*
*  Parameters:
*       hDma  dma handler
*
*  Return value:
*   EPDK_OK/FAIL
****************************************************************************************************
*/
EFI_STATUS
EFIAPI 
sunxi_dma_release(UINT32 hdma)
{
  struct sunxi_dma_source_t  *dma_channal = (struct sunxi_dma_source_t *)hdma;

  if(!dma_channal->used)
  {
    return -1;
  }

  sunxi_dma_disable_int(hdma);
  sunxi_dma_free_int(hdma);

  dma_channal->channal->enable = 0;
  dma_channal->used   = 0;

  return EFI_SUCCESS;
}
/*
****************************************************************************************************
*
*             sunxi_dma_setting
*
*  Description:
*       start interrupt
*
*  Parameters:
*
*
*
*
*
*
*  Return value:
*
*
**********************************************************************************************************************
*/
EFI_STATUS
EFIAPI 
sunxi_dma_setting(UINT32 hdma, sunxi_dma_setting_t *cfg)
{
  UINT32   *config_addr;
  UINT32   commit_para;
  sunxi_dma_setting_t       *dma_set     = cfg;
  struct sunxi_dma_source_t   *dma_channal = (struct sunxi_dma_source_t *)hdma;

  if(!dma_channal->used)
  {
    return EFI_DEVICE_ERROR;
  }

  config_addr = (UINT32 *)&(dma_set->cfg);
  if(dma_set->loop_mode)
  {
    dma_channal->config->link = (UINT32 )(dma_channal->config);
  }
  else
  {
    dma_channal->config->link = SUNXI_DMA_LINK_NULL;
  }

  commit_para  = (dma_set->wait_cyc & 0xff);
  commit_para |= (dma_set->data_block_size & 0xff ) << 8;
  dma_channal->config->commit_para = commit_para;
  dma_channal->config->config = *config_addr;

  return EFI_SUCCESS;
}

/*
**********************************************************************************************************************
*
*             sunxi_dma_start
*
*  Description:
*
*
*  Parameters:
*
*
*  Return value:
*
*
****************************************************************************************************
*/
EFI_STATUS
EFIAPI 
sunxi_dma_start(UINT32 hdma, UINT32 saddr, UINT32 daddr, UINT32 bytes)
{
  struct sunxi_dma_source_t   *dma_channal = (struct sunxi_dma_source_t *)hdma;

  if(!dma_channal->used)
  {
    return EFI_DEVICE_ERROR;
  }

  dma_channal->config->source_addr = saddr;
  dma_channal->config->dest_addr   = daddr;
  dma_channal->config->byte_count  = bytes;

  dma_channal->channal->start_addr = (UINT32)(dma_channal->config);

  //flush_cache((UINT32)&(dma_channal->config), sizeof(sunxi_dma_start_t));

  dma_channal->channal->enable = 1;

  return EFI_SUCCESS;
}
/*
**********************************************************************************************************************
*
*             sunxi_dma_stop
*
*  Description:
*
*
*  Parameters:
*
*
*  Return value:
*
*
**********************************************************************************************************************
*/
EFI_STATUS
EFIAPI 
sunxi_dma_stop(UINT32 hdma)
{
  struct sunxi_dma_source_t   *dma_channal = (struct sunxi_dma_source_t *)hdma;

  if(!dma_channal->used)
  {
    return -1;
  }

  dma_channal->channal->enable = 0;

  return 0;
}
/*
**********************************************************************************************************************
*
*             sunxi_dma_querystatus
*
*  Description:
*
*
*  Parameters:
*
*
*  Return value:
*
*
**********************************************************************************************************************
*/
INT32 
EFIAPI
sunxi_dma_querystatus(UINT32 hdma)
{
  UINT32  channal_count;
  struct sunxi_dma_source_t   *dma_channal = (struct sunxi_dma_source_t *)hdma;
  sunxi_dma_int_set       *dma_int   = (sunxi_dma_int_set *)SUNXI_DMA_BASE;

  if(!dma_channal->used)
  {
    return -1;
  }
  channal_count = dma_channal->channal_count;

  return (dma_int->status >> channal_count) & 0x01;
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
EFI_STATUS
EFIAPI 
sunxi_dma_install_int(UINT32 hdma, sunxi_dma_int_hander_t dma_int_func, VOID *p)
{
  sunxi_dma_source     *dma_channal = (sunxi_dma_source  *)hdma;
  sunxi_dma_int_set    *dma_status  = (sunxi_dma_int_set *)SUNXI_DMA_BASE;
  UINT32  channal_count;

  if(!dma_channal->used)
  {
    return EFI_DEVICE_ERROR;
  }
  channal_count = dma_channal->channal_count;

  if(channal_count < 8)
  {
    dma_status->irq_pending0 = (7 << channal_count*4);
  }
  else
  {
    dma_status->irq_pending1 = (7 << (channal_count - 8)*4);
  }

  if(!dma_channal->dma_func.m_func)
  {
    dma_channal->dma_func.m_func = dma_int_func;
    dma_channal->dma_func.m_data = p;
  }
  else
  {
    DEBUG((DEBUG_ERROR,"dma 0x%x int is used already, you have to free it first\n", hdma));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
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
EFI_STATUS
EFIAPI 
sunxi_dma_enable_int(UINT32 hdma)
{
  sunxi_dma_source     *dma_channal = (sunxi_dma_source *)hdma;
  sunxi_dma_int_set    *dma_status  = (sunxi_dma_int_set *)SUNXI_DMA_BASE;
  UINT32  channal_count;

  if(!dma_channal->used)
  {
    return -1;
  }

  channal_count = dma_channal->channal_count;
  if(channal_count < 8)
  {
    if(dma_status->irq_en0 & (DMA_PKG_END_INT << channal_count*4))
    {
      DEBUG((DEBUG_ERROR,"dma 0x%x int is avaible already\n", hdma));
      return EFI_DEVICE_ERROR;
    }
    dma_status->irq_en0 |= (DMA_PKG_END_INT << channal_count*4);
  }
  else
  {
    if(dma_status->irq_en1 & (DMA_PKG_END_INT << (channal_count - 8)*4))
    {
      DEBUG((DEBUG_ERROR,"dma 0x%x int is avaible already\n", hdma));
      return EFI_DEVICE_ERROR;
    }
    dma_status->irq_en1 |= (DMA_PKG_END_INT << (channal_count - 8)*4);
  }

  if(!dma_int_count)
  {
    gInterrupt->EnableInterruptSource(gInterrupt,AW_IRQ_DMA);
  }
  
  dma_int_count ++;

  return EFI_SUCCESS;
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
EFI_STATUS
EFIAPI 
sunxi_dma_disable_int(UINT32 hdma)
{
  sunxi_dma_source     *dma_channal = (sunxi_dma_source *)hdma;
  sunxi_dma_int_set    *dma_status  = (sunxi_dma_int_set *)SUNXI_DMA_BASE;
  UINT32  channal_count;

  if(!dma_channal->used)
  {
    return -1;
  }

  channal_count = dma_channal->channal_count;
  if(channal_count < 8)
  {
    if(!(dma_status->irq_en0 & (DMA_PKG_END_INT << channal_count*4)))
    {
      DEBUG((DEBUG_ERROR,"dma 0x%x int is not used yet\n", hdma));
      return EFI_DEVICE_ERROR;
    }
    dma_status->irq_en0 &= ~(DMA_PKG_END_INT << channal_count*4);
  }
  else
  {
    if(!(dma_status->irq_en1 & (DMA_PKG_END_INT << (channal_count - 8)*4)))
    {
      DEBUG((DEBUG_ERROR,"dma 0x%x int is not used yet\n", hdma));
      return EFI_DEVICE_ERROR;
    }
    dma_status->irq_en1 &= ~(DMA_PKG_END_INT << (channal_count - 8)*4);
  }

  //disable golbal int
  if(dma_int_count > 0)
  {
    dma_int_count --;
  }
  if(!dma_int_count)
  {
    gInterrupt->DisableInterruptSource(gInterrupt,AW_IRQ_DMA);
  }

  return EFI_SUCCESS;
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
EFI_STATUS
EFIAPI 
sunxi_dma_free_int(UINT32 hdma)
{
  sunxi_dma_source     *dma_channal = (sunxi_dma_source *)hdma;
  sunxi_dma_int_set    *dma_status  = (sunxi_dma_int_set *)SUNXI_DMA_BASE;
  UINT32  channal_count;

  if(!dma_channal->used)
  {
    return EFI_DEVICE_ERROR;
  }
  channal_count = dma_channal->channal_count;
  if(channal_count < 8)
  {
    dma_status->irq_pending0 = (7 << channal_count);
  }
  else
  {
    dma_status->irq_pending1 = (7 << (channal_count - 8));
  }

  if(dma_channal->dma_func.m_func)
  {
    dma_channal->dma_func.m_func = NULL;
    dma_channal->dma_func.m_data = NULL;
  }
  else
  {
    DEBUG((DEBUG_ERROR,"dma 0x%x int is free, you do not need to free it again\n", hdma));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SunxiDmaLibConstructor (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS              Status;
  DEBUG((DEBUG_INFO,"++%a\n", __func__));
  // Get the Cpu protocol for later use
  Status = gBS->LocateProtocol (&gHardwareInterruptProtocolGuid, NULL, (VOID **)&gInterrupt);
  ASSERT_EFI_ERROR(Status);
 
  Status = sunxi_dma_init();
  
  return Status;
}

