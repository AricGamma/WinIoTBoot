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

#include "de_smbl.h"

#if defined(SUPPORT_SMBL)
u16 pwrsv_lgc_tab[1408][256];




//u8 spatial_coeff[9]={228,241,228,241,255,241,228,241,228};

u8 smbl_filter_coeff[272];

u8 hist_thres_drc[8];
u8 hist_thres_pwrsv[8];
u8 drc_filter[IEP_LH_PWRSV_NUM];
u32 csc_bypass_coeff[12];
#endif

