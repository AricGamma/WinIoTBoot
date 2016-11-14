/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  wangwei <wangwei@allwinnertech.com>
*  
*  This program and the accompanying materials                          
*  are licensed and made available under the terms and conditions of the BSD License         
*  which accompanies this distribution.  The full text of the license may be found at        
*  http://opensource.org/licenses/bsd-license.php                                            
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
*
Module Name:

  Graphics.c

Abstract:

  Support for Basic Graphics operations.

  BugBug: Currently *.BMP files are supported. This will be replaced
          when Tiano graphics format is supported.

--*/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/SunxiFileLib.h>

#include <Protocol/GraphicsOutput.h>

#include <Guid/FileInfo.h>

#include <IndustryStandard/Bmp.h>


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
{
  UINT8                         *Image;
  UINT8                         *ImageHeader;
  BMP_IMAGE_HEADER              *BmpHeader;
  BMP_COLOR_MAP                 *BmpColorMap;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Blt;
  UINT64                        BltBufferSize;
  UINTN                         Index;
  UINTN                         Height;
  UINTN                         Width;
  UINTN                         ImageIndex;
  UINTN                         DataSizePerLine;
  BOOLEAN                       IsAllocated;
  UINT32                        ColorMapNum;

  if (sizeof (BMP_IMAGE_HEADER) > BmpImageSize) {
    return EFI_INVALID_PARAMETER;
  }

  BmpHeader = (BMP_IMAGE_HEADER *) BmpImage;

  if (BmpHeader->CharB != 'B' || BmpHeader->CharM != 'M') {
    return EFI_UNSUPPORTED;
  }

  //
  // Doesn't support compress.
  //
  if (BmpHeader->CompressionType != 0) {
    return EFI_UNSUPPORTED;
  }

  //
  // Only support BITMAPINFOHEADER format.
  // BITMAPFILEHEADER + BITMAPINFOHEADER = BMP_IMAGE_HEADER
  //
  if (BmpHeader->HeaderSize != sizeof (BMP_IMAGE_HEADER) - ((UINTN) &(((BMP_IMAGE_HEADER *)0)->HeaderSize))) {
    return EFI_UNSUPPORTED;
  }

  //
  // The data size in each line must be 4 byte alignment.
  //
  DataSizePerLine = ((BmpHeader->PixelWidth * BmpHeader->BitPerPixel + 31) >> 3) & (~0x3);
  BltBufferSize = MultU64x32 (DataSizePerLine, BmpHeader->PixelHeight);
  if (BltBufferSize > (UINT32) ~0) {
    return EFI_INVALID_PARAMETER;
  }

  if ((BmpHeader->Size != BmpImageSize)  
      || (BmpHeader->Size < BmpHeader->ImageOffset)
      // || (BmpHeader->Size - BmpHeader->ImageOffset !=  BmpHeader->PixelHeight * DataSizePerLine)
      ) 
  {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Calculate Color Map offset in the image.
  //
  Image       = BmpImage;
  BmpColorMap = (BMP_COLOR_MAP *) (Image + sizeof (BMP_IMAGE_HEADER));
  if (BmpHeader->ImageOffset < sizeof (BMP_IMAGE_HEADER)) {
    return EFI_INVALID_PARAMETER;
  }

  if (BmpHeader->ImageOffset > sizeof (BMP_IMAGE_HEADER)) {
    switch (BmpHeader->BitPerPixel) {
      case 1:
        ColorMapNum = 2;
        break;
      case 4:
        ColorMapNum = 16;
        break;
      case 8:
        ColorMapNum = 256;
        break;
      default:
        ColorMapNum = 0;
        break;
      }
    if (BmpHeader->ImageOffset - sizeof (BMP_IMAGE_HEADER) != sizeof (BMP_COLOR_MAP) * ColorMapNum) {
      return EFI_INVALID_PARAMETER;
    }
  }

  //
  // Calculate graphics image data address in the image
  //
  Image         = ((UINT8 *) BmpImage) + BmpHeader->ImageOffset;
  ImageHeader   = Image;

  BltBufferSize = MultU64x32 ((UINT64) BmpHeader->PixelWidth, BmpHeader->PixelHeight);
  //
  // Ensure the BltBufferSize * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL) doesn't overflow
  //
  if (BltBufferSize > DivU64x32 ((UINTN) ~0, sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL))) {
    return EFI_UNSUPPORTED;
  }
  BltBufferSize = MultU64x32 (BltBufferSize, sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL));

  IsAllocated   = FALSE;
  if (*GopBlt == NULL) {
    *GopBltSize = (UINTN) BltBufferSize;
    *GopBlt     = AllocatePool (*GopBltSize);
    IsAllocated = TRUE;
    if (*GopBlt == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
  } else {
    if (*GopBltSize < (UINTN) BltBufferSize) {
      *GopBltSize = (UINTN) BltBufferSize;
      return EFI_BUFFER_TOO_SMALL;
    }
  }

  *PixelWidth   = BmpHeader->PixelWidth;
  *PixelHeight  = BmpHeader->PixelHeight;

  //
  // Convert image from BMP to Blt buffer format
  //
  BltBuffer = *GopBlt;
  for (Height = 0; Height < BmpHeader->PixelHeight; Height++) {
    Blt = &BltBuffer[(BmpHeader->PixelHeight - Height - 1) * BmpHeader->PixelWidth];
    for (Width = 0; Width < BmpHeader->PixelWidth; Width++, Image++, Blt++) {
      switch (BmpHeader->BitPerPixel) {
        case 1:
          //
          // Convert 1bit BMP to 24-bit color
          //
          for (Index = 0; Index < 8 && Width < BmpHeader->PixelWidth; Index++) {
            Blt->Red    = BmpColorMap[((*Image) >> (7 - Index)) & 0x1].Red;
            Blt->Green  = BmpColorMap[((*Image) >> (7 - Index)) & 0x1].Green;
            Blt->Blue   = BmpColorMap[((*Image) >> (7 - Index)) & 0x1].Blue;
            Blt++;
            Width++;
          }

          Blt --;
          Width --;
          break;

        case 4:
          //
          // Convert BMP Palette to 24-bit color
          //
          Index       = (*Image) >> 4;
          Blt->Red    = BmpColorMap[Index].Red;
          Blt->Green  = BmpColorMap[Index].Green;
          Blt->Blue   = BmpColorMap[Index].Blue;
          if (Width < (BmpHeader->PixelWidth - 1)) {
            Blt++;
            Width++;
            Index       = (*Image) & 0x0f;
            Blt->Red    = BmpColorMap[Index].Red;
            Blt->Green  = BmpColorMap[Index].Green;
            Blt->Blue   = BmpColorMap[Index].Blue;
          }
          break;

        case 8:
          //
          // Convert BMP Palette to 24-bit color
          //
          Blt->Red    = BmpColorMap[*Image].Red;
          Blt->Green  = BmpColorMap[*Image].Green;
          Blt->Blue   = BmpColorMap[*Image].Blue;
          break;

        case 24:
          Blt->Blue   = *Image++;
          Blt->Green  = *Image++;
          Blt->Red    = *Image;
          break;
        case 32:
          Blt->Blue   = *Image++;
          Blt->Green  = *Image++;
          Blt->Red    = *Image++;
          Blt->Reserved = *Image;
          break;

        default:
          if (IsAllocated) {
            FreePool (*GopBlt);
            *GopBlt = NULL;
          }
          return EFI_UNSUPPORTED;
          break;
      };
    }

    ImageIndex = (UINTN) (Image - ImageHeader);
    if ((ImageIndex % 4) != 0) {
      //
      // Bmp Image starts each row on a 32-bit boundary!
      //
      Image = Image + (4 - (ImageIndex % 4));
    }
  }

  return EFI_SUCCESS;
}



EFI_STATUS
SunxiDisplayBmp (
  IN  CHAR8* szBmpFile
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
{
  EFI_STATUS                    Status;
  //EFI_CONSOLE_CONTROL_PROTOCOL  *ConsoleControl;
  UINT32                        SizeOfX;
  UINT32                        SizeOfY;
  INTN                          DestX;
  INTN                          DestY;
  UINT8                         *ImageData = NULL;
  UINTN                         ImageSize;
  UINTN                         BltSize;
  //UINTN                         CoordinateX;
  //UINTN                         CoordinateY;
  UINTN                         Height;
  UINTN                         Width;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Blt = NULL;
  CHAR16                        BmpFile[64]={0};


 // UINT32                        ColorDepth;
 // UINT32                        RefreshRate;
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;

  //
  // Try to open GOP first
  //
  //Status = gBS->HandleProtocol (gST->ConsoleOutHandle, &gEfiGraphicsOutputProtocolGuid, (VOID**)&GraphicsOutput);
  Status = gBS->LocateProtocol (&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **) &GraphicsOutput);
  if (EFI_ERROR (Status)) {
    AsciiPrint("get GOP fail\n");
    return EFI_UNSUPPORTED; 
  }

//  ConsoleControl->SetMode (ConsoleControl, EfiConsoleControlScreenGraphics);

  AsciiStrToUnicodeStr(szBmpFile,BmpFile);

  SizeOfX = GraphicsOutput->Mode->Info->HorizontalResolution;
  SizeOfY = GraphicsOutput->Mode->Info->VerticalResolution;

  //while(*((volatile UINT32*)(0x2a000000)) !=8);
  while (1) {
    ImageData = NULL;
    ImageSize = 0;

    Status =ReadFileFromAwFs (BmpFile,  (VOID**)&ImageData, &ImageSize);
    if (EFI_ERROR (Status)) {
      AsciiPrint("Read Logo file Fail, Please Check file exist\n");
      return EFI_UNSUPPORTED;
    }

    AsciiPrint("Read bmp file Ok, Size = %d\n", ImageSize);

    Blt = NULL;
    Status = ConvertBmpToGopBlt (
              ImageData,
              ImageSize,
              (VOID**)&Blt,
              &BltSize,
              &Height,
              &Width
              );
    if (EFI_ERROR (Status)) {
      break;
    }


    DestX = (SizeOfX - Width) / 2;
    DestY = (SizeOfY - Height) / 2;
     
    if ((DestX >= 0) && (DestY >= 0)) {
      Status = GraphicsOutput->Blt (
                          GraphicsOutput,
                          Blt,
                          EfiBltBufferToVideo,
                          0,
                          0,
                          (UINTN) DestX,
                          (UINTN) DestY,
                          Width,
                          Height,
                          Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
                          );
    }
    break;
  }

  if(ImageData)
  {
    gBS->FreePool (ImageData);
  }

  if(Blt)
  {
    gBS->FreePool (Blt);
  }

  return Status;
}
