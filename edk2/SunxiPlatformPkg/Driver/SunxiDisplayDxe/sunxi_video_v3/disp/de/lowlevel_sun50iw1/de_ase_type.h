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


#ifndef __DE_ASE_TYPE_H__
#define __DE_ASE_TYPE_H__

#include "de_rtmx.h"

#define ASE_PARA_NUM  1
#define ASE_MODE_NUM  4
typedef union
{
  unsigned int dwval;
  struct
  {
    unsigned int en          :  1 ;    // Default: 0x0;
    unsigned int win_en            :  1 ;    // Default: 0x0;
    unsigned int res0              :  30;    // Default: ;
  } bits;
} ASE_CTRL_REG;

typedef union
{
  unsigned int dwval;
  struct
  {
    unsigned int width        :12;// Default: 0x0;
    unsigned int res0       :4; // Default: ;
    unsigned int height       :12;// Default: 0x0;
    unsigned int res1       :4; // Default: ;
  } bits;
} ASE_SIZE_REG;

typedef union
{
  unsigned int dwval;
  struct
  {
    unsigned int left       :12;// Default: 0x0;
    unsigned int res0       :4; // Default: ;
    unsigned int top        :12;// Default: 0x0;
    unsigned int res1       :4; // Default: ;
  } bits;
} ASE_WIN0_REG;

typedef union
{
  unsigned int dwval;
  struct
  {
    unsigned int right        :12;// Default: 0x0;
    unsigned int res0       :4; // Default: ;
    unsigned int bot        :12;// Default: 0x0;
    unsigned int res1       :4; // Default: ;
  } bits;
} ASE_WIN1_REG;


typedef union
{
  unsigned int dwval;
  struct
  {
    unsigned int gain       :5;   // Default: 0x0;
    unsigned int res0       :27;  // Default: ;
  } bits;
} ASE_GAIN_REG;

typedef struct
{
  ASE_CTRL_REG    ctrl;     //0x00
  ASE_SIZE_REG    size;     //0x04
  ASE_WIN0_REG    win0;     //0x08
  ASE_WIN1_REG    win1;     //0x0c
  ASE_GAIN_REG    gain;     //0x10

}__ase_reg_t;

typedef struct
{
  //ase
  unsigned int ase_en;
  unsigned int gain;

  //window
  unsigned int win_en;
  de_rect win;
}__ase_config_data;
#endif
