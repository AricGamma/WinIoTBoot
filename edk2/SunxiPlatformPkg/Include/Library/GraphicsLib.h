/*++

Copyright (c) 2004 - 2006, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  GraphicsLib.h

Abstract:

 
--*/

#ifndef _EFI_GRAPHICS_LIB_H_
#define _EFI_GRAPHICS_LIB_H_

EFI_STATUS
ConvertBmpToGopBlt (
  IN  VOID      *BmpImage,
  IN  UINTN     BmpImageSize,
  IN OUT VOID   **GopBlt,
  IN OUT UINTN  *GopBltSize,
  OUT UINTN     *PixelHeight,
  OUT UINTN     *PixelWidth
  )
/*++

Routine Description:

  Convert a *.BMP graphics image to a GOP/UGA blt buffer. If a NULL Blt buffer
  is passed in a GopBlt buffer will be allocated by this routine. If a GopBlt
  buffer is passed in it will be used if it is big enough.

Arguments:

  BmpImage      - Pointer to BMP file

  BmpImageSize  - Number of bytes in BmpImage

  GopBlt        - Buffer containing GOP version of BmpImage.

  GopBltSize    - Size of GopBlt in bytes.

  PixelHeight   - Height of GopBlt/BmpImage in pixels

  PixelWidth    - Width of GopBlt/BmpImage in pixels


Returns: 

  EFI_SUCCESS           - GopBlt and GopBltSize are returned. 
  EFI_UNSUPPORTED       - BmpImage is not a valid *.BMP image
  EFI_BUFFER_TOO_SMALL  - The passed in GopBlt buffer is not big enough.
                          GopBltSize will contain the required size.
  EFI_OUT_OF_RESOURCES  - No enough buffer to allocate

--*/
;

EFI_STATUS
SunxiDisplayBmp (
  IN  CHAR8  *BmpFile
  )
/*++

Routine Description:

  Use Console Control to turn off UGA based Simple Text Out consoles from going
  to the UGA device. Put up LogoFile on every UGA device that is a console

Arguments:

  LogoFile - File name of logo to display on the center of the screen.


Returns: 

  EFI_SUCCESS           - ConsoleControl has been flipped to graphics and logo
                          displayed.
  EFI_UNSUPPORTED       - Logo not found

--*/
;

#endif
