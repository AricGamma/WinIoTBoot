/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  Jerry Wang <wangflord@allwinnertech.com>
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


#ifndef _SUNXI_CLOCK_H
#define _SUNXI_CLOCK_H


extern int sunxi_clock_get_corepll(void);
extern int sunxi_clock_set_corepll(int frequency);
extern int sunxi_clock_get_pll6(void);
extern int sunxi_clock_get_ahb(void);
extern int sunxi_clock_get_apb(void);
extern int sunxi_clock_get_axi(void);
extern int sunxi_clock_get_ahb2(void);
extern int sunxi_clock_get_mbus(void);

extern void set_pll( void );
extern void set_gpio_gate(void);
extern void reset_pll(void);
extern void set_pll_in_secure_mode( void );

#endif /* _SUNXI_CLOCK_H */
