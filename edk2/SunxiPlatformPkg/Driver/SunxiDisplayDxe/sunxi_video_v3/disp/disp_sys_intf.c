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

#include "de/bsp_display.h"
#include "disp_sys_intf.h"


/* cache flush flags */
#define  CACHE_FLUSH_I_CACHE_REGION       0
#define  CACHE_FLUSH_D_CACHE_REGION       1
#define  CACHE_FLUSH_CACHE_REGION         2
#define  CACHE_CLEAN_D_CACHE_REGION       3
#define  CACHE_CLEAN_FLUSH_D_CACHE_REGION 4
#define  CACHE_CLEAN_FLUSH_CACHE_REGION   5

/*
*******************************************************************************
*                     OSAL_CacheRangeFlush
*
* Description:
*    Cache flush
*
* Parameters:
*    address    :  start address to be flush
*    length     :  size
*    flags      :  flush flags
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void disp_sys_cache_flush(void*address, u32 length, u32 flags)
{
  if (address == NULL || length == 0) {
    return;
  }

  switch(flags) {
  case CACHE_FLUSH_I_CACHE_REGION:
  break;

  case CACHE_FLUSH_D_CACHE_REGION:
  break;

  case CACHE_FLUSH_CACHE_REGION:
  break;

  case CACHE_CLEAN_D_CACHE_REGION:
  break;

  case CACHE_CLEAN_FLUSH_D_CACHE_REGION:
  break;

  case CACHE_CLEAN_FLUSH_CACHE_REGION:
  break;

  default:
  break;
  }
  return;
}

#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/HardwareInterrupt.h>

EFI_HARDWARE_INTERRUPT_PROTOCOL *gInterrupt = NULL;

void Disp_InterruptInit(void)
{
    EFI_STATUS Status;
    Status = gBS->LocateProtocol (&gHardwareInterruptProtocolGuid, NULL, (VOID **)&gInterrupt);
    ASSERT_EFI_ERROR (Status);
    
}

/*
*******************************************************************************
*                     disp_sys_register_irq
*
* Description:
*    irq register
*
* Parameters:
*    irqno          ??input.  irq no
*    flags          ??input.
*    Handler        ??input.  isr handler
*    pArg           ??input.  para
*    DataSize       ??input.  len of para
*    prio         ??input.    priority

*
* Return value:
*
*
* note:
*    typedef s32 (*ISRCallback)( void *pArg)??
*
*******************************************************************************
*/
static UINT32 DispIrqPara[256] = {0};
void disp_sys_set_irq_para(u32 IrqNo,void *pArg)
{
  if(IrqNo < ARRAY_SIZE(DispIrqPara))
  {
    DispIrqPara[IrqNo] = (UINT32)pArg;
  }
  else
  {
    __wrn("error: irqNo=%d \n",IrqNo );
  }
}


UINT32 disp_sys_get_irq_para(u32 idnum)
{
  if(idnum < ARRAY_SIZE(DispIrqPara))
  {
    return DispIrqPara[idnum];
  }
  else
  {
    return 0;
  }
}


int disp_sys_register_irq(u32 IrqNo, u32 Flags, void* Handler,void *pArg,u32 DataSize,u32 Prio)
{
  __wrn("%a, irqNo=%d, Handler=0x%p, pArg=0x%p\n", __func__, IrqNo, Handler, pArg);
  //irq_install_handler(IrqNo, (interrupt_handler_t *)Handler,  pArg);

  EFI_STATUS  Status;
  Status = gInterrupt->RegisterInterruptSource (gInterrupt, IrqNo, Handler);
  ASSERT_EFI_ERROR (Status);
  disp_sys_set_irq_para(IrqNo, pArg);
  return 0;
}

/*
*******************************************************************************
*                     disp_sys_unregister_irq
*
* Description:
*    irq unregister
*
* Parameters:
*    irqno      ??input.  irq no
*    handler    ??input.  isr handler
*    Argment  ??input.    para
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void disp_sys_unregister_irq(u32 IrqNo, void* Handler, void *pArg)
{
  //irq_free_handler(IrqNo);
}

/*
*******************************************************************************
*                     disp_sys_enable_irq
*
* Description:
*    enable irq
*
* Parameters:
*    irqno ??input.  irq no
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void disp_sys_enable_irq(u32 IrqNo)
{
  //irq_enable(IrqNo);
    EFI_STATUS  Status;
    Status = gInterrupt->EnableInterruptSource(gInterrupt, IrqNo);
    ASSERT_EFI_ERROR (Status);
}

/*
*******************************************************************************
*                     disp_sys_disable_irq
*
* Description:
*    disable irq
*
* Parameters:
*     irqno ??input.  irq no
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void disp_sys_disable_irq(u32 IrqNo)
{
  //irq_disable(IrqNo);
  EFI_STATUS  Status;
  Status = gInterrupt->DisableInterruptSource(gInterrupt, IrqNo);
  ASSERT_EFI_ERROR (Status);
}
#if 1
void spin_lock_init(spinlock_t* lock)
{
  return;
}

void mutex_init(struct mutex* lock)
{
  return;
}

void mutex_destroy(struct mutex* lock)
{
  return;
}

void mutex_lock(struct mutex* lock)
{
  return;
}

void mutex_unlock(struct mutex* lock)
{
  return;
}

void tasklet_init(struct tasklet_struct *tasklet, void (*func), unsigned long data)
{
  if ((NULL == tasklet) || (NULL == func)) {
    __wrn("tasklet_init, para is NULL, tasklet=0x%p, func=0x%p\n", tasklet, func);
    return ;
  }
  tasklet->func = func;
  tasklet->data = data;

  return ;
}

void tasklet_schedule(struct tasklet_struct *tasklet)
{
  if (NULL == tasklet) {
    __wrn("tasklet_schedule, para is NULL, tasklet=0x%p\n", tasklet);
    return ;
  }
  tasklet->func(tasklet->data);
}

void * kmalloc(u32 Size, u32 flag)
{
  void * addr;

  addr = malloc(Size);
  if(addr)
    memset(addr, 0, Size);

  return addr;
}

void kfree(void *Addr)
{
  free(Addr);
}

#include <Protocol/AxpPower.h>
static AXP_POWER_PROTOCOL  *AxpPowerProtocol = NULL;

void disp_private_init(void)
{
  EFI_STATUS Status;
  Status = gBS->LocateProtocol (&gAxpPowerProtocolGuid, NULL, (VOID **)&AxpPowerProtocol);
  if(EFI_ERROR(Status))
  {
    printf("%a: find AxpPowerProtocol fail\n",__func__);
  }
  Disp_InterruptInit();

}

//example string  :    "axp81x_dcdc6 vdd-sys vdd-usb0-09 vdd-hdmi-09"
static int find_regulator_str(const char* src, const char* des)
{
    int i,len_src, len_des ;
    int token_index;
    len_src = strlen(src);
    len_des = strlen(des);

    if(len_des > len_src) return 0;

    token_index = 0;
    for(i =0 ; i < len_src+1; i++)
    {
        if(src[i]==' ' || src[i] == '\t' || src[i] == '\0')
        {
            if(i-token_index == len_des)
            {
                if(memcmp(src+token_index,des,len_des) == 0)
                {
                    return 1;
                }
            }
            token_index = i+1;
        }
    }
    return 0;
}

int axp_set_supply_status_byregulator(const char* id, int onoff)
{
    char main_key[32] = {0};
    char pmu_type[32] = {0};
    char vol_type[32] ={0};
    char sub_key_name[32] = {0};
    char sub_key_value[256] = {0};
    int main_hd = 0;
    unsigned int i = 0, j = 0, index = 0;
    int ldo_count = 0;
    int find_flag = 0;
    EFI_STATUS Status = 0;


    for(i = 0; i < 1; i++)
    {
        sprintf(main_key,"pmu%d_regu", i);

        main_hd = script_parser_fetch(main_key,"regulator_count",&ldo_count, 1);
        if(main_hd != 0)
        {
            //printf("unable to get ldo_count  from [%a] \n",main_key);
        break;
      }
        for(index = 1; index <= ldo_count; index++)
        {
            sprintf(sub_key_name, "regulator%d", index);
            main_hd = script_parser_fetch(main_key,sub_key_name,(int*)(sub_key_value), sizeof(sub_key_value)/sizeof(int));
            if(main_hd != 0)
            {
                //printf("unable to get subkey %a from [%a]\n",sub_key_name, main_key);
            break;
          }
            if(find_regulator_str(sub_key_value, id))
            {
                find_flag = 1;
                break;
            }

        }
        if(find_flag)
            break;
    }

    if(!find_flag)
    {
        printf("unable to find regulator %a from [pmu1_regu] or [pmu2_regu] \n",id);
        return -1;
    }

    //example :    ldo6      = "axp81x_dcdc6 vdd-sys vdd-usb0-09 vdd-hdmi-09"
    memset(pmu_type, 0, sizeof(pmu_type));
    memset(vol_type, 0, sizeof(vol_type));
    //get pmu type
    for(j = 0,i =0; i < strlen(sub_key_value); i++)
    {
        if(sub_key_value[i] == '_')
        {
            i++;
            break;
        }
        pmu_type[j++] = sub_key_value[i];
    }
    //get vol type
    j = 0;
    for(; i < strlen(sub_key_value); i++)
    {
        if(sub_key_value[i] == ' ')
        {
            break;
        }
        vol_type[j++]= sub_key_value[i];
    }

    //para vol = 0  indcate not set vol,only open or close  voltage switch
    Status = AxpPowerProtocol->SetSupplyStatusByName(vol_type,0, onoff);
    if(EFI_ERROR(Status))
    {
         printf("error: supply regelator %a=id, pmu_type=%s_%s onoff=%d fail\n", id,pmu_type,vol_type,onoff );
   return -1;
    }
    return 0;
}

/* type: 0:invalid, 1: int; 2:str, 3: gpio */
int disp_sys_script_get_item(char *main_name, char *sub_name, int value[], int type)
{
  int ret = 0;
  user_gpio_set_t  gpio_info;
  disp_gpio_set_t  *gpio_list;
  //int i, node_index = 0;
  //bool find_flag = false;
  //AsciiPrint("DISP: main key: %a, sub key: %a request\n", main_name, sub_name);
  
  if (1 == type) {
    if (script_parser_fetch(main_name, sub_name,  value, 1))
      __inf("script_parser_fetch %a.%a fail\n", main_name, sub_name);
    else
      ret = type;
  } else if (2 == type) {
    char str[64];
    if (script_parser_fetch(main_name, sub_name,  (INT32*)str, 64/4))
      __inf("script_parser_fetch %a.%a fail\n", main_name, sub_name);
    else {
      ret = type;
      memcpy((void*)value, str, strlen(str)+1);
    }
  } else if (3 == type) {
    if (script_parser_fetch(main_name, sub_name,  (INT32*)&gpio_info,sizeof(user_gpio_set_t)/sizeof(INT32) ))
      __inf("script_parser_fetch %a.%a fail\n", main_name, sub_name);
    else {
      gpio_list = (disp_gpio_set_t  *)value;
      gpio_list->port = gpio_info.port;
      gpio_list->port_num = gpio_info.port_num;
      gpio_list->mul_sel = gpio_info.mul_sel;
      gpio_list->drv_level = gpio_info.drv_level;
      gpio_list->pull = gpio_info.pull;
      gpio_list->data = gpio_info.data;

      memcpy(gpio_info.gpio_name, sub_name, strlen(sub_name)+1);
      __inf("%a.%a gpio=%d,mul_sel=%d,data:%d\n",main_name, sub_name, gpio_list->gpio, gpio_list->mul_sel, gpio_list->data);
      ret = type;
    }
  }

  return ret;
}
EXPORT_SYMBOL(disp_sys_script_get_item);

int disp_sys_get_ic_ver(void)
{
    return 0;
}

int disp_sys_gpio_request(disp_gpio_set_t *gpio_list, u32 group_count_max)
{
  user_gpio_set_t gpio_info;
  gpio_info.port = gpio_list->port;
  gpio_info.port_num = gpio_list->port_num;
  gpio_info.mul_sel = gpio_list->mul_sel;
  gpio_info.drv_level = gpio_list->drv_level;
  gpio_info.data = gpio_list->data;

  __inf("disp_sys_gpio_request, port:%d, port_num:%d, mul_sel:%d, pull:%d, drv_level:%d, data:%d\n", gpio_list->port, gpio_list->port_num, gpio_list->mul_sel, gpio_list->pull, gpio_list->drv_level, gpio_list->data);
   //gpio_list->port, gpio_list->port_num, gpio_list->mul_sel, gpio_list->pull, gpio_list->drv_level, gpio_list->data);
#if 0
  if(gpio_list->port == 0xffff) {
    __u32 on_off;
    on_off = gpio_list->data;
    //axp_set_dc1sw(on_off);
    axp_set_supply_status(0, PMU_SUPPLY_DC1SW, 0, on_off);

    return 0xffff;
  }
#endif
  return gpio_request(&gpio_info, group_count_max);
}
EXPORT_SYMBOL(disp_sys_gpio_request);

int disp_sys_gpio_request_simple(disp_gpio_set_t *gpio_list, u32 group_count_max)
{
	int ret = 0;
	user_gpio_set_t gpio_info;

	gpio_info.port = gpio_list->port;
	gpio_info.port_num = gpio_list->port_num;
	gpio_info.mul_sel = gpio_list->mul_sel;
	gpio_info.drv_level = gpio_list->drv_level;
	gpio_info.data = gpio_list->data;

	ret = gpio_request_early(&gpio_info, group_count_max, 1);
	return ret;
}
int disp_sys_gpio_release(int p_handler, s32 if_release_to_default_status)
{
  if(p_handler != 0xffff)
  {
    gpio_release(p_handler, if_release_to_default_status);
  }
  return 0;
}
EXPORT_SYMBOL(disp_sys_gpio_release);

/* direction: 0:input, 1:output */
int disp_sys_gpio_set_direction(u32 p_handler, u32 direction, const char *gpio_name)
{
  return gpio_set_one_pin_io_status(p_handler, direction, gpio_name);
}

int disp_sys_gpio_get_value(u32 p_handler, const char *gpio_name)
{
  return gpio_read_one_pin_value(p_handler, gpio_name);
}

int disp_sys_gpio_set_value(u32 p_handler, u32 value_to_gpio, const char *gpio_name)
{
  return gpio_write_one_pin_value(p_handler, value_to_gpio, gpio_name);
}

int disp_sys_pin_set_state(char *dev_name, char *name)
{
  char compat[32];
  u32 len = 0;
  int state = 0;
  int ret = -1;
  //int nodeoffset;

    /* Do not execute, for fix DSI panel reset issue */
	return ret;
	if (!strcmp(name, DISP_PIN_STATE_ACTIVE))
		state = 1;
	else
		state = 0;

  len = sprintf(compat, "%a", dev_name);
  if (len > 32)
    __wrn("disp_sys_set_state, size of mian_name is out of range\n");

#if 0
  nodeoffset = disp_fdt_nodeoffset(compat);
  if(nodeoffset < 0)
  {
    printf("nodeoffset is wrong!\n");
    return ret;
  }
  ret = fdt_set_all_pin_by_offset(nodeoffset, (1 == state)?"pinctrl-0":"pinctrl-1");
  if (0 != ret)
    __wrn("%a, fdt_set_all_pin, ret=%d\n", __func__, ret);
#endif
  printf("%a: mainkey: %s, state = %d\n",__func__,compat, state);
  ret = gpio_request_simple(compat, NULL);
  if (0 != ret)
    printf("%a, gpio_request_simple, ret=%d\n", __func__, ret);


  return ret;
}
EXPORT_SYMBOL(disp_sys_pin_set_state);

int disp_sys_power_enable(char *name)
{
  int ret = 0;
  if(0 == strlen(name)) {
    return 0;
  }
  ret = axp_set_supply_status_byregulator(name, 1);
  printf("enable power %a, ret=%d\n", name, ret);

  return 0;
}
EXPORT_SYMBOL(disp_sys_power_enable);

int disp_sys_power_disable(char *name)
{
  int ret = 0;

  ret = axp_set_supply_status_byregulator(name, 0);
  printf("disable power %a, ret=%d\n", name, ret);
  return 0;
}
EXPORT_SYMBOL(disp_sys_power_disable);

uintptr_t disp_sys_pwm_request(u32 pwm_id)
{
#if defined(CONFIG_ARCH_SUN50IW1P1)
  pwm_request(pwm_id, "lcd");
#endif
  return (pwm_id + 0x100);
}

int disp_sys_pwm_free(uintptr_t p_handler)
{
  return 0;
}

int disp_sys_pwm_enable(uintptr_t p_handler)
{
  int ret = 0;
  int pwm_id = p_handler - 0x100;

#if defined(CONFIG_ARCH_SUN50IW1P1)
  ret = pwm_enable(pwm_id);
#else
  ret = sunxi_pwm_enable(pwm_id);
#endif

  return ret;

}

int disp_sys_pwm_disable(uintptr_t p_handler)
{
  int ret = 0;
  int pwm_id = p_handler - 0x100;

#if defined(CONFIG_ARCH_SUN50IW1P1)
  pwm_disable(pwm_id);
#else
  sunxi_pwm_disable(pwm_id);
#endif

  return ret;
}

int disp_sys_pwm_config(uintptr_t p_handler, int duty_ns, int period_ns)
{
  int ret = 0;
  int pwm_id = p_handler - 0x100;

#if defined(CONFIG_ARCH_SUN50IW1P1)
  ret = pwm_config(pwm_id, duty_ns, period_ns);
#else
  ret = sunxi_pwm_config(pwm_id, duty_ns, period_ns);
#endif
  return ret;
}

int disp_sys_pwm_set_polarity(uintptr_t p_handler, int polarity)
{
  int ret = 0;
  int pwm_id = p_handler - 0x100;

#if defined(CONFIG_ARCH_SUN50IW1P1)
  ret = pwm_set_polarity(pwm_id, polarity);
#else
  ret = sunxi_pwm_set_polarity(pwm_id, polarity);
#endif

  return ret;
}

#endif

void * memcpy(void *dest, const void *src, UINT32 count)
{
  UINT32 *dl = (UINT32 *)dest, *sl = (UINT32 *)src;
  char *d8, *s8;

  if (src == dest)
    return dest;

  /* while all data is aligned (common case), copy a word at a time */
  if ( (((UINT32)dest | (UINT32)src) & (sizeof(*dl) - 1)) == 0) {
    while (count >= sizeof(*dl)) {
      *dl++ = *sl++;
      count -= sizeof(*dl);
    }
  }
  /* copy the reset one byte at a time */
  d8 = (char *)dl;
  s8 = (char *)sl;
  while (count--)
    *d8++ = *s8++;

  return dest;
}

