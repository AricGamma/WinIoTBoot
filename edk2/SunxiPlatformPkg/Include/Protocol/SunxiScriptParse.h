/** @file

  Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __SUNXI_SCRIPT_PARSE_H__
#define __SUNXI_SCRIPT_PARSE_H__

#define   SCRIPT_PARSER_OK                   (0)
#define   SCRIPT_PARSER_EMPTY_BUFFER         (-1)
#define   SCRIPT_PARSER_KEYNAME_NULL         (-2)
#define   SCRIPT_PARSER_DATA_VALUE_NULL      (-3)
#define   SCRIPT_PARSER_KEY_NOT_FIND         (-4)
#define   SCRIPT_PARSER_BUFFER_NOT_ENOUGH    (-5)

typedef enum
{
  SCIRPT_PARSER_VALUE_TYPE_INVALID = 0,
  SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD,
  SCIRPT_PARSER_VALUE_TYPE_STRING,
  SCIRPT_PARSER_VALUE_TYPE_MULTI_WORD,
  SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD
} script_parser_value_type_t;

typedef struct
{
  CHAR8  gpio_name[32];
  INT32 port;
  INT32 port_num;
  INT32 mul_sel;
  INT32 pull;
  INT32 drv_level;
  INT32 data;
} script_gpio_set_t;

typedef struct
{
  INT32 main_key_count;
  INT32 version[3];
} script_head_t;

typedef struct
{
  char main_name[32];
  INT32 lenth;
  INT32 offset;
} script_main_key_t;

typedef struct
{
  CHAR8 sub_name[32];
  INT32 offset;
  INT32 pattern;
} script_sub_key_t;


#define   EGPIO_FAIL             (-1)
#define   EGPIO_SUCCESS          (0)

typedef enum
{
  PIN_PULL_DEFAULT  =   0xFF,
  PIN_PULL_DISABLE  = 0x00,
  PIN_PULL_UP       = 0x01,
  PIN_PULL_DOWN     = 0x02,
  PIN_PULL_RESERVED = 0x03
} pin_pull_level_t;

typedef enum
{
  PIN_MULTI_DRIVING_DEFAULT = 0xFF,
  PIN_MULTI_DRIVING_0     = 0x00,
  PIN_MULTI_DRIVING_1     = 0x01,
  PIN_MULTI_DRIVING_2     = 0x02,
  PIN_MULTI_DRIVING_3     = 0x03
} pin_drive_level_t;

typedef enum
{
  PIN_DATA_LOW,
  PIN_DATA_HIGH,
  PIN_DATA_DEFAULT = 0XFF
} pin_data_t;

#define PIN_PHY_GROUP_A     0x00
#define PIN_PHY_GROUP_B     0x01
#define PIN_PHY_GROUP_C     0x02
#define PIN_PHY_GROUP_D     0x03
#define PIN_PHY_GROUP_E     0x04
#define PIN_PHY_GROUP_F     0x05
#define PIN_PHY_GROUP_G     0x06
#define PIN_PHY_GROUP_H     0x07
#define PIN_PHY_GROUP_I     0x08
#define PIN_PHY_GROUP_J     0x09

typedef struct
{
  CHAR8  gpio_name[32];
  INT32 port;
  INT32 port_num;
  INT32 mul_sel;
  INT32 pull;
  INT32 drv_level;
  INT32 data;
} user_gpio_set_t;

/* functions for early boot */
extern INT32 sw_cfg_get_int(CONST CHAR8 *script_buf, CONST CHAR8 *main_key, CONST CHAR8 *sub_key);
extern CHAR8 *sw_cfg_get_str(CONST CHAR8 *script_buf, CONST CHAR8 *main_key, CONST CHAR8 *sub_key, CHAR8 *buf);

/* script operations */
extern INT32 script_parser_init(CHAR8 *script_buf);
extern INT32 script_parser_exit(void);
extern UINT32 script_parser_fetch_subkey_start(CHAR8 *main_name);
extern INT32 script_parser_fetch_subkey_next(UINT32 hd, CHAR8 *sub_name, INT32 value[], INT32*index);
extern INT32 script_parser_fetch(CHAR8 *main_name, CHAR8 *sub_name, INT32 value[], INT32 count);
extern INT32 script_parser_fetch_ex(CHAR8 *main_name, CHAR8 *sub_name, INT32 value[],
               script_parser_value_type_t *type, INT32 count);
extern INT32 script_parser_patch(CHAR8 *main_name, CHAR8 *sub_name, void *str, INT32 str_size);
extern INT32 script_parser_subkey_count(CHAR8 *main_name);
extern INT32 script_parser_mainkey_count(void);
extern INT32 script_parser_mainkey_get_gpio_count(CHAR8 *main_name);
extern INT32 script_parser_mainkey_get_gpio_cfg(CHAR8 *main_name, void *gpio_cfg, INT32 gpio_count);

/* gpio operations */
extern INT32 gpio_init(void);
extern INT32 gpio_exit(void);
extern UINT32 gpio_request(user_gpio_set_t *gpio_list, UINT32 group_count_max);
extern UINT32 gpio_request_ex(CHAR8 *main_name, CONST CHAR8 *sub_name);
extern INT32 gpio_release(UINT32 p_handler, INT32 if_release_to_default_status);
extern INT32 gpio_get_all_pin_status(UINT32 p_handler, user_gpio_set_t *gpio_status, UINT32 gpio_count_max, UINT32 if_get_from_hardware);
extern INT32 gpio_get_one_pin_status(UINT32 p_handler, user_gpio_set_t *gpio_status, CONST CHAR8 *gpio_name, UINT32 if_get_from_hardware);
extern INT32 gpio_set_one_pin_status(UINT32 p_handler, user_gpio_set_t *gpio_status, CONST CHAR8 *gpio_name, UINT32 if_set_to_current_input_status);
extern INT32 gpio_set_one_pin_io_status(UINT32 p_handler, UINT32 if_set_to_output_status, CONST CHAR8 *gpio_name);
extern INT32 gpio_set_one_pin_pull(UINT32 p_handler, UINT32 set_pull_status, CONST CHAR8 *gpio_name);
extern INT32 gpio_set_one_pin_driver_level(UINT32 p_handler, UINT32 set_driver_level, CONST CHAR8 *gpio_name);
extern INT32 gpio_read_one_pin_value(UINT32 p_handler, CONST CHAR8 *gpio_name);
extern INT32 gpio_write_one_pin_value(UINT32 p_handler, UINT32 value_to_gpio, CONST CHAR8 *gpio_name);

//
// Protocol interface structure
//
typedef struct _SUNXI_SCRIPT_PARSE_PROTOCOL SUNXI_SCRIPT_PARSE_PROTOCOL;

typedef
INT32
(EFIAPI *SCRIPT_PARSE_FETCH) (
  IN CHAR8      *main_name,
  IN CHAR8      *sub_name,
  OUT INT32     value[],
  IN INT32      count
  );

typedef
INT32
(EFIAPI *SCRIPT_PARSE_FETCH_EX) (
  IN CHAR8      *main_name,
  IN CHAR8      *sub_name,
  OUT INT32     value[],
  IN script_parser_value_type_t *type,
  IN INT32      count
  );

typedef
UINT32
(EFIAPI *SCRIPT_PARSE_FETCH_SUBKEY_START) (
  IN CHAR8      *main_name
  );

typedef
INT32
(EFIAPI *SCRIPT_PARSE_FETCH_SUBKEY_NEXT) (
  IN UINT32     hd,
  IN CHAR8      *sub_name,
  OUT INT32     value[],
  IN INT32      *index
  );

typedef
INT32
(EFIAPI *SCRIPT_PARSE_PATCH) (
  IN CHAR8      *main_name,
  IN CHAR8      *sub_name,
  IN VOID     *str,
  IN INT32           str_size
  );

typedef
INT32
(EFIAPI *SCRIPT_PARSE_MAINKEY_COUNT) (
 VOID
  );

typedef
INT32
(EFIAPI *SCRIPT_PARSE_SUBKEY_COUNT) (
  IN CHAR8      *main_key
  );

typedef
INT32
(EFIAPI *SCRIPT_PARSE_MAINKEY_GET_GPIO_COUNT) (
  IN CHAR8      *main_key
  );

typedef
INT32
(EFIAPI *SCRIPT_PARSE_MAINKEY_GET_GPIO_CFG) (
  IN CHAR8      *main_key,
  IN VOID     *gpio_cfg,
  IN INT32           gpio_count
  );

typedef
UINT32
(EFIAPI *GPIO_REQUEST) (
  IN user_gpio_set_t  *gpio_list,
  IN UINT32 group_count_max
  );

typedef
UINT32
(EFIAPI *GPIO_REQUEST_EX) (
  IN CHAR8  *main_name,
  IN CONST CHAR8 *sub_name
  );

typedef
INT32
(EFIAPI *GPIO_RELEASE) (
  IN UINT32 p_handler,
  IN INT32 if_release_to_default_status
  );

typedef
INT32
(EFIAPI *GPIO_GET_ALL_PIN_STATUS) (
  IN  UINT32 p_handler,
  OUT user_gpio_set_t *gpio_status,
  IN  UINT32 gpio_count_max,
  IN  UINT32 if_get_from_hardware
  );

typedef
INT32
(EFIAPI *GPIO_GET_ONE_PIN_STATUS) (
  IN  UINT32 p_handler,
  OUT user_gpio_set_t *gpio_status,
  IN  CONST CHAR8 *gpio_name,
  IN  UINT32 if_get_from_hardware
  );

typedef
INT32
(EFIAPI *GPIO_SET_ONE_PIN_STATUS) (
  IN  UINT32 p_handler,
  IN user_gpio_set_t *gpio_status,
  IN  CONST CHAR8 *gpio_name,
  IN  UINT32 if_get_from_hardware
  );

typedef
INT32
(EFIAPI *GPIO_SET_ONE_PIN_IO_STATUS) (
  IN  UINT32 p_handler,
  IN  UINT32 if_set_to_output_status,
  IN  CONST CHAR8 *gpio_name
  );

typedef
INT32
(EFIAPI *GPIO_SET_ONE_PIN_PULL) (
  IN  UINT32 p_handler,
  IN  UINT32 set_pull_status,
  IN  CONST CHAR8 *gpio_name
  );

typedef
INT32
(EFIAPI *GPIO_SET_ONE_PIN_DRIVER_LEVEL) (
  IN  UINT32 p_handler,
  IN  UINT32 set_driver_level,
  IN  CONST CHAR8 *gpio_name
  );

typedef
INT32
(EFIAPI *GPIO_READ_ONE_PIN_VALUE) (
  IN  UINT32 p_handler,
  IN  CONST CHAR8 *gpio_name
  );

typedef
INT32
(EFIAPI *GPIO_WRITE_ONE_PIN_VALUE) (
  IN  UINT32 p_handler,
  IN  UINT32 value_to_gpio,
  IN  CONST CHAR8 *gpio_name
  );


///
/// This protocol allows parse the sunxi script to config the system.
///
struct _SUNXI_SCRIPT_PARSE_PROTOCOL {

  SCRIPT_PARSE_FETCH            ScriptParserFetch;
  SCRIPT_PARSE_FETCH_SUBKEY_START     ScriptParserFetchSubkeyStart;
  SCRIPT_PARSE_FETCH_SUBKEY_NEXT      ScriptParserFetchSubkeyNext;
  SCRIPT_PARSE_PATCH            ScriptParserPatch;
  SCRIPT_PARSE_MAINKEY_COUNT        ScriptParserMainkeyCount;
  SCRIPT_PARSE_SUBKEY_COUNT         ScriptParserSubkeyCount;
  SCRIPT_PARSE_MAINKEY_GET_GPIO_COUNT ScriptParserMainkeyGetGpioCount;
  SCRIPT_PARSE_MAINKEY_GET_GPIO_CFG   ScriptParserMainkeyGetGpioCfg;
  
  GPIO_REQUEST                GpioRequest;
  GPIO_REQUEST_EX           GpioRequestEx;
  GPIO_RELEASE              GpioRelease;
  GPIO_GET_ALL_PIN_STATUS       GpioGetAllPinStatus;
  GPIO_SET_ONE_PIN_STATUS       GpioSetOnePinStatus;
  GPIO_SET_ONE_PIN_IO_STATUS      GpioSetOnePinIoStatus;
  GPIO_SET_ONE_PIN_PULL         GpioSetOnePinPull;
  GPIO_SET_ONE_PIN_DRIVER_LEVEL       GpioSetOnePinDriverLevel;
  GPIO_READ_ONE_PIN_VALUE       GpioReadOnePinValue;
  GPIO_WRITE_ONE_PIN_VALUE        GpioWriteOnePinValue;
};


extern EFI_GUID gSunxiPlatformScriptParseProtocolGuid;

#endif
