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


#ifndef  __dram_head_h__
#define  __dram_head_h__

typedef struct _boot_dram_para_t
{
  //normal configuration
  unsigned int        dram_clk;
  unsigned int        dram_type;    //dram_type     DDR2: 2       DDR3: 3       LPDDR2: 6 DDR3L: 31
  unsigned int        dram_zq;
  unsigned int    dram_odt_en;

  //control configuration
  unsigned int    dram_para1;
  unsigned int    dram_para2;

  //timing configuration
  unsigned int    dram_mr0;
  unsigned int    dram_mr1;
  unsigned int    dram_mr2;
  unsigned int    dram_mr3;
  unsigned int    dram_tpr0;
  unsigned int    dram_tpr1;
  unsigned int    dram_tpr2;
  unsigned int    dram_tpr3;
  unsigned int    dram_tpr4;
  unsigned int    dram_tpr5;
  unsigned int    dram_tpr6;

  //reserved for future use
  unsigned int    dram_tpr7;
  unsigned int    dram_tpr8;
  unsigned int    dram_tpr9;
  unsigned int    dram_tpr10;
  unsigned int    dram_tpr11;
  unsigned int    dram_tpr12;
  unsigned int    dram_tpr13;

}__dram_para_t;

extern int init_DRAM( int type, __dram_para_t *buff );
#endif


