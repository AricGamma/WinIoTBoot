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


#include <malloc.h>
#include <string.h>
#include "spare_head.h"
#include "private_uefi.h"
#include <ctype.h>
#include <unistd.h>

#define  MAX_PATH             (260)
int  script_length;
int  align_size;

void *script_file_decode(char *script_name);
int merge_uboot(char *source_uboot_name, char *script_name);
int gen_spare_head_for_uboot(char *uboot_name);
extern struct spare_boot_head_t  uefi_spare_head;
//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
int IsFullName(const char *FilePath)
{
  if (isalpha(FilePath[0]) && ':' == FilePath[1])
  {
    return 1;
  }
  else
  {
    return 0;
  }
}
//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
void GetFullPath(char *dName, const char *sName)
{
  char Buffer[MAX_PATH];

  if(IsFullName(sName))
  {
    strcpy(dName, sName);
    return ;
  }

  /* Get the current working directory: */
  if(getcwd(Buffer, MAX_PATH ) == NULL)
  {
    perror( "getcwd error" );
    return ;
  }
  sprintf(dName, "%s/%s", Buffer, sName);
}

//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
void Usage(void)
{
  printf("\n");
  printf("Usage:\n");
  printf("./gen_spare_head boot1_file.bin \n\n");
}

int main(int argc, char* argv[])
{
  char   str1[] = "uefi_spare_head.bin";
  char   str2[] = "D:\\winners\\eBase\\eGON\\EGON2_TRUNK\\boot_23\\workspace\\wboot\\bootfs\\script.bin";
  char   source_uboot_name[MAX_PATH];
//  int    source_length, script_length;
//  int    total_length;
//  char   *pbuf_source, *pbuf_script;
  char   *script_buf = NULL;

#if 0
  if(argc == 2)
  {
    if(argv[1] == NULL)
    {
      printf("update error: one of the input file names is empty\n");

      return __LINE__;
    }
  }
  else
  {
    Usage();

    return __LINE__;
  }
  GetFullPath(source_uboot_name,  argv[1]);
#else
  strcpy(source_uboot_name, str1);
#endif

  printf("\n");
  printf("output spare header is: %s\n", source_uboot_name);
  printf("\n");
  //读取原始uboot
  if(gen_spare_head_for_uboot(source_uboot_name))
  {
    printf("update uboot error: update error\n");

    return -1;
  }

  return 0;

}

int gen_spare_head_for_uboot(char *uboot_name)
{
  FILE *uboot_file = NULL;
  struct spare_boot_head_t  *uboot_head;
  char *uboot_buf = NULL;
//  int   i;
  int   ret = -1;
  int   align_size, uboot_length;
  int   origin_length;
  char  buffer[512];

  uboot_file = fopen(uboot_name, "wb+");
  if(uboot_file == NULL)
  {
    printf("genchecksum uboot error : unable to open uboot file\n");
    goto _err_uboot_out;
  }
  //fseek(uboot_file, 0, SEEK_END);
  //origin_length = ftell(uboot_file);
  fseek(uboot_file, 0, SEEK_SET);    
  printf("uefi_spare_head size is %ld bytes \n",sizeof(uefi_spare_head));
  fwrite((void*)&uefi_spare_head, sizeof(uefi_spare_head), 1, uboot_file);
  ret = 0;
_ok_uboot_out:
  if(uboot_buf)
  {
    free(uboot_buf);
  }
  if(uboot_file)
  {
    fclose(uboot_file);
  }

  return ret;

_err_uboot_out:
  if(uboot_buf)
  {
    free(uboot_buf);
  }
  if(uboot_file)
  {
    fclose(uboot_file);
  }

  return -1;
}




