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

#ifndef __DE_VEP_TAB__
#define __DE_VEP_TAB__

extern int y2r[192];
extern int r2y[128];
extern int y2y[64];
extern int r2r[32];
extern int bypass_csc[12];
extern unsigned int sin_cos[128];
extern int fcc_range_gain[6];
extern unsigned char ce_bypass_lut[256];
extern unsigned char ce_constant_lut[256];

#endif
