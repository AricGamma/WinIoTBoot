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

#include <Interinc/private_uefi.h>
#include <Interinc/private_toc.h>

#include <boot0_include/dram.h>
#include <boot0_include/config.h>
#include <boot0_include/boot0_helper.h>

static void PrintVersion(void);
static int Boot0ClearEnv(void);
static void UpdateUbootInfo(UINT32 dram_size);
static int Boot0CheckUartInput(void);

extern char boot0_hash_value[64];
extern void SetDebugModeFlag(void);

int mmc_config_addr;
boot0_file_head_t* boot0_head;


/*******************************************************
we should implement below interfaces if platform support
handler standby flag in boot0
*******************************************************/
void __attribute__((weak)) handler_super_standby(void)
{
  return;
}

/*******************************************************************************
main:   body for c runtime 
*******************************************************************************/
void Boot0Main(void)
{
  UINT32 status;
  signed int dram_size;
  UINT32 fel_flag;
  UINT32 boot_cpu=0;
  int use_monitor = 0;

  boot0_head =get_boot0_head(PcdGet32 (PcdFdBaseAddress));
  mmc_config_addr = (int)(boot0_head->prvt_head.storage_data);

  timer_init();
  sunxi_serial_init(boot0_head->prvt_head.uart_port, (void *)boot0_head->prvt_head.uart_ctrl, 6 );
  SetDebugModeFlag();
  printf("key_press  \n");
  printf("HELLO! BOOT0 is starting!\n");
  //printf("boot0 commit : %s \n",boot0_hash_value);
  PrintVersion();

  SetPll();
  Boot0CheckUartInput();

  if(boot0_head->prvt_head.enable_jtag )
  {
    boot_set_gpio((normal_gpio_cfg *)boot0_head->prvt_head.jtag_gpio, 6, 1);
  }

  fel_flag = rtc_region_probe_fel_flag();
  if(fel_flag == SUNXI_RUN_EFEX_FLAG)
  {
    rtc_region_clear_fel_flag();
    printf("eraly jump fel\n");
    goto __boot0_entry_err0;
  }

#ifdef FPGA_PLATFORM
  dram_size = mctl_init((void *)boot0_head->prvt_head.dram_para);
#else
  dram_size = init_DRAM(0, (void *)boot0_head->prvt_head.dram_para);
#endif
  if(dram_size)
  {
    printf("dram size =%d\n", dram_size);
  }
  else
  {
    printf("initializing SDRAM Fail.\n");
    goto  __boot0_entry_err0;
  }
  //on some platform, boot0 should handler standby flag.
  handler_super_standby();

  MmuSetup(dram_size);
  status = LoadBoot1();
  if(status == 0 )
  {
    use_monitor = 0;
    status = LoadFip(&use_monitor);
  }

  printf("Ready to disable icache.\n");
    // disable instruction cache
  MmuTurnOff( ); 

  if( status == 0 )
  {
    //update bootpackage size for uboot
    UpdateUbootInfo(dram_size);
    //update flash para
    UpdateFlashPara();
    //update dram para before jmp to boot1
    SetDramPara((void *)&boot0_head->prvt_head.dram_para, dram_size, boot_cpu);
    printf("Jump to secend Boot.\n");
        if(use_monitor)
    {
      Boot0JmpMonitor();
    }
    else
    {
      Boot0JmpBoot1(CONFIG_SYS_TEXT_BASE);
    }
  }

__boot0_entry_err0:
  Boot0ClearEnv();
  Boot0JmpOther(FEL_BASE);
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
static void PrintVersion()
{
  //brom modify: nand-4bytes, sdmmc-2bytes
  printf("boot0 version : %s\n", boot0_head->boot_head.platform + 4);

  return;
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
static int Boot0ClearEnv(void)
{

  ResetPll();
  MmuTurnOff();

  __msdelay(10);
    
  return 0;
}

//
static void UpdateUbootInfo(UINT32 dram_size)
{
  //struct spare_boot_head_t  *bfh = (struct spare_boot_head_t *) CONFIG_SYS_TEXT_BASE;
  //struct sbrom_toc1_head_info *toc1_head = (struct sbrom_toc1_head_info *)CONFIG_BOOTPKG_STORE_IN_DRAM_BASE;
  //bfh->boot_data.boot_package_size = toc1_head->valid_len;
  //bfh->boot_data.dram_scan_size = dram_size;
  //printf("boot package size: 0x%x\n",bfh->boot_data.boot_package_size);
}

static int Boot0CheckUartInput(void)
{
  int c = 0;
  int i = 0;
  for(i = 0;i < 3;i++)
  {
    __msdelay(10);
    if(sunxi_serial_tstc())
    {
      c = sunxi_serial_getc();
      break;
    }
  }

  if(c == '2')
  {
    printf("enter 0x%x,ready jump to fes\n", c-0x30);  // ASCII to decimal digit
    Boot0ClearEnv();
    Boot0JmpOther(FEL_BASE);
  }
  return 0;
}

void SetDramPara(void *dram_addr , UINT32 dram_size, UINT32 boot_cpu)
{
  struct spare_boot_head_t  *uboot_buf = (struct spare_boot_head_t *)CONFIG_SYS_TEXT_BASE;

  memcpy((void *)uboot_buf->boot_data.dram_para, dram_addr, 32 * sizeof(int));
#ifdef CONFIG_BOOT_A15
  uboot_buf->boot_data.reserved[0] = boot_cpu;
#endif
  return;
}

