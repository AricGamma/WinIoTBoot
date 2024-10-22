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

#include "../CardBurn.h"

uint sprite_cartoon_create(void)
{
  Print(L"%a call\n", __FUNCTION__);
  return 0;
}

int sprite_cartoon_upgrade(int rate)
{
  Print(L"%a:%d%%\n", __FUNCTION__,rate); 
  return 0;
}

int sprite_cartoon_destroy(void)
{
  Print(L"%a call\n", __FUNCTION__);
  return 0;
}