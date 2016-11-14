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


#include "../CardBurn.h"
#include "sprite_card.h"
#include "sprite_erase.h"
#include <Interinc/sunxi_uefi.h>


void sunxi_update_subsequent_processing(int next_work)
{
  printf("next work %d\n", next_work);
  switch(next_work)
  {
    case SUNXI_UPDATE_NEXT_ACTION_REBOOT: //重启
      printf("SUNXI_UPDATE_NEXT_ACTION_REBOOT\n");
      gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);

      break;
    case SUNXI_UPDATE_NEXT_ACTION_SHUTDOWN: //关机
      printf("SUNXI_UPDATE_NEXT_ACTION_SHUTDOWN\n");
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);

      break;
    case SUNXI_UPDATE_NEXT_ACTION_REUPDATE:
      printf("SUNXI_UPDATE_NEXT_ACTION_REUPDATE\n");
      //sunxi_board_run_fel();      //进行量产

      break;
    case SUNXI_UPDATE_NEXT_ACTION_BOOT:
    case SUNXI_UPDATE_NEXT_ACTION_NORMAL:
    default:
      printf("SUNXI_UPDATE_NEXT_ACTION_NULL\n");
      break;
  }

  return ;
}


/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static void __dump_dlmap(sunxi_download_info *dl_info)
{
  dl_one_part_info    *part_info;
  u32 i;
  char buffer[32];

  printf("*************DOWNLOAD MAP DUMP************\n");
  printf("total download part %d\n", dl_info->download_count);
  printf("\n");
  for(part_info = dl_info->one_part_info, i=0;i<dl_info->download_count;i++, part_info++)
  {
    memset(buffer, 0, 32);
    memcpy(buffer, part_info->name, 16);
    printf("download part[%d] name          :%s\n", i, buffer);
    memset(buffer, 0, 32);
    memcpy(buffer, part_info->dl_filename, 16);
    printf("download part[%d] download file :%s\n", i, buffer);
    memset(buffer, 0, 32);
    memcpy(buffer, part_info->vf_filename, 16);
    printf("download part[%d] verify file   :%s\n", i, buffer);
    printf("download part[%d] lenlo         :0x%x\n", i, part_info->lenlo);
    printf("download part[%d] addrlo        :0x%x\n", i, part_info->addrlo);
    printf("download part[%d] encrypt       :0x%x\n", i, part_info->encrypt);
    printf("download part[%d] verify        :0x%x\n", i, part_info->verify);
    printf("\n");
  }
}


static void __dump_mbr(sunxi_mbr_t *mbr_info)
{
  sunxi_partition     *part_info;
  u32 i;
  char buffer[32];

  printf("*************MBR DUMP***************\n");
  printf("total mbr part %d\n", mbr_info->PartCount);
  printf("\n");
  for(part_info = mbr_info->array, i=0;i<mbr_info->PartCount;i++, part_info++)
  {
    memset(buffer, 0, 32);
    memcpy(buffer, part_info->name, 16);
    printf("part[%d] name      :%s\n", i, buffer);
    memset(buffer, 0, 32);
    memcpy(buffer, part_info->classname, 16);
    printf("part[%d] classname :%s\n", i, buffer);
    printf("part[%d] addrlo    :0x%x\n", i, part_info->addrlo);
    printf("part[%d] lenlo     :0x%x\n", i, part_info->lenlo);
    printf("part[%d] user_type :0x%x\n", i, part_info->user_type);
    printf("part[%d] keydata   :0x%x\n", i, part_info->keydata);
    printf("part[%d] ro        :0x%x\n", i, part_info->ro);
    printf("\n");
  }
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :  workmode: 升级模式选择：0，卡上固件形式升级；1，文件形式升级
*
*           name    : 文件升级时的名词
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int sunxi_card_sprite_main(int workmode, char *name)
{
  
  //uchar  img_mbr[1024 * 1024];      //mbr
  uchar  *img_mbr = NULL;     //mbr
  sunxi_download_info  dl_map;      //dlinfo
  int    sprite_next_work;

  tick_printf("sunxi sprite begin\n");
  img_mbr = malloc(1024*1024);
  if(img_mbr == NULL)
  {
    printf("sunxi sprite malloc for mbr fail\n");
    return -1;
  }
    
  //启动动画显示
  sprite_cartoon_create();
  //检查固件合法性
  if(sprite_card_firmware_probe(name))
  {
    printf("sunxi sprite firmware probe fail\n");
    goto __ERR_END;
  }
  sprite_cartoon_upgrade(5);
  tick_printf("firmware probe ok\n");
  //获取dl_map文件，用于指引下载的数据
  tick_printf("fetch download map\n");
  if(sprite_card_fetch_download_map(&dl_map))
  {
    printf("sunxi sprite error : fetch download map error\n");
    goto __ERR_END;
  }
  __dump_dlmap(&dl_map);
    //获取mbr
  tick_printf("fetch mbr\n");
  if(sprite_card_fetch_mbr(img_mbr))
  {
    printf("sunxi sprite error : fetch mbr error\n");
    goto __ERR_END;
  }
  __dump_mbr((sunxi_mbr_t *)img_mbr);
  //根据mbr，决定擦除时候是否要保留数据
  tick_printf("begin to erase flash\n");
  //nand_get_mbr((char *)img_mbr, 16 * 1024);-------wangwei
  if(sunxi_sprite_erase_flash(img_mbr))
  {
    printf("sunxi sprite error: erase flash err\n");
    goto __ERR_END;
  }
    
  tick_printf("successed in erasing flash\n"); 
  if(sunxi_sprite_init(0))
  {
    printf("sunxi sprite error: init flash\n");
    sunxi_sprite_exit(1);
    goto __ERR_END;
  }
  //down mbr
  if(sunxi_sprite_download_mbr(img_mbr, sizeof(sunxi_mbr_t) * SUNXI_MBR_COPY_NUM))
  {
    printf("sunxi sprite error: download mbr err\n");
    sunxi_sprite_exit(1);
    goto __ERR_END;
  }

  sprite_cartoon_upgrade(10);
  tick_printf("begin to download part\n");
  //开始烧写分区
  if(sunxi_sprite_deal_part(&dl_map))
  {
    printf("sunxi sprite error : download part error\n");
    sunxi_sprite_exit(1);
    goto __ERR_END;
  }
  tick_printf("successed in downloading part\n");
  sprite_cartoon_upgrade(80);
  //sunxi_sprite_exit(1);

  if(sunxi_sprite_deal_uboot())
  {
    printf("sunxi sprite error : download uboot error\n");
    sunxi_sprite_exit(1);
    goto __ERR_END;
  }
  tick_printf("successed in downloading uboot\n");

  sprite_cartoon_upgrade(90);
  if(sunxi_sprite_deal_boot0())
  {
    printf("sunxi sprite error : download boot0 error\n");
    sunxi_sprite_exit(1);
    goto __ERR_END;
  }
  sunxi_sprite_exit(1);
  tick_printf("successed in downloading boot0\n");
  sprite_cartoon_upgrade(100);
  sprite_uichar_printf("CARD OK\n");
  //烧写结束
  //__msdelay(3000);
  SpriteDelayMS(3000);
  //处理烧写完成后的动作
  if(script_parser_fetch("card_boot", "next_work", &sprite_next_work, 1))
  {
    sprite_next_work = SUNXI_UPDATE_NEXT_ACTION_SHUTDOWN;
    //sprite_next_work = SUNXI_UPDATE_NEXT_ACTION_REUPDATE;
  }
  sunxi_update_subsequent_processing(sprite_next_work);

  free(img_mbr);
  return 0;
    
__ERR_END:
  free(img_mbr);
  return -1;
}


