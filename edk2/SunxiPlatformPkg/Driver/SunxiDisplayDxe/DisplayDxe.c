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



#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/UncachedMemoryAllocationLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/DevicePath.h>
#include <Protocol/HardwareInterrupt.h>
#include <Protocol/Cpu.h>
#include <sunxi_display/de.h>
#include <drv_display.h>



#include <Library/MemoryAllocationLib.h>
#include <Library/UncachedMemoryAllocationLib.h>
#include <Library/SunxiBootInfoLib.h>
#include <Interinc/sunxi_uefi.h>




typedef struct {
    VENDOR_DEVICE_PATH DisplayDevicePath;
    EFI_DEVICE_PATH EndDevicePath;
} DISPLAY_DEVICE_PATH;

DISPLAY_DEVICE_PATH gDisplayDevicePath =
{
  {
    HARDWARE_DEVICE_PATH,
    HW_VENDOR_DP,
    (UINT8)(sizeof(VENDOR_DEVICE_PATH)),
    (UINT8)((sizeof(VENDOR_DEVICE_PATH)) >> 8),
    EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    sizeof (EFI_DEVICE_PATH_PROTOCOL),
    0
  }
};

unsigned int    g_layer_para,g_layer_para1;
unsigned int    g_layer_hd,g_layer_hd1,g_layer_hd2;



extern int drv_disp_init(void);
//extern __s32 gpu_init(void);
//extern __s32 gpu_clk_on (void);



VOID Display_Initialize(UINTN FbBase)
{
  INT32 bitcount=32;
  INT32 screen_width,screen_height;

  drv_disp_init();
  /*open lcd*/
  
  board_display_device_open_lcd(0);
  board_display_device_open_hdmi(1);
  //board_display_layer_request(1);

  //get screen width and heigth
  screen_width = board_display_get_width(0);
  screen_height = board_display_get_heigth(0);
  DEBUG((DEBUG_INFO,"lcd screen_width =%d, screen_height =%d\n", screen_width, screen_height));
  ZeroMem((VOID*)(FbBase),screen_height*screen_width*4);
  board_display_framebuffer_set(screen_width,screen_height, bitcount, (VOID*)FbBase);

  screen_width = board_display_get_width(1);
  screen_height = board_display_get_heigth(1);
  DEBUG((DEBUG_INFO,"hdmi screen_width =%d, screen_height =%d\n", screen_width, screen_height));
  board_display_framebuffer_set_hdmi(screen_width,screen_height, bitcount, (VOID*)FbBase);

  board_display_show(0);
  board_display_show(1);
  
  //gpu_init();
  //gpu_clk_on();

}

EFI_STATUS
EFIAPI
DisplayQueryMode(
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL          *This,
  IN  UINT32                                ModeNumber,
  OUT UINTN                                 *SizeOfInfo,
  OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  **Info
  );

EFI_STATUS
EFIAPI
DisplaySetMode(
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
  IN  UINT32                       ModeNumber
  );

EFI_STATUS
EFIAPI
DisplayBlt(
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL            *This,
  IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL           *BltBuffer,   OPTIONAL
  IN  EFI_GRAPHICS_OUTPUT_BLT_OPERATION       BltOperation,
  IN  UINTN                                   SourceX,
  IN  UINTN                                   SourceY,
  IN  UINTN                                   DestinationX,
  IN  UINTN                                   DestinationY,
  IN  UINTN                                   Width,
  IN  UINTN                                   Height,
  IN  UINTN                                   Delta         OPTIONAL
  );

EFI_GRAPHICS_OUTPUT_PROTOCOL gDisplay = {
  DisplayQueryMode,
  DisplaySetMode,
  DisplayBlt,
  NULL
};


EFI_STATUS
EFIAPI
DisplayQueryMode(
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL          *This,
  IN  UINT32                                ModeNumber,
  OUT UINTN                                 *SizeOfInfo,
  OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  **Info
  )
{
  EFI_STATUS Status;

  Status = gBS->AllocatePool(
      EfiBootServicesData,
      sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION),
      (VOID **)Info
      );
  ASSERT_EFI_ERROR(Status);

  *SizeOfInfo = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);

  (*Info)->Version = This->Mode->Info->Version;
  (*Info)->HorizontalResolution = This->Mode->Info->HorizontalResolution;
  (*Info)->VerticalResolution = This->Mode->Info->VerticalResolution;
  (*Info)->PixelFormat = This->Mode->Info->PixelFormat;
  (*Info)->PixelsPerScanLine = This->Mode->Info->PixelsPerScanLine;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
DisplaySetMode(
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
  IN  UINT32                       ModeNumber
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
DisplayBlt(
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL            *This,
  IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL           *BltBuffer,   OPTIONAL
  IN  EFI_GRAPHICS_OUTPUT_BLT_OPERATION       BltOperation,
  IN  UINTN                                   SourceX,
  IN  UINTN                                   SourceY,
  IN  UINTN                                   DestinationX,
  IN  UINTN                                   DestinationY,
  IN  UINTN                                   Width,
  IN  UINTN                                   Height,
  IN  UINTN                                   Delta         OPTIONAL
  )
{
  UINT8 *VidBuf, *BltBuf, *VidBuf1;
  UINTN i;

  switch(BltOperation) {
    case EfiBltVideoFill:
      BltBuf = (UINT8 *)BltBuffer;
      for(i=0;i<Height;i++) {
        VidBuf = (UINT8 *)((UINT32)This->Mode->FrameBufferBase + \
          (DestinationY + i)*This->Mode->Info->PixelsPerScanLine*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) + \
           DestinationX*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
        gBS->SetMem((VOID *)VidBuf,sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)*Width, *BltBuf);
      }
      break;

    case EfiBltVideoToBltBuffer:
      if(Delta == 0)
        Delta = Width * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);

      for(i=0;i<Height;i++) {
        VidBuf = (UINT8 *)((UINT32)This->Mode->FrameBufferBase + \
          (SourceY + i)*This->Mode->Info->PixelsPerScanLine*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) + \
          SourceX*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
        BltBuf = (UINT8 *)((UINT32)BltBuffer + (DestinationY + i)*Delta + DestinationX*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
        gBS->CopyMem((VOID *)BltBuf, (VOID *)VidBuf, sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)*Width);
      }
      break;

    case EfiBltBufferToVideo:
      if(Delta == 0)
        Delta = Width * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);

      for(i=0;i<Height;i++) {
        VidBuf = (UINT8 *)((UINT32)This->Mode->FrameBufferBase + \
         (DestinationY + i)*This->Mode->Info->PixelsPerScanLine*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) + \
        DestinationX*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
        BltBuf = (UINT8 *)((UINT32)BltBuffer + (SourceY + i)*Delta + SourceX*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
        gBS->CopyMem((VOID *)VidBuf, (VOID *)BltBuf, sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)*Width);
      }
      break;

    case EfiBltVideoToVideo:
      for(i=0;i<Height;i++) {
        VidBuf = (UINT8 *)((UINT32)This->Mode->FrameBufferBase + \
        (SourceY + i)*This->Mode->Info->PixelsPerScanLine*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) + \
         SourceX*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));

        VidBuf1 = (UINT8 *)((UINT32)This->Mode->FrameBufferBase + \
         (DestinationY + i)*This->Mode->Info->PixelsPerScanLine*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) + \
         DestinationX*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
        gBS->CopyMem((VOID *)VidBuf1, (VOID *)VidBuf, sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)*Width);
      }
      break;

    default:
      ASSERT_EFI_ERROR(EFI_SUCCESS);
  }

  return EFI_SUCCESS;
}


#define readl(addr) (*(volatile u32 *) (addr))

/**
  Initialize the state information for the Display Dxe

  @param  ImageHandle   of the loaded driver
  @param  SystemTable   Pointer to the System Table

  @retval EFI_SUCCESS           Protocol registered
  @retval EFI_OUT_OF_RESOURCES  Cannot allocate protocol data structure
  @retval EFI_DEVICE_ERROR      Hardware problems

**/
EFI_STATUS
EFIAPI
DisplayDxeInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS Status = EFI_SUCCESS;
  EFI_HANDLE      gUEFIDisplayHandle = NULL;
  EFI_GUID GraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  EFI_GUID DevicePathProtocolGuid = EFI_DEVICE_PATH_PROTOCOL_GUID;

  struct spare_boot_head_t* spare_head;
  UINT64 DramSize = 0;
  UINTN FbBase = 0;
  INT32 screen_width,screen_height;
  #if 0
  if(SunxiGetBootWorkMode()!=WORK_MODE_BOOT){
    return Status;
  }
  #endif
  spare_head = get_spare_head(PcdGet32 (PcdFdBaseAddress));
  DramSize = (spare_head->boot_data.dram_para[4]&0xffff)*1024*1024;
  //DramSize = 2<<30;
  FbBase  = PcdGet64(PcdSystemMemoryBase) + DramSize - PcdGet64(PcdFrameBufferSize);

  DEBUG((DEBUG_INFO, "FbBase=%x\n", FbBase));
  /* Initialize Display */
  Display_Initialize(FbBase);


  screen_width = board_display_get_width(0);
  screen_height = board_display_get_heigth(0);

  if(gDisplay.Mode == NULL){
    Status = gBS->AllocatePool(
        EfiBootServicesData,
        sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE),
        (VOID **)&gDisplay.Mode
        );
    ASSERT_EFI_ERROR(Status);
    ZeroMem(gDisplay.Mode,sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE));
  }
  if(gDisplay.Mode->Info==NULL){
    Status = gBS->AllocatePool(
        EfiBootServicesData,
        sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION),
        (VOID **)&gDisplay.Mode->Info
        );
    ASSERT_EFI_ERROR(Status);
    ZeroMem(gDisplay.Mode->Info,sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));
  }
  #if 1
  /* Fill out mode information */
  gDisplay.Mode->MaxMode = 1;
  gDisplay.Mode->Mode = 0;
  gDisplay.Mode->Info->Version = 0;
  gDisplay.Mode->Info->HorizontalResolution = screen_width;//gpanel_info[0].lcd_x;
  gDisplay.Mode->Info->VerticalResolution =screen_height;// gpanel_info[0].lcd_y;
  gDisplay.Mode->Info->PixelFormat = PixelBlueGreenRedReserved8BitPerColor;
  gDisplay.Mode->Info->PixelsPerScanLine = screen_width;//gpanel_info[0].lcd_x;
  gDisplay.Mode->SizeOfInfo = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
  gDisplay.Mode->FrameBufferBase = (EFI_PHYSICAL_ADDRESS)FbBase;
  gDisplay.Mode->FrameBufferSize = (EFI_PHYSICAL_ADDRESS)PcdGet64(PcdFrameBufferSize);
 // gDisplay.Mode->FrameBufferSize = (gpanel_info[0].lcd_x * gpanel_info[0].lcd_y * 4);
  #endif
  Status = gBS->InstallMultipleProtocolInterfaces (
               &gUEFIDisplayHandle,
               &DevicePathProtocolGuid,
               &gDisplayDevicePath,
               &GraphicsOutputProtocolGuid,
               &gDisplay,
               NULL);
  return Status;
}
