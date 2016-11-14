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

/*
*************************************************************************************
*                               eBsp
*            Operation System Adapter Layer
*
*       (c) Copyright 2006-2010, All winners Co,Ld.
*             All Rights Reserved
*
* File Name   : OSAL_Pin.h
*
* Author    : javen
*
* Description   : C�⺯��
*
* History     :
*      <author>       <time>        <version >        <desc>
*       javen          2010-09-07          1.0         create this word
*       holi         2010-12-02          1.1         ��Ӿ���Ľӿڣ�
*************************************************************************************
*/
#include "OSAL_Pin.h"
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/AxpPower.h>

__hdle OSAL_GPIO_Request(disp_gpio_set_t *gpio_list, __u32 group_count_max)
{
    EFI_STATUS Status;
  AXP_POWER_PROTOCOL  *AxpPowerProtocol;
  user_gpio_set_t gpio_info;
  gpio_info.port = gpio_list->port;
  gpio_info.port_num = gpio_list->port_num;
  gpio_info.mul_sel = gpio_list->mul_sel;
  gpio_info.drv_level = gpio_list->drv_level;
  gpio_info.data = gpio_list->data;

  __inf("OSAL_GPIO_Request, port:%d, port_num:%d, mul_sel:%d, pull:%d, drv_level:%d, data:%d\n", gpio_list->port, gpio_list->port_num, gpio_list->mul_sel, gpio_list->pull, gpio_list->drv_level, gpio_list->data);
   //gpio_list->port, gpio_list->port_num, gpio_list->mul_sel, gpio_list->pull, gpio_list->drv_level, gpio_list->data);
    if(gpio_list->port == 0xffff) {
    Status = gBS->LocateProtocol (&gAxpPowerProtocolGuid, NULL, (VOID **)&AxpPowerProtocol);
       ASSERT_EFI_ERROR(Status);

    __u32 on_off;
    on_off = gpio_list->data;
        AxpPowerProtocol->SetSupplyStatus(PMU_SUPPLY_DC1SW, 0, on_off);
    return 0xffff;
  }
  return gpio_request(&gpio_info, group_count_max);
}

__hdle OSAL_GPIO_Request_Ex(char *main_name, const char *sub_name)
{
    return gpio_request_ex(main_name, sub_name);
}

//if_release_to_default_status:
    //�����0����1����ʾ�ͷź��GPIO��������״̬������״״̬���ᵼ���ⲿ��ƽ�Ĵ���
    //�����2����ʾ�ͷź��GPIO״̬���䣬���ͷŵ�ʱ�򲻹���ǰGPIO��Ӳ���Ĵ�����
__s32 OSAL_GPIO_Release(__hdle p_handler, __s32 if_release_to_default_status)
{
    //__inf("OSAL_GPIO_Release\n");
    if(p_handler != 0xffff)
    {
        gpio_release(p_handler, if_release_to_default_status);
    }

    return 0;
}

__s32 OSAL_GPIO_DevGetAllPins_Status(unsigned p_handler, disp_gpio_set_t *gpio_status, unsigned gpio_count_max, unsigned if_get_from_hardware)
{
    return gpio_get_all_pin_status(p_handler, (user_gpio_set_t *)gpio_status, gpio_count_max, if_get_from_hardware);
}

__s32 OSAL_GPIO_DevGetONEPins_Status(unsigned p_handler, disp_gpio_set_t *gpio_status,const char *gpio_name,unsigned if_get_from_hardware)
{
    return gpio_get_one_pin_status(p_handler, (user_gpio_set_t *)gpio_status,gpio_name,if_get_from_hardware);
}

__s32 OSAL_GPIO_DevSetONEPin_Status(u32 p_handler, disp_gpio_set_t *gpio_status, const char *gpio_name, __u32 if_set_to_current_input_status)
{
    return gpio_set_one_pin_status(p_handler, (user_gpio_set_t *)gpio_status, gpio_name, if_set_to_current_input_status);
}

__s32 OSAL_GPIO_DevSetONEPIN_IO_STATUS(u32 p_handler, __u32 if_set_to_output_status, const char *gpio_name)
{
    return gpio_set_one_pin_io_status(p_handler, if_set_to_output_status, gpio_name);
}

__s32 OSAL_GPIO_DevSetONEPIN_PULL_STATUS(u32 p_handler, __u32 set_pull_status, const char *gpio_name)
{
    return gpio_set_one_pin_pull(p_handler, set_pull_status, gpio_name);
}

__s32 OSAL_GPIO_DevREAD_ONEPIN_DATA(u32 p_handler, const char *gpio_name)
{
    return gpio_read_one_pin_value(p_handler, gpio_name);
}

__s32 OSAL_GPIO_DevWRITE_ONEPIN_DATA(u32 p_handler, __u32 value_to_gpio, const char *gpio_name)
{
    return gpio_write_one_pin_value(p_handler, value_to_gpio, gpio_name);
}


