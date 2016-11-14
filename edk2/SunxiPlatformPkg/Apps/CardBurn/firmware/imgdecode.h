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


#ifndef __IMAGE_DECODE_H____
#define __IMAGE_DECODE_H____  

//------------------------------------------------------------------------------------------------------------
#define PLUGIN_TYPE       IMGDECODE_PLUGIN_TYPE
#define PLUGIN_NAME       "imgDecode"       //scott note
#define PLUGIN_VERSION      0x0100
#define PLUGIN_AUTHOR     "scottyu"
#define PLUGIN_COPYRIGHT    "scottyu"

//------------------------------------------------------------------------------------------------------------
//插件的通用接口
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
// image 解析接口
//------------------------------------------------------------------------------------------------------------
typedef void *    HIMAGE;

typedef void *    HIMAGEITEM;

extern   HIMAGE     Img_Open    (char * ImageFile);

extern   long long      Img_GetSize     (HIMAGE hImage);

extern   HIMAGEITEM   Img_OpenItem  (HIMAGE hImage, char * MainType, char * subType);

extern   long long    Img_GetItemSize (HIMAGE hImage, HIMAGEITEM hItem);

extern   uint       Img_GetItemStart(HIMAGE hImage, HIMAGEITEM hItem);

extern   uint       Img_ReadItem  (HIMAGE hImage, HIMAGEITEM hItem, void *buffer, uint buffer_size);

extern   int      Img_CloseItem (HIMAGE hImage, HIMAGEITEM hItem);

extern   void       Img_Close   (HIMAGE hImage);

//------------------------------------------------------------------------------------------------------------
// THE END !
//------------------------------------------------------------------------------------------------------------

#endif //__IMAGE_DECODE_H____

