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

#ifndef __DE_TOP_CLK_H__
#define __DE_TOP_CLK_H__

#include "ebios_de.h"

#define DE_TOP_CLK(_mod_id, _div_reg, _div_shift, _div_width, \
            _enable_reg, _enable_shift, _reset_reg,  _reset_shift, \
            _bus_gate_reg, _bus_gate_shift, _drm_gate_reg, _dram_gate_shift) \
{                                                       \
  .mod_id = _mod_id,                                    \
  .divider = {                                          \
    .reg_off = _div_reg,                                \
    .shift   = _div_shift,                              \
    .width   = _div_width,                              \
  },                                                    \
  .gate = {                                             \
    .enable_off = _enable_reg,                          \
    .reset_off  = _reset_reg,                           \
    .bus_off    = _bus_gate_reg,                        \
    .dram_off   = _drm_gate_reg,                        \
    .enable_shift  = _enable_shift,                     \
    .reset_shift  = _reset_shift,                       \
    .bus_shift  = _bus_gate_shift,                      \
    .dram_shift  = _dram_gate_shift,                    \
  },                                                    \
},

struct de_top_clk_gate
{
  u32             flags;
  u32             enable_off;//reg offset
  u32             reset_off;
  u32             bus_off;
  u32             dram_off;
  u32             enable_shift;
  u32             reset_shift;
  u32             bus_shift;
  u32             dram_shift;
};

struct de_top_clk_div
{
  u32             reg_off;//reg offset
  u32             shift;
  u32             width;
};

struct de_top_clk
{
  u32                        mod_id;
  struct de_top_clk_div      divider;
  struct de_top_clk_gate     gate;
};

s32 de_top_clk_enable(u32 mod_id, u32 enable);
s32 de_top_clk_div(u32 mod_id, u32 div);
s32 de_top_set_reg_base(u32 reserve, u32 reg_base);

#endif
