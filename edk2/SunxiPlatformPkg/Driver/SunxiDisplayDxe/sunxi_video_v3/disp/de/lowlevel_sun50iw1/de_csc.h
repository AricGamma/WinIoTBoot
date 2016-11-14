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

#ifndef __DE_CSC_H__
#define __DE_CSC_H__

#include "de_rtmx.h"

int de_dcsc_apply(unsigned int sel, struct disp_csc_config *config);
int de_dcsc_init(disp_bsp_init_para *para);
int de_dcsc_update_regs(unsigned int sel);
int de_dcsc_get_config(unsigned int sel, struct disp_csc_config *config);

int de_ccsc_apply(unsigned int sel, unsigned int ch_id, struct disp_csc_config *config);
int de_ccsc_update_regs(unsigned int sel);
int de_ccsc_init(disp_bsp_init_para *para);
int de_csc_coeff_calc(unsigned int infmt, unsigned int incscmod, unsigned int outfmt, unsigned int outcscmod,
                unsigned int brightness, unsigned int contrast, unsigned int saturation, unsigned int hue,
                unsigned int out_color_range, int *csc_coeff);

#endif

