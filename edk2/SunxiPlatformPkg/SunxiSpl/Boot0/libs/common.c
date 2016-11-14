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


#include <Interinc/private_boot0.h>
#include <boot0_include/boot0_helper.h>

extern const boot0_file_head_t  *boot0_head;
extern int debug_mode;
/*
************************************************************************************************************
*
*                                             function
*
*    name          :set_debugmode_flag
*
*    parmeters     :void
*
*    return        :
*
*    note          :if BT0_head.prvt_head.debug_mode_off = 1,do not print any message to uart
*
*
************************************************************************************************************
*/
void SetDebugModeFlag(void)
{
  char c = 0;
  int i = 0;
  for( i = 0 ; i < 3 ; i++)
  {
    __msdelay(10);
    if(sunxi_serial_tstc())
    {
      printf("key_press  \n");
      c = sunxi_serial_getc();
      printf("0x%x \n",c);
      break;
    }
  }
  if(c  == 's')
  {
    debug_mode = 1;
    return ;
  }
  if(boot0_head->prvt_head.debug_mode)
    debug_mode = 1;
  else
    debug_mode = 0;
  return ;

}
