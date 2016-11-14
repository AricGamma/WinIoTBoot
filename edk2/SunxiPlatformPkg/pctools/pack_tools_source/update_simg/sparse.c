/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * (C) Copyright 2011
 * Texas Instruments, <www.ti.com>
 * Author: Vikram Pandita <vikram.pandita@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include  "update_simg.h"
#include  "sparse.h"
#include  "sparse_format.h"

#define  SPARSE_HEADER_MAJOR_VER 1

unsigned int  sparse_format_type;
unsigned int  chunk_count;
int  last_rest_size;
int  chunk_length;
sparse_header_t globl_header;


static int wirte_to_0xff(FILE *file, unsigned int length)
{
	char buffer[4 * 1024];
	unsigned int tmp_length;

	tmp_length = length;
	memset(buffer, 0xff, 4 * 1024);
	while(tmp_length >= 4 * 1024)
	{
		fwrite(buffer, 4 * 1024, 1, file);
		tmp_length -= 4 * 1024;
	}
	if(tmp_length)
	{
		fwrite(buffer, tmp_length, 1, file);
	}

	return 0;
}
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
int unsparse_probe(char *source, unsigned int length)
{
	sparse_header_t *header = (sparse_header_t*) source;

	if (header->magic != SPARSE_HEADER_MAGIC)
	{
		printf("sparse: bad magic\n");

		return -1;
	}

	if ((header->major_version != SPARSE_HEADER_MAJOR_VER) ||
	    (header->file_hdr_sz != sizeof(sparse_header_t)) ||
	    (header->chunk_hdr_sz != sizeof(chunk_header_t)))
	{
		printf("sparse: incompatible format\n");

		return -1;
	}
	last_rest_size = 0;
	chunk_count = 0;
	chunk_length = 0;
	sparse_format_type = SPARSE_FORMAT_TYPE_TOTAL_HEAD;

	return 0;
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
int  unsparse_direct_write(void *pbuf, int length, FILE *dfile)
{
	int   unenough_length;
	int   this_rest_size;
	int   tmp_down_size;
	char *tmp_buf, *tmp_dest_buf;
	chunk_header_t   *chunk;

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
				//printf("chunk index = %d\n", chunk_count ++);

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
						wirte_to_0xff(dfile, chunk_length);
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
					fwrite(tmp_buf, chunk_length, 1, dfile);
					if(chunk_length & 511)
					{
						printf("data is not sector align 0\n");

						return -1;
					}
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
					fwrite(tmp_buf, tmp_down_size, 1, dfile);
					if(tmp_down_size & 511)
					{
						printf("data is not sector align 1\n");

						return -1;
					}
					tmp_buf += tmp_down_size;
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

