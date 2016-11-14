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

  //���ű�buffer�Ƿ����
  if(!script_mod_buf)
  {
    return SCRIPT_PARSER_EMPTY_BUFFER;
  }
  //����������ƺ��Ӽ������Ƿ�Ϊ��
  if((main_name == NULL) || (sub_name == NULL))
  {
    return SCRIPT_PARSER_KEYNAME_NULL;
  }
  //�������buffer�Ƿ�Ϊ��
  if(str == NULL)
  {
    return SCRIPT_PARSER_DATA_VALUE_NULL;
  }
  //�����������ƺ��Ӽ����ƣ��������31�ֽ����ȡ31�ֽ�
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
    if(AsciiStrCmp(main_key->main_name, main_CHAR8))    //���������ƥ�䣬Ѱ����һ������
    {
      continue;
    }
    //����ƥ�䣬Ѱ���Ӽ�����ƥ��
    for(j=0;j<main_key->lenth;j++)
    {
      sub_key = (script_sub_key_t *)(script_mod_buf + (main_key->offset<<2) + (j * sizeof(script_sub_key_t)));
      if(AsciiStrCmp(sub_key->sub_name, sub_CHAR8))    //���������ƥ�䣬Ѱ����һ������
      {
        continue;
      }
      pattern    = (sub_key->pattern>>16) & 0xffff;             //��ȡ���ݵ�����
      word_count = (sub_key->pattern>> 0) & 0xffff;             //��ȡ��ռ�õ�word����
      //ȡ������
      if(pattern == SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD)                      //��word��������
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
    return PIO_REG_CFG(port, (port_num>>3));   //���¹��ܼĴ�����ַ
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
    return PIO_REG_PULL(port, (port_num>>4));  //����pull�Ĵ���
  }
  else
  {   
    return R_PIO_REG_PULL(port, (port_num>>4));  //����pull�Ĵ���
  }
}

UINT32 GetGroupDlevelAddrByPort(INT32 port,INT32 port_num)
{
  if(port < 12)
  {
    return PIO_REG_DLEVEL(port, (port_num>>4));//����level�Ĵ���
  }
  else
  {
    return R_PIO_REG_DLEVEL(port, (port_num>>4));//����level�Ĵ���
  }
}

UINT32 GetGroupDataAddrByPort(INT32 port)
{
  if(port < 12)
  {
    return PIO_REG_DATA(port);                 //����data�Ĵ���
  }
  else
  {
    return R_PIO_REG_DATA(port);               //����data�Ĵ���
  }
}


#if 1
/*
************************************************************************************************************
*
*                                             normal_gpio_cfg
*
*    �������ƣ�
*
*    �����б�
*
*
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
INT32 gpio_request_early(void  *user_gpio_list, UINT32 group_count_max, INT32 set_gpio)
{
  user_gpio_set_t    *tmp_user_gpio_data, *gpio_list;
  UINT32         first_port=0;                      //����������Ч��GPIO�ĸ���
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

    //׼����һ��GPIO����
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
    port     = tmp_user_gpio_data->port;                         //�����˿���ֵ
    port_num = tmp_user_gpio_data->port_num;                     //�����˿��е�ĳһ��GPIO
    if(!port)
    {
      continue;
    }
          
    port_num_func = (port_num >> 3);
    port_num_pull = (port_num >> 4);

    tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //���¹��ܼĴ�����ַ
    tmp_group_pull_addr    = (volatile UINT32 *)GetGroupPullAddrByPort(port, port_num);  //����pull�Ĵ���
    tmp_group_dlevel_addr  = (volatile UINT32 *)GetGroupDlevelAddrByPort(port, port_num);//����level�Ĵ���
    tmp_group_data_addr    = (volatile UINT32 *)GetGroupDataAddrByPort(port);  //����data�Ĵ���
  
    tmp_group_func_data    = GPIO_REG_READ(tmp_group_func_addr);
    tmp_group_pull_data    = GPIO_REG_READ(tmp_group_pull_addr);
    tmp_group_dlevel_data  = GPIO_REG_READ(tmp_group_dlevel_addr);
    tmp_group_data_data    = GPIO_REG_READ(tmp_group_data_addr);

    /*printf("func_addr:%x,pull_addr:%x,level_addr:%x,data_addr%x\n",tmp_group_func_data,tmp_group_pull_data,tmp_group_dlevel_data,tmp_group_data_data);*/
    pre_port          = port;
    pre_port_num_func = port_num_func;
    pre_port_num_pull = port_num_pull;
    //���¹��ܼĴ���
    tmp_val = (port_num - (port_num_func << 3)) << 2;
    tmp_group_func_data &= ~(                              0x07  << tmp_val);
    if(set_gpio)
    {
      tmp_group_func_data |=  (tmp_user_gpio_data->mul_sel & 0x07) << tmp_val;
    }
    //����pull��ֵ�����Ƿ����pull�Ĵ���
    tmp_val              =  (port_num - (port_num_pull << 4)) << 1;
    if(tmp_user_gpio_data->pull >= 0)
    {
      tmp_group_pull_data &= ~(                           0x03  << tmp_val);
      tmp_group_pull_data |=  (tmp_user_gpio_data->pull & 0x03) << tmp_val;
    }
    //����driver level��ֵ�����Ƿ����driver level�Ĵ���
    if(tmp_user_gpio_data->drv_level >= 0)
    {
      tmp_group_dlevel_data &= ~(                                0x03  << tmp_val);
      tmp_group_dlevel_data |=  (tmp_user_gpio_data->drv_level & 0x03) << tmp_val;
    }
    //�����û����룬�Լ����ܷ�������Ƿ����data�Ĵ���
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
  //����Ƿ������ݴ���
  if(first_port >= group_count_max)
  {
    return -1;
  }
  //�����û�����
  for(i = first_port + 1; i < group_count_max; i++)
  {
    tmp_user_gpio_data = gpio_list + i;                 //gpio_set����ָ���û���ÿ��GPIO�����Ա
    port     = tmp_user_gpio_data->port;                //�����˿���ֵ
    port_num = tmp_user_gpio_data->port_num;            //�����˿��е�ĳһ��GPIO
    if(!port)
    {
      break;
    }
 
    port_num_func = (port_num >> 3);
    port_num_pull = (port_num >> 4);

    if((port_num_pull != pre_port_num_pull) || (port != pre_port))    //������ֵ�ǰ���ŵĶ˿ڲ�һ�£��������ڵ�pull�Ĵ�����һ��
    {
      GPIO_REG_WRITE(tmp_group_func_addr, tmp_group_func_data);     //��д���ܼĴ���
      GPIO_REG_WRITE(tmp_group_pull_addr, tmp_group_pull_data);     //��дpull�Ĵ���
      GPIO_REG_WRITE(tmp_group_dlevel_addr, tmp_group_dlevel_data); //��дdriver level�Ĵ���
      if(data_change)
      {
        data_change = 0;
        GPIO_REG_WRITE(tmp_group_data_addr, tmp_group_data_data); //��дdata�Ĵ���
      }

      tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //���¹��ܼĴ�����ַ
      tmp_group_pull_addr    = (volatile UINT32 *)GetGroupPullAddrByPort(port, port_num);  //����pull�Ĵ���
      tmp_group_dlevel_addr  = (volatile UINT32 *)GetGroupDlevelAddrByPort(port, port_num);//����level�Ĵ���
      tmp_group_data_addr    = (volatile UINT32 *)GetGroupDataAddrByPort(port);  //����data�Ĵ���


      tmp_group_func_data    = GPIO_REG_READ(tmp_group_func_addr);
      tmp_group_pull_data    = GPIO_REG_READ(tmp_group_pull_addr);
      tmp_group_dlevel_data  = GPIO_REG_READ(tmp_group_dlevel_addr);
      tmp_group_data_data    = GPIO_REG_READ(tmp_group_data_addr);        
    }
    else if(pre_port_num_func != port_num_func)                       //������ֵ�ǰ���ŵĹ��ܼĴ�����һ��
    {
      GPIO_REG_WRITE(tmp_group_func_addr, tmp_group_func_data);    //��ֻ��д���ܼĴ���
      tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //���¹��ܼĴ�����ַ

      tmp_group_func_data    = GPIO_REG_READ(tmp_group_func_addr);
    }
    //���浱ǰӲ���Ĵ�������
    pre_port_num_pull = port_num_pull;                      //���õ�ǰGPIO��Ϊǰһ��GPIO
    pre_port_num_func = port_num_func;
    pre_port          = port;

    //���¹��ܼĴ���
    tmp_val = (port_num - (port_num_func << 3)) << 2;
    if(tmp_user_gpio_data->mul_sel >= 0)
    {
      tmp_group_func_data &= ~(                              0x07  << tmp_val);
      if(set_gpio)
      {
        tmp_group_func_data |=  (tmp_user_gpio_data->mul_sel & 0x07) << tmp_val;
      }
    }
    //����pull��ֵ�����Ƿ����pull�Ĵ���
    tmp_val              =  (port_num - (port_num_pull << 4)) << 1;
    if(tmp_user_gpio_data->pull >= 0)
    {
      tmp_group_pull_data &= ~(                           0x03  << tmp_val);
      tmp_group_pull_data |=  (tmp_user_gpio_data->pull & 0x03) << tmp_val;
    }
    //����driver level��ֵ�����Ƿ����driver level�Ĵ���
    if(tmp_user_gpio_data->drv_level >= 0)
    {
      tmp_group_dlevel_data &= ~(                                0x03  << tmp_val);
      tmp_group_dlevel_data |=  (tmp_user_gpio_data->drv_level & 0x03) << tmp_val;
    }
    //�����û����룬�Լ����ܷ�������Ƿ����data�Ĵ���
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
  //forѭ��������������ڻ�û�л�д�ļĴ���������д�ص�Ӳ������
  if(tmp_group_func_addr)                         //ֻҪ���¹��Ĵ�����ַ���Ϳ��Զ�Ӳ����ֵ
  {                                               //��ô�����е�ֵȫ����д��Ӳ���Ĵ���
    GPIO_REG_WRITE(tmp_group_func_addr,   tmp_group_func_data);   //��д���ܼĴ���
    GPIO_REG_WRITE(tmp_group_pull_addr,   tmp_group_pull_data);   //��дpull�Ĵ���
    GPIO_REG_WRITE(tmp_group_dlevel_addr, tmp_group_dlevel_data); //��дdriver level�Ĵ���
    if(data_change)
    {
      GPIO_REG_WRITE(tmp_group_data_addr, tmp_group_data_data); //��дdata�Ĵ���
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
*    �������ƣ�
*
*    �����б�gpio_list      ��������õ���GPIO���ݵ����飬GPIO��ֱ��ʹ���������
*
*               group_count_max  ����ĳ�Ա������GPIO�趨��ʱ�򣬽�������GPIO��󲻳������ֵ
*
*    ����ֵ  ��
*
*    ˵��    ����ʱû������ͻ���
*
*
************************************************************************************************************
*/
UINT32 gpio_request(user_gpio_set_t *gpio_list, UINT32 group_count_max)
{
  CHAR8               *user_gpio_buf;                                        //����CHAR8��������
  system_gpio_set_t  *user_gpio_set, *tmp_sys_gpio_data;                      //user_gpio_set���������ڴ�ľ��
  user_gpio_set_t  *tmp_user_gpio_data;
  UINT32                real_gpio_count = 0, first_port;                      //����������Ч��GPIO�ĸ���
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
    tmp_user_gpio_data = gpio_list + i;                 //gpio_set����ָ��ÿ��GPIO�����Ա
    if(!tmp_user_gpio_data->port)
    {
        continue;
    }
    real_gpio_count ++;
  }

  //SYSCONFIG_DEBUG("to AllocatePool  space for pin \n");
  user_gpio_buf = (CHAR8 *)AllocatePool (16 + sizeof(system_gpio_set_t) * real_gpio_count);   //�����ڴ棬������16���ֽڣ����ڴ��GPIO��������Ϣ
  if(!user_gpio_buf)
  {
    return (UINT32)0;
  }
  SetMem(user_gpio_buf,16 + sizeof(system_gpio_set_t) * real_gpio_count,0);         //����ȫ������
  *(INT32 *)user_gpio_buf = real_gpio_count;                                           //������Ч��GPIO����
  user_gpio_set = (system_gpio_set_t *)(user_gpio_buf + 16);                         //ָ���һ���ṹ��
  //׼����һ��GPIO����
  for(first_port = 0; first_port < group_count_max; first_port++)
  {
    tmp_user_gpio_data = gpio_list + first_port;
    port     = tmp_user_gpio_data->port;                         //�����˿���ֵ
    port_num = tmp_user_gpio_data->port_num;                     //�����˿��е�ĳһ��GPIO
    if(!port)
    {
      continue;
    }
    port_num_func = (port_num >> 3);
    port_num_pull = (port_num >> 4);

    tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //���¹��ܼĴ�����ַ
    tmp_group_pull_addr    = (volatile UINT32 *)GetGroupPullAddrByPort(port, port_num);  //����pull�Ĵ���
    tmp_group_dlevel_addr  = (volatile UINT32 *)GetGroupDlevelAddrByPort(port, port_num);//����level�Ĵ���
    tmp_group_data_addr    = (volatile UINT32 *)GetGroupDataAddrByPort(port);  //����data�Ĵ���

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
  //�����û�����
  for(i = first_port; i < group_count_max; i++)
  {
    tmp_sys_gpio_data  = user_gpio_set + i;             //tmp_sys_gpio_dataָ�������GPIO�ռ�
    tmp_user_gpio_data = gpio_list + i;                 //gpio_set����ָ���û���ÿ��GPIO�����Ա
    port     = tmp_user_gpio_data->port;                //�����˿���ֵ
    port_num = tmp_user_gpio_data->port_num;            //�����˿��е�ĳһ��GPIO
    if(!port)
    {
      continue;
    }
    //��ʼ�����û�����
    AsciiStrCpy(tmp_sys_gpio_data->gpio_name, tmp_user_gpio_data->gpio_name);
    tmp_sys_gpio_data->port                       = port;
    tmp_sys_gpio_data->port_num                   = port_num;
    tmp_sys_gpio_data->user_gpio_status.mul_sel   = tmp_user_gpio_data->mul_sel;
    tmp_sys_gpio_data->user_gpio_status.pull      = tmp_user_gpio_data->pull;
    tmp_sys_gpio_data->user_gpio_status.drv_level = tmp_user_gpio_data->drv_level;
    tmp_sys_gpio_data->user_gpio_status.data      = tmp_user_gpio_data->data;

    port_num_func = (port_num >> 3);
    port_num_pull = (port_num >> 4);

    if((port_num_pull != pre_port_num_pull) || (port != pre_port))    //������ֵ�ǰ���ŵĶ˿ڲ�һ�£��������ڵ�pull�Ĵ�����һ��
    {
      if(func_change)
      {
        *tmp_group_func_addr   = tmp_group_func_data;    //��д���ܼĴ���
        func_change = 0;
      }
      if(pull_change)
      {
        pull_change = 0;
        *tmp_group_pull_addr   = tmp_group_pull_data;    //��дpull�Ĵ���
      }
      if(dlevel_change)
      {
        dlevel_change = 0;
        *tmp_group_dlevel_addr = tmp_group_dlevel_data;  //��дdriver level�Ĵ���
      }
      if(data_change)
      {
        data_change = 0;
        *tmp_group_data_addr   = tmp_group_data_data;    //��д
      }

      tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //���¹��ܼĴ�����ַ
      tmp_group_pull_addr    = (volatile UINT32 *)GetGroupPullAddrByPort(port, port_num);  //����pull�Ĵ���
      tmp_group_dlevel_addr  = (volatile UINT32 *)GetGroupDlevelAddrByPort(port, port_num);//����level�Ĵ���
      tmp_group_data_addr    = (volatile UINT32 *)GetGroupDataAddrByPort(port);  //����data�Ĵ���

      tmp_group_func_data    = *tmp_group_func_addr;
      tmp_group_pull_data    = *tmp_group_pull_addr;
      tmp_group_dlevel_data  = *tmp_group_dlevel_addr;
      tmp_group_data_data    = *tmp_group_data_addr;

    }
    else if(pre_port_num_func != port_num_func)                       //������ֵ�ǰ���ŵĹ��ܼĴ�����һ��
    {
      *tmp_group_func_addr   = tmp_group_func_data;    //��ֻ��д���ܼĴ���
      tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //���¹��ܼĴ�����ַ

      tmp_group_func_data    = *tmp_group_func_addr;
    }
    //���浱ǰӲ���Ĵ�������
    pre_port_num_pull = port_num_pull;                      //���õ�ǰGPIO��Ϊǰһ��GPIO
    pre_port_num_func = port_num_func;
    pre_port          = port;

    //���¹��ܼĴ���
    if(tmp_user_gpio_data->mul_sel >= 0)
    {
      tmp_val = (port_num - (port_num_func<<3)) << 2;
      tmp_sys_gpio_data->hardware_gpio_status.mul_sel = (tmp_group_func_data >> tmp_val) & 0x07;
      tmp_group_func_data &= ~(                              0x07  << tmp_val);
      tmp_group_func_data |=  (tmp_user_gpio_data->mul_sel & 0x07) << tmp_val;
      func_change = 1;
    }
    //����pull��ֵ�����Ƿ����pull�Ĵ���

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
    //����driver level��ֵ�����Ƿ����driver level�Ĵ���
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
    //�����û����룬�Լ����ܷ�������Ƿ����data�Ĵ���
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
  //forѭ��������������ڻ�û�л�д�ļĴ���������д�ص�Ӳ������
  if(tmp_group_func_addr)                         //ֻҪ���¹��Ĵ�����ַ���Ϳ��Զ�Ӳ����ֵ
  {                                               //��ô�����е�ֵȫ����д��Ӳ���Ĵ���
    *tmp_group_func_addr   = tmp_group_func_data;       //��д���ܼĴ���
    if(pull_change)
    {
      *tmp_group_pull_addr   = tmp_group_pull_data;    //��дpull�Ĵ���
    }
    if(dlevel_change)
    {
      *tmp_group_dlevel_addr = tmp_group_dlevel_data;  //��дdriver level�Ĵ���
    }
    if(data_change)
    {
      *tmp_group_data_addr   = tmp_group_data_data;    //��дdata�Ĵ���
    }
  }

  return (UINT32)user_gpio_buf;
}

/*
************************************************************************************************************
*
*                                             CSP_GPIO_Request_EX
*
*    �������ƣ�
*
*    ����˵��: main_name   �������������ƣ�ƥ��ģ��(��������)
*
*               sub_name    �������Ӽ����ƣ�����ǿգ���ʾȫ��������Ѱ�ҵ�ƥ��ĵ���GPIO
*
*    ����ֵ  ��0 :    err
*              other: success
*
*    ˵��    ����ʱû������ͻ���
*
*
************************************************************************************************************
*/
UINT32 gpio_request_ex(CHAR8 *main_name, const CHAR8 *sub_name)  //�豸����GPIO������չ�ӿ�
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
    gpio_list = (user_gpio_set_t *)AllocatePool (sizeof(system_gpio_set_t) * gpio_count); //����һƬ��ʱ�ڴ棬���ڱ����û�����
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
*    �������ƣ�
*
*    ����˵��: main_name   �������������ƣ�ƥ��ģ��(��������)
*
*               sub_name    �������Ӽ����ƣ�����ǿգ���ʾȫ��������Ѱ�ҵ�ƥ��ĵ���GPIO
*
*    ����ֵ  ��0 :    err
*              other: success
*
*    ˵��    ����ʱû������ͻ���
*
*
************************************************************************************************************
*/
INT32 gpio_request_simple(CHAR8 *main_name, const CHAR8 *sub_name)  //�豸����GPIO������չ�ӿ�
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
*       �ͷ�ĳ�߼��豸��pin
*
*  Parameters:
*         p_handler    :    handler
*       if_release_to_default_status : �Ƿ��ͷŵ�ԭʼ״̬(�Ĵ���ԭ��״̬)
*
*  Return value:
*        EGPIO_SUCCESS/EGPIO_FAIL
****************************************************************************************************
*/
INT32 gpio_release(UINT32 p_handler, INT32 if_release_to_default_status)
{
  CHAR8               *tmp_buf;                                        //ת����CHAR8����
  UINT32               group_count_max, first_port;                    //���GPIO����
  system_gpio_set_t  *user_gpio_set, *tmp_sys_gpio_data;
  UINT32               tmp_group_func_data = 0;
  UINT32               tmp_group_pull_data = 0;
  UINT32               tmp_group_dlevel_data = 0;
  volatile UINT32     *tmp_group_func_addr = NULL,   *tmp_group_pull_addr = NULL;
  volatile UINT32     *tmp_group_dlevel_addr = NULL;
  UINT32               port, port_num, port_num_pull, port_num_func;
  UINT32               pre_port = 0x7fffffff, pre_port_num_func = 0x7fffffff, pre_port_num_pull = 0x7fffffff;
  UINT32               i, tmp_val;

  //��鴫���ľ������Ч��
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
  //��ȡ�û�����
  for(first_port = 0; first_port < group_count_max; first_port++)
  {
    tmp_sys_gpio_data  = user_gpio_set + first_port;
    port     = tmp_sys_gpio_data->port;                 //�����˿���ֵ
    port_num = tmp_sys_gpio_data->port_num;             //�����˿��е�ĳһ��GPIO
    if(!port)
    {
        continue;
    }
    port_num_func = (port_num >> 3);
    port_num_pull = (port_num >> 4);


    tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //���¹��ܼĴ�����ַ
    tmp_group_pull_addr    = (volatile UINT32 *)GetGroupPullAddrByPort(port, port_num);  //����pull�Ĵ���
    tmp_group_dlevel_addr  = (volatile UINT32 *)GetGroupDlevelAddrByPort(port, port_num);//����level�Ĵ���
   

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
    tmp_sys_gpio_data  = user_gpio_set + i;             //tmp_sys_gpio_dataָ�������GPIO�ռ�
    port     = tmp_sys_gpio_data->port;                 //�����˿���ֵ
    port_num = tmp_sys_gpio_data->port_num;             //�����˿��е�ĳһ��GPIO

    port_num_func = (port_num >> 3);
    port_num_pull = (port_num >> 4);

    if((port_num_pull != pre_port_num_pull) || (port != pre_port))    //������ֵ�ǰ���ŵĶ˿ڲ�һ�£��������ڵ�pull�Ĵ�����һ��
    {
      *tmp_group_func_addr   = tmp_group_func_data;    //��д���ܼĴ���
      *tmp_group_pull_addr   = tmp_group_pull_data;    //��дpull�Ĵ���
      *tmp_group_dlevel_addr = tmp_group_dlevel_data;  //��дdriver level�Ĵ���

      tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //���¹��ܼĴ�����ַ
      tmp_group_pull_addr    = (volatile UINT32 *)GetGroupPullAddrByPort(port, port_num);  //����pull�Ĵ���
      tmp_group_dlevel_addr  = (volatile UINT32 *)GetGroupDlevelAddrByPort(port, port_num);//����level�Ĵ���


      tmp_group_func_data    = *tmp_group_func_addr;
      tmp_group_pull_data    = *tmp_group_pull_addr;
      tmp_group_dlevel_data  = *tmp_group_dlevel_addr;
    }
    else if(pre_port_num_func != port_num_func)                       //������ֵ�ǰ���ŵĹ��ܼĴ�����һ��
    {
      *tmp_group_func_addr   = tmp_group_func_data;                 //��ֻ��д���ܼĴ���
      tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //���¹��ܼĴ�����ַ
      tmp_group_func_data    = *tmp_group_func_addr;
    }

    pre_port_num_pull = port_num_pull;
    pre_port_num_func = port_num_func;
    pre_port          = port;
    //���¹��ܼĴ���
    tmp_group_func_data &= ~(0x07 << ((port_num - (port_num_func<<3)) << 2));
    //����pull״̬�Ĵ���
    tmp_val              =  (port_num - (port_num_pull<<4)) << 1;
    tmp_group_pull_data &= ~(0x03  << tmp_val);
    tmp_group_pull_data |= (tmp_sys_gpio_data->hardware_gpio_status.pull & 0x03) << tmp_val;
    //����driver״̬�Ĵ���
    tmp_val              =  (port_num - (port_num_pull<<4)) << 1;
    tmp_group_dlevel_data &= ~(0x03  << tmp_val);
    tmp_group_dlevel_data |= (tmp_sys_gpio_data->hardware_gpio_status.drv_level & 0x03) << tmp_val;
  }
  if(tmp_group_func_addr)                              //ֻҪ���¹��Ĵ�����ַ���Ϳ��Զ�Ӳ����ֵ
  {                                                    //��ô�����е�ֵȫ����д��Ӳ���Ĵ���
    *tmp_group_func_addr   = tmp_group_func_data;    //��д���ܼĴ���
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
*                ��ȡ�û������������GPIO��״̬
* Arguments  :
*        p_handler    :    handler
*        gpio_status    :    �����û����ݵ�����
*        gpio_count_max    :    ��������������������Խ��
*       if_get_user_set_flag   :   ��ȡ��־����ʾ��ȡ�û��趨���ݻ�����ʵ������
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
INT32  gpio_get_all_pin_status(UINT32 p_handler, user_gpio_set_t *gpio_status, UINT32 gpio_count_max, UINT32 if_get_from_hardware)
{
  CHAR8               *tmp_buf;                                        //ת����CHAR8����
  UINT32               group_count_max, first_port;                    //���GPIO����
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
  //��ȡ�û�����
  //��ʾ��ȡ�û�����������
  if(!if_get_from_hardware)
  {
    for(i = 0; i < group_count_max; i++)
    {
      tmp_sys_gpio_data = user_gpio_set + i;             //tmp_sys_gpio_dataָ�������GPIO�ռ�
      script_gpio       = gpio_status + i;               //script_gpioָ���û������Ŀռ�

      script_gpio->port      = tmp_sys_gpio_data->port;                       //����port����
      script_gpio->port_num  = tmp_sys_gpio_data->port_num;                   //����port_num����
      script_gpio->pull      = tmp_sys_gpio_data->user_gpio_status.pull;      //����pull����
      script_gpio->mul_sel   = tmp_sys_gpio_data->user_gpio_status.mul_sel;   //������������
      script_gpio->drv_level = tmp_sys_gpio_data->user_gpio_status.drv_level; //����������������
      script_gpio->data      = tmp_sys_gpio_data->user_gpio_status.data;      //����data����
      AsciiStrCpy(script_gpio->gpio_name, tmp_sys_gpio_data->gpio_name);
    }
  }
  else
  {
    for(first_port = 0; first_port < group_count_max; first_port++)
    {
      tmp_sys_gpio_data  = user_gpio_set + first_port;
      port     = tmp_sys_gpio_data->port;               //�����˿���ֵ
      port_num = tmp_sys_gpio_data->port_num;           //�����˿��е�ĳһ��GPIO

      if(!port)
      {
          continue;
      }
      port_num_func = (port_num >> 3);
      port_num_pull = (port_num >> 4);
      tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //���¹��ܼĴ�����ַ
      tmp_group_pull_addr    = (volatile UINT32 *)GetGroupPullAddrByPort(port, port_num);  //����pull�Ĵ���
      tmp_group_dlevel_addr  = (volatile UINT32 *)GetGroupDlevelAddrByPort(port, port_num);//����level�Ĵ���
      tmp_group_data_addr    = (volatile UINT32 *)GetGroupDataAddrByPort(port);  //����data�Ĵ���

      break;
    }
    if(first_port >= group_count_max)
    {
      return 0;
    }
    for(i = first_port; i < group_count_max; i++)
    {
      tmp_sys_gpio_data = user_gpio_set + i;             //tmp_sys_gpio_dataָ�������GPIO�ռ�
      script_gpio       = gpio_status + i;               //script_gpioָ���û������Ŀռ�

      port     = tmp_sys_gpio_data->port;                //�����˿���ֵ
      port_num = tmp_sys_gpio_data->port_num;            //�����˿��е�ĳһ��GPIO

      script_gpio->port = port;                          //����port����
      script_gpio->port_num  = port_num;                 //����port_num����
      AsciiStrCpy(script_gpio->gpio_name, tmp_sys_gpio_data->gpio_name);

      port_num_func = (port_num >> 3);
      port_num_pull = (port_num >> 4);

      if((port_num_pull != pre_port_num_pull) || (port != pre_port))    //������ֵ�ǰ���ŵĶ˿ڲ�һ�£��������ڵ�pull�Ĵ�����һ��
      {
        tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //���¹��ܼĴ�����ַ
        tmp_group_pull_addr    = (volatile UINT32 *)GetGroupPullAddrByPort(port, port_num);  //����pull�Ĵ���
        tmp_group_dlevel_addr  = (volatile UINT32 *)GetGroupDlevelAddrByPort(port, port_num);//����level�Ĵ���
        tmp_group_data_addr    = (volatile UINT32 *)GetGroupDataAddrByPort(port);  //����data�Ĵ���
      }
      else if(pre_port_num_func != port_num_func)                       //������ֵ�ǰ���ŵĹ��ܼĴ�����һ��
      {
        tmp_group_func_addr    = (volatile UINT32 *)GetGroupFuncAddrByPort(port, port_num);   //���¹��ܼĴ�����ַ
      }

      pre_port_num_pull = port_num_pull;
      pre_port_num_func = port_num_func;
      pre_port          = port;
      //���û��ؼ���ֵ
      script_gpio->pull      = (*tmp_group_pull_addr   >> ((port_num - (port_num_pull<<4))<<1)) & 0x03;    //����pull����
      script_gpio->drv_level = (*tmp_group_dlevel_addr >> ((port_num - (port_num_pull<<4))<<1)) & 0x03;    //������������
      script_gpio->mul_sel   = (*tmp_group_func_addr   >> ((port_num - (port_num_func<<3))<<2)) & 0x07;    //������������
      if(script_gpio->mul_sel <= 1)
      {
        script_gpio->data  = (*tmp_group_data_addr   >>   port_num) & 0x01;                              //����data����
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
*                ��ȡ�û������������GPIO��״̬
* Arguments  :
*        p_handler    :    handler
*        gpio_status    :    �����û����ݵ�����
*        gpio_name    :    Ҫ������GPIO������
*       if_get_user_set_flag   :   ��ȡ��־����ʾ��ȡ�û��趨���ݻ�����ʵ������
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
INT32  gpio_get_one_pin_status(UINT32 p_handler, user_gpio_set_t *gpio_status, const CHAR8 *gpio_name, UINT32 if_get_from_hardware)
{
  CHAR8               *tmp_buf;                                        //ת����CHAR8����
  UINT32               group_count_max;                                //���GPIO����
  system_gpio_set_t  *user_gpio_set, *tmp_sys_gpio_data;
  UINT32               port_num_func, port_num_pull;
  UINT32               port, port_num;
  UINT32               i, tmp_val1, tmp_val2;

  //��鴫���ľ������Ч��
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
  //��ȡ�û�����
  //��ʾ��ȡ�û�����������
  for(i = 0; i < group_count_max; i++)
  {
    tmp_sys_gpio_data = user_gpio_set + i;             //tmp_sys_gpio_dataָ�������GPIO�ռ�
    if(AsciiStrCmp(gpio_name, tmp_sys_gpio_data->gpio_name))
    {
      continue;
    }
    AsciiStrCpy(gpio_status->gpio_name, tmp_sys_gpio_data->gpio_name);
    port                   = tmp_sys_gpio_data->port;
    port_num               = tmp_sys_gpio_data->port_num;
    gpio_status->port      = port;                                              //����port����
    gpio_status->port_num  = port_num;                                          //����port_num����

    if(!if_get_from_hardware)                                                    //��ǰҪ������û���Ƶ�����
    {
      gpio_status->mul_sel   = tmp_sys_gpio_data->user_gpio_status.mul_sel;   //���û����������ж�����������
      gpio_status->pull      = tmp_sys_gpio_data->user_gpio_status.pull;      //���û����������ж���pull����
      gpio_status->drv_level = tmp_sys_gpio_data->user_gpio_status.drv_level; //���û����������ж���������������
      gpio_status->data      = tmp_sys_gpio_data->user_gpio_status.data;      //���û����������ж���data����
    }
    else                                                                        //��ǰ�����Ĵ���ʵ�ʵĲ���
    {
      port_num_func = (port_num >> 3);
      port_num_pull = (port_num >> 4);

      tmp_val1 = ((port_num - (port_num_func << 3)) << 2);
      tmp_val2 = ((port_num - (port_num_pull << 4)) << 1);
      if(port < 12)
      {
        gpio_status->mul_sel   = (PIO_REG_CFG_VALUE(port, port_num_func)>>tmp_val1) & 0x07;       //��Ӳ���ж������ܼĴ���
        gpio_status->pull      = (PIO_REG_PULL_VALUE(port, port_num_pull)>>tmp_val2) & 0x03;      //��Ӳ���ж���pull�Ĵ���
        gpio_status->drv_level = (PIO_REG_DLEVEL_VALUE(port, port_num_pull)>>tmp_val2) & 0x03;    //��Ӳ���ж���level�Ĵ���
      }
      else
      {          
        gpio_status->mul_sel   = (R_PIO_REG_CFG_VALUE(port, port_num_func)>>tmp_val1) & 0x07;       //��Ӳ���ж������ܼĴ���
        gpio_status->pull      = (R_PIO_REG_PULL_VALUE(port, port_num_pull)>>tmp_val2) & 0x03;      //��Ӳ���ж���pull�Ĵ���
        gpio_status->drv_level = (R_PIO_REG_DLEVEL_VALUE(port, port_num_pull)>>tmp_val2) & 0x03;    //��Ӳ���ж���level�Ĵ���
      }

      if(gpio_status->mul_sel <= 1)
      {
        if(port < 12)
          gpio_status->data = (PIO_REG_DATA_VALUE(port) >> port_num) & 0x01;                     //��Ӳ���ж���data�Ĵ���
        else
          gpio_status->data = (R_PIO_REG_DATA_VALUE(port) >> port_num) & 0x01;                     //��Ӳ���ж���data�Ĵ���             
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
*                ��ȡ�û��������GPIO��ĳһ����״̬
* Arguments  :
*        p_handler    :    handler
*        gpio_status    :    �����û����ݵ�����
*        gpio_name    :    Ҫ������GPIO������
*       if_get_user_set_flag   :   ��ȡ��־����ʾ��ȡ�û��趨���ݻ�����ʵ������
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/

INT32  gpio_set_one_pin_status(UINT32 p_handler, user_gpio_set_t *gpio_status, const CHAR8 *gpio_name, UINT32 if_set_to_current_input_status)
{
  CHAR8               *tmp_buf;                                        //ת����CHAR8����
  UINT32               group_count_max;                                //���GPIO����
  system_gpio_set_t  *user_gpio_set, *tmp_sys_gpio_data;
  user_gpio_set_t     script_gpio;
  volatile UINT32     *tmp_addr;
  UINT32               port_num_func, port_num_pull;
  UINT32               port, port_num;
  UINT32               i, reg_val, tmp_val;

  //��鴫���ľ������Ч��
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
  //��ȡ�û�����
  //��ʾ��ȡ�û�����������
  for(i = 0; i < group_count_max; i++)
  {
    tmp_sys_gpio_data = user_gpio_set + i;             //tmp_sys_gpio_dataָ�������GPIO�ռ�
    if(AsciiStrCmp(gpio_name, tmp_sys_gpio_data->gpio_name))
    {
      continue;
    }

    port          = tmp_sys_gpio_data->port;                           //����port����
    port_num      = tmp_sys_gpio_data->port_num;                       //����port_num����
    port_num_func = (port_num >> 3);
    port_num_pull = (port_num >> 4);

    if(if_set_to_current_input_status)                                 //���ݵ�ǰ�û��趨����
    {
      //�޸�FUCN�Ĵ���
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
      reg_val = *tmp_addr;                                                       //�޸�FUNC�Ĵ���
      tmp_val = (port_num - (port_num_func<<3))<<2;
      reg_val &= ~(0x07 << tmp_val);
      reg_val |=  (script_gpio.mul_sel) << tmp_val;
      *tmp_addr = reg_val;
    }
    //�޸�PULL�Ĵ���
    if(script_gpio.pull >= 0)
    {
      tmp_addr = (volatile UINT32 *)GetGroupPullAddrByPort(port, port_num);
      reg_val = *tmp_addr;                                                     //�޸�FUNC�Ĵ���
      tmp_val = (port_num - (port_num_pull<<4))<<1;
      reg_val &= ~(0x03 << tmp_val);
      reg_val |=  (script_gpio.pull) << tmp_val;
      *tmp_addr = reg_val;
    }
    //�޸�DLEVEL�Ĵ���
    if(script_gpio.drv_level >= 0)
    {
      tmp_addr = (volatile UINT32 *)GetGroupDlevelAddrByPort(port, port_num);
      reg_val = *tmp_addr;                                                         //�޸�FUNC�Ĵ���
      tmp_val = (port_num - (port_num_pull<<4))<<1;
      reg_val &= ~(0x03 << tmp_val);
      reg_val |=  (script_gpio.drv_level) << tmp_val;
      *tmp_addr = reg_val;
    }
    //�޸�data�Ĵ���
    if(script_gpio.mul_sel == 1)
    {
      if(script_gpio.data >= 0)
      {
        tmp_addr = (volatile UINT32 *)GetGroupDataAddrByPort(port);
        reg_val = *tmp_addr;                                                      //�޸�DATA�Ĵ���
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
*                �޸��û��������GPIO�е�ĳһ��IO�ڵģ��������״̬
* Arguments  :
*        p_handler    :    handler
*        if_set_to_output_status    :    ���ó����״̬��������״̬
*        gpio_name    :    Ҫ������GPIO������
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
INT32  gpio_set_one_pin_io_status(UINT32 p_handler, UINT32 if_set_to_output_status, const CHAR8 *gpio_name)
{
  CHAR8               *tmp_buf;                                        //ת����CHAR8����
  UINT32               group_count_max;                                //���GPIO����
  system_gpio_set_t  *user_gpio_set = NULL, *tmp_sys_gpio_data;
  volatile UINT32      *tmp_group_func_addr = NULL;
  UINT32               port, port_num, port_num_func;
  UINT32                i, reg_val;

  //��鴫���ľ������Ч��
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
*                �޸��û��������GPIO�е�ĳһ��IO�ڵģ�PULL״̬
* Arguments  :
*        p_handler    :    handler
*        if_set_to_output_status    :    �����õ�pull״̬
*        gpio_name    :    Ҫ������GPIO������
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
INT32  gpio_set_one_pin_pull(UINT32 p_handler, UINT32 set_pull_status, const CHAR8 *gpio_name)
{
  CHAR8               *tmp_buf;                                        //ת����CHAR8����
  UINT32               group_count_max;                                //���GPIO����
  system_gpio_set_t  *user_gpio_set = NULL, *tmp_sys_gpio_data;
  volatile UINT32      *tmp_group_pull_addr = NULL;
  UINT32               port, port_num, port_num_pull;
  UINT32                i, reg_val;
  //��鴫���ľ������Ч��
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
*                �޸��û��������GPIO�е�ĳһ��IO�ڵģ���������
* Arguments  :
*        p_handler    :    handler
*        if_set_to_output_status    :    �����õ����������ȼ�
*        gpio_name    :    Ҫ������GPIO������
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
INT32  gpio_set_one_pin_driver_level(UINT32 p_handler, UINT32 set_driver_level, const CHAR8 *gpio_name)
{
  CHAR8               *tmp_buf;                                        //ת����CHAR8����
  UINT32               group_count_max;                                //���GPIO����
  system_gpio_set_t  *user_gpio_set = NULL, *tmp_sys_gpio_data;
  volatile UINT32      *tmp_group_dlevel_addr = NULL;
  UINT32               port, port_num, port_num_dlevel;
  UINT32                i, reg_val;
  //��鴫���ľ������Ч��
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
*                ��ȡ�û��������GPIO�е�ĳһ��IO�ڵĶ˿ڵĵ�ƽ
* Arguments  :
*        p_handler    :    handler
*        gpio_name    :    Ҫ������GPIO������
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
INT32  gpio_read_one_pin_value(UINT32 p_handler, const CHAR8 *gpio_name)
{
  CHAR8               *tmp_buf;                                        //ת����CHAR8����
  UINT32               group_count_max;                                //���GPIO����
  system_gpio_set_t  *user_gpio_set = NULL, *tmp_sys_gpio_data;
  UINT32               port, port_num, port_num_func, func_val;
  UINT32                i, reg_val;
  //��鴫���ľ������Ч��
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
*                �޸��û��������GPIO�е�ĳһ��IO�ڵĶ˿ڵĵ�ƽ
* Arguments  :
*        p_handler    :    handler
*       value_to_gpio:  Ҫ���õĵ�ƽ�ĵ�ѹ
*        gpio_name    :    Ҫ������GPIO������
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
INT32  gpio_write_one_pin_value(UINT32 p_handler, UINT32 value_to_gpio, const CHAR8 *gpio_name)
{
  CHAR8               *tmp_buf;                                        //ת����CHAR8����
  UINT32               group_count_max;                                //���GPIO����
  system_gpio_set_t  *user_gpio_set = NULL, *tmp_sys_gpio_data;
  volatile UINT32     *tmp_group_data_addr = NULL;
  UINT32               port, port_num, port_num_func, func_val;
  UINT32                i, reg_val;
  //��鴫���ľ������Ч��
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
