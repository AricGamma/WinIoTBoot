// update.cpp : Defines the entry point for the console application.
//
#include <malloc.h>
#include "types.h"
#include <string.h>
#include "script.h"
#include "crc.h"
#include "sunxi_gpt.h"
#include <ctype.h>
#include <unistd.h>

#define  MAX_PATH  260
static unsigned int g_align;

int IsFullName(const char *FilePath);
__s32  update_gpt_crc(sunxi_gpt_t *gpt_info, __s32 gpt_count, FILE *gpt_file);
__s32 update_for_part_info(sunxi_gpt_t *gpt_info, sunxi_download_info *dl_map, __s32 gpt_count, __s32 gpt_total_size);
__s32 update_dl_info_crc(sunxi_download_info *dl_map, FILE *dl_file);
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
int get_file_name(char *path, char *name)
{
  char buffer[MAX_PATH];
  int  i, length;
  char *pt;

  memset(buffer, 0, MAX_PATH);
  if(!IsFullName(path))
  {
     if(getcwd(buffer, MAX_PATH ) == NULL)
     {
      perror( "getcwd error" );
      return -1;
     }
     sprintf(buffer, "%s/%s", buffer, path);
  }
  else
  {
    strcpy(buffer, path);
  }

  length = strlen(buffer);
  pt = buffer + length - 1;

  while((*pt != '/') && (*pt != '\\'))
  {
    pt --;
  }
  i =0;
  pt ++;
  while(*pt)
  {
    if(*pt == '.')
    {
      name[i++] = '_';
      pt ++;
    }
    else if((*pt >= 'a') && (*pt <= 'z'))
    {
      name[i++] = *pt++ - ('a' - 'A');
    }
    else
    {
      name[i++] = *pt ++;
    }
    if(i>=16)
    {
      break;
    }
  }

  return 0;
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
  printf("update.exe script file path para file path\n\n");
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
void create_gpt_guid(char *str, void *data)
{
  char temp_data1[9];
  char temp_data2[5];
  char temp_data3[5];
  char temp_data4[5];
  char temp_data5[13];
  char *temp;
  int  data1, data2, data3, data4;
  unsigned long data5;
  efi_guid *guid;
  int i;

  guid = (efi_guid *)data;

  printf("deine guid string:%s\n", str);
  temp = str;
  /* data1 */
  memcpy(temp_data1, temp, 8);
  temp_data1[9] = '\0';
  temp += 9;
  sscanf(temp_data1,"%x", &data1);
  guid->data1 = (unsigned int)data1;
  /* data2 */
  memcpy(temp_data2, temp, 4);
  temp_data2[5] = '\0';
  temp += 5;
  sscanf(temp_data2,"%x", &data2);
  guid->data2 = (unsigned short)data2;
  /* data3 */
  memcpy(temp_data3, temp, 4);
  temp_data3[5] = '\0';
  temp += 5;
  sscanf(temp_data3,"%x", &data3);
  guid->data3 = (unsigned short)data3;
  /* data4 low data */
  memcpy(temp_data4, temp, 4);
  temp_data4[5] = '\0';
  temp += 5;
  sscanf(temp_data4,"%x", &data4);
  guid->data4[0] = (data4>>8)&0xff;
  guid->data4[1] = (data4)&0xff;
  /* data4 high data */
  memcpy(temp_data5, temp, 12);
  temp_data5[13] = '\0';
  sscanf(temp_data5,"%lx", &data5);
  guid->data4[2] = (data5>>40)&0xff;
  guid->data4[3] = (data5>>32)&0xff;
  guid->data4[4] = (data5>>24)&0xff;
  guid->data4[5] = (data5>>16)&0xff;
  guid->data4[6] = (data5>>8)&0xff;
  guid->data4[7] = (data5)&0xff;

  printf("GPT partition type guid: 0x%x, 0x%x, 0x%x", guid->data1, guid->data2, guid->data3);
  for(i = 0; i < 8; i++)
  {
      printf(",0x%x", guid->data4[i]);
  }
  printf("\n");
}

int main(int argc, char* argv[])
{
  char   str1[] = "sys_config.bin";
  char   str2[] = "sunxi_gpt.fex";
  char   source[MAX_PATH];
  char   gpt_name[MAX_PATH];
  char   dl_name[MAX_PATH];
  FILE  *gpt_file = NULL;
  FILE  *dl_file = NULL;
  char  *pbuf = NULL;
  FILE  *source_file = NULL;
  int    script_len;
  int    gpt_count = 4, gpt_size;
  int    ret;
  sunxi_gpt_t    gpt_info;
  sunxi_download_info   dl_info;

  memset(source, 0, MAX_PATH);
  memset(gpt_name, 0, MAX_PATH);
  memset(dl_name, 0, MAX_PATH);
#if 1
  if(argc == 2)
  {
    if(argv[1] == NULL)
    {
      printf("update gpt error: one of the input file names is empty\n");

      return __LINE__;
    }

    GetFullPath(source,     argv[1]);
    gpt_count = 4;
  }
  else if (argc == 3)
  {
    if(argv[1] == NULL)
    {
      printf("update gpt error: one of the input file names is empty\n");

      return __LINE__;
    }

    GetFullPath(source,   argv[1]);
    gpt_count = atoi(argv[2]);
    printf("gpt count = %d\n", gpt_count);
  }
  else
  {
    printf("parameters is invalid\n");

    return __LINE__;
  }
#else
  gpt_count = 4;
  GetFullPath(source,   str1);
//  GetFullPath(gpt_name, str2);
#endif
  GetFullPath(gpt_name, SUNXI_GPT_NAME);
  GetFullPath(dl_name, DOWNLOAD_MAP_NAME);

  printf("\n");
  printf("partitation file Path=%s\n", source);
  printf("gpt_name file Path=%s\n", gpt_name);
  printf("download_name file Path=%s\n", dl_name);
  printf("\n");
  //把文件打成一个ini脚本
  source_file = fopen(source, "rb");
  if(!source_file)
  {
    printf("unable to open script file\n");

    goto _err_out;
  }
  //读出脚本的数据
  //首先获取脚本的长度
  fseek(source_file, 0, SEEK_END);
  script_len = ftell(source_file);
  fseek(source_file, 0, SEEK_SET);
  //读出脚本所有的内容
  pbuf = (char *)malloc(script_len);
  if(!pbuf)
  {
    printf("unable to malloc memory for script\n");

    goto _err_out;
  }
  memset(pbuf, 0, script_len);
  fread(pbuf, 1, script_len, source_file);
  fclose(source_file);
  source_file = NULL;
  //script使用初始化
  if(SCRIPT_PARSER_OK != script_parser_init(pbuf))
  {
    goto _err_out;
  }
  if(script_parser_fetch("gpt", "start", &gpt_size) || (!gpt_size))
  {
    gpt_size = 256;
  }
  printf("gpt size = %d\n", gpt_size);

  if(script_parser_fetch("gpt", "align", &g_align) || (!g_align))
  {
    g_align = 256;
  }
  printf("global align size = %d\n", g_align);

  //调用 script_parser_fetch函数取得任意数据项
  ret = update_for_part_info(&gpt_info, &dl_info, gpt_count, gpt_size);
  printf("update_for_part_info %d\n", ret);
  if(!ret)
  {
    gpt_file = fopen(gpt_name, "wb");
    if(!gpt_file)
    {
      ret = -1;
      printf("update gpt fail: unable to create file %s\n", gpt_name);
    }
    else
    {
      ret = update_gpt_crc(&gpt_info, gpt_count, gpt_file);
      if(ret >= 0)
        dl_file = fopen(dl_name, "wb");
      if(!dl_file)
      {
        ret = -1;
        printf("update gpt fail: unable to create download map %s\n", dl_name);
      }
      else
      {
        ret = update_dl_info_crc(&dl_info, dl_file);
      }
    }
  }
  // script使用完成，关闭退出
  script_parser_exit();
  //打印结果
  if(!ret)
  {
    printf("update gpt file ok\n");
  }
  else
  {
    printf("update gpt file fail\n");
  }
_err_out:
  if(pbuf)
  {
    free(pbuf);
  }
  if(source_file)
  {
    fclose(source_file);
  }
  if(gpt_file)
  {
    fclose(gpt_file);
  }
  if(dl_file)
  {
    fclose(dl_file);
  }

  return 0;
}


__s32 update_for_part_info(sunxi_gpt_t *gpt_info, sunxi_download_info *dl_map, __s32 gpt_count, __s32 gpt_size)
{
  int  value[8];
  char fullname[260];
  char filename[32];
  char temp_data[128];
  int  part_index;
  int  part_handle;
  int  down_index, down_flag;
  int  udisk_exist, is_udisk;
  __int64  start_sector;
  int  ret;
  unsigned int align;
  int tmp_data;
  int count;

  part_index = 0;
  down_index = 0;
  //初始化GPT
  memset(gpt_info, 0, sizeof(sunxi_gpt_t));
  memset(dl_map, 0, sizeof(sunxi_download_info));
  //固定不变的信息
  gpt_info->version = 0x00000200;
  strcpy((char *)gpt_info->magic, SUNXI_GPT_MAGIC);
  printf("gpt magic %s\n", gpt_info->magic);
  gpt_info->copy    = gpt_count;
  if(!gpt_count)
  {
    gpt_info->copy = SUNXI_GPT_COPY_NUM;
  }
  dl_map->version = 0x00000200;
  strcpy((char *)dl_map->magic, SUNXI_GPT_MAGIC);
  //根据分区情况会有变化的信息
  udisk_exist = 0;
  while(1)
  {
    down_flag = 0;
    is_udisk  = 0;
    part_handle = script_parser_fetch_partition();
    if(part_handle <= 0)
    {
      break;
    }
    strcpy((char *)gpt_info->array[part_index].classname, "DISK");
    //获取分区 name
    memset(value, 0, 8 * sizeof(int));
    if(!script_parser_fetch_mainkey_sub("name", part_handle, value))
    {
      printf("disk name=%s\n", (char *)value);
      if(!strcmp((char *)value, "Data"))
      {
        udisk_exist = 1;
        is_udisk    = 1;
      }
      strcpy((char *)gpt_info->array[part_index].name, (char *)value);
      memset(fullname, 0, 260);
      ret = script_parser_fetch_mainkey_sub("downloadfile", part_handle, (int *)fullname);
      if(!ret)
      {
        down_flag = 1;
        memset(filename, '0', 16);
        memset(filename + 16, 0, 16);
        get_file_name(fullname, filename);
        strcpy((char *)dl_map->one_part_info[down_index].name, (char *)value);
        memcpy((char *)dl_map->one_part_info[down_index].dl_filename, filename, 16);
      }
    }
    else
    {
      printf("GPT Create FAIL: Unable to find partition name in partition index %d\n", part_index);

      return -1;
    }
    //靠靠 靠靠
    if(!script_parser_fetch_mainkey_sub("align", part_handle, value))
    {
      align = value[0];
    }
    else
    {
      align = g_align;
    }
    //获取分区 用户类型
    if(!script_parser_fetch_mainkey_sub("user_type", part_handle, value))
    {
      gpt_info->array[part_index].user_type = value[0];
    }
    //获取分区 读写属性
    if(!script_parser_fetch_mainkey_sub("ro", part_handle, value))
    {
      gpt_info->array[part_index].ro = value[0];
    }
    //获取分区 写保护属性
    if(!script_parser_fetch_mainkey_sub("keydata", part_handle, value))
    {
      gpt_info->array[part_index].keydata = value[0];
    }

    //获取分区 类型GUID
    if(!script_parser_fetch_mainkey_sub("type", part_handle, (int *)temp_data))
    {
      printf("get type guid string\n");
      create_gpt_guid(temp_data,&gpt_info->array[part_index].type);
    }

    //获取分区标志GUID
    if(!script_parser_fetch_mainkey_sub("uniqueguid", part_handle, (int *)temp_data))
    {
      printf("get unique guid string\n");
      create_gpt_guid(temp_data,&gpt_info->array[part_index].uniqueguid);
    }

    gpt_info->array[part_index].attrlo = 0;
    //获取分区属性System 标志位
    if(!script_parser_fetch_mainkey_sub("system", part_handle, value))
    {
      gpt_info->array[part_index].attrlo |= value[0]<<0;
    }

    //获取分区属性BlockIo 标志位
    if(!script_parser_fetch_mainkey_sub("attrhi", part_handle, value))
    {
      gpt_info->array[part_index].attrhi = value[0];
    }

    //获取分区属性传统BIOS 引导标志位
    if(!script_parser_fetch_mainkey_sub("attrlo", part_handle, value))
    {
      gpt_info->array[part_index].attrlo = value[0];
    }

    //获取分区 是否需要校验
    if(down_flag)
    {
      value[0] = 1;
      ret = script_parser_fetch_mainkey_sub("verify", part_handle, value);
      if((!ret) && (!value[0]))
      {
        dl_map->one_part_info[down_index].verify = 0;
      }
      else
      {
        dl_map->one_part_info[down_index].verify = value[0];
        memset((char *)dl_map->one_part_info[down_index].vf_filename, '0', 16);
        dl_map->one_part_info[down_index].vf_filename[0] = 'V';
        memcpy((char *)dl_map->one_part_info[down_index].vf_filename + 1, filename, 15);
      }
    }
    //获取分区 加密属性
    if(down_flag)
    {
      if(!script_parser_fetch_mainkey_sub("encrypt", part_handle, value))
      {
        dl_map->one_part_info[down_index].encrypt = value[0];
      }
    }
    if(!is_udisk)
    {
      //获取分区 大小低位
      if(!script_parser_fetch_mainkey_sub( "size", part_handle, value))
      {
        gpt_info->array[part_index].lenlo = value[0];
        if(down_flag)
        {
          dl_map->one_part_info[down_index].lenlo = value[0];
        }
      }
      else
      {
        printf("GPT Create FAIL: Unable to find partition size in partition %d\n", part_index);

        return -1;
      }
    }
    //生成分区 起始地址
    if(!part_index)
    {
      gpt_info->array[part_index].addrhi = 0;
      gpt_info->array[part_index].addrlo = gpt_size;
      if(down_flag)
      {
        dl_map->one_part_info[down_index].addrhi = 0;
        dl_map->one_part_info[down_index].addrlo = gpt_size;
      }
    }
    else
    {
      start_sector = gpt_info->array[part_index-1].addrlo;
      start_sector |= (__int64)gpt_info->array[part_index-1].addrhi << 32;
      start_sector += gpt_info->array[part_index-1].lenlo;

      tmp_data = start_sector % align;
      if(tmp_data != 0)
	start_sector += align;
      count = start_sector / align;
      start_sector = align * count;

      gpt_info->array[part_index].addrlo = (__u32)(start_sector & 0xffffffff);
      gpt_info->array[part_index].addrhi = (__u32)((start_sector >>32) & 0xffffffff);

      if(down_flag)
      {
        dl_map->one_part_info[down_index].addrlo = gpt_info->array[part_index].addrlo;
        dl_map->one_part_info[down_index].addrhi = gpt_info->array[part_index].addrhi;
      }
    }

    part_index ++;
    if(down_flag)
    {
      down_index ++;
    }
  }

  if(part_handle < 0)
  {
    return -1;
  }

  if(!udisk_exist)
  {
    start_sector = gpt_info->array[part_index-1].addrlo;
    start_sector |= (__int64)gpt_info->array[part_index-1].addrhi << 32;
    start_sector += gpt_info->array[part_index-1].lenlo;

    gpt_info->array[part_index].addrlo = (__u32)(start_sector & 0xffffffff);
    gpt_info->array[part_index].addrhi = (__u32)((start_sector >>32) & 0xffffffff);

    strcpy((char *)gpt_info->array[part_index].classname, "DISK");
    strcpy((char *)gpt_info->array[part_index].name, "Data");
    gpt_info->PartCount = part_index + 1;
  }
  else
  {
    gpt_info->PartCount = part_index;
  }

  dl_map->download_count = down_index;


  return 0;
}

__s32  update_gpt_crc(sunxi_gpt_t *gpt_info, __s32 gpt_count, FILE *gpt_file)
{
  int i;
  unsigned crc32_total;

  for(i=0;i<gpt_count;i++)
  {
    gpt_info->index = i;
    crc32_total = calc_crc32((void *)&(gpt_info->version), (sizeof(sunxi_gpt_t) - 4));
    gpt_info->crc32 = crc32_total;
    printf("crc %d = %x\n", i, crc32_total);
    fwrite(gpt_info, sizeof(sunxi_gpt_t), 1, gpt_file);
  }

  return 0;
}


__s32 update_dl_info_crc(sunxi_download_info *dl_map, FILE *dl_file)
{
  __u32 crc32_total;

  crc32_total = calc_crc32((void *)&(dl_map->version), (sizeof(sunxi_download_info) - 4));
  dl_map->crc32 = crc32_total;
  fwrite(dl_map, sizeof(sunxi_download_info), 1, dl_file);

  return 0;
}


