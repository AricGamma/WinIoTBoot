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

#ifndef  __brom_h
#define  __brom_h


/******************************************************************************/
/*                              file head of Brom                             */
/******************************************************************************/
typedef struct _brom_file_head
{ 
  __u32  jump_instruction;   // one intruction jumping to real code
  __u8   magic[8];           // ="eGON.BRM", not C-style string.
  __u32  Brom_head_size;     // the size of brom_file_head_t
  __u8   file_head_vsn[4];   // the version of brom_file_head_t
  __u8   Brom_vsn[4];        // Brom version
  __u8   platform[8];        // platform information
}brom_file_head_t;

#define BROM_FILE_HEAD_VERSION         "1100"     // X.X.XX
#define BROM_MAGIC                     "eGON.BRM"


#endif     //  ifndef __brom_h

/* end of brom.h */
