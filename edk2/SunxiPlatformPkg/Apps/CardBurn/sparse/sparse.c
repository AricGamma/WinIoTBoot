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
#include "sparse_format.h"
#include "sparse.h"
#include "../sprite_verify.h"
#include "../SunxiSprite.h"




#define   SPARSE_FORMAT_TYPE_TOTAL_HEAD       0xff00
#define   SPARSE_FORMAT_TYPE_CHUNK_HEAD       0xff01
#define   SPARSE_FORMAT_TYPE_CHUNK_DATA       0xff02


static uint  android_format_checksum = 0;
static uint  sparse_format_type = 0;
static uint  chunk_count = 0;
static int  last_rest_size = 0;
static int  chunk_length = 0;
static uint  flash_start = 0;
static sparse_header_t globl_header = {0};
static uint  total_chunks = 0;
/*
************************************************************************************************************
*
*                                             unsparse_probe
*
*    �������ƣ�
*
*    �����б�
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int unsparse_probe(char *source, uint length, uint android_format_flash_start)
{
  sparse_header_t *header = (sparse_header_t*) source;

  if (header->magic != SPARSE_HEADER_MAGIC)
  {
    printf("sparse: bad magic\n");
    return ANDROID_FORMAT_BAD;
  }

  if ((header->major_version != SPARSE_HEADER_MAJOR_VER) ||
      (header->file_hdr_sz != sizeof(sparse_header_t)) ||
      (header->chunk_hdr_sz != sizeof(chunk_header_t)))
  {
    printf("sparse: incompatible format\n");
    return ANDROID_FORMAT_BAD;
  }
  android_format_checksum  = 0;
  last_rest_size = 0;
  chunk_count = 0;
  chunk_length = 0;
  sparse_format_type = SPARSE_FORMAT_TYPE_TOTAL_HEAD;
  flash_start = android_format_flash_start;
  total_chunks = header->total_chunks;

  return ANDROID_FORMAT_DETECT;
}
/*
************************************************************************************************************
*
*                                             DRAM_Write
*
*    �������ƣ�
*
*    �����б�
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int  unsparse_direct_write(void *pbuf, uint length)
{
  int   unenough_length;
  int   this_rest_size;
  int   tmp_down_size;
  char *tmp_buf, *tmp_dest_buf;
  chunk_header_t   *chunk;
    //���ȼ��㴫�������ݵ�У���
  android_format_checksum += SunxiAddSum(pbuf, length);

  this_rest_size = last_rest_size + length;
  tmp_buf = (char *)pbuf - last_rest_size;
  last_rest_size = 0;

  while(this_rest_size > 0)
  {
    switch(sparse_format_type)
    {
      case SPARSE_FORMAT_TYPE_TOTAL_HEAD:
      {
        memcpy(&globl_header, tmp_buf, sizeof(sparse_header_t));
        this_rest_size -= sizeof(sparse_header_t);
        tmp_buf += sizeof(sparse_header_t);

        sparse_format_type = SPARSE_FORMAT_TYPE_CHUNK_HEAD;

        break;
      }
      case SPARSE_FORMAT_TYPE_CHUNK_HEAD:
      {
        if(this_rest_size < sizeof(chunk_header_t))
        {
          printf("sparse: chunk head data is not enough\n");
          last_rest_size = this_rest_size;
          tmp_dest_buf = (char *)pbuf - this_rest_size;
          memcpy(tmp_dest_buf, tmp_buf, this_rest_size);
          this_rest_size = 0;

          break;
        }
        chunk = (chunk_header_t *)tmp_buf;
        /* move to next chunk */
        tmp_buf += sizeof(chunk_header_t);        //��ʱtmp_buf�Ѿ�ָ����һ��chunk����data��ʼ��ַ
        this_rest_size -= sizeof(chunk_header_t); //ʣ������ݳ���
        chunk_length = chunk->chunk_sz * globl_header.blk_sz;   //��ǰ���ݿ���Ҫд������ݳ���
        printf("chunk %d(%d)\n", chunk_count ++, total_chunks);

        switch (chunk->chunk_type)
        {
          case CHUNK_TYPE_RAW:
            if (chunk->total_sz != (chunk_length + sizeof(chunk_header_t)))
            {
              printf("sparse: bad chunk size for chunk %d, type Raw\n", chunk_count);
              return -1;
            }
            //���ﲻ�������ݲ��֣�ת����һ��״̬
            sparse_format_type = SPARSE_FORMAT_TYPE_CHUNK_DATA;
            break;

          case CHUNK_TYPE_DONT_CARE:
            if (chunk->total_sz != sizeof(chunk_header_t))
            {
              printf("sparse: bogus DONT CARE chunk\n");
              return -1;
            }
            flash_start += (chunk_length>>9);
            sparse_format_type = SPARSE_FORMAT_TYPE_CHUNK_HEAD;

            break;

          default:
            printf("sparse: unknown chunk ID %x\n", chunk->chunk_type);
            return -1;
        }
        break;
      }
      case SPARSE_FORMAT_TYPE_CHUNK_DATA:
      {
        //�����ж������Ƿ��㹻��ǰchunk����,������㣬����������Ҫ�����ݳ���
        unenough_length = (chunk_length >= this_rest_size)? (chunk_length - this_rest_size):0;
        if(!unenough_length)
        {
          //�����㹻��ֱ��д��
          if(!sunxi_sprite_write(flash_start, chunk_length>>9, tmp_buf))
          {
            printf("sparse: flash write failed\n");
            return -1;
          }
          if(chunk_length & 511)
          {
            printf("data is not sector align 0\n");
            return -1;
          }
          flash_start += (chunk_length>>9);
          tmp_buf += chunk_length;
          this_rest_size -= chunk_length;
          chunk_length = 0;

          sparse_format_type = SPARSE_FORMAT_TYPE_CHUNK_HEAD;
        }
        else  //����ȱʧ���ݵ����
        {
          if(this_rest_size < 8 * 1024) //�ȿ����������Ƿ���8k
          {
            //������ʱ����������ݷŵ���һ�����ݵ�ǰ�����ȴ���һ�δ���
            tmp_dest_buf = (char *)pbuf - this_rest_size;
            memcpy(tmp_dest_buf, tmp_buf, this_rest_size);
            last_rest_size = this_rest_size;
            this_rest_size = 0;

            break;
          }
          //���������ݳ���16kʱ
          //��ȱʧ���ݳ��Ȳ���4kʱ,����ֻȱ��ʮ���ֽ�
          if(unenough_length < 4 * 1024)
          {
            //����ƴ�ӷ���������д�����������ݣ�Ȼ������һ�ΰ�δ��д���������ݺ�ȱʧ����һ����¼
            tmp_down_size = this_rest_size + unenough_length - 4 * 1024;
          }
          else //���ﴦ��ȱʧ���ݳ���8k(����)�����,ͬʱ��������Ҳ����16k
          {
            //ֱ����¼��ǰȫ������;
            tmp_down_size = this_rest_size & (~(512 -1));  //��������
          }
          if(!sunxi_sprite_write(flash_start, tmp_down_size>>9, tmp_buf))
          {
            printf("sparse: flash write failed\n");
            return -1;
          }
          if(tmp_down_size & 511)
          {
            printf("data is not sector align 1\n");
            return -1;
          }
          tmp_buf += tmp_down_size;
          flash_start += (tmp_down_size>>9);
          chunk_length -= tmp_down_size;
          this_rest_size -= tmp_down_size;
          tmp_dest_buf = (char *)pbuf - this_rest_size;
          memcpy(tmp_dest_buf, tmp_buf, this_rest_size);
          last_rest_size = this_rest_size;
          this_rest_size = 0;

          sparse_format_type = SPARSE_FORMAT_TYPE_CHUNK_DATA;
        }

        break;
      }

      default:
      {
        printf("sparse: unknown status\n");
        return -1;
      }
    }
  }

  return 0;
}
/*
************************************************************************************************************
*
*                                             unsparse_checksum
*
*    �������ƣ�
*
*    �����б�
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
uint unsparse_checksum(void)
{
  return android_format_checksum;
}


