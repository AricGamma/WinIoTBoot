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

#include "disp_manager.h"
#include <Library/BaseLib.h> 
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h> 

struct disp_manager_info {
  disp_color_info back_color;
  disp_colorkey ck;
};

struct disp_manager_private_data {
  bool user_info_dirty;
  struct disp_manager_info user_info;

  bool info_dirty;
  struct disp_manager_info info;

  bool shadow_info_dirty;

  /* If true, a display is enabled using this manager */
  bool enabled;

  bool extra_info_dirty;//??
  bool shadow_extra_info_dirty;//??

  disp_output_type output;
  u32 width, height;

  u32 de_flicker_status;
  bool b_outinterlerlace;
  disp_out_csc_type output_csc_type;

  s32 (*shadow_protect)(u32 sel, bool protect);

  u32 reg_base;
  u32 irq_no;
  disp_clk_info_t  clk;
};

#if defined(__LINUX_PLAT__)
static spinlock_t mgr_data_lock;
#endif

static struct disp_manager *mgrs = NULL;
static struct disp_manager_private_data *mgr_private;

struct disp_manager* disp_get_layer_manager(u32 screen_id)
{
  u32 num_screens;

  num_screens = bsp_disp_feat_get_num_screens();
  if(screen_id >= num_screens) {
    DE_WRN("screen_id %d out of range\n", screen_id);
    return NULL;
  }

  if(!disp_al_query_be_mod(screen_id)) {
    DE_WRN("manager %d is not registered\n", screen_id);
    return NULL;
  }

  return &mgrs[screen_id];
}
struct disp_manager_private_data *disp_mgr_get_priv(struct disp_manager *mgr)
{
  if(NULL == mgr) {
    DE_WRN("NULL hdl!\n");
    return NULL;
  }

  if(!disp_al_query_be_mod(mgr->channel_id)) {
    DE_WRN("manager %d is not registered\n", mgr->channel_id);
    return NULL;
  }

  return &mgr_private[mgr->channel_id];
}

s32 disp_mgr_clk_init(struct disp_manager *mgr)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }

  disp_al_manager_clk_init(mgrp->clk.clk);

  return 0;
}

s32 disp_mgr_clk_exit(struct disp_manager *mgr)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }

  disp_al_manager_clk_exit(mgrp->clk.clk);

  mgrp->clk.enabled = 0;

  return 0;
}

s32 disp_mgr_clk_enable(struct disp_manager *mgr)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }

  disp_al_manager_clk_enable(mgrp->clk.clk);

  mgrp->clk.enabled = 1;

  return 0;
}

s32 disp_mgr_clk_disable(struct disp_manager *mgr)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }

  disp_al_manager_clk_disable(mgrp->clk.clk);
  mgrp->clk.enabled = 0;
  return 0;
}

s32 disp_mgr_init(struct disp_manager *mgr)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }

  disp_mgr_clk_init(mgr);
  disp_al_manager_init(mgr->channel_id);

  return 0;
}

s32 disp_mgr_exit(struct disp_manager *mgr)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }

  if(!disp_al_query_be_mod(mgr->channel_id)) {
    DE_WRN("manager %d is not register\n", mgr->channel_id);
    return -1;
  }

  disp_al_manager_enable(mgr->channel_id, 0);
  disp_mgr_clk_exit(mgr);
#if defined(__LINUX_PLAT__)
//  OSAL_InterruptDisable(mgrp->irq_no);
#endif
//  OSAL_UnRegISR(mgrp->irq_no, manager_event_proc,(void*)mgr->channel_id);

  return 0;
}

s32 disp_mgr_set_back_color(struct disp_manager *mgr, disp_color_info *back_color)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }
#if defined(__LINUX_PLAT__)
  {
    unsigned long flags;
    spin_lock_irqsave(&mgr_data_lock, flags);
#endif
    CopyMem(&mgrp->user_info.back_color, back_color, sizeof(disp_color_info));
    mgrp->user_info_dirty = true;
#if defined(__LINUX_PLAT__)
    spin_unlock_irqrestore(&mgr_data_lock, flags);
  }
#endif

  return DIS_SUCCESS;
}

s32 disp_mgr_get_back_color(struct disp_manager *mgr, disp_color_info *back_color)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }
#if defined(__LINUX_PLAT__)
  {
    unsigned long flags;
    spin_lock_irqsave(&mgr_data_lock, flags);
#endif
    CopyMem(back_color, &mgrp->user_info.back_color, sizeof(disp_color_info));
#if defined(__LINUX_PLAT__)
    spin_unlock_irqrestore(&mgr_data_lock, flags);
  }
#endif

  return DIS_SUCCESS;
}

s32 disp_mgr_set_color_key(struct disp_manager *mgr, disp_colorkey *ck)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }
#if defined(__LINUX_PLAT__)
  {
    unsigned long flags;
    spin_lock_irqsave(&mgr_data_lock, flags);
#endif
    CopyMem(&mgrp->user_info.ck, ck, sizeof(disp_colorkey));
    mgrp->user_info_dirty = true;
#if defined(__LINUX_PLAT__)
    spin_unlock_irqrestore(&mgr_data_lock, flags);
  }
#endif

  return DIS_SUCCESS;
}

s32 disp_mgr_get_color_key(struct disp_manager *mgr, disp_colorkey *ck)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }
#if defined(__LINUX_PLAT__)
  {
    unsigned long flags;
    spin_lock_irqsave(&mgr_data_lock, flags);
#endif
    CopyMem(ck, &mgrp->user_info.ck, sizeof(disp_colorkey));
#if defined(__LINUX_PLAT__)
    spin_unlock_irqrestore(&mgr_data_lock, flags);
  }
#endif

  return DIS_SUCCESS;
}

s32 disp_mgr_set_output_type(struct disp_manager *mgr, disp_output_type output_type)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }
#if defined(__LINUX_PLAT__)
  {
    unsigned long flags;
    spin_lock_irqsave(&mgr_data_lock, flags);
#endif
    mgrp->output = output_type;
#if defined(__LINUX_PLAT__)
    spin_unlock_irqrestore(&mgr_data_lock, flags);
  }
#endif

  return DIS_SUCCESS;
}

s32 disp_mgr_get_output_type(struct disp_manager *mgr, disp_output_type *output_type)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }

  *output_type = DISP_OUTPUT_TYPE_NONE;
#if defined(__LINUX_PLAT__)
  {
    unsigned long flags;
    spin_lock_irqsave(&mgr_data_lock, flags);
#endif
    if(mgrp->enabled)
      *output_type = mgrp->output;
#if defined(__LINUX_PLAT__)
    spin_unlock_irqrestore(&mgr_data_lock, flags);
  }
#endif

  return DIS_SUCCESS;
}

s32 disp_mgr_sync(struct disp_manager *mgr)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);
  struct disp_layer *lyr;
#if defined(__LINUX_PLAT__)
  unsigned long flags;
#endif

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }

#if defined(__LINUX_PLAT__)
    spin_lock_irqsave(&mgr_data_lock, flags);
#endif
    if(!mgrp->enabled) {
#if defined(__LINUX_PLAT__)
    spin_unlock_irqrestore(&mgr_data_lock, flags);
#endif
    return -1;
    }
#if defined(__LINUX_PLAT__)
    spin_unlock_irqrestore(&mgr_data_lock, flags);
#endif
    disp_al_manager_sync(mgr->channel_id);
    list_for_each_entry(lyr, &mgr->lyr_list, list) {
      if(lyr->sync)
        lyr->sync(lyr);
    }
#if defined(__LINUX_PLAT__)
    spin_lock_irqsave(&mgr_data_lock, flags);
#endif
    mgrp->shadow_info_dirty = false;
#if defined(__LINUX_PLAT__)
    spin_unlock_irqrestore(&mgr_data_lock, flags);
#endif

  return DIS_SUCCESS;
}

s32 disp_mgr_update_regs(struct disp_manager *mgr)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);
#if defined(__LINUX_PLAT__)
  unsigned long flags;
#endif

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }
  __inf("disp_mgr_update_regs, mgr%d\n", mgr->channel_id);

#if defined(__LINUX_PLAT__)
    spin_lock_irqsave(&mgr_data_lock, flags);
#endif
    if(!mgrp->enabled || !mgrp->info_dirty) {
#if defined(__LINUX_PLAT__)
      spin_unlock_irqrestore(&mgr_data_lock, flags);
#endif
      return -1;
    }
#if defined(__LINUX_PLAT__)
  spin_unlock_irqrestore(&mgr_data_lock, flags);
#endif
  disp_mgr_shadow_protect(mgr, 1);
  disp_al_manager_set_backcolor(mgr->channel_id, &mgrp->info.back_color);
  disp_al_manager_set_color_key(mgr->channel_id, &mgrp->info.ck);
  disp_mgr_shadow_protect(mgr, 0);
#if defined(__LINUX_PLAT__)
  spin_lock_irqsave(&mgr_data_lock, flags);
#endif
  mgrp->info_dirty = false;
  mgrp->shadow_info_dirty = true;
#if defined(__LINUX_PLAT__)
  spin_unlock_irqrestore(&mgr_data_lock, flags);
#endif

  return DIS_SUCCESS;
}

s32 disp_mgr_apply(struct disp_manager *mgr)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }
  DE_INF("mgr %d apply\n", mgr->channel_id);

#if defined(__LINUX_PLAT__)
  {
    unsigned long flags;
    spin_lock_irqsave(&mgr_data_lock, flags);
#endif
    if(mgrp->user_info_dirty) {
      CopyMem(&mgrp->info, &mgrp->user_info, sizeof(struct disp_manager_info));
      mgrp->user_info_dirty = false;
      mgrp->info_dirty = true;
    }
#if defined(__LINUX_PLAT__)
    spin_unlock_irqrestore(&mgr_data_lock, flags);
  }
#endif
  disp_mgr_update_regs(mgr);

  return DIS_SUCCESS;
}

s32 disp_mgr_force_update_regs(struct disp_manager *mgr)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);
  struct disp_layer *lyr;

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }
  __inf("disp_mgr_force_update_regs, mgr%d\n", mgr->channel_id);

#if defined(__LINUX_PLAT__)
  {
    unsigned long flags;
    spin_lock_irqsave(&mgr_data_lock, flags);
#endif
    mgrp->user_info_dirty = true;
#if defined(__LINUX_PLAT__)
    spin_unlock_irqrestore(&mgr_data_lock, flags);
  }
#endif

  disp_mgr_apply(mgr);
  list_for_each_entry(lyr, &mgr->lyr_list, list) {
    DE_INF("force_update_regs, mgr%d lyr%d\n", lyr->channel_id, lyr->layer_id);
    if(lyr->force_update_regs)
      lyr->force_update_regs(lyr);
  }
  return 0;
}

s32 disp_mgr_clear_regs(struct disp_manager *mgr)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);
  struct disp_layer *lyr;

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }
  __inf("disp_mgr_clear_regs, mgr%d\n", mgr->channel_id);

#if defined(__LINUX_PLAT__)
  {
    unsigned long flags;
    spin_lock_irqsave(&mgr_data_lock, flags);
#endif
    ZeroMem(&mgrp->info, sizeof(struct disp_manager_info));
    mgrp->info_dirty = true;
#if defined(__LINUX_PLAT__)
    spin_unlock_irqrestore(&mgr_data_lock, flags);
  }
#endif

  disp_mgr_update_regs(mgr);
  list_for_each_entry(lyr, &mgr->lyr_list, list) {
    DE_INF("clear_regs, mgr%d lyr%d\n", lyr->channel_id, lyr->layer_id);
    if(lyr->clear_regs)
      lyr->clear_regs(lyr);
  }
  return 0;
}

s32 disp_mgr_set_screen_size(struct disp_manager *mgr, u32 width, u32 height)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }

#if defined(__LINUX_PLAT__)
  {
    unsigned long flags;
    spin_lock_irqsave(&mgr_data_lock, flags);
#endif
    mgrp->width = width;
    mgrp->height = height;
#if defined(__LINUX_PLAT__)
    spin_unlock_irqrestore(&mgr_data_lock, flags);
  }
#endif

  return DIS_SUCCESS;
}

s32 disp_mgr_get_screen_size(struct disp_manager *mgr, u32 *width, u32 *height)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }

#if defined(__LINUX_PLAT__)
  {
    unsigned long flags;
    spin_lock_irqsave(&mgr_data_lock, flags);
#endif
  disp_al_manager_get_display_size(mgr->channel_id, width, height);
  if((*width == 1) || (*height == 1) || (*width == 65536) || (*height == 65536)) {
    *width = mgrp->width;
    *height = mgrp->height;
  }
#if defined(__LINUX_PLAT__)
    spin_unlock_irqrestore(&mgr_data_lock, flags);
  }
#endif

  return DIS_SUCCESS;
}

s32 disp_mgr_add_layer(struct disp_manager *mgr, struct disp_layer* lyr)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);

  if((NULL == mgr) || (NULL == mgrp) || (NULL == lyr)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }
  __inf("disp_mgr_add_layer, mgr%d <--- mgr%d lyr%d\n", mgr->channel_id, lyr->channel_id, lyr->layer_id);

#if defined(__LINUX_PLAT__)
  {
    unsigned long flags;
    spin_lock_irqsave(&mgr_data_lock, flags);
#endif
    list_add_tail(&(lyr->list), &(mgr->lyr_list));
#if defined(__LINUX_PLAT__)
    spin_unlock_irqrestore(&mgr_data_lock, flags);
  }
#endif

  return DIS_SUCCESS;
}

s32 disp_mgr_enable(struct disp_manager *mgr)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }
  DE_INF("mgr %d enable\n", mgr->channel_id);

  disp_mgr_clk_enable(mgr);
  disp_al_manager_enable(mgr->channel_id, 1);
#if defined(__LINUX_PLAT__)
  {
    unsigned long flags;
    spin_lock_irqsave(&mgr_data_lock, flags);
#endif
    mgrp->enabled = 1;
#if defined(__LINUX_PLAT__)
    spin_unlock_irqrestore(&mgr_data_lock, flags);
  }
#endif
  disp_mgr_force_update_regs(mgr);

  return 0;
}

s32 disp_mgr_disable(struct disp_manager *mgr)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }

  DE_INF("mgr %d disable\n", mgr->channel_id);

  disp_mgr_clear_regs(mgr);
  disp_al_manager_enable(mgr->channel_id, 0);
  disp_mgr_clk_disable(mgr);
#if defined(__LINUX_PLAT__)
  {
    unsigned long flags;
    spin_lock_irqsave(&mgr_data_lock, flags);
#endif
    mgrp->enabled = 0;
#if defined(__LINUX_PLAT__)
    spin_unlock_irqrestore(&mgr_data_lock, flags);
  }
#endif
  return 0;
}

s32 disp_mgr_is_enabled(struct disp_manager *mgr)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }

  return mgrp->enabled;

}

s32 disp_mgr_shadow_protect(struct disp_manager *mgr, bool protect)
{
  struct disp_manager_private_data *mgrp = disp_mgr_get_priv(mgr);

  if((NULL == mgr) || (NULL == mgrp)) {
    DE_WRN("NULL hdl!\n");
    return -1;
  }

  if(mgrp->shadow_protect)
    return mgrp->shadow_protect(mgr->channel_id, protect);

  return -1;
}

s32 disp_init_mgr(__disp_bsp_init_para * para)
{
  u32 num_screens;
  u32 screen_id;
  struct disp_manager *mgr;
  struct disp_manager_private_data *mgrp;

  DE_INF("disp_init_mgr\n");

#if defined(__LINUX_PLAT__)
  spin_lock_init(&mgr_data_lock);
#endif
  num_screens = bsp_disp_feat_get_num_screens();
  mgrs = (struct disp_manager *)OSAL_malloc(sizeof(struct disp_manager) * num_screens);
  if(NULL == mgrs) {
    DE_WRN("malloc memory fail!\n");
    return DIS_FAIL;
  }
  mgr_private = (struct disp_manager_private_data *)OSAL_malloc(sizeof(struct disp_manager_private_data) * num_screens);
  if(NULL == mgr_private) {
    DE_WRN("malloc memory fail! size=0x%x x %d\n", sizeof(struct disp_manager_private_data), num_screens);
    return DIS_FAIL;
  }

  for(screen_id=0; screen_id<num_screens; screen_id++) {
    if(!disp_al_query_be_mod(screen_id))
      continue;

    mgr = &mgrs[screen_id];
    mgrp = &mgr_private[screen_id];

    switch(screen_id) {
    case 0:
      mgr->name = "mgr0";
      mgr->channel_id = 0;
      mgr->output_type = DISP_OUTPUT_TYPE_NONE;
      mgrp->irq_no = para->irq_no[DISP_MOD_BE0];
      mgrp->reg_base = para->reg_base[DISP_MOD_BE0];
      mgrp->clk.clk = MOD_CLK_DEBE0;
      mgrp->shadow_protect = para->shadow_protect;
      break;

    case 1:
      mgr->name = "mgr1";
      mgr->channel_id = 1;
      mgr->output_type = DISP_OUTPUT_TYPE_NONE;
      mgrp->irq_no = para->irq_no[DISP_MOD_BE1];
      mgrp->reg_base = para->reg_base[DISP_MOD_BE1];
      mgrp->clk.clk = MOD_CLK_DEBE1;
      mgrp->shadow_protect = para->shadow_protect;
      break;

    case 2:
      mgr->name = "mgr2";
      mgr->channel_id = 2;
      mgr->output_type = DISP_OUTPUT_TYPE_NONE;
      mgrp->irq_no = para->irq_no[DISP_MOD_BE2];
      mgrp->reg_base = para->reg_base[DISP_MOD_BE2];
      mgrp->clk.clk = MOD_CLK_DEBE2;
      mgrp->shadow_protect = para->shadow_protect;

      break;
    }

    mgr->enable = disp_mgr_enable;
    mgr->disable = disp_mgr_disable;
    mgr->is_enabled = disp_mgr_is_enabled;
    mgr->set_screen_size = disp_mgr_set_screen_size;
    mgr->get_screen_size = disp_mgr_get_screen_size;
    mgr->set_color_key = disp_mgr_set_color_key;
    mgr->get_color_key = disp_mgr_get_color_key;
    mgr->set_back_color = disp_mgr_set_back_color;
    mgr->get_back_color = disp_mgr_get_back_color;
    mgr->set_output_type = disp_mgr_set_output_type;
    mgr->get_output_type = disp_mgr_get_output_type;
    mgr->add_layer = disp_mgr_add_layer;

    mgr->init = disp_mgr_init;
    mgr->exit = disp_mgr_exit;

    mgr->apply = disp_mgr_apply;
    mgr->update_regs = disp_mgr_update_regs;
    mgr->force_update_regs = disp_mgr_force_update_regs;
    mgr->sync = disp_mgr_sync;

    INIT_LIST_HEAD(&mgr->lyr_list);

    mgr->init(mgr);
  }

  return 0;
}
