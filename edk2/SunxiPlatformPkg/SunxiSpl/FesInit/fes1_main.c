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
#include <boot0_include/boot0_helper.h>
#include <boot0_include/dram.h>
#include <boot0_include/config.h>

#include <Library/TimerLib.h>
#include <Library/SerialPortLib.h>
#include <Library/SysConfigLib.h>
#include <Library/SunxiCommonLib.h>
#include <Library/PcdLib.h>

boot0_file_head_t  *fes1_head;

typedef struct __fes_aide_info{
  unsigned int dram_init_flag;       /* Dram初始化完成标志       */
  unsigned int dram_update_flag;     /* Dram 参数是否被修改标志  */
  unsigned int dram_paras[SUNXI_DRAM_PARA_MAX];
}fes_aide_info_t;


//note: this function for linker error
int raise (int signum)
{
  return 0;
}

/* Dummy function to avoid linker complaints */
void __aeabi_unwind_cpp_pr0(void)
{

};


/*
************************************************************************************
*                          note_dram_log
*
* Description:
*     ???????
* Parameters:
*   void
* Return value:
*     0: success
*      !0: fail
* History:
*       void
************************************************************************************
*/
static void  note_dram_log(int dram_init_flag)
{
  fes_aide_info_t *fes_aide = (fes_aide_info_t *)CONFIG_FES1_RET_ADDR;

  memset(fes_aide, 0, sizeof(fes_aide_info_t));
  fes_aide->dram_init_flag    = SYS_PARA_LOG;
  fes_aide->dram_update_flag  = dram_init_flag;

  memcpy(fes_aide->dram_paras, fes1_head->prvt_head.dram_para, SUNXI_DRAM_PARA_MAX * 4);
  memcpy((void *)DRAM_PARA_STORE_ADDR, fes1_head->prvt_head.dram_para, SUNXI_DRAM_PARA_MAX * 4);
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
int FesMain(void)
{
  signed int dram_size=0;
  fes1_head =get_boot0_head(PcdGet32 (PcdFdBaseAddress));

  timer_init();
  
  sunxi_serial_init(fes1_head->prvt_head.uart_port, (void *)fes1_head->prvt_head.uart_ctrl, 2);
  
  SetPll();

  //enable gpio gate
  //set_gpio_gate();
  //dram init
  printf("beign to init dram\n");

#ifdef FPGA_PLATFORM
  dram_size = mctl_init((void *)fes1_head->prvt_head.dram_para);
#else
  dram_size = init_DRAM(0, (void *)fes1_head->prvt_head.dram_para);
#endif
  
  if (dram_size)
  {
    note_dram_log(1);
    printf("init dram ok\n");
  }
  else
  {
    note_dram_log(0);
    printf("init dram fail\n");
  }
  int i=0;
  for(i=0;i<8;i++)
  {
    printf("para%d=%x\n",i,fes1_head->prvt_head.dram_para[i]);
  }

  __msdelay(10);

  return dram_size;
}
 
