/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  Martin.Zheng <martinzheng@allwinnertech.com>
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

#include <Library/IoLib.h>
#include <Sun8iW1/platform.h>
#include "Ss_hal.h"

static UINT32 aw_endian4(UINT32 data)

{

  UINT32 d1, d2, d3, d4;

  d1= (data&0xff)<<24;

  d2= (data&0xff00)<<8;

  d3= (data&0xff0000)>>8;

  d4= (data&0xff000000)>>24;

  return (d1|d2|d3|d4);  

}
#if 0
static UINT32 aw_endian2(UINT32 data)

{

  UINT32 d1, d2;

  d1= (data&0xff)<<8;

  d2= (data&0xff00)>>8;

  return (d1|d2);

}
#endif
 


//*****************************************************************************
//  void ss_init(void)
//  Description:  Start SS
//
//  Arguments:    none
//  
//  Return Value: None
//*****************************************************************************
void ss_init(void)
{
}
//*****************************************************************************
//  void ss_exit(void)
//  Description:  Disable SS
//
//  Arguments:    none
//  
//  Return Value: None
//*****************************************************************************
void ss_exit(void)
{
  UINT32 reg_val;
  
  reg_val = ss_read_w(SS_CTL);
  reg_val &= ~0x1;
  ss_write_w(SS_CTL, reg_val);  
}
//*****************************************************************************
//  UINT32 ss_io_ctl(UINT32 io_type, UINT32* io_data)
//  Description:  Multi IO control function for SS
//
//  Arguments:    UINT32 io_type      
//                UINT32* io_data
//  
//  Return Value: None
//*****************************************************************************
UINT32 ss_io_ctl(UINT32 io_type, UINT32* io_data)
{
  UINT32 i, k;
  UINT32 reg_val;
  
  switch(io_type)
  {
    case SSIO_SET_KEY:
      k = *io_data;     //key number in word unit
      for(i=0; i<k; i++)
      {
        ss_write_w(SS_KEY + 4*i, *(io_data + i + 1));
//        aw_delay(0x100);
      }
      break;
    case SSIO_SET_IV:
      k = *io_data;     //IV number in word unit
      if(k > 4)
      {
        for(i=0; i<4; i++)
        {
          ss_write_w(SS_IV + 4*i, *(io_data + i + 1));
//          aw_delay(0x100);
        }
        ss_write_w(SS_CNT, *(io_data + 5));
//        aw_delay(0x100);
      }
      else
      {
        for(i=0; i<k; i++)
        {
          ss_write_w(SS_IV + 4*i, *(io_data + i + 1));
//          aw_delay(0x100);
        }
      }
      break;
    case SSIO_SET_CNT:
      k = *io_data;     //CNT number in word unit
      for(i=0; i<k; i++)
      {
        ss_write_w(SS_CNT + 4*i, *(io_data + i + 1));
//        aw_delay(0x100);
      }
      break;
    case SSIO_SET_CTS_LENGTH:
      ss_write_w(SS_CTS_LEN, *io_data);
      break;
    case SSIO_SETUP_AES_DES:
      reg_val = ss_read_w(SS_CTL);
      reg_val &= ~(0x7<<4);   
      reg_val |= (*io_data&0x7)<<4;   //0: AES, 1: DES, 2: 3DES     
      reg_val &= ~(0x1<<7);     
      if(*(io_data + 1))
        reg_val |= 0x1<<7;      //decryption
      reg_val &= ~(0x3<<8);
      if(*(io_data + 2) == 256)
        reg_val |= 0x2<<8;      //256 bits key
      else if(*(io_data + 2) == 192)
        reg_val |= 0x1<<8;      //192 bits key
      else
        reg_val |= 0x0<<8;      //128 bits key
      reg_val &= ~(0x3<<12);
      reg_val |= (*(io_data + 3)&0x3)<<12;    //operation mode(ECB, CBC, CNT)
      reg_val &= ~(0x3<<10);
      if(*(io_data + 4) == 64)
        reg_val |= 0x2<<10;     //64 bits CNT width for CNT mode
      else if(*(io_data + 4) == 32)
        reg_val |= 0x1<<10;     //32 bits CNT width for CNT mode
      else
        reg_val |= 0x0<<10;     //16 bits CNT width for CNT mode      
      ss_write_w(SS_CTL, reg_val);
      break;
    case SSIO_SETUP_SHA1_MD5:
      reg_val = ss_read_w(SS_CTL);
      reg_val &= ~(0x7<<4);   
      if((*io_data&0x7) == 0)
        reg_val |= 0x3<<4;      //SHA1
      else if((*io_data&0x7) == 1)
        reg_val |= 0x4<<4;      //MD5       
      reg_val &= ~(0x1<<14);
      if(*(io_data + 1))
        reg_val |= 0x1<<14;     //IV steady of its constants  
      ss_write_w(SS_CTL, reg_val);    
      break;
    case SSIO_START:
      reg_val = ss_read_w(SS_CTL);
      reg_val |= 0x1;
      ss_write_w(SS_CTL, reg_val);      
      break;
    case SSIO_STOP:
      reg_val = ss_read_w(SS_CTL);
      reg_val &= ~0x1;
      ss_write_w(SS_CTL, reg_val);      
      break;
    case SSIO_ENABLE_DMA:
      reg_val = ss_read_w(SS_ICSR);
      reg_val |= 0x1<<4;
      ss_write_w(SS_ICSR, reg_val);           
      break;
    case SSIO_DISABLE_DMA:
      reg_val = ss_read_w(SS_ICSR);
      reg_val &= ~(0x1<<4);
      ss_write_w(SS_ICSR, reg_val);                 
      break;
    case SSIO_TX_INT:
      reg_val = ss_read_w(SS_ICSR);
      if(*(io_data))
        reg_val |= 0x1;       
      else
        reg_val &= ~0x1;
      ss_write_w(SS_ICSR, reg_val);                 
      break;
    case SSIO_RX_INT:
      reg_val = ss_read_w(SS_ICSR);
      if(*(io_data))
        reg_val |= 0x1<<2;        
      else
        reg_val &= ~(0x1<<2);
      ss_write_w(SS_ICSR, reg_val);                 
      break;
    case SSIO_RX_TRIGGER_LEVEL:
      reg_val = ss_read_w(SS_FCSR);
      reg_val &= ~(0x1f<<8);
      reg_val |= ((*io_data - 1)&0x1f)<<8;
      ss_write_w(SS_FCSR, reg_val);                 
      break;
    case SSIO_TX_TRIGGER_LEVEL:
      reg_val = ss_read_w(SS_FCSR);
      reg_val &= ~0x1f;
      reg_val |= (*io_data - 1)&0x1f;
      ss_write_w(SS_FCSR, reg_val);                 
      break;
    case SSIO_SETUP_RANDOM:
      reg_val = ss_read_w(SS_CTL);
      reg_val &= ~(0x7<<4);   
      reg_val |= 0x5<<4;      //random data
      reg_val &= ~(0x1<<15);
      if(*(io_data))
        reg_val |= 0x1<<15;   //0: one shot mode, 1: continue mode  
      ss_write_w(SS_CTL, reg_val);    
      break;
    case SSIO_RANDOM_ONESHOT_START:
      reg_val = ss_read_w(SS_CTL);
      reg_val |= 0x1<<1;
//      reg_val |= 0x1;
      ss_write_w(SS_CTL, reg_val);    
      break;
    case SSIO_KEY_SELECT:
      reg_val = ss_read_w(SS_CTL);
      reg_val &= ~(0xf<<24);
      reg_val |= ((*io_data)<<24);      
      ss_write_w(SS_CTL, reg_val);    
      break;
    case SSIO_GET_IV:
      for(i=0; i<4; i++)
      {
        *(io_data + i ) = ss_read_w(SS_IV + 4*i);
      }
      break;
    default:
      break;    
  } 
  return (1); 
}

//*****************************************************************************
//  UINT32 ss_get_rxfifo_room_size(void)
//  Description:  Get RX FIFO empty room in word unit
//                The 32 words RX FIFO is used for sending data (plaintext/ciphertext)
//                to SS engine
//  Arguments:    None      
//                
//  
//  Return Value: RX FIFO empry room size in word unit
//*****************************************************************************
UINT32 ss_get_rxfifo_room_size(void)
{
  UINT32 reg_val;
  
  reg_val = ss_read_w(SS_FCSR);
  reg_val = reg_val>>24;
  reg_val &= 0x3f;
  
  return reg_val;
}

//*****************************************************************************
//  UINT32 ss_get_txfifo_data_cnt(void)
//  Description:  Get TX FIFO available data counter in word unit
//                The 32 words TX FIFO is used for receiving data (plaintext/ciphertext)
//                from SS engine
//  Arguments:    None      
//                
//  
//  Return Value: TX FIFO available data counter in word unit
//*****************************************************************************
UINT32 ss_get_txfifo_data_cnt(void)
{
  UINT32 reg_val;
  
  reg_val = ss_read_w(SS_FCSR);
  reg_val = reg_val>>16;
  reg_val &= 0x3f;
  
  return reg_val;
}

//*****************************************************************************
//  void ss_send_data(UINT32 data)
//  Description:  
//                Sending one word data (plaintext/ciphertext) to SS engine
//  Arguments:    None      
//                
//  
//  Return Value: void
//*****************************************************************************
void ss_send_data(UINT32 data)
{
  ss_write_w(SS_RXFIFO, data);
}
//*****************************************************************************
//  void ss_send_data(UINT32 data)
//  Description:  
//                Sending one word data (plaintext/ciphertext) to SS engine
//  Arguments:    None      
//                
//  
//  Return Value: void
//*****************************************************************************
void ss_receive_data(UINT32* data)
{
  *data = ss_read_w(SS_TXFIFO);
}

//*****************************************************************************
//  void ss_sha1md5_dataend(void)
//  Description:  
//                Set data end flag for SHA1/MD5 engine
//  Arguments:    None      
//                
//  
//  Return Value: void
//*****************************************************************************
void ss_sha1md5_dataend(void)
{
  UINT32 reg_val;
  
  //set end flag for SHA1/MD5
  reg_val = ss_read_w(SS_CTL);
  reg_val |= 0x1<<2;
  ss_write_w(SS_CTL, reg_val);
  
  //check whether SHA1/MD5 has finished operation
  while(ss_read_w(SS_CTL)&(0x1<<2)) {};
//  for(i=0; i<0x10000; i++){};                       //delay for data end
}
//*****************************************************************************
//  void ss_get_md(UINT32* md_buf)
//  Description:  Get Message Digest (MD) from SHA1/MD5 engine
//  Arguments:    UINT32* md_buf    Buffer for storing MD     
//                
//  
//  Return Value: void
//*****************************************************************************
void ss_get_md(UINT32* md_buf)
{
  UINT32 i;
  UINT32 reg_val;
  
  for(i=0; i<5; i++)
    md_buf[i] = ss_read_w(SS_MD + i*4);

  reg_val = ss_read_w(SS_CTL);
  reg_val >>= 4;
  reg_val &= 0x7;
  if(reg_val == 4){ //md5
    for(i=0; i<4; i++){
      md_buf[i] = aw_endian4(md_buf[i]);
    }
  }
}

void ss_debug(void)
{
  UINT32 reg_val;
  
  reg_val = ss_read_w(SS_CTL);
  reg_val |= 0x1U<<31;
  ss_write_w(SS_CTL, reg_val);  
}
//*****************************************************************************
//  void ss_send_text_to_sha1md5(UINT32 data_size, UINT32 total_size, UINT32 last, UINT32* text)
//  Description:  Send text to SHA1/MD5 engine for generating message digest 
//                and padding_buf bits are added by this function
//  Arguments:    UINT32 data_size
//                       text number of text buffer (in byte unit, <=64 bytes)
//                       If it is not the last text, data_size should be 64.
//                UINT32 total_size  
//                       total text number (in byte unit)
//                UINT32 last
//                       text last flag (0: not last text, 1: last text)
//                UINT32* text
//                       pointer to text buffer
//  
//  Return Value: void
//*****************************************************************************
void ss_sha1md5_text(UINT32 total_size, UINT32* in)
{
  UINT32 i;
  UINT32 k, q;
  UINT8* p_byte;
  
  k = total_size/64;
  q = total_size%64;
  p_byte = (UINT8*)in;
  
  if(q==0)
  {
    for(i=0; i< (k-1); i++)
    {
      ss_send_text_to_sha1md5(64, total_size, 0, (UINT32*)p_byte);
      p_byte +=64;
    }
    ss_send_text_to_sha1md5(64, total_size, 1, (UINT32*)p_byte);              
  }
  else
  {
    for(i=0; i< k; i++)
    {
      ss_send_text_to_sha1md5(64, total_size, 0, (UINT32*)p_byte);
      p_byte +=64;
    }
    ss_send_text_to_sha1md5(q, total_size, 1, (UINT32*)p_byte);
  }
  
}


void ss_send_text_to_sha1md5(UINT32 data_size, UINT32 total_size, UINT32 last, UINT32* text)
{
  UINT32 i;
  UINT32 padding_buf[16];
  UINT8 *ptext, *ptext_in;
  UINT32 reg_val;
  UINT32 md5_flag;

  reg_val = ss_read_w(SS_CTL);
  reg_val >>= 4;
  reg_val &= 0x7;
  if(reg_val == 4)
    md5_flag = 1;
  else
    md5_flag = 0;

  if( (last == 1) && (data_size == 64) )
  {
    //send 512-bits text
    for(i=0; i<16; i++)
    {
      while(ss_get_rxfifo_room_size()== 0){};
      ss_send_data(text[i]);        
    }
    //sending 512-bits padding_buf to SHA1/MD5
    for(i=0; i<16; i++)
      padding_buf[i] = 0x0;
    padding_buf[0] = 0x00000080;
    if(md5_flag)
    {
      padding_buf[14] = total_size<<3;
      padding_buf[15] = total_size>>29;
    }
    else
    {
      padding_buf[14] = total_size>>29;       
      padding_buf[15] = total_size<<3;
      padding_buf[14] = aw_endian4(padding_buf[14]);
      padding_buf[15] = aw_endian4(padding_buf[15]);        
    }
    for(i=0; i<16; i++)
    {
      while(ss_get_rxfifo_room_size()== 0){};
      ss_send_data(padding_buf[i]);       
    }     
  }
  else if( (last == 1) && (data_size < 56) )
  {
    //send text with padding_buf bits (total 512 bits) to SHA1/MD5
    for(i=0; i<16; i++)
      padding_buf[i] = 0x0;
    
    ptext = (UINT8*)padding_buf;
    ptext_in = (UINT8*)text;
    for(i=0; i<data_size; i++)
      ptext[i] = ptext_in[i];
    
    ptext[data_size] = 0x80;      
    if(md5_flag)
    {
      padding_buf[14] = total_size<<3;
      padding_buf[15] = total_size>>29;
    }
    else
    {
      padding_buf[14] = total_size>>29;       
      padding_buf[15] = total_size<<3;
      padding_buf[14] = aw_endian4(padding_buf[14]);
      padding_buf[15] = aw_endian4(padding_buf[15]);        
    }
    for(i=0; i<16; i++)
    {
      while(ss_get_rxfifo_room_size()== 0){};
      ss_send_data(padding_buf[i]);       
    }           
  }
  else if( (last == 1) && (data_size >= 56) )
  {
    //send text with padding_buf to SHA1/MD5
    for(i=0; i<16; i++)
      padding_buf[i] = 0x0;
    
    ptext = (UINT8*)padding_buf;
    ptext_in = (UINT8*)text;
    for(i=0; i<data_size; i++)
      ptext[i] = ptext_in[i];
    
    ptext[data_size] = 0x80;      
    for(i=0; i<16; i++)
    {
      while(ss_get_rxfifo_room_size()== 0){};
      ss_send_data(padding_buf[i]);     
    }           
    //send last 512-bits text to SHA1/MD5
    for(i=0; i<16; i++)
      padding_buf[i] = 0x0;
    if(md5_flag)
    {
      padding_buf[14] = total_size<<3;
      padding_buf[15] = total_size>>29;
    }
    else
    {
      padding_buf[14] = total_size>>29;       
      padding_buf[15] = total_size<<3;
      padding_buf[14] = aw_endian4(padding_buf[14]);
      padding_buf[15] = aw_endian4(padding_buf[15]);        
    }
    for(i=0; i<16; i++)
    {
      while(ss_get_rxfifo_room_size()== 0){};
      ss_send_data(padding_buf[i]);       
    }                 
  }   
  else //not last text
  {
    //send 512-bits text      
    for(i=0; i<16; i++)
    {
      while(ss_get_rxfifo_room_size()== 0){};
      ss_send_data(text[i]);        
    }
  }
}

UINT32 sha1md5_padding(UINT32 md5_flag, UINT32 data_size, UINT8* text)
{
  UINT32 i;
  UINT32 k, q;
  UINT32 size;
  UINT32 padding_buf[16];
  UINT8 *ptext;
  
  k = data_size/64;
  q = data_size%64;
  
  ptext = (UINT8*)padding_buf;
  if(q==0)
  {
    for(i=0; i<16; i++)
      padding_buf[i] = 0x0;
    
    padding_buf[0] = 0x00000080;
    if(md5_flag)
    {
      padding_buf[14] = data_size<<3;
      padding_buf[15] = data_size>>29;
    }
    else
    {
      padding_buf[14] = data_size>>29;        
      padding_buf[15] = data_size<<3;
      padding_buf[14] = aw_endian4(padding_buf[14]);
      padding_buf[15] = aw_endian4(padding_buf[15]);        
    }
    for(i=0; i<64; i++)
      text[k*64 + i] = ptext[i];  
    
    size = (k + 1)*64;
  }
  else if(q<56)
  {
    for(i=0; i<16; i++)
      padding_buf[i] = 0x0;
    for(i=0; i<q; i++)
      ptext[i] = text[k*64 + i];
    ptext[q] = 0x80;      
    if(md5_flag)
    {
      padding_buf[14] = data_size<<3;
      padding_buf[15] = data_size>>29;
    }
    else
    {
      padding_buf[14] = data_size>>29;        
      padding_buf[15] = data_size<<3;
      padding_buf[14] = aw_endian4(padding_buf[14]);
      padding_buf[15] = aw_endian4(padding_buf[15]);        
    }
    for(i=0; i<64; i++)
      text[k*64 + i] = ptext[i];
    size = (k + 1)*64;    
  }
  else
  {
    for(i=0; i<16; i++)
      padding_buf[i] = 0x0;
    for(i=0; i<q; i++)
      ptext[i] = text[k*64 + i];
    ptext[q] = 0x80;      
    for(i=0; i<64; i++)
      text[k*64 + i] = ptext[i];

    //send last 512-bits text to SHA1/MD5
    for(i=0; i<16; i++)
      padding_buf[i] = 0x0;
    if(md5_flag)
    {
      padding_buf[14] = data_size<<3;
      padding_buf[15] = data_size>>29;
    }
    else
    {
      padding_buf[14] = data_size>>29;        
      padding_buf[15] = data_size<<3;
      padding_buf[14] = aw_endian4(padding_buf[14]);
      padding_buf[15] = aw_endian4(padding_buf[15]);        
    }
    for(i=0; i<64; i++)
      text[(k + 1)*64 + i] = ptext[i];      
    size = (k + 2)*64;        
  }
  
  return size;  
}
//*****************************************************************************
//  void ss_check_randomend(void)
//  Description:  
//                Check whether random end in one shot mode
//  Arguments:    None      
//                
//  
//  Return Value: void
//*****************************************************************************
void ss_check_randomend(void)
{

  //check whether one group random data is OK in one shot mode
  while(ss_read_w(SS_CTL)&(0x1<<1)) {};
}

void  ss_get_random(UINT32* random_buf)
{
  UINT32 i;

  for(i=0; i<5; i++)
    random_buf[i] = ss_read_w(SS_MD + i*4); 
}
