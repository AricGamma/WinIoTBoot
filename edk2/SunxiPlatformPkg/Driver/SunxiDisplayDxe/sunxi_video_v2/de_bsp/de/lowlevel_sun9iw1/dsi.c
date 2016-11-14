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

#include <Sunxi_type/Sunxi_type.h>
#include "ebios_lcdc_tve.h"

typedef void (*dsi_print)(const char *c);
typedef void (*dsi_print_val)(u32 val);

//------------------------------------------------------------


int dsi_set_print_func(dsi_print print)
{
  return 0;
}
int dsi_set_print_val_func(dsi_print_val print_val)
{
  return 0;
}

s32 dsi_init(struct dsi_init_para* para)
{
  return 0;
}

s32 dsi_set_reg_base(u32 sel, u32 base)
{
    return 0;
}
u32 dsi_get_reg_base(u32 sel)
{
    return 0;
}
u32 dsi_irq_query(u32 sel,__dsi_irq_id_t id)
{
    return 0;
}
s32 dsi_cfg(u32 sel,disp_panel_para * panel)
{
    return 0;
}
s32 dsi_exit(u32 sel)
{
    return 0;
}
s32 dsi_open(u32 sel,disp_panel_para * panel)
{
    return 0;
}
s32 dsi_close(u32 sel)
{
    return 0;
}
s32 dsi_inst_busy(u32 sel)
{
    return 0;
}
s32 dsi_tri_start(u32 sel)
{ 
  return 0;
}
s32 dsi_dcs_wr(u32 sel,__u32 cmd,u8* para_p,u32 para_num)
{
    return 0;
}
s32 dsi_dcs_wr_index(u32 sel,u8 index)
{
    return 0;
}
s32 dsi_dcs_wr_data(u32 sel,u8 data)
{
    return 0;
}
u32 dsi_get_start_delay(u32 sel)
{
    return 0;
}
u32 dsi_get_cur_line(u32 sel)
{
    return 0;
}
u32 dsi_io_open(u32 sel,disp_panel_para * panel)
{
    return 0;
}
u32 dsi_io_close(u32 sel)
{
    return 0;
}
s32 dsi_clk_enable(u32 sel, u32 en)
{
    return 0;
}

__s32 dsi_dcs_wr_0para(__u32 sel,__u8 cmd)
{
    return 0;
}
__s32 dsi_dcs_wr_1para(__u32 sel,__u8 cmd,__u8 para)
{
    return 0;
}
__s32 dsi_dcs_wr_2para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2)
{
    return 0;
}
__s32 dsi_dcs_wr_3para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2,__u8 para3)
{
    return 0;
}
__s32 dsi_dcs_wr_4para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2,__u8 para3,__u8 para4)
{
    return 0;
}
__s32 dsi_dcs_wr_5para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2,__u8 para3,__u8 para4,__u8 para5)
{
    return 0;
}

__s32 dsi_gen_wr_0para(__u32 sel,__u8 cmd)
{
    return 0;
}
__s32 dsi_gen_wr_1para(__u32 sel,__u8 cmd,__u8 para)
{
    return 0;
}
__s32 dsi_gen_wr_2para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2)
{
    return 0;
}
__s32 dsi_gen_wr_3para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2,__u8 para3)
{
    return 0;
}
__s32 dsi_gen_wr_4para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2,__u8 para3,__u8 para4)
{
    return 0;
}
__s32 dsi_gen_wr_5para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2,__u8 para3,__u8 para4,__u8 para5)
{
    return 0;
}
__s32 dsi_dcs_rd(__u32 sel,__u8 cmd,__u8* para_p,__u32* num_p)
{
  return 0;
}