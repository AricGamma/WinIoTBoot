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


#ifndef __DE_VEP_H__
#define __DE_VEP_H__

int de_fcc_init(unsigned int sel, unsigned int reg_base);
int de_fcc_update_regs(unsigned int sel);
int de_fcc_set_reg_base(unsigned int sel, unsigned int chno, void *base);
int de_fcc_csc_set(unsigned int sel, unsigned int chno, unsigned int en, unsigned int mode);

#endif
