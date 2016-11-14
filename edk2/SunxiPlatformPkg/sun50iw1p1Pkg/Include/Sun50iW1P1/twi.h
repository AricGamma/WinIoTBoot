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

#ifndef _TWIC_REGS_H_
#define _TWIC_REGS_H_

#include <linux/types.h>
/*
*********************************************************************************************************
*   Interrupt controller define
*********************************************************************************************************
*/
#define TWI_CONTROL_OFFSET             0x400
#define SUNXI_I2C_CONTROLLER             3

struct sunxi_twi_reg
{
  volatile unsigned int addr;        /* slave address     */
  volatile unsigned int xaddr;       /* extend address    */
  volatile unsigned int data;        /* data              */
  volatile unsigned int ctl;         /* control           */
  volatile unsigned int status;      /* status            */
  volatile unsigned int clk;         /* clock             */
  volatile unsigned int srst;        /* soft reset        */
  volatile unsigned int eft;         /* enhanced future   */
  volatile unsigned int lcr;         /* line control      */
  volatile unsigned int dvfs;        /* dvfs control      */
};


#endif  /* _TWIC_REGS_H_ */

