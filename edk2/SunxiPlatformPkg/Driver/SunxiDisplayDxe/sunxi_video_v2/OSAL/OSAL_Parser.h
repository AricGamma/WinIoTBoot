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

#ifndef  __OSAL_PARSER_H__
#define  __OSAL_PARSER_H__

#include "OSAL.h"

int OSAL_Script_FetchParser_Data(char *main_name, char *sub_name, int value[], int count);
int OSAL_Script_FetchParser_Data_Ex(char *main_name, char *sub_name, int value[], script_parser_value_type_t *type, int count);
int OSAL_sw_get_ic_ver(void);

#endif
