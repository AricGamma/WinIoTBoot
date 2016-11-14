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

#include "panels.h"
#include "default_panel.h"
#include "tft720x1280.h"
#include "lp907qx.h"
#include "starry768x1024.h"
#include "B116XAN03.h"

extern __lcd_panel_t sl698ph_720p_panel;
extern __lcd_panel_t lp079x01_panel;

__lcd_panel_t* panel_array[] = {
  &default_panel,
  &tft720x1280_panel,
  &lp907qx_panel,
  &starry768x1024_panel,
  &sl698ph_720p_panel,
  &lp079x01_panel,
  &B116XAN03_panel,
  /* add new panel below */

  NULL,
};

