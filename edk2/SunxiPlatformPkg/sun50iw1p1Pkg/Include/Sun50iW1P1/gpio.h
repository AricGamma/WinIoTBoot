
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


#ifndef _SUNXI_GPIO_H
#define _SUNXI_GPIO_H

#include <boot0_include/boot0_helper.h>
#include "platform.h"

#define GPIO_REG_READ(reg)              readl((reg))
#define GPIO_REG_WRITE(reg, value)      writel((value), (reg))


#define PIOC_REG_o_CFG0                 0x00
#define PIOC_REG_o_CFG1                 0x04
#define PIOC_REG_o_CFG2                 0x08
#define PIOC_REG_o_CFG3                 0x0C
#define PIOC_REG_o_DATA                 0x10
#define PIOC_REG_o_DRV0                 0x14
#define PIOC_REG_o_DRV1                 0x18
#define PIOC_REG_o_PUL0                 0x1C
#define PIOC_REG_o_PUL1                 0x20



/**#############################################################################################################
 *
 *                           GPIO(PIN) Operations
 *
-##############################################################################################################*/
#define PIO_REG_CFG(n, i)               ((volatile unsigned int *)( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x00))
#define PIO_REG_DLEVEL(n, i)            ((volatile unsigned int *)( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x14))
#define PIO_REG_PULL(n, i)              ((volatile unsigned int *)( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x1C))
#define PIO_REG_DATA(n)                   ((volatile unsigned int *)( SUNXI_PIO_BASE + ((n)-1)*0x24 + 0x10))

#define PIO_REG_CFG_VALUE(n, i)          readl( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x00)
#define PIO_REG_DLEVEL_VALUE(n, i)       readl( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x14)
#define PIO_REG_PULL_VALUE(n, i)         readl( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x1C)
#define PIO_REG_DATA_VALUE(n)            readl( SUNXI_PIO_BASE + ((n)-1)*0x24 + 0x10)
#define PIO_REG_BASE(n)                    ((volatile unsigned int *)(SUNXI_PIO_BASE +((n)-1)*24))

#ifdef SUNXI_R_PIO_BASE
#define R_PIO_REG_CFG(n, i)               ((volatile unsigned int *)( SUNXI_R_PIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x00))
#define R_PIO_REG_DLEVEL(n, i)            ((volatile unsigned int *)( SUNXI_R_PIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x14))
#define R_PIO_REG_PULL(n, i)              ((volatile unsigned int *)( SUNXI_R_PIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x1C))
#define R_PIO_REG_DATA(n)                 ((volatile unsigned int *)( SUNXI_R_PIO_BASE + ((n)-12)*0x24 + 0x10))

#define R_PIO_REG_CFG_VALUE(n, i)          readl( SUNXI_R_PIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x00)
#define R_PIO_REG_DLEVEL_VALUE(n, i)       readl( SUNXI_R_PIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x14)
#define R_PIO_REG_PULL_VALUE(n, i)         readl( SUNXI_R_PIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x1C)
#define R_PIO_REG_DATA_VALUE(n)            readl( SUNXI_R_PIO_BASE + ((n)-12)*0x24 + 0x10)
#define R_PIO_REG_BASE(n)                    ((volatile unsigned int *)(SUNXI_R_PIO_BASE +((n)-12)*24))
#endif


//struct for gpio
typedef struct
{
  unsigned char      port;                       //端口号
  unsigned char      port_num;                   //端口内编号
  char               mul_sel;                    //功能编号
  char               pull;                       //电阻状态
  char               drv_level;                  //驱动驱动能力
  char               data;                       //输出电平
  unsigned char      reserved[2];                //保留位，保证对齐
}
normal_gpio_set_t;

int boot_set_gpio(void  *user_gpio_list, unsigned int group_count_max, int set_gpio);

#endif /* _SUNXI_GPIO_H */
