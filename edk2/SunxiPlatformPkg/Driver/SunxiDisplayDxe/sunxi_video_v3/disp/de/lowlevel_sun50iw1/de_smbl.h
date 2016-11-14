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

#ifndef __DE_SMBL_H__
#define __DE_SMBL_H__

#include "de_rtmx.h"
#include "de_smbl_type.h"

int de_smbl_tasklet(unsigned int sel);
int de_smbl_apply(unsigned int sel, struct disp_smbl_info *info);
int de_smbl_update_regs(unsigned int sel);
int de_smbl_set_reg_base(unsigned int sel, void *base);
int de_smbl_enable(unsigned int sel, unsigned int en);
int de_smbl_set_window(unsigned int sel, unsigned int win_enable, struct disp_rect window);
int de_smbl_set_para(unsigned int sel, unsigned int width, unsigned int height);
int de_smbl_set_lut(unsigned int sel, unsigned short *lut);
int de_smbl_get_hist(unsigned int sel, unsigned int *cnt);
int de_smbl_get_status(unsigned int sel);
int de_smbl_init(unsigned int sel, uintptr_t reg_base);

#if defined(SUPPORT_SMBL)
extern u16 pwrsv_lgc_tab[1408][256];
extern u8 smbl_filter_coeff[272];
extern u8 hist_thres_drc[8];
extern u8 hist_thres_pwrsv[8];
extern u8 drc_filter[IEP_LH_PWRSV_NUM];
extern u32 csc_bypass_coeff[12];
#endif

#endif
