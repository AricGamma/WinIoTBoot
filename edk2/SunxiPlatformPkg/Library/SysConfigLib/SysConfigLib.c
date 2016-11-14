/** @file
  Implement EFI Random Number Generator runtime services via Rng Lib.
  
  Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>
  
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi/UefiBaseType.h>
#include <Pi/PiPeiCis.h>

#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Pi/PiBootMode.h>
#include <Pi/PiHob.h>
#include <Library/HobLib.h>
#include <Library/SysConfigLib.h>
#include <Guid/SunxiScriptParseHob.h>
#include <Interinc/sunxi_uefi.h>

static script_sub_key_t *sw_cfg_get_subkey(const CHAR8 *script_buf, const CHAR8 *main_key, const CHAR8 *sub_key)
{
  script_head_t *hd = (script_head_t *)script_buf;
  script_main_key_t *mk = (script_main_key_t *)(hd + 1);
  script_sub_key_t *sk = NULL;
  INT32 i, j;

  for (i = 0; i < hd->main_key_count; i++) {
    if (AsciiStrCmp(main_key, mk->main_name)) {
      mk++;
      continue;
    }

    for (j = 0; j < mk->lenth; j++) {
      sk = (script_sub_key_t *)(script_buf + (mk->offset<<2) + j * sizeof(script_sub_key_t));
      if (!AsciiStrCmp(sub_key, sk->sub_name)) return sk;
    }
  }
  return NULL;
}

INT32 sw_cfg_get_int(const CHAR8 *script_buf, const CHAR8 *main_key, const CHAR8 *sub_key)
{
  script_sub_key_t *sk = NULL;
  CHAR8 *pdata;
  INT32 value;

  sk = sw_cfg_get_subkey(script_buf, main_key, sub_key);
  if (sk == NULL) {
    return -1;
  }

  if (((sk->pattern >> 16) & 0xffff) == SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD) {
    pdata = (CHAR8 *)(script_buf + (sk->offset<<2));
    value = *((INT32 *)pdata);
    return value;
  }

  return -1;
}

CHAR8 *sw_cfg_get_str(const CHAR8 *script_buf, const CHAR8 *main_key, const CHAR8 *sub_key, CHAR8 *buf)
{
  script_sub_key_t *sk = NULL;
  CHAR8 *pdata;

  sk = sw_cfg_get_subkey(script_buf, main_key, sub_key);
  if (sk == NULL) {
    return NULL;
  }

  if (((sk->pattern >> 16) & 0xffff) == SCIRPT_PARSER_VALUE_TYPE_STRING) {
    pdata = (CHAR8 *)(script_buf + (sk->offset<<2));
    CopyMem(buf, pdata, ((sk->pattern >> 0) & 0xffff));
    return (CHAR8 *)buf;
  }

  return NULL;
}


/**########################################################################################
 *
 *                        Script Operations
 *
-#########################################################################################*/
static  CHAR8  *script_mod_buf = 0; //pointer to first key
static  INT32    script_main_key_count = -1;

static  INT32   _test_str_length(CHAR8 *str)
{
  INT32 length = 0;

  while(str[length++])
  {
    if(length > 32)
    {
      length = 32;
      break;
    }
  }

  return length;
}

INT32 script_parser_init(CHAR8 *script_buf)
{
  script_head_t   *script_head;

  if(script_buf)
  {
    script_mod_buf = script_buf;
    script_head = (script_head_t *)script_buf;
    script_main_key_count = script_head->main_key_count;

    return SCRIPT_PARSER_OK;
  }
  else
  {
    return SCRIPT_PARSER_EMPTY_BUFFER;
  }
}

INT32 script_parser_exit(void)
{
  script_mod_buf = NULL;
  script_main_key_count = 0;

  return SCRIPT_PARSER_OK;
}


UINT32 script_parser_fetch_subkey_start(CHAR8 *main_name)
{
  CHAR8   main_bkname[32];
  CHAR8   *main_CHAR8;
  script_main_key_t  *main_key = NULL;
  INT32    i;
  /* check params */
  if((!script_mod_buf) || (script_main_key_count <= 0))
  {
    return 0;
  }

  if(main_name == NULL)
  {
    return 0;
  }

  /* truncate string if size >31 bytes */
  main_CHAR8 = main_name;
  if(_test_str_length(main_name) > 31)
  {
    SetMem(main_bkname, 0, 32);
    CopyMem(main_bkname, main_name, 31);
    main_CHAR8 = main_bkname;
  }

  for(i=0;i<script_main_key_count;i++)
  {
    main_key = (script_main_key_t *)(script_mod_buf + (sizeof(script_head_t)) + i * sizeof(script_main_key_t));
    if(AsciiStrCmp(main_key->main_name, main_CHAR8))
    {
      continue;
    }

  return (UINT32)main_key;
}

  return 0;
}

INT32 script_parser_fetch_subkey_next(UINT32 hd, CHAR8 *sub_name, INT32 value[], INT32 *index)
{
  script_main_key_t  *main_key;
  script_sub_key_t   *sub_key = NULL;
  INT32    j;
  INT32    pattern;

  if(!hd)
  {
    return -1;
  }

  main_key = (script_main_key_t *)hd;
  /* now find sub key */
  for(j = *index; j < main_key->lenth; j++)
  {
    sub_key = (script_sub_key_t *)(script_mod_buf + (main_key->offset<<2) + (j * sizeof(script_sub_key_t)));
    pattern    = (sub_key->pattern>>16) & 0xffff; /* get datatype */
    if(pattern == SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD)
    {
      value[0] = *(INT32 *)(script_mod_buf + (sub_key->offset<<2));
      AsciiStrCpy(sub_name, sub_key->sub_name);
      *index = j + 1;

      return SCRIPT_PARSER_OK;
    }
    else if(pattern == SCIRPT_PARSER_VALUE_TYPE_STRING)
    {
      AsciiStrCpy((void *)value, script_mod_buf + (sub_key->offset<<2));
      AsciiStrCpy(sub_name, sub_key->sub_name);
      *index = j + 1;

      return SCRIPT_PARSER_OK;
    }
  }

  return SCRIPT_PARSER_KEY_NOT_FIND;
}

INT32 script_parser_fetch(CHAR8 *main_name, CHAR8 *sub_name, INT32 value[], INT32 count)
{
  CHAR8   main_bkname[32], sub_bkname[32];
  CHAR8   *main_CHAR8, *sub_CHAR8;
  script_main_key_t  *main_key = NULL;
  script_sub_key_t   *sub_key = NULL;
  INT32    i, j;
  INT32    pattern, word_count;
  /* check params */
  if((!script_mod_buf) || (script_main_key_count <= 0))
  {
    return SCRIPT_PARSER_EMPTY_BUFFER;
  }

  if((main_name == NULL) || (sub_name == NULL))
  {
    return SCRIPT_PARSER_KEYNAME_NULL;
  }

  if(value == NULL)
  {
    return SCRIPT_PARSER_DATA_VALUE_NULL;
  }

  /* truncate string if size >31 bytes */
  main_CHAR8 = main_name;
  if(_test_str_length(main_name) > 31)
  {
    SetMem(main_bkname, 0, 32);
    AsciiStrnCpy(main_bkname, main_name, 31);
    main_CHAR8 = main_bkname;
  }
  sub_CHAR8 = sub_name;
  if(_test_str_length(sub_name) > 31)
  {
    SetMem(sub_bkname, 0, 32);
    AsciiStrnCpy(sub_bkname, sub_name, 31);
    sub_CHAR8 = sub_bkname;
  }
  for(i=0;i<script_main_key_count;i++)
  {
    main_key = (script_main_key_t *)(script_mod_buf + (sizeof(script_head_t)) + i * sizeof(script_main_key_t));
    if(AsciiStrCmp(main_key->main_name, main_CHAR8))
    {
      continue;
    }

    /* now find sub key */
    for(j=0;j<main_key->lenth;j++)
    {
      sub_key = (script_sub_key_t *)(script_mod_buf + (main_key->offset<<2) + (j * sizeof(script_sub_key_t)));
      if(AsciiStrCmp(sub_key->sub_name, sub_CHAR8))
      {
          continue;
      }
      pattern    = (sub_key->pattern>>16) & 0xffff; /* get datatype */
      word_count = (sub_key->pattern>> 0) & 0xffff; /*get count of word */

      switch(pattern)
      {
        case SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD:
          value[0] = *(INT32 *)(script_mod_buf + (sub_key->offset<<2));
          break;

        case SCIRPT_PARSER_VALUE_TYPE_STRING:
          if(count < word_count)
          {
            word_count = count;
          }
          CopyMem((CHAR8 *)value, script_mod_buf + (sub_key->offset<<2), word_count << 2);
          break;

        case SCIRPT_PARSER_VALUE_TYPE_MULTI_WORD:
            break;
        case SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD:
          {
            script_gpio_set_t  *user_gpio_cfg = (script_gpio_set_t *)value;
            /* buffer space enough? */
            if(sizeof(script_gpio_set_t) > (count<<2))
            {
                return SCRIPT_PARSER_BUFFER_NOT_ENOUGH;
            }
            AsciiStrCpy( user_gpio_cfg->gpio_name, sub_CHAR8);
            CopyMem(&user_gpio_cfg->port, script_mod_buf + (sub_key->offset<<2),  sizeof(script_gpio_set_t) - 32);
            break;
          }
        case SCIRPT_PARSER_VALUE_TYPE_DATA_EMPTY:
           return SCRIPT_PARSER_DATA_VALUE_NULL;
  
      }

      return SCRIPT_PARSER_OK;
    }
  }

  return SCRIPT_PARSER_KEY_NOT_FIND;
}


INT32 script_parser_fetch_ex(CHAR8 *main_name, CHAR8 *sub_name, INT32 value[], script_parser_value_type_t *type, INT32 count)
{
  CHAR8   main_bkname[32], sub_bkname[32];
  CHAR8   *main_CHAR8, *sub_CHAR8;
  script_main_key_t  *main_key = NULL;
  script_sub_key_t   *sub_key = NULL;
  INT32    i, j;
  INT32    pattern, word_count;
  /* check params */
  if((!script_mod_buf) || (script_main_key_count <= 0))
  {
    return SCRIPT_PARSER_EMPTY_BUFFER;
  }

  if((main_name == NULL) || (sub_name == NULL))
  {
    return SCRIPT_PARSER_KEYNAME_NULL;
  }

  if(value == NULL)
  {
    return SCRIPT_PARSER_DATA_VALUE_NULL;
  }

  /* truncate string if size >31 bytes */
  main_CHAR8 = main_name;
  if(_test_str_length(main_name) > 31)
  {
    SetMem(main_bkname, 0, 32);
    AsciiStrnCpy(main_bkname, main_name, 31);
    main_CHAR8 = main_bkname;
  }
  sub_CHAR8 = sub_name;
  if(_test_str_length(sub_name) > 31)
  {
    SetMem(sub_bkname, 0, 32);
    AsciiStrnCpy(sub_bkname, sub_name, 31);
    sub_CHAR8 = sub_bkname;
  }
  for(i=0;i<script_main_key_count;i++)
  {
    main_key = (script_main_key_t *)(script_mod_buf + (sizeof(script_head_t)) + i * sizeof(script_main_key_t));
    if(AsciiStrCmp(main_key->main_name, main_CHAR8))
    {
      continue;
    }

    /* now find sub key */
    for(j=0;j<main_key->lenth;j++)
    {
      sub_key = (script_sub_key_t *)(script_mod_buf + (main_key->offset<<2) + (j * sizeof(script_sub_key_t)));
      if(AsciiStrCmp(sub_key->sub_name, sub_CHAR8))
      {
        continue;
      }
      pattern    = (sub_key->pattern>>16) & 0xffff; /* get datatype */
      word_count = (sub_key->pattern>> 0) & 0xffff; /*get count of word */

      switch(pattern)
      {
        case SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD:
          value[0] = *(INT32 *)(script_mod_buf + (sub_key->offset<<2));
          *type = SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD;
          break;

        case SCIRPT_PARSER_VALUE_TYPE_STRING:
          if(count < word_count)
          {
            word_count = count;
          }
          CopyMem((CHAR8 *)value, script_mod_buf + (sub_key->offset<<2), word_count << 2);
          *type = SCIRPT_PARSER_VALUE_TYPE_STRING;
          break;

        case SCIRPT_PARSER_VALUE_TYPE_MULTI_WORD:
          *type = SCIRPT_PARSER_VALUE_TYPE_MULTI_WORD;
          break;
        case SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD:
        {
          script_gpio_set_t  *user_gpio_cfg = (script_gpio_set_t *)value;
          /* buffer space enough? */
          if(sizeof(script_gpio_set_t) > (count<<2))
          {
            return SCRIPT_PARSER_BUFFER_NOT_ENOUGH;
          }
          AsciiStrCpy( user_gpio_cfg->gpio_name, sub_CHAR8);
          CopyMem(&user_gpio_cfg->port, script_mod_buf + (sub_key->offset<<2),  sizeof(script_gpio_set_t) - 32);
          *type = SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD;
          break;
        }
      }

      return SCRIPT_PARSER_OK;
    }
  }

  return SCRIPT_PARSER_KEY_NOT_FIND;
}



INT32 script_parser_patch(CHAR8 *main_name, CHAR8 *sub_name, void *str, INT32 str_size)
{
  CHAR8   main_bkname[32], sub_bkname[32];
  CHAR8   *main_CHAR8, *sub_CHAR8;
  script_main_key_t  *main_key = NULL;
  script_sub_key_t   *sub_key = NULL;
  INT32    i, j;
  INT32    pattern, word_count;

  //检查脚本buffer是否存在
  if(!script_mod_buf)
  {
    return SCRIPT_PARSER_EMPTY_BUFFER;
  }
  //检查主键名称和子键名称是否为空
  if((main_name == NULL) || (sub_name == NULL))
  {
    return SCRIPT_PARSER_KEYNAME_NULL;
  }
  //检查数据buffer是否为空
  if(str == NULL)
  {
    return SCRIPT_PARSER_DATA_VALUE_NULL;
  }
  //保存主键名称和子键名称，如果超过31字节则截取31字节
  main_CHAR8 = main_name;
  if(_test_str_length(main_name) > 31)
  {
    SetMem(main_bkname, 0, 32);
    CopyMem(main_bkname, main_name, 31);
    main_CHAR8 = main_bkname;
  }
  sub_CHAR8 = sub_name;
  if(_test_str_length(sub_name) > 31)
  {
    SetMem(sub_bkname, 0, 32);
    CopyMem(sub_bkname, sub_name, 31);
    sub_CHAR8 = sub_bkname;
  }
  for(i=0;i<script_main_key_count;i++)
  {
    main_key = (script_main_key_t *)(script_mod_buf + (sizeof(script_head_t)) + i * sizeof(script_main_key_t));
    if(AsciiStrCmp(main_key->main_name, main_CHAR8))    //如果主键不匹配，寻找下一个主键
    {
      continue;
    }
    //主键匹配，寻找子键名称匹配
    for(j=0;j<main_key->lenth;j++)
    {
      sub_key = (script_sub_key_t *)(script_mod_buf + (main_key->offset<<2) + (j * sizeof(script_sub_key_t)));
      if(AsciiStrCmp(sub_key->sub_name, sub_CHAR8))    //如果主键不匹配，寻找下一个主键
      {
        continue;
      }
      pattern    = (sub_key->pattern>>16) & 0xffff;             //获取数据的类型
      word_count = (sub_key->pattern>> 0) & 0xffff;             //获取所占用的word个数
      //取出数据
      if(pattern == SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD)                      //单word数据类型
      {
        *(INT32 *)(script_mod_buf + (sub_key->offset<<2)) = *(INT32 *)str;

          return SCRIPT_PARSER_OK;
        }
        else if(pattern == SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD)
        {
          script_gpio_set_t  *user_gpio_cfg = (script_gpio_set_t *)str;

          SetMem(script_mod_buf + (sub_key->offset<<2), 0, 24);
          CopyMem(script_mod_buf + (sub_key->offset<<2), &user_gpio_cfg->port, 24);

          return SCRIPT_PARSER_OK;
        }
        else if(pattern == SCIRPT_PARSER_VALUE_TYPE_STRING)
        {
          if(str_size > word_count)
        {
          str_size = word_count;
        }
        SetMem(script_mod_buf + (sub_key->offset<<2), 0, word_count << 2);
        CopyMem(script_mod_buf + (sub_key->offset<<2), str, str_size << 2);

        return SCRIPT_PARSER_OK;
      }
    }
  }

  return SCRIPT_PARSER_KEY_NOT_FIND;
}

INT32 script_parser_subkey_count(CHAR8 *main_name)
{
  CHAR8   main_bkname[32];
  CHAR8   *main_CHAR8;
  script_main_key_t  *main_key = NULL;
  INT32    i;

  if(!script_mod_buf)
  {
    return SCRIPT_PARSER_EMPTY_BUFFER;
  }

  if(main_name == NULL)
  {
    return SCRIPT_PARSER_KEYNAME_NULL;
  }

  main_CHAR8 = main_name;
  if(_test_str_length(main_name) > 31)
  {
    SetMem(main_bkname, 0, 32);
    CopyMem(main_bkname, main_name, 31);
    main_CHAR8 = main_bkname;
  }

  for(i=0;i<script_main_key_count;i++)
  {
    main_key = (script_main_key_t *)(script_mod_buf + (sizeof(script_head_t)) + i * sizeof(script_main_key_t));
    if(AsciiStrCmp(main_key->main_name, main_CHAR8))
    {
      continue;
    }

    return main_key->lenth;
  }

  return -1;
}

INT32 script_parser_mainkey_count(void)
{
  if(!script_mod_buf)
  {
    return SCRIPT_PARSER_EMPTY_BUFFER;
  }

  return  script_main_key_count;
}

INT32 script_parser_mainkey_get_gpio_count(CHAR8 *main_name)
{
  CHAR8   main_bkname[32];
  CHAR8   *main_CHAR8;
  script_main_key_t  *main_key = NULL;
  script_sub_key_t   *sub_key = NULL;
  INT32    i, j;
  INT32    pattern, gpio_count = 0;

  if(!script_mod_buf)
  {
    return SCRIPT_PARSER_EMPTY_BUFFER;
  }

  if(main_name == NULL)
  {
    return SCRIPT_PARSER_KEYNAME_NULL;
  }

  main_CHAR8 = main_name;
  if(_test_str_length(main_name) > 31)
  {
    SetMem(main_bkname, 0, 32);
    AsciiStrnCpy(main_bkname, main_name, 31);
    main_CHAR8 = main_bkname;
  }

  for(i=0;i<script_main_key_count;i++)
  {
    main_key = (script_main_key_t *)(script_mod_buf + (sizeof(script_head_t)) + i * sizeof(script_main_key_t));
    if(AsciiStrCmp(main_key->main_name, main_CHAR8))
    {
      continue;
    }

    for(j=0;j<main_key->lenth;j++)
    {
      sub_key = (script_sub_key_t *)(script_mod_buf + (main_key->offset<<2) + (j * sizeof(script_sub_key_t)));

      pattern    = (sub_key->pattern>>16) & 0xffff;

      if(SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD == pattern)
      {
        gpio_count ++;
      }
    }
  }

  return gpio_count;
}

INT32 script_parser_mainkey_get_gpio_cfg(CHAR8 *main_name, void *gpio_cfg, INT32 gpio_count)
{
  CHAR8   main_bkname[32];
  CHAR8   *main_CHAR8;
  script_main_key_t  *main_key = NULL;
  script_sub_key_t   *sub_key = NULL;
  script_gpio_set_t  *user_gpio_cfg = (script_gpio_set_t *)gpio_cfg;
  INT32    i, j;
  INT32    pattern, user_index;

  if(!script_mod_buf)
  {
    return SCRIPT_PARSER_EMPTY_BUFFER;
  }

  if(main_name == NULL)
  {
    return SCRIPT_PARSER_KEYNAME_NULL;
  }

  SetMem(user_gpio_cfg, 0, sizeof(script_gpio_set_t) * gpio_count);

  main_CHAR8 = main_name;
  if(_test_str_length(main_name) > 31)
  {
    SetMem(main_bkname, 32,0);
    CopyMem(main_bkname, main_name, 31);
    main_CHAR8 = main_bkname;
  }

  for(i=0;i<script_main_key_count;i++)
  {
    main_key = (script_main_key_t *)(script_mod_buf + (sizeof(script_head_t)) + i * sizeof(script_main_key_t));
    if(AsciiStrCmp(main_key->main_name, main_CHAR8))
    {
      continue;
    }

    /* printf("mainkey name = %s\n", main_key->main_name);*/
    user_index = 0;
    for(j=0;j<main_key->lenth;j++)
    {
      sub_key = (script_sub_key_t *)(script_mod_buf + (main_key->offset<<2) + (j * sizeof(script_sub_key_t)));
      /*  printf("subkey name = %s\n", sub_key->sub_name);*/
      pattern    = (sub_key->pattern>>16) & 0xffff;
      /* printf("subkey pattern = %d\n", pattern);*/

      if(SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD == pattern)
      {
        AsciiStrCpy( user_gpio_cfg[user_index].gpio_name, sub_key->sub_name);
        CopyMem(&user_gpio_cfg[user_index].port, script_mod_buf + (sub_key->offset<<2), sizeof(script_gpio_set_t) - 32);
        user_index++;
        if(user_index >= gpio_count)
        {
            break;
        }
      }
    }
    return SCRIPT_PARSER_OK;
  }

  return SCRIPT_PARSER_KEY_NOT_FIND;
}

typedef struct
{
  INT32 mul_sel;
  INT32 pull;
  INT32 drv_level;
  INT32 data;
} gpio_status_set_t;

typedef struct
{
  CHAR8    gpio_name[32];
  INT32     port;
  INT32     port_num;
  gpio_status_set_t user_gpio_status;
  gpio_status_set_t hardware_gpio_status;
} system_gpio_set_t;

/*
****************************************************************************************************
*
*             CSP_PIN_init
*
*  Description:
*       init
*
*  Parameters:
*  Return value:
*        EGPIO_SUCCESS/EGPIO_FAIL
****************************************************************************************************
*/

INT32 sw_gpio_init(void)
{
  // return script_parser_init((CHAR8 *)PcdGet32(PcdScriptEarlyBase));
  return 0;
}

/*
****************************************************************************************************
*
*             CSP_PIN_exit
*
*  Description:
*       exit
*
*  Parameters:
*
*  Return value:
*        EGPIO_SUCCESS/EGPIO_FAIL
****************************************************************************************************
*/
INT32 gpio_exit(void)
{
  return 0;
}


#define GPIO_REG_READ(reg)              (*(volatile INTN *)(reg))
#define GPIO_REG_WRITE(reg, value)      ((*(volatile INTN *)(reg)) = (value))
#define PIOC_REG_o_CFG0                 0x00
#define PIOC_REG_o_CFG1                 0x04
#define PIOC_REG_o_CFG2                 0x08
#define PIOC_REG_o_CFG3                 0x0C
#define PIOC_REG_o_DATA                 0x10
#define PIOC_REG_o_DRV0                 0x14
#define PIOC_REG_o_DRV1                 0x18
#define PIOC_REG_o_PUL0                 0x1C
#define PIOC_REG_o_PUL1                 0x20

#define SUNXI_PIO_BASE  (PcdGet32(PcdGpioBase))
#define SUNXI_CPUS_PIO_BASE (PcdGet32(PcdCpusGpioBase))

#define readl(addr)               ( *((volatile INTN *)(addr)))
#define writel(value, addr)         ((*((volatile INTN *)(addr))) = (value))

/**#############################################################################################################
 *
 *                           GPIO(PIN) Operations
 *
-##############################################################################################################*/
//cpux gpio
#define PIO_REG_CFG(n, i)               (( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x00))
#define PIO_REG_DLEVEL(n, i)            (( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x14))
#define PIO_REG_PULL(n, i)              (( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x1C))
#define PIO_REG_DATA(n)                 (( SUNXI_PIO_BASE + ((n)-1)*0x24 + 0x10))


#define PIO_REG_CFG_VALUE(n, i)          readl( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x00)
#define PIO_REG_DLEVEL_VALUE(n, i)       readl( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x14)
#define PIO_REG_PULL_VALUE(n, i)         readl( SUNXI_PIO_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x1C)
#define PIO_REG_DATA_VALUE(n)            readl( SUNXI_PIO_BASE + ((n)-1)*0x24 + 0x10)

//cpus gpio
#define R_PIO_REG_CFG(n, i)               (( SUNXI_CPUS_PIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x00))
#define R_PIO_REG_DLEVEL(n, i)            (( SUNXI_CPUS_PIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x14))
#define R_PIO_REG_PULL(n, i)              (( SUNXI_CPUS_PIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x1C))
#define R_PIO_REG_DATA(n)                 (( SUNXI_CPUS_PIO_BASE + ((n)-12)*0x24 + 0x10))

#define R_PIO_REG_CFG_VALUE(n, i)          readl( SUNXI_CPUS_PIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x00)
#define R_PIO_REG_DLEVEL_VALUE(n, i)       readl( SUNXI_CPUS_PIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x14)
#define R_PIO_REG_PULL_VALUE(n, i)         readl( SUNXI_CPUS_PIO_BASE + ((n)-12)*0x24 + ((i)<<2) + 0x1C)
#define R_PIO_REG_DATA_VALUE(n)            readl( SUNXI_CPUS_PIO_BASE + ((n)-12)*0x24 + 0x10)
#define R_PIO_REG_BASE(n)                 ((volatile UINTN *)(SUNXI_CPUS_PIO_BASE +((n)-12)*24))


UINT32 GetGroupFuncAddrByPort(INT32 port, INT32 port_num)
{
  if(port < 12)
  {
    return PIO_REG_CFG(port, (port_num>>3));   //更新功能寄存器地址
  }
  else
  {
    return R_PIO_REG_CFG(port, (port_num>>3));  
  }
}

UINT32 GetGroupPullAddrByPort(INT32 port, INT32 port_num)
{
  if(port < 12)
  {
    return PIO_REG_PULL(port, (port_num>>4));  //更新pull寄存器
  }
  else
  {   
    return R_PIO_REG_PULL(port, (port_num>>4));  //更新pull寄存器
  }
}

UINT32 GetGroupDlevelAddrByPort(INT32 port,INT32 port_num)
{
  if(port < 12)
  {
    return PIO_REG_DLEVEL(port, (port_num>>4));//更新level寄存器
  }
  else
  {
    return R_PIO_REG_DLEVEL(port, (port_num>>4));//更新level寄存器
  }
}

UINT32 GetGroupDataAddrByPort(INT32 port)
{
  if(port < 12)
  {
    return PIO_REG_DATA(port);                 //更新data寄存器
  }
  else
  {
    return R_PIO_REG_DATA(port);               //更新data寄存器
  }
}


#if 1
/*
************************************************************************************************************
*
*                                             normal_gpio_cfg
*
*    函数名称：
*
*    参数列表：
*
*
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
INT32 gpio_request_early(void  *user_gpio_list, UINT32 group_count_max, INT32 set_gpio)
{
  user_gpio_set_t    *tmp_user_gpio_data, *gpio_list;
  UINT32         first_port=0;                      //保存真正有效的GPIO的个数
  UINT32               tmp_group_func_data=0;
  UINT32               tmp_group_pull_data=0;
  UINT32               tmp_group_dlevel_data=0;
  UINT32               tmp_group_data_data=0;
  UINT32               data_change = 0;
//  UINT32         *tmp_group_port_addr;
  volatile UINT32     *tmp_group_func_addr=NULL,   *tmp_group_pull_addr=NULL;
  volatile UINT32     *tmp_group_dlevel_addr=NULL, *tmp_group_data_addr=NULL;
  UINT32          port, port_num, port_num_func, port_num_pull=0;
  UINT32          pre_port=0, pre_port_num_func=0;
  UINT32          pre_port_num_pull=0;
  INT32               i, tmp_val;

    //准备第一个GPIO数据
    /*if(BOOT_STORAGE_CODE_BASE == ((UINT32)user_gpio_list & BOOT_STORAGE_CODE_BASE))
    {
      INT32 tmp = (UINT32)user_gpio_list & 0x03;

      tmp <<= 3;
      gpio_list = (normal_gpio_cfg *)&BT1_head.prvt_head.storage_gpio[tmp];
    }
    else*/
  {
    gpio_list = user_gpio_list;
  }

  for(first_port = 0; first_port < group_count_max; first_port++)
  {
    tmp_user_gpio_data = gpio_list + first_port;
    port     = tmp_user_gpio_data->port;                         //读出端口数值
    port_num = tmp_user_gpio_data->port_num;                     //读出端口中的某一个GPIO
    if(!port)
    {
      continue;
    }
          
    port_num_func = (port_num >> 3);
    port_num_pull = (port_num >> 4);

    tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //更新功能寄存器地址
    tmp_group_pull_addr    = (volatile UINT32 *)GetGroupPullAddrByPort(port, port_num);  //更新pull寄存器
    tmp_group_dlevel_addr  = (volatile UINT32 *)GetGroupDlevelAddrByPort(port, port_num);//更新level寄存器
    tmp_group_data_addr    = (volatile UINT32 *)GetGroupDataAddrByPort(port);  //更新data寄存器
  
    tmp_group_func_data    = GPIO_REG_READ(tmp_group_func_addr);
    tmp_group_pull_data    = GPIO_REG_READ(tmp_group_pull_addr);
    tmp_group_dlevel_data  = GPIO_REG_READ(tmp_group_dlevel_addr);
    tmp_group_data_data    = GPIO_REG_READ(tmp_group_data_addr);

    /*printf("func_addr:%x,pull_addr:%x,level_addr:%x,data_addr%x\n",tmp_group_func_data,tmp_group_pull_data,tmp_group_dlevel_data,tmp_group_data_data);*/
    pre_port          = port;
    pre_port_num_func = port_num_func;
    pre_port_num_pull = port_num_pull;
    //更新功能寄存器
    tmp_val = (port_num - (port_num_func << 3)) << 2;
    tmp_group_func_data &= ~(                              0x07  << tmp_val);
    if(set_gpio)
    {
      tmp_group_func_data |=  (tmp_user_gpio_data->mul_sel & 0x07) << tmp_val;
    }
    //根据pull的值决定是否更新pull寄存器
    tmp_val              =  (port_num - (port_num_pull << 4)) << 1;
    if(tmp_user_gpio_data->pull >= 0)
    {
      tmp_group_pull_data &= ~(                           0x03  << tmp_val);
      tmp_group_pull_data |=  (tmp_user_gpio_data->pull & 0x03) << tmp_val;
    }
    //根据driver level的值决定是否更新driver level寄存器
    if(tmp_user_gpio_data->drv_level >= 0)
    {
      tmp_group_dlevel_data &= ~(                                0x03  << tmp_val);
      tmp_group_dlevel_data |=  (tmp_user_gpio_data->drv_level & 0x03) << tmp_val;
    }
    //根据用户输入，以及功能分配决定是否更新data寄存器
    if(tmp_user_gpio_data->mul_sel == 1)
    {
      if(tmp_user_gpio_data->data >= 0)
      {
        tmp_val = tmp_user_gpio_data->data & 1;
        tmp_group_data_data &= ~(1 << port_num);
        tmp_group_data_data |= tmp_val << port_num;
        data_change = 1;
      }
    }

    break;
  }
  //检查是否有数据存在
  if(first_port >= group_count_max)
  {
    return -1;
  }
  //保存用户数据
  for(i = first_port + 1; i < group_count_max; i++)
  {
    tmp_user_gpio_data = gpio_list + i;                 //gpio_set依次指向用户的每个GPIO数组成员
    port     = tmp_user_gpio_data->port;                //读出端口数值
    port_num = tmp_user_gpio_data->port_num;            //读出端口中的某一个GPIO
    if(!port)
    {
      break;
    }
 
    port_num_func = (port_num >> 3);
    port_num_pull = (port_num >> 4);

    if((port_num_pull != pre_port_num_pull) || (port != pre_port))    //如果发现当前引脚的端口不一致，或者所在的pull寄存器不一致
    {
      GPIO_REG_WRITE(tmp_group_func_addr, tmp_group_func_data);     //回写功能寄存器
      GPIO_REG_WRITE(tmp_group_pull_addr, tmp_group_pull_data);     //回写pull寄存器
      GPIO_REG_WRITE(tmp_group_dlevel_addr, tmp_group_dlevel_data); //回写driver level寄存器
      if(data_change)
      {
        data_change = 0;
        GPIO_REG_WRITE(tmp_group_data_addr, tmp_group_data_data); //回写data寄存器
      }

      tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //更新功能寄存器地址
      tmp_group_pull_addr    = (volatile UINT32 *)GetGroupPullAddrByPort(port, port_num);  //更新pull寄存器
      tmp_group_dlevel_addr  = (volatile UINT32 *)GetGroupDlevelAddrByPort(port, port_num);//更新level寄存器
      tmp_group_data_addr    = (volatile UINT32 *)GetGroupDataAddrByPort(port);  //更新data寄存器


      tmp_group_func_data    = GPIO_REG_READ(tmp_group_func_addr);
      tmp_group_pull_data    = GPIO_REG_READ(tmp_group_pull_addr);
      tmp_group_dlevel_data  = GPIO_REG_READ(tmp_group_dlevel_addr);
      tmp_group_data_data    = GPIO_REG_READ(tmp_group_data_addr);        
    }
    else if(pre_port_num_func != port_num_func)                       //如果发现当前引脚的功能寄存器不一致
    {
      GPIO_REG_WRITE(tmp_group_func_addr, tmp_group_func_data);    //则只回写功能寄存器
      tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //更新功能寄存器地址

      tmp_group_func_data    = GPIO_REG_READ(tmp_group_func_addr);
    }
    //保存当前硬件寄存器数据
    pre_port_num_pull = port_num_pull;                      //设置当前GPIO成为前一个GPIO
    pre_port_num_func = port_num_func;
    pre_port          = port;

    //更新功能寄存器
    tmp_val = (port_num - (port_num_func << 3)) << 2;
    if(tmp_user_gpio_data->mul_sel >= 0)
    {
      tmp_group_func_data &= ~(                              0x07  << tmp_val);
      if(set_gpio)
      {
        tmp_group_func_data |=  (tmp_user_gpio_data->mul_sel & 0x07) << tmp_val;
      }
    }
    //根据pull的值决定是否更新pull寄存器
    tmp_val              =  (port_num - (port_num_pull << 4)) << 1;
    if(tmp_user_gpio_data->pull >= 0)
    {
      tmp_group_pull_data &= ~(                           0x03  << tmp_val);
      tmp_group_pull_data |=  (tmp_user_gpio_data->pull & 0x03) << tmp_val;
    }
    //根据driver level的值决定是否更新driver level寄存器
    if(tmp_user_gpio_data->drv_level >= 0)
    {
      tmp_group_dlevel_data &= ~(                                0x03  << tmp_val);
      tmp_group_dlevel_data |=  (tmp_user_gpio_data->drv_level & 0x03) << tmp_val;
    }
    //根据用户输入，以及功能分配决定是否更新data寄存器
    if(tmp_user_gpio_data->mul_sel == 1)
    {
      if(tmp_user_gpio_data->data >= 0)
      {
        tmp_val = tmp_user_gpio_data->data & 1;
        tmp_group_data_data &= ~(1 << port_num);
        tmp_group_data_data |= tmp_val << port_num;
        data_change = 1;
      }
    }
  }
  //for循环结束，如果存在还没有回写的寄存器，这里写回到硬件当中
  if(tmp_group_func_addr)                         //只要更新过寄存器地址，就可以对硬件赋值
  {                                               //那么把所有的值全部回写到硬件寄存器
    GPIO_REG_WRITE(tmp_group_func_addr,   tmp_group_func_data);   //回写功能寄存器
    GPIO_REG_WRITE(tmp_group_pull_addr,   tmp_group_pull_data);   //回写pull寄存器
    GPIO_REG_WRITE(tmp_group_dlevel_addr, tmp_group_dlevel_data); //回写driver level寄存器
    if(data_change)
    {
      GPIO_REG_WRITE(tmp_group_data_addr, tmp_group_data_data); //回写data寄存器
    }
  }

  return 0;
}
#endif 

#if 1
/*
************************************************************************************************************
*
*                                             CSP_GPIO_Request
*
*    函数名称：
*
*    参数列表：gpio_list      存放所有用到的GPIO数据的数组，GPIO将直接使用这个数组
*
*               group_count_max  数组的成员个数，GPIO设定的时候，将操作的GPIO最大不超过这个值
*
*    返回值  ：
*
*    说明    ：暂时没有做冲突检查
*
*
************************************************************************************************************
*/
UINT32 gpio_request(user_gpio_set_t *gpio_list, UINT32 group_count_max)
{
  CHAR8               *user_gpio_buf;                                        //按照CHAR8类型申请
  system_gpio_set_t  *user_gpio_set, *tmp_sys_gpio_data;                      //user_gpio_set将是申请内存的句柄
  user_gpio_set_t  *tmp_user_gpio_data;
  UINT32                real_gpio_count = 0, first_port;                      //保存真正有效的GPIO的个数
  UINT32               tmp_group_func_data = 0;
  UINT32               tmp_group_pull_data = 0;
  UINT32               tmp_group_dlevel_data = 0;
  UINT32               tmp_group_data_data = 0;
  UINT32               func_change = 0, pull_change = 0;
  UINT32               dlevel_change = 0, data_change = 0;
  volatile UINT32  *tmp_group_func_addr = NULL, *tmp_group_pull_addr = NULL;
  volatile UINT32  *tmp_group_dlevel_addr = NULL, *tmp_group_data_addr = NULL;
  UINT32  port, port_num, port_num_func, port_num_pull;
  UINT32  pre_port = 0x7fffffff, pre_port_num_func = 0x7fffffff;
  UINT32  pre_port_num_pull = 0x7fffffff;
  INT32  i, tmp_val;

  if((!gpio_list) || (!group_count_max))
  {
    return (UINT32)0;
  }
  for(i = 0; i < group_count_max; i++)
  {
    tmp_user_gpio_data = gpio_list + i;                 //gpio_set依次指向每个GPIO数组成员
    if(!tmp_user_gpio_data->port)
    {
        continue;
    }
    real_gpio_count ++;
  }

  //SYSCONFIG_DEBUG("to AllocatePool  space for pin \n");
  user_gpio_buf = (CHAR8 *)AllocatePool (16 + sizeof(system_gpio_set_t) * real_gpio_count);   //申请内存，多申请16个字节，用于存放GPIO个数等信息
  if(!user_gpio_buf)
  {
    return (UINT32)0;
  }
  SetMem(user_gpio_buf,16 + sizeof(system_gpio_set_t) * real_gpio_count,0);         //首先全部清零
  *(INT32 *)user_gpio_buf = real_gpio_count;                                           //保存有效的GPIO个数
  user_gpio_set = (system_gpio_set_t *)(user_gpio_buf + 16);                         //指向第一个结构体
  //准备第一个GPIO数据
  for(first_port = 0; first_port < group_count_max; first_port++)
  {
    tmp_user_gpio_data = gpio_list + first_port;
    port     = tmp_user_gpio_data->port;                         //读出端口数值
    port_num = tmp_user_gpio_data->port_num;                     //读出端口中的某一个GPIO
    if(!port)
    {
      continue;
    }
    port_num_func = (port_num >> 3);
    port_num_pull = (port_num >> 4);

    tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //更新功能寄存器地址
    tmp_group_pull_addr    = (volatile UINT32 *)GetGroupPullAddrByPort(port, port_num);  //更新pull寄存器
    tmp_group_dlevel_addr  = (volatile UINT32 *)GetGroupDlevelAddrByPort(port, port_num);//更新level寄存器
    tmp_group_data_addr    = (volatile UINT32 *)GetGroupDataAddrByPort(port);  //更新data寄存器

    tmp_group_func_data    = *tmp_group_func_addr;
    tmp_group_pull_data    = *tmp_group_pull_addr;
    tmp_group_dlevel_data  = *tmp_group_dlevel_addr;
    tmp_group_data_data    = *tmp_group_data_addr;
    
    break;
  }
  if(first_port >= group_count_max)
  {
    return 0;
  }
  //保存用户数据
  for(i = first_port; i < group_count_max; i++)
  {
    tmp_sys_gpio_data  = user_gpio_set + i;             //tmp_sys_gpio_data指向申请的GPIO空间
    tmp_user_gpio_data = gpio_list + i;                 //gpio_set依次指向用户的每个GPIO数组成员
    port     = tmp_user_gpio_data->port;                //读出端口数值
    port_num = tmp_user_gpio_data->port_num;            //读出端口中的某一个GPIO
    if(!port)
    {
      continue;
    }
    //开始保存用户数据
    AsciiStrCpy(tmp_sys_gpio_data->gpio_name, tmp_user_gpio_data->gpio_name);
    tmp_sys_gpio_data->port                       = port;
    tmp_sys_gpio_data->port_num                   = port_num;
    tmp_sys_gpio_data->user_gpio_status.mul_sel   = tmp_user_gpio_data->mul_sel;
    tmp_sys_gpio_data->user_gpio_status.pull      = tmp_user_gpio_data->pull;
    tmp_sys_gpio_data->user_gpio_status.drv_level = tmp_user_gpio_data->drv_level;
    tmp_sys_gpio_data->user_gpio_status.data      = tmp_user_gpio_data->data;

    port_num_func = (port_num >> 3);
    port_num_pull = (port_num >> 4);

    if((port_num_pull != pre_port_num_pull) || (port != pre_port))    //如果发现当前引脚的端口不一致，或者所在的pull寄存器不一致
    {
      if(func_change)
      {
        *tmp_group_func_addr   = tmp_group_func_data;    //回写功能寄存器
        func_change = 0;
      }
      if(pull_change)
      {
        pull_change = 0;
        *tmp_group_pull_addr   = tmp_group_pull_data;    //回写pull寄存器
      }
      if(dlevel_change)
      {
        dlevel_change = 0;
        *tmp_group_dlevel_addr = tmp_group_dlevel_data;  //回写driver level寄存器
      }
      if(data_change)
      {
        data_change = 0;
        *tmp_group_data_addr   = tmp_group_data_data;    //回写
      }

      tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //更新功能寄存器地址
      tmp_group_pull_addr    = (volatile UINT32 *)GetGroupPullAddrByPort(port, port_num);  //更新pull寄存器
      tmp_group_dlevel_addr  = (volatile UINT32 *)GetGroupDlevelAddrByPort(port, port_num);//更新level寄存器
      tmp_group_data_addr    = (volatile UINT32 *)GetGroupDataAddrByPort(port);  //更新data寄存器

      tmp_group_func_data    = *tmp_group_func_addr;
      tmp_group_pull_data    = *tmp_group_pull_addr;
      tmp_group_dlevel_data  = *tmp_group_dlevel_addr;
      tmp_group_data_data    = *tmp_group_data_addr;

    }
    else if(pre_port_num_func != port_num_func)                       //如果发现当前引脚的功能寄存器不一致
    {
      *tmp_group_func_addr   = tmp_group_func_data;    //则只回写功能寄存器
      tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //更新功能寄存器地址

      tmp_group_func_data    = *tmp_group_func_addr;
    }
    //保存当前硬件寄存器数据
    pre_port_num_pull = port_num_pull;                      //设置当前GPIO成为前一个GPIO
    pre_port_num_func = port_num_func;
    pre_port          = port;

    //更新功能寄存器
    if(tmp_user_gpio_data->mul_sel >= 0)
    {
      tmp_val = (port_num - (port_num_func<<3)) << 2;
      tmp_sys_gpio_data->hardware_gpio_status.mul_sel = (tmp_group_func_data >> tmp_val) & 0x07;
      tmp_group_func_data &= ~(                              0x07  << tmp_val);
      tmp_group_func_data |=  (tmp_user_gpio_data->mul_sel & 0x07) << tmp_val;
      func_change = 1;
    }
    //根据pull的值决定是否更新pull寄存器

    tmp_val = (port_num - (port_num_pull<<4)) << 1;

    if(tmp_user_gpio_data->pull >= 0)
    {
      tmp_sys_gpio_data->hardware_gpio_status.pull = (tmp_group_pull_data >> tmp_val) & 0x03;
      if(tmp_user_gpio_data->pull >= 0)
      {
        tmp_group_pull_data &= ~(                           0x03  << tmp_val);
        tmp_group_pull_data |=  (tmp_user_gpio_data->pull & 0x03) << tmp_val;
        pull_change = 1;
      }
    }
    //根据driver level的值决定是否更新driver level寄存器
    if(tmp_user_gpio_data->drv_level >= 0)
    {
      tmp_sys_gpio_data->hardware_gpio_status.drv_level = (tmp_group_dlevel_data >> tmp_val) & 0x03;
      if(tmp_user_gpio_data->drv_level >= 0)
      {
        tmp_group_dlevel_data &= ~(                                0x03  << tmp_val);
        tmp_group_dlevel_data |=  (tmp_user_gpio_data->drv_level & 0x03) << tmp_val;
        dlevel_change = 1;
      }
    }
    //根据用户输入，以及功能分配决定是否更新data寄存器
    if(tmp_user_gpio_data->mul_sel == 1)
    {
      if(tmp_user_gpio_data->data >= 0)
      {
        tmp_val = tmp_user_gpio_data->data;
        tmp_val &= 1;
        tmp_group_data_data &= ~(1 << port_num);
        tmp_group_data_data |= tmp_val << port_num;
        data_change = 1;
      }
    }
  }
  //for循环结束，如果存在还没有回写的寄存器，这里写回到硬件当中
  if(tmp_group_func_addr)                         //只要更新过寄存器地址，就可以对硬件赋值
  {                                               //那么把所有的值全部回写到硬件寄存器
    *tmp_group_func_addr   = tmp_group_func_data;       //回写功能寄存器
    if(pull_change)
    {
      *tmp_group_pull_addr   = tmp_group_pull_data;    //回写pull寄存器
    }
    if(dlevel_change)
    {
      *tmp_group_dlevel_addr = tmp_group_dlevel_data;  //回写driver level寄存器
    }
    if(data_change)
    {
      *tmp_group_data_addr   = tmp_group_data_data;    //回写data寄存器
    }
  }

  return (UINT32)user_gpio_buf;
}

/*
************************************************************************************************************
*
*                                             CSP_GPIO_Request_EX
*
*    函数名称：
*
*    参数说明: main_name   传进的主键名称，匹配模块(驱动名称)
*
*               sub_name    传进的子键名称，如果是空，表示全部，否则寻找到匹配的单独GPIO
*
*    返回值  ：0 :    err
*              other: success
*
*    说明    ：暂时没有做冲突检查
*
*
************************************************************************************************************
*/
UINT32 gpio_request_ex(CHAR8 *main_name, const CHAR8 *sub_name)  //设备申请GPIO函数扩展接口
{
  user_gpio_set_t    *gpio_list=NULL;
  user_gpio_set_t     one_gpio;
  INT32               gpio_handle;
  INT32               gpio_count;

  if(!sub_name){
    gpio_count = script_parser_mainkey_get_gpio_count(main_name);
    if(gpio_count <= 0)
    {
      /*printf("err: gpio count < =0 ,gpio_count is: %d \n", gpio_count);*/
      return 0;
    }
    gpio_list = (user_gpio_set_t *)AllocatePool (sizeof(system_gpio_set_t) * gpio_count); //申请一片临时内存，用于保存用户数据
    if(!gpio_list){
      /*   printf("AllocatePool  gpio_list error \n");*/
      return 0;
    }
    if(!script_parser_mainkey_get_gpio_cfg(main_name,gpio_list,gpio_count)){
      gpio_handle = gpio_request(gpio_list, gpio_count);
      FreePool(gpio_list);
    }else{
      return 0;
    }
  }else{
    if(script_parser_fetch((CHAR8 *)main_name, (CHAR8 *)sub_name, (INT32 *)&one_gpio, (sizeof(user_gpio_set_t) >> 2)) < 0){
      /* prINT32f("script parser fetch err. \n");*/
      return 0;
    }

    gpio_handle = gpio_request(&one_gpio, 1);
  }

  return gpio_handle;
}

/*
************************************************************************************************************
*
*                                             gpio_request_simple
*
*    函数名称：
*
*    参数说明: main_name   传进的主键名称，匹配模块(驱动名称)
*
*               sub_name    传进的子键名称，如果是空，表示全部，否则寻找到匹配的单独GPIO
*
*    返回值  ：0 :    err
*              other: success
*
*    说明    ：暂时没有做冲突检查
*
*
************************************************************************************************************
*/
INT32 gpio_request_simple(CHAR8 *main_name, const CHAR8 *sub_name)  //设备申请GPIO函数扩展接口
{
  user_gpio_set_t     gpio_list[32];
  INT32               gpio_count;
  INT32 ret = -1;

  if(!sub_name)
  {
    gpio_count = script_parser_mainkey_get_gpio_count(main_name);
    if(gpio_count <= 0)
    {
      DEBUG((DEBUG_INIT ,"err: gpio count < =0 ,gpio_count is: %d \n",gpio_count));
      return -1;
    }
    SetMem(gpio_list,16 * sizeof(user_gpio_set_t),0);
    if(!script_parser_mainkey_get_gpio_cfg(main_name, gpio_list, gpio_count))
    {
      ret = gpio_request_early(gpio_list, gpio_count, 1);
    }
  }
  else
  {
    if(script_parser_fetch((char *)main_name, (char *)sub_name, (int *)gpio_list, (sizeof(user_gpio_set_t) >> 2)) < 0)
    {
      DEBUG((DEBUG_INIT ,"script parser fetch err. \n"));
      return 0;
    }

    ret = gpio_request_early(gpio_list, 1, 1);
  }

  return ret;
}


/*
****************************************************************************************************
*
*             CSP_PIN_DEV_release
*
*  Description:
*       释放某逻辑设备的pin
*
*  Parameters:
*         p_handler    :    handler
*       if_release_to_default_status : 是否释放到原始状态(寄存器原有状态)
*
*  Return value:
*        EGPIO_SUCCESS/EGPIO_FAIL
****************************************************************************************************
*/
INT32 gpio_release(UINT32 p_handler, INT32 if_release_to_default_status)
{
  CHAR8               *tmp_buf;                                        //转换成CHAR8类型
  UINT32               group_count_max, first_port;                    //最大GPIO个数
  system_gpio_set_t  *user_gpio_set, *tmp_sys_gpio_data;
  UINT32               tmp_group_func_data = 0;
  UINT32               tmp_group_pull_data = 0;
  UINT32               tmp_group_dlevel_data = 0;
  volatile UINT32     *tmp_group_func_addr = NULL,   *tmp_group_pull_addr = NULL;
  volatile UINT32     *tmp_group_dlevel_addr = NULL;
  UINT32               port, port_num, port_num_pull, port_num_func;
  UINT32               pre_port = 0x7fffffff, pre_port_num_func = 0x7fffffff, pre_port_num_pull = 0x7fffffff;
  UINT32               i, tmp_val;

  //检查传进的句柄的有效性
  if(!p_handler)
  {
    return EGPIO_FAIL;
  }
  tmp_buf = (CHAR8 *)p_handler;
  group_count_max = *(INT32 *)tmp_buf;
  if(!group_count_max)
  {
    return EGPIO_FAIL;
  }
  if(if_release_to_default_status == 2)
  {
    //SYSCONFIG_DEBUG("gpio module :  release p_handler = %x\n",p_handler);
    FreePool((CHAR8 *)p_handler);

    return EGPIO_SUCCESS;
  }
  user_gpio_set = (system_gpio_set_t *)(tmp_buf + 16);
  //读取用户数据
  for(first_port = 0; first_port < group_count_max; first_port++)
  {
    tmp_sys_gpio_data  = user_gpio_set + first_port;
    port     = tmp_sys_gpio_data->port;                 //读出端口数值
    port_num = tmp_sys_gpio_data->port_num;             //读出端口中的某一个GPIO
    if(!port)
    {
        continue;
    }
    port_num_func = (port_num >> 3);
    port_num_pull = (port_num >> 4);


    tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //更新功能寄存器地址
    tmp_group_pull_addr    = (volatile UINT32 *)GetGroupPullAddrByPort(port, port_num);  //更新pull寄存器
    tmp_group_dlevel_addr  = (volatile UINT32 *)GetGroupDlevelAddrByPort(port, port_num);//更新level寄存器
   

    tmp_group_func_data    = *tmp_group_func_addr;
    tmp_group_pull_data    = *tmp_group_pull_addr;
    tmp_group_dlevel_data  = *tmp_group_dlevel_addr;
    break;
  }
  if(first_port >= group_count_max)
  {
    return 0;
  }
  for(i = first_port; i < group_count_max; i++)
  {
    tmp_sys_gpio_data  = user_gpio_set + i;             //tmp_sys_gpio_data指向申请的GPIO空间
    port     = tmp_sys_gpio_data->port;                 //读出端口数值
    port_num = tmp_sys_gpio_data->port_num;             //读出端口中的某一个GPIO

    port_num_func = (port_num >> 3);
    port_num_pull = (port_num >> 4);

    if((port_num_pull != pre_port_num_pull) || (port != pre_port))    //如果发现当前引脚的端口不一致，或者所在的pull寄存器不一致
    {
      *tmp_group_func_addr   = tmp_group_func_data;    //回写功能寄存器
      *tmp_group_pull_addr   = tmp_group_pull_data;    //回写pull寄存器
      *tmp_group_dlevel_addr = tmp_group_dlevel_data;  //回写driver level寄存器

      tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //更新功能寄存器地址
      tmp_group_pull_addr    = (volatile UINT32 *)GetGroupPullAddrByPort(port, port_num);  //更新pull寄存器
      tmp_group_dlevel_addr  = (volatile UINT32 *)GetGroupDlevelAddrByPort(port, port_num);//更新level寄存器


      tmp_group_func_data    = *tmp_group_func_addr;
      tmp_group_pull_data    = *tmp_group_pull_addr;
      tmp_group_dlevel_data  = *tmp_group_dlevel_addr;
    }
    else if(pre_port_num_func != port_num_func)                       //如果发现当前引脚的功能寄存器不一致
    {
      *tmp_group_func_addr   = tmp_group_func_data;                 //则只回写功能寄存器
      tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //更新功能寄存器地址
      tmp_group_func_data    = *tmp_group_func_addr;
    }

    pre_port_num_pull = port_num_pull;
    pre_port_num_func = port_num_func;
    pre_port          = port;
    //更新功能寄存器
    tmp_group_func_data &= ~(0x07 << ((port_num - (port_num_func<<3)) << 2));
    //更新pull状态寄存器
    tmp_val              =  (port_num - (port_num_pull<<4)) << 1;
    tmp_group_pull_data &= ~(0x03  << tmp_val);
    tmp_group_pull_data |= (tmp_sys_gpio_data->hardware_gpio_status.pull & 0x03) << tmp_val;
    //更新driver状态寄存器
    tmp_val              =  (port_num - (port_num_pull<<4)) << 1;
    tmp_group_dlevel_data &= ~(0x03  << tmp_val);
    tmp_group_dlevel_data |= (tmp_sys_gpio_data->hardware_gpio_status.drv_level & 0x03) << tmp_val;
  }
  if(tmp_group_func_addr)                              //只要更新过寄存器地址，就可以对硬件赋值
  {                                                    //那么把所有的值全部回写到硬件寄存器
    *tmp_group_func_addr   = tmp_group_func_data;    //回写功能寄存器
  }
  if(tmp_group_pull_addr)
  {
    *tmp_group_pull_addr   = tmp_group_pull_data;
  }
  if(tmp_group_dlevel_addr)
  {
    *tmp_group_dlevel_addr = tmp_group_dlevel_data;
  }

  FreePool((CHAR8 *)p_handler);

  return EGPIO_SUCCESS;
}

/*
**********************************************************************************************************************
*                                               CSP_PIN_Get_All_Gpio_Status
*
* Description:
*                获取用户申请过的所有GPIO的状态
* Arguments  :
*        p_handler    :    handler
*        gpio_status    :    保存用户数据的数组
*        gpio_count_max    :    数组最大个数，避免数组越界
*       if_get_user_set_flag   :   读取标志，表示读取用户设定数据或者是实际数据
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
INT32  gpio_get_all_pin_status(UINT32 p_handler, user_gpio_set_t *gpio_status, UINT32 gpio_count_max, UINT32 if_get_from_hardware)
{
  CHAR8               *tmp_buf;                                        //转换成CHAR8类型
  UINT32               group_count_max, first_port;                    //最大GPIO个数
  system_gpio_set_t  *user_gpio_set, *tmp_sys_gpio_data;
  user_gpio_set_t  *script_gpio;
  UINT32               port_num_func, port_num_pull;
  volatile UINT32     *tmp_group_func_addr = NULL, *tmp_group_pull_addr=NULL;
  volatile UINT32     *tmp_group_data_addr=NULL, *tmp_group_dlevel_addr=NULL;
  UINT32               port, port_num;
  UINT32               pre_port = 0x7fffffff, pre_port_num_func = 0x7fffffff, pre_port_num_pull = 0x7fffffff;
  UINT32               i;

  if((!p_handler) || (!gpio_status))
  {
    return EGPIO_FAIL;
  }
  if(gpio_count_max <= 0)
  {
    return EGPIO_FAIL;
  }
  tmp_buf = (CHAR8 *)p_handler;
  group_count_max = *(INT32 *)tmp_buf;
  if(group_count_max <= 0)
  {
    return EGPIO_FAIL;
  }
  user_gpio_set = (system_gpio_set_t *)(tmp_buf + 16);
  if(group_count_max > gpio_count_max)
  {
    group_count_max = gpio_count_max;
  }
  //读取用户数据
  //表示读取用户给定的数据
  if(!if_get_from_hardware)
  {
    for(i = 0; i < group_count_max; i++)
    {
      tmp_sys_gpio_data = user_gpio_set + i;             //tmp_sys_gpio_data指向申请的GPIO空间
      script_gpio       = gpio_status + i;               //script_gpio指向用户传进的空间

      script_gpio->port      = tmp_sys_gpio_data->port;                       //读出port数据
      script_gpio->port_num  = tmp_sys_gpio_data->port_num;                   //读出port_num数据
      script_gpio->pull      = tmp_sys_gpio_data->user_gpio_status.pull;      //读出pull数据
      script_gpio->mul_sel   = tmp_sys_gpio_data->user_gpio_status.mul_sel;   //读出功能数据
      script_gpio->drv_level = tmp_sys_gpio_data->user_gpio_status.drv_level; //读出驱动能力数据
      script_gpio->data      = tmp_sys_gpio_data->user_gpio_status.data;      //读出data数据
      AsciiStrCpy(script_gpio->gpio_name, tmp_sys_gpio_data->gpio_name);
    }
  }
  else
  {
    for(first_port = 0; first_port < group_count_max; first_port++)
    {
      tmp_sys_gpio_data  = user_gpio_set + first_port;
      port     = tmp_sys_gpio_data->port;               //读出端口数值
      port_num = tmp_sys_gpio_data->port_num;           //读出端口中的某一个GPIO

      if(!port)
      {
          continue;
      }
      port_num_func = (port_num >> 3);
      port_num_pull = (port_num >> 4);
      tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //更新功能寄存器地址
      tmp_group_pull_addr    = (volatile UINT32 *)GetGroupPullAddrByPort(port, port_num);  //更新pull寄存器
      tmp_group_dlevel_addr  = (volatile UINT32 *)GetGroupDlevelAddrByPort(port, port_num);//更新level寄存器
      tmp_group_data_addr    = (volatile UINT32 *)GetGroupDataAddrByPort(port);  //更新data寄存器

      break;
    }
    if(first_port >= group_count_max)
    {
      return 0;
    }
    for(i = first_port; i < group_count_max; i++)
    {
      tmp_sys_gpio_data = user_gpio_set + i;             //tmp_sys_gpio_data指向申请的GPIO空间
      script_gpio       = gpio_status + i;               //script_gpio指向用户传进的空间

      port     = tmp_sys_gpio_data->port;                //读出端口数值
      port_num = tmp_sys_gpio_data->port_num;            //读出端口中的某一个GPIO

      script_gpio->port = port;                          //读出port数据
      script_gpio->port_num  = port_num;                 //读出port_num数据
      AsciiStrCpy(script_gpio->gpio_name, tmp_sys_gpio_data->gpio_name);

      port_num_func = (port_num >> 3);
      port_num_pull = (port_num >> 4);

      if((port_num_pull != pre_port_num_pull) || (port != pre_port))    //如果发现当前引脚的端口不一致，或者所在的pull寄存器不一致
      {
        tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //更新功能寄存器地址
        tmp_group_pull_addr    = (volatile UINT32 *)GetGroupPullAddrByPort(port, port_num);  //更新pull寄存器
        tmp_group_dlevel_addr  = (volatile UINT32 *)GetGroupDlevelAddrByPort(port, port_num);//更新level寄存器
        tmp_group_data_addr    = (volatile UINT32 *)GetGroupDataAddrByPort(port);  //更新data寄存器
      }
      else if(pre_port_num_func != port_num_func)                       //如果发现当前引脚的功能寄存器不一致
      {
        tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //更新功能寄存器地址
      }

      pre_port_num_pull = port_num_pull;
      pre_port_num_func = port_num_func;
      pre_port          = port;
      //给用户控件赋值
      script_gpio->pull      = (*tmp_group_pull_addr   >> ((port_num - (port_num_pull<<4))<<1)) & 0x03;    //读出pull数据
      script_gpio->drv_level = (*tmp_group_dlevel_addr >> ((port_num - (port_num_pull<<4))<<1)) & 0x03;    //读出功能数据
      script_gpio->mul_sel   = (*tmp_group_func_addr   >> ((port_num - (port_num_func<<3))<<2)) & 0x07;    //读出功能数据
      if(script_gpio->mul_sel <= 1)
      {
        script_gpio->data  = (*tmp_group_data_addr   >>   port_num) & 0x01;                              //读出data数据
      }
      else
      {
        script_gpio->data = -1;
      }
    }
  }

  return EGPIO_SUCCESS;
}

/*
**********************************************************************************************************************
*                                               CSP_GPIO_Get_One_PIN_Status
*
* Description:
*                获取用户申请过的所有GPIO的状态
* Arguments  :
*        p_handler    :    handler
*        gpio_status    :    保存用户数据的数组
*        gpio_name    :    要操作的GPIO的名称
*       if_get_user_set_flag   :   读取标志，表示读取用户设定数据或者是实际数据
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
INT32  gpio_get_one_pin_status(UINT32 p_handler, user_gpio_set_t *gpio_status, const CHAR8 *gpio_name, UINT32 if_get_from_hardware)
{
  CHAR8               *tmp_buf;                                        //转换成CHAR8类型
  UINT32               group_count_max;                                //最大GPIO个数
  system_gpio_set_t  *user_gpio_set, *tmp_sys_gpio_data;
  UINT32               port_num_func, port_num_pull;
  UINT32               port, port_num;
  UINT32               i, tmp_val1, tmp_val2;

  //检查传进的句柄的有效性
  if((!p_handler) || (!gpio_status))
  {
    return EGPIO_FAIL;
  }
  tmp_buf = (CHAR8 *)p_handler;
  group_count_max = *(INT32 *)tmp_buf;
  if(group_count_max <= 0)
  {
    return EGPIO_FAIL;
  }
  else if((group_count_max > 1) && (!gpio_name))
  {
    return EGPIO_FAIL;
  }
  user_gpio_set = (system_gpio_set_t *)(tmp_buf + 16);
  //读取用户数据
  //表示读取用户给定的数据
  for(i = 0; i < group_count_max; i++)
  {
    tmp_sys_gpio_data = user_gpio_set + i;             //tmp_sys_gpio_data指向申请的GPIO空间
    if(AsciiStrCmp(gpio_name, tmp_sys_gpio_data->gpio_name))
    {
      continue;
    }
    AsciiStrCpy(gpio_status->gpio_name, tmp_sys_gpio_data->gpio_name);
    port                   = tmp_sys_gpio_data->port;
    port_num               = tmp_sys_gpio_data->port_num;
    gpio_status->port      = port;                                              //读出port数据
    gpio_status->port_num  = port_num;                                          //读出port_num数据

    if(!if_get_from_hardware)                                                    //当前要求读出用户设计的数据
    {
      gpio_status->mul_sel   = tmp_sys_gpio_data->user_gpio_status.mul_sel;   //从用户传进数据中读出功能数据
      gpio_status->pull      = tmp_sys_gpio_data->user_gpio_status.pull;      //从用户传进数据中读出pull数据
      gpio_status->drv_level = tmp_sys_gpio_data->user_gpio_status.drv_level; //从用户传进数据中读出驱动能力数据
      gpio_status->data      = tmp_sys_gpio_data->user_gpio_status.data;      //从用户传进数据中读出data数据
    }
    else                                                                        //当前读出寄存器实际的参数
    {
      port_num_func = (port_num >> 3);
      port_num_pull = (port_num >> 4);

      tmp_val1 = ((port_num - (port_num_func << 3)) << 2);
      tmp_val2 = ((port_num - (port_num_pull << 4)) << 1);
      if(port < 12)
      {
        gpio_status->mul_sel   = (PIO_REG_CFG_VALUE(port, port_num_func)>>tmp_val1) & 0x07;       //从硬件中读出功能寄存器
        gpio_status->pull      = (PIO_REG_PULL_VALUE(port, port_num_pull)>>tmp_val2) & 0x03;      //从硬件中读出pull寄存器
        gpio_status->drv_level = (PIO_REG_DLEVEL_VALUE(port, port_num_pull)>>tmp_val2) & 0x03;    //从硬件中读出level寄存器
      }
      else
      {          
        gpio_status->mul_sel   = (R_PIO_REG_CFG_VALUE(port, port_num_func)>>tmp_val1) & 0x07;       //从硬件中读出功能寄存器
        gpio_status->pull      = (R_PIO_REG_PULL_VALUE(port, port_num_pull)>>tmp_val2) & 0x03;      //从硬件中读出pull寄存器
        gpio_status->drv_level = (R_PIO_REG_DLEVEL_VALUE(port, port_num_pull)>>tmp_val2) & 0x03;    //从硬件中读出level寄存器
      }

      if(gpio_status->mul_sel <= 1)
      {
        if(port < 12)
          gpio_status->data = (PIO_REG_DATA_VALUE(port) >> port_num) & 0x01;                     //从硬件中读出data寄存器
        else
          gpio_status->data = (R_PIO_REG_DATA_VALUE(port) >> port_num) & 0x01;                     //从硬件中读出data寄存器             
      }
      else
      {
        gpio_status->data = -1;
      }
    }

    break;
  }

  return EGPIO_SUCCESS;
}

/*
**********************************************************************************************************************
*                                               CSP_PIN_Set_One_Gpio_Status
*
* Description:
*                获取用户申请过的GPIO的某一个的状态
* Arguments  :
*        p_handler    :    handler
*        gpio_status    :    保存用户数据的数组
*        gpio_name    :    要操作的GPIO的名称
*       if_get_user_set_flag   :   读取标志，表示读取用户设定数据或者是实际数据
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/

INT32  gpio_set_one_pin_status(UINT32 p_handler, user_gpio_set_t *gpio_status, const CHAR8 *gpio_name, UINT32 if_set_to_current_input_status)
{
  CHAR8               *tmp_buf;                                        //转换成CHAR8类型
  UINT32               group_count_max;                                //最大GPIO个数
  system_gpio_set_t  *user_gpio_set, *tmp_sys_gpio_data;
  user_gpio_set_t     script_gpio;
  volatile UINT32     *tmp_addr;
  UINT32               port_num_func, port_num_pull;
  UINT32               port, port_num;
  UINT32               i, reg_val, tmp_val;

  //检查传进的句柄的有效性
  if((!p_handler) || (!gpio_name))
  {
    return EGPIO_FAIL;
  }
  if((if_set_to_current_input_status) && (!gpio_status))
  {
    return EGPIO_FAIL;
  }
  tmp_buf = (CHAR8 *)p_handler;
  group_count_max = *(INT32 *)tmp_buf;
  if(group_count_max <= 0)
  {
    return EGPIO_FAIL;
  }
  user_gpio_set = (system_gpio_set_t *)(tmp_buf + 16);
  //读取用户数据
  //表示读取用户给定的数据
  for(i = 0; i < group_count_max; i++)
  {
    tmp_sys_gpio_data = user_gpio_set + i;             //tmp_sys_gpio_data指向申请的GPIO空间
    if(AsciiStrCmp(gpio_name, tmp_sys_gpio_data->gpio_name))
    {
      continue;
    }

    port          = tmp_sys_gpio_data->port;                           //读出port数据
    port_num      = tmp_sys_gpio_data->port_num;                       //读出port_num数据
    port_num_func = (port_num >> 3);
    port_num_pull = (port_num >> 4);

    if(if_set_to_current_input_status)                                 //根据当前用户设定修正
    {
      //修改FUCN寄存器
      script_gpio.mul_sel   = gpio_status->mul_sel;
      script_gpio.pull      = gpio_status->pull;
      script_gpio.drv_level = gpio_status->drv_level;
      script_gpio.data      = gpio_status->data;
    }
    else
    {
      script_gpio.mul_sel   = tmp_sys_gpio_data->user_gpio_status.mul_sel;
      script_gpio.pull      = tmp_sys_gpio_data->user_gpio_status.pull;
      script_gpio.drv_level = tmp_sys_gpio_data->user_gpio_status.drv_level;
      script_gpio.data      = tmp_sys_gpio_data->user_gpio_status.data;
    }

    if(script_gpio.mul_sel >= 0)
    {
      tmp_addr = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);
      reg_val = *tmp_addr;                                                       //修改FUNC寄存器
      tmp_val = (port_num - (port_num_func<<3))<<2;
      reg_val &= ~(0x07 << tmp_val);
      reg_val |=  (script_gpio.mul_sel) << tmp_val;
      *tmp_addr = reg_val;
    }
    //修改PULL寄存器
    if(script_gpio.pull >= 0)
    {
      tmp_addr = (volatile UINT32 *)GetGroupPullAddrByPort(port, port_num);
      reg_val = *tmp_addr;                                                     //修改FUNC寄存器
      tmp_val = (port_num - (port_num_pull<<4))<<1;
      reg_val &= ~(0x03 << tmp_val);
      reg_val |=  (script_gpio.pull) << tmp_val;
      *tmp_addr = reg_val;
    }
    //修改DLEVEL寄存器
    if(script_gpio.drv_level >= 0)
    {
      tmp_addr = (volatile UINT32 *)GetGroupDlevelAddrByPort(port, port_num);
      reg_val = *tmp_addr;                                                         //修改FUNC寄存器
      tmp_val = (port_num - (port_num_pull<<4))<<1;
      reg_val &= ~(0x03 << tmp_val);
      reg_val |=  (script_gpio.drv_level) << tmp_val;
      *tmp_addr = reg_val;
    }
    //修改data寄存器
    if(script_gpio.mul_sel == 1)
    {
      if(script_gpio.data >= 0)
      {
        tmp_addr = (volatile UINT32 *)GetGroupDataAddrByPort(port);
        reg_val = *tmp_addr;                                                      //修改DATA寄存器
        reg_val &= ~(0x01 << port_num);
        reg_val |=  (script_gpio.data & 0x01) << port_num;
        *tmp_addr = reg_val;
      }
    }

    break;
  }

  return EGPIO_SUCCESS;
}

/*
**********************************************************************************************************************
*                                               CSP_GPIO_Set_One_PIN_IO_Status
*
* Description:
*                修改用户申请过的GPIO中的某一个IO口的，输入输出状态
* Arguments  :
*        p_handler    :    handler
*        if_set_to_output_status    :    设置成输出状态还是输入状态
*        gpio_name    :    要操作的GPIO的名称
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
INT32  gpio_set_one_pin_io_status(UINT32 p_handler, UINT32 if_set_to_output_status, const CHAR8 *gpio_name)
{
  CHAR8               *tmp_buf;                                        //转换成CHAR8类型
  UINT32               group_count_max;                                //最大GPIO个数
  system_gpio_set_t  *user_gpio_set = NULL, *tmp_sys_gpio_data;
  volatile UINT32      *tmp_group_func_addr = NULL;
  UINT32               port, port_num, port_num_func;
  UINT32                i, reg_val;

  //检查传进的句柄的有效性
  if(!p_handler)
  {
    return EGPIO_FAIL;
  }
  if(if_set_to_output_status > 1)
  {
    return EGPIO_FAIL;
  }
  tmp_buf = (CHAR8 *)p_handler;
  group_count_max = *(INT32 *)tmp_buf;
  tmp_sys_gpio_data = (system_gpio_set_t *)(tmp_buf + 16);
  if(group_count_max == 0)
  {
    return EGPIO_FAIL;
  }
  else if(group_count_max == 1)
  {
    user_gpio_set = tmp_sys_gpio_data;
  }
  else if(gpio_name)
  {
    for(i=0; i<group_count_max; i++)
    {
      if(AsciiStrCmp(gpio_name, tmp_sys_gpio_data->gpio_name))
      {
        tmp_sys_gpio_data ++;
        continue;
      }
      user_gpio_set = tmp_sys_gpio_data;
      break;
    }
  }
  if(!user_gpio_set)
  {
    return EGPIO_FAIL;
  }

  port     = user_gpio_set->port;
  port_num = user_gpio_set->port_num;
  port_num_func = port_num >> 3;

  tmp_group_func_addr = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);
  reg_val = *tmp_group_func_addr;
  reg_val &= ~(0x07 << (((port_num - (port_num_func<<3))<<2)));
  reg_val |=   if_set_to_output_status << (((port_num - (port_num_func<<3))<<2));
  *tmp_group_func_addr = reg_val;

  return EGPIO_SUCCESS;
}

/*
**********************************************************************************************************************
*                                               CSP_GPIO_Set_One_PIN_Pull
*
* Description:
*                修改用户申请过的GPIO中的某一个IO口的，PULL状态
* Arguments  :
*        p_handler    :    handler
*        if_set_to_output_status    :    所设置的pull状态
*        gpio_name    :    要操作的GPIO的名称
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
INT32  gpio_set_one_pin_pull(UINT32 p_handler, UINT32 set_pull_status, const CHAR8 *gpio_name)
{
  CHAR8               *tmp_buf;                                        //转换成CHAR8类型
  UINT32               group_count_max;                                //最大GPIO个数
  system_gpio_set_t  *user_gpio_set = NULL, *tmp_sys_gpio_data;
  volatile UINT32      *tmp_group_pull_addr = NULL;
  UINT32               port, port_num, port_num_pull;
  UINT32                i, reg_val;
  //检查传进的句柄的有效性
  if(!p_handler)
  {
    return EGPIO_FAIL;
  }
  if(set_pull_status >= 4)
  {
    return EGPIO_FAIL;
  }
  tmp_buf = (CHAR8 *)p_handler;
  group_count_max = *(INT32 *)tmp_buf;
  tmp_sys_gpio_data = (system_gpio_set_t *)(tmp_buf + 16);
  if(group_count_max == 0)
  {
    return EGPIO_FAIL;
  }
  else if(group_count_max == 1)
  {
    user_gpio_set = tmp_sys_gpio_data;
  }
  else if(gpio_name)
  {
    for(i=0; i<group_count_max; i++)
    {
      if(AsciiStrCmp(gpio_name, tmp_sys_gpio_data->gpio_name))
      {
        tmp_sys_gpio_data ++;
        continue;
      }
      user_gpio_set = tmp_sys_gpio_data;
      break;
    }
  }
  if(!user_gpio_set)
  {
    return EGPIO_FAIL;
  }

  port     = user_gpio_set->port;
  port_num = user_gpio_set->port_num;
  port_num_pull = port_num >> 4;

  tmp_group_pull_addr = (volatile UINT32 *)GetGroupPullAddrByPort(port, port_num);
  reg_val = *tmp_group_pull_addr;
  reg_val &= ~(0x03 << (((port_num - (port_num_pull<<4))<<1)));
  reg_val |=  (set_pull_status << (((port_num - (port_num_pull<<4))<<1)));
  *tmp_group_pull_addr = reg_val;

  return EGPIO_SUCCESS;
}

/*
**********************************************************************************************************************
*                                               CSP_GPIO_Set_One_PIN_driver_level
*
* Description:
*                修改用户申请过的GPIO中的某一个IO口的，驱动能力
* Arguments  :
*        p_handler    :    handler
*        if_set_to_output_status    :    所设置的驱动能力等级
*        gpio_name    :    要操作的GPIO的名称
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
INT32  gpio_set_one_pin_driver_level(UINT32 p_handler, UINT32 set_driver_level, const CHAR8 *gpio_name)
{
  CHAR8               *tmp_buf;                                        //转换成CHAR8类型
  UINT32               group_count_max;                                //最大GPIO个数
  system_gpio_set_t  *user_gpio_set = NULL, *tmp_sys_gpio_data;
  volatile UINT32      *tmp_group_dlevel_addr = NULL;
  UINT32               port, port_num, port_num_dlevel;
  UINT32                i, reg_val;
  //检查传进的句柄的有效性
  if(!p_handler)
  {
    return EGPIO_FAIL;
  }
  if(set_driver_level >= 4)
  {
    return EGPIO_FAIL;
  }
  tmp_buf = (CHAR8 *)p_handler;
  group_count_max = *(INT32 *)tmp_buf;
  tmp_sys_gpio_data = (system_gpio_set_t *)(tmp_buf + 16);

  if(group_count_max == 0)
  {
    return EGPIO_FAIL;
  }
  else if(group_count_max == 1)
  {
    user_gpio_set = tmp_sys_gpio_data;
  }
  else if(gpio_name)
  {
    for(i=0; i<group_count_max; i++)
    {
      if(AsciiStrCmp(gpio_name, tmp_sys_gpio_data->gpio_name))
      {
        tmp_sys_gpio_data ++;
        continue;
      }
      user_gpio_set = tmp_sys_gpio_data;
      break;
    }
  }
  if(!user_gpio_set)
  {
    return EGPIO_FAIL;
  }

  port     = user_gpio_set->port;
  port_num = user_gpio_set->port_num;
  port_num_dlevel = port_num >> 4;

  tmp_group_dlevel_addr = (volatile UINT32 *)GetGroupDlevelAddrByPort(port, port_num);
  reg_val = *tmp_group_dlevel_addr;
  reg_val &= ~(0x03 << (((port_num - (port_num_dlevel<<4))<<1)));
  reg_val |=  (set_driver_level << (((port_num - (port_num_dlevel<<4))<<1)));
  *tmp_group_dlevel_addr = reg_val;

  return EGPIO_SUCCESS;
}

/*
**********************************************************************************************************************
*                                               CSP_GPIO_Read_One_PIN_Value
*
* Description:
*                读取用户申请过的GPIO中的某一个IO口的端口的电平
* Arguments  :
*        p_handler    :    handler
*        gpio_name    :    要操作的GPIO的名称
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
INT32  gpio_read_one_pin_value(UINT32 p_handler, const CHAR8 *gpio_name)
{
  CHAR8               *tmp_buf;                                        //转换成CHAR8类型
  UINT32               group_count_max;                                //最大GPIO个数
  system_gpio_set_t  *user_gpio_set = NULL, *tmp_sys_gpio_data;
  UINT32               port, port_num, port_num_func, func_val;
  UINT32                i, reg_val;
  //检查传进的句柄的有效性
  if(!p_handler)
  {
    return EGPIO_FAIL;
  }
  tmp_buf = (CHAR8 *)p_handler;
  group_count_max = *(INT32 *)tmp_buf;
  tmp_sys_gpio_data = (system_gpio_set_t *)(tmp_buf + 16);

  if(group_count_max == 0)
  {
    return EGPIO_FAIL;
  }
  else if(group_count_max == 1)
  {
    user_gpio_set = tmp_sys_gpio_data;
  }
  else if(gpio_name)
  {
    for(i=0; i<group_count_max; i++)
    {
      if(AsciiStrCmp(gpio_name, tmp_sys_gpio_data->gpio_name))
      {
        tmp_sys_gpio_data ++;
        continue;
      }
      user_gpio_set = tmp_sys_gpio_data;
      break;
    }
  }

  if(!user_gpio_set)
  {
    return EGPIO_FAIL;
  }

  port     = user_gpio_set->port;
  port_num = user_gpio_set->port_num;
  port_num_func = port_num >> 3;
  if(port < 12)
    reg_val  = PIO_REG_CFG_VALUE(port, port_num_func);
  else
    reg_val  = R_PIO_REG_CFG_VALUE(port, port_num_func);
  func_val = (reg_val >> ((port_num - (port_num_func<<3))<<2)) & 0x07;

  if(func_val == 0)
  {     
    if(port < 12)
      reg_val = (PIO_REG_DATA_VALUE(port) >> port_num) & 0x01;
    else
      reg_val = (R_PIO_REG_DATA_VALUE(port) >> port_num) & 0x01;

    return reg_val;
  }

  return EGPIO_FAIL;
}

/*
**********************************************************************************************************************
*                                               CSP_GPIO_Write_One_PIN_Value
*
* Description:
*                修改用户申请过的GPIO中的某一个IO口的端口的电平
* Arguments  :
*        p_handler    :    handler
*       value_to_gpio:  要设置的电平的电压
*        gpio_name    :    要操作的GPIO的名称
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
INT32  gpio_write_one_pin_value(UINT32 p_handler, UINT32 value_to_gpio, const CHAR8 *gpio_name)
{
  CHAR8               *tmp_buf;                                        //转换成CHAR8类型
  UINT32               group_count_max;                                //最大GPIO个数
  system_gpio_set_t  *user_gpio_set = NULL, *tmp_sys_gpio_data;
  volatile UINT32     *tmp_group_data_addr = NULL;
  UINT32               port, port_num, port_num_func, func_val;
  UINT32                i, reg_val;
  //检查传进的句柄的有效性
  if(!p_handler)
  {
    return EGPIO_FAIL;
  }
  if(value_to_gpio >= 2)
  {
    return EGPIO_FAIL;
  }
  tmp_buf = (CHAR8 *)p_handler;
  group_count_max = *(INT32 *)tmp_buf;
  tmp_sys_gpio_data = (system_gpio_set_t *)(tmp_buf + 16);

  if(group_count_max == 0)
  {
    return EGPIO_FAIL;
  }
  else if(group_count_max == 1)
  {
    user_gpio_set = tmp_sys_gpio_data;
  }
  else if(gpio_name)
  {
    for(i=0; i<group_count_max; i++)
    {
      if(AsciiStrCmp(gpio_name, tmp_sys_gpio_data->gpio_name))
      {
        tmp_sys_gpio_data ++;
        continue;
      }
      user_gpio_set = tmp_sys_gpio_data;
      break;
    }
  }
  if(!user_gpio_set)
  {
    return EGPIO_FAIL;
  }

  port     = user_gpio_set->port;
  port_num = user_gpio_set->port_num;
  port_num_func = port_num >> 3;
  if(port < 12)
    reg_val  = PIO_REG_CFG_VALUE(port, port_num_func);
  else
    reg_val  = R_PIO_REG_CFG_VALUE(port, port_num_func);

  func_val = (reg_val >> ((port_num - (port_num_func<<3))<<2)) & 0x07;
  if(func_val == 1)
  {
    tmp_group_data_addr = (volatile UINT32 *)GetGroupDataAddrByPort(port);
    reg_val = *tmp_group_data_addr;
    reg_val &= ~(1 << port_num);
    reg_val |=  (value_to_gpio << port_num);
    *tmp_group_data_addr = reg_val;

    return EGPIO_SUCCESS;
  }

  return EGPIO_FAIL;
}
#endif

#define SUNXI_SCRIPT_TEST 0
#if SUNXI_SCRIPT_TEST

#define TEST_MAIN_KEY "pmu_para"
#define TEST_SUB_KEY1 "pmu_used"
#define TEST_SUB_KEY2 "pmu_twi_addr"
#define TEST_SUB_KEY3 "pmu_twi_id"
#define TEST_SUB_KEY4 "pmu_battery_rdc"

EFI_STATUS 
EFIAPI 
SunxiScriptTest(void)
{
  INT32 MainKeyCount;
  UINT32 SubKeyCount;
  INT32 value[1];

  MainKeyCount = script_parser_mainkey_count();

  DEBUG((DEBUG_INIT ,"++TestTotalMainkeyCount is %d\n",MainKeyCount));

  SubKeyCount = script_parser_subkey_count(TEST_MAIN_KEY);
  DEBUG((DEBUG_INIT ,"++mainkey: %a,SubkeyCount is %d\n",TEST_MAIN_KEY,SubKeyCount));

  script_parser_fetch(TEST_MAIN_KEY,TEST_SUB_KEY1,value,1);
  DEBUG((DEBUG_INIT ,"++%a is %d\n",TEST_SUB_KEY1,value[0]));

  script_parser_fetch(TEST_MAIN_KEY,TEST_SUB_KEY2,value,1);
  DEBUG((DEBUG_INIT ,"++%a is 0x%x\n",TEST_SUB_KEY2,value[0]));

  script_parser_fetch(TEST_MAIN_KEY,TEST_SUB_KEY3,value,1);
  DEBUG((DEBUG_INIT ,"++%a is %d\n",TEST_SUB_KEY3,value[0]));

  script_parser_fetch(TEST_MAIN_KEY,TEST_SUB_KEY4,value,1);
  DEBUG((DEBUG_INIT ,"++%a is %d\n",TEST_SUB_KEY4,value[0]));

  return EFI_SUCCESS;

}
#endif
/**
  Initialize the state information for the RngDxe

  @param  ImageHandle   of the loaded driver
  @param  SystemTable   PoINT32er to the System Table

  @retval EFI_SUCCESS           Protocol registered
  @retval EFI_OUT_OF_RESOURCES  Cannot allocate protocol data structure
  @retval EFI_DEVICE_ERROR      Hardware problems

**/
EFI_STATUS
EFIAPI
SysConfigConstructor (
VOID
  )
{

  SUNXI_SCRIPT_PARSE_HOB      *GuidHob;
  
  EFI_STATUS Status = EFI_SUCCESS;
  
  DEBUG((DEBUG_INFO, "++%a\n",__func__));
  
  GuidHob = GetFirstGuidHob (&gSunxiScriptParseHobGuid);
  
  if (GuidHob == NULL) {
    DEBUG((DEBUG_WARN, "--%s:No Suxi Script Parse Guid found, this protocol will not install. \n",__FUNCTION__));
    return Status;
  }
  
  DEBUG((DEBUG_INFO, "--%a:SunxiScriptParseBase=%lx\n",__func__,GuidHob->SunxiScriptParseBase));
  DEBUG((DEBUG_INFO, "--%a:SunxiScriptParseSize=%x\n",__func__,GuidHob->SunxiScriptParseSize));

  Status = script_parser_init((CHAR8*)(INTN)(GuidHob->SunxiScriptParseBase));

  #if SUNXI_SCRIPT_TEST
  SunxiScriptTest();
  #endif
  
  return Status;
}
