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

#ifndef _DISP_FEATURES_H_
#define _DISP_FEATURES_H_

#include "bsp_display.h"
#include "disp_private.h"

struct disp_features {
  const u32 num_screens;
  const u32 *num_layers;
  const u32 num_scalers;
  const u32 num_captures;
  const u32 num_smart_backlights;
  const u32 num_color_managers;
  const disp_output_type *supported_output_types;
  const enum __disp_layer_feat *layer_feats;
  const enum __disp_layer_feat *scaler_layer_feats;
  const u32 *smart_backlight_support;
  const u32 *image_enhance_support;
  const u32 *smart_color_support;
};

s32 bsp_disp_feat_get_num_screens(void);
s32 bsp_disp_feat_get_num_layers(u32 screen_id);
s32 bsp_disp_feat_get_num_scalers(void);
s32 bsp_disp_feat_get_num_color_managers(void);
s32 bsp_disp_feat_get_num_smart_backlights(void);
disp_output_type bsp_disp_feat_get_supported_output_types(u32 screen_id);
//enum sunxi_disp_output_id sunxi_disp_feat_get_supported_outputs(u32 screen_id);
enum __disp_layer_feat bsp_disp_feat_get_layer_feats(u32 screen_id,
    disp_layer_mode mode, u32 scaler_index);
s32 bsp_disp_feat_get_smart_backlight_support(u32 screen_id);
s32 bsp_disp_feat_get_image_enhance_support(u32 screen_id);

s32 disp_init_feat(void);
#endif

