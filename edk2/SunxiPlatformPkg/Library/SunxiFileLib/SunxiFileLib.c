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

#include <Protocol/BlockIo.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DevicePath.h>


#include <Guid/FileInfo.h>


VOID *
EFIAPI
AwGetFileBufferByFilePath (
  IN CONST EFI_DEVICE_PATH_PROTOCOL    *FilePath,
  OUT      UINTN                       *FileSize )
{
  EFI_DEVICE_PATH_PROTOCOL          *DevicePathNode;
  EFI_DEVICE_PATH_PROTOCOL          *OrigDevicePathNode;
  EFI_DEVICE_PATH_PROTOCOL          *TempDevicePathNode;
  EFI_HANDLE                        Handle;
  UINT8                             *ImageBuffer;
  UINTN                             ImageBufferSize;

  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *Volume;
  EFI_FILE_HANDLE                   FileHandle;
  EFI_FILE_HANDLE                   LastHandle;
  EFI_FILE_INFO                     *FileInfo;
  UINTN                             FileInfoSize;

  EFI_STATUS                        Status;

  //
  // Check input File device path.
  //
  if (FilePath == NULL || FileSize == NULL ) {
    return NULL;
  }

  //
  // Init local variable
  //
  TempDevicePathNode  = NULL;
  FileInfo            = NULL;
  FileHandle          = NULL;
  ImageBuffer         = NULL;
  ImageBufferSize     = 0;
  
  //
  // Copy File Device Path
  //
  OrigDevicePathNode = DuplicateDevicePath (FilePath);
  if (OrigDevicePathNode == NULL) {
    return NULL;
  }

  //
  // Attempt to access the file via a file system interface
  //
  DevicePathNode = OrigDevicePathNode;
  Status = gBS->LocateDevicePath (&gEfiSimpleFileSystemProtocolGuid, &DevicePathNode, &Handle);
  if (!EFI_ERROR (Status)) {
    Status = gBS->HandleProtocol (Handle, &gEfiSimpleFileSystemProtocolGuid, (VOID**)&Volume);
    if (!EFI_ERROR (Status)) {
      //
      // Open the Volume to get the File System handle
      //
      Status = Volume->OpenVolume (Volume, &FileHandle);
      if (!EFI_ERROR (Status)) {
        //
        // Duplicate the device path to avoid the access to unaligned device path node.
        // Because the device path consists of one or more FILE PATH MEDIA DEVICE PATH
        // nodes, It assures the fields in device path nodes are 2 byte aligned.
        //
        TempDevicePathNode = DuplicateDevicePath (DevicePathNode);
        if (TempDevicePathNode == NULL) {
          FileHandle->Close (FileHandle);
          //
          // Setting Status to an EFI_ERROR value will cause the rest of
          // the file system support below to be skipped.
          //
          Status = EFI_OUT_OF_RESOURCES;
        }
        //
        // Parse each MEDIA_FILEPATH_DP node. There may be more than one, since the
        // directory information and filename can be seperate. The goal is to inch
        // our way down each device path node and close the previous node
        //
        DevicePathNode = TempDevicePathNode;
        while (!EFI_ERROR (Status) && !IsDevicePathEnd (DevicePathNode)) {
          if (DevicePathType (DevicePathNode) != MEDIA_DEVICE_PATH ||
              DevicePathSubType (DevicePathNode) != MEDIA_FILEPATH_DP) {
            Status = EFI_UNSUPPORTED;
            break;
          }
  
          LastHandle = FileHandle;
          FileHandle = NULL;
  
          Status = LastHandle->Open (
                                LastHandle,
                                &FileHandle,
                                ((FILEPATH_DEVICE_PATH *) DevicePathNode)->PathName,
                                EFI_FILE_MODE_READ,
                                0
                                );
  
          //
          // Close the previous node
          //
          LastHandle->Close (LastHandle);
  
          DevicePathNode = NextDevicePathNode (DevicePathNode);
        }
  
        if (!EFI_ERROR (Status)) {
          //
          // We have found the file. Now we need to read it. Before we can read the file we need to
          // figure out how big the file is.
          //
          FileInfo = NULL;
          FileInfoSize = 0;
          Status = FileHandle->GetInfo (
                                FileHandle,
                                &gEfiFileInfoGuid,
                                &FileInfoSize,
                                FileInfo
                                );
  
          if (Status == EFI_BUFFER_TOO_SMALL) {
            FileInfo = AllocatePool (FileInfoSize);
            if (FileInfo == NULL) {
              Status = EFI_OUT_OF_RESOURCES;
            } else {
              Status = FileHandle->GetInfo (
                                    FileHandle,
                                    &gEfiFileInfoGuid,
                                    &FileInfoSize,
                                    FileInfo
                                    );
            }
          }
          
          if (!EFI_ERROR (Status) && (FileInfo != NULL)) {
            //
            // Allocate space for the file
            //
            ImageBuffer = AllocatePool ((UINTN)FileInfo->FileSize);
            if (ImageBuffer == NULL) {
              Status = EFI_OUT_OF_RESOURCES;
            } else {
              //
              // Read the file into the buffer we allocated
              //
              ImageBufferSize = (UINTN)FileInfo->FileSize;
              Status          = FileHandle->Read (FileHandle, &ImageBufferSize, ImageBuffer);
            }
          }
        }
        //
        // Close the file and Free FileInfo and TempDevicePathNode since we are done
        // 
        if (FileInfo != NULL) {
          FreePool (FileInfo);
        }
        if (FileHandle != NULL) {
          FileHandle->Close (FileHandle);
        }
        if (TempDevicePathNode != NULL) {
          FreePool (TempDevicePathNode);
        }
      }
    }
    if (!EFI_ERROR (Status)) {
      goto Finish;
    }
  }

  

Finish:

  if (EFI_ERROR (Status)) {
    if (ImageBuffer != NULL) {
      FreePool (ImageBuffer);
      ImageBuffer = NULL;
    }
    *FileSize = 0;
  } else {
    *FileSize = ImageBufferSize;
  }

  FreePool (OrigDevicePathNode);
  return ImageBuffer;
}


EFI_STATUS 
EFIAPI
AwWriteFileBufferByFilePath (
  IN CONST EFI_DEVICE_PATH_PROTOCOL    *FilePath,
  IN       VOID                        *FileBuffer,
  IN       UINTN                        FileSize)
{
  EFI_DEVICE_PATH_PROTOCOL          *DevicePathNode;
  EFI_DEVICE_PATH_PROTOCOL          *OrigDevicePathNode;
  EFI_DEVICE_PATH_PROTOCOL          *TempDevicePathNode;
  EFI_HANDLE                        Handle;
  UINT8                             *ImageBuffer;
  UINTN                             ImageBufferSize;

  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *Volume;
  EFI_FILE_HANDLE                   FileHandle;
  EFI_FILE_HANDLE                   LastHandle;
  //EFI_FILE_INFO                     *FileInfo;
  //UINTN                             FileInfoSize;

  EFI_STATUS                        Status;
  UINT64                            OpenMode;

  //
  // Check input File device path.
  //
  if (FileBuffer == NULL || FilePath == NULL || FileSize == 0 ) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Init local variable
  //
  TempDevicePathNode  = NULL;
  //FileInfo            = NULL;
  FileHandle          = NULL;
  ImageBufferSize = FileSize;
  ImageBuffer = FileBuffer;

  //
  // Copy File Device Path
  //
  OrigDevicePathNode = DuplicateDevicePath (FilePath);
  if (OrigDevicePathNode == NULL) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Attempt to access the file via a file system interface
  //
  DevicePathNode = OrigDevicePathNode;
  Status = gBS->LocateDevicePath (&gEfiSimpleFileSystemProtocolGuid, &DevicePathNode, &Handle);
  if (!EFI_ERROR (Status)) {
    Status = gBS->HandleProtocol (Handle, &gEfiSimpleFileSystemProtocolGuid, (VOID**)&Volume);
    if (!EFI_ERROR (Status)) {
      //
      // Open the Volume to get the File System handle
      //
      Status = Volume->OpenVolume (Volume, &FileHandle);
      if (!EFI_ERROR (Status)) {
        //
        // Duplicate the device path to avoid the access to unaligned device path node.
        // Because the device path consists of one or more FILE PATH MEDIA DEVICE PATH
        // nodes, It assures the fields in device path nodes are 2 byte aligned.
        //
        TempDevicePathNode = DuplicateDevicePath (DevicePathNode);
        if (TempDevicePathNode == NULL) {
          FileHandle->Close (FileHandle);
          //
          // Setting Status to an EFI_ERROR value will cause the rest of
          // the file system support below to be skipped.
          //
          Status = EFI_OUT_OF_RESOURCES;
        }
        //
        // Parse each MEDIA_FILEPATH_DP node. There may be more than one, since the
        // directory information and filename can be seperate. The goal is to inch
        // our way down each device path node and close the previous node
        //
        DevicePathNode = TempDevicePathNode;
        while (!EFI_ERROR (Status) && !IsDevicePathEnd (DevicePathNode)) {
          if (DevicePathType (DevicePathNode) != MEDIA_DEVICE_PATH ||
              DevicePathSubType (DevicePathNode) != MEDIA_FILEPATH_DP) {
            Status = EFI_UNSUPPORTED;
            break;
          }
  
          LastHandle = FileHandle;
          FileHandle = NULL;
          OpenMode = EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE;
          Status = LastHandle->Open (
                                LastHandle,
                                &FileHandle,
                                ((FILEPATH_DEVICE_PATH *) DevicePathNode)->PathName,
                                OpenMode,
                                0
                                );
  
          //
          // Close the previous node
          //
          LastHandle->Close (LastHandle);
  
          DevicePathNode = NextDevicePathNode (DevicePathNode);
        }

        if (!EFI_ERROR (Status))
        {
          Status   = FileHandle->Write(FileHandle, &ImageBufferSize, ImageBuffer);
        }
        else
        {
          AsciiPrint("Open File Error: %r\n", Status);
        }

        if(!EFI_ERROR(Status))
        {
          Status = FileHandle->Flush(FileHandle);
          if(Status)
          {
            AsciiPrint("Flush File Error: %r\n", Status);
          }
        }
        
        if (FileHandle != NULL) {
          FileHandle->Close (FileHandle);
        }
        if (TempDevicePathNode != NULL) {
          FreePool (TempDevicePathNode);
        }
      }
    }
    if (!EFI_ERROR (Status)) 
    {
      goto Finish;
    }
  }

Finish:

  FreePool (OrigDevicePathNode);
  return Status;
}


EFI_STATUS
ReadFileFromAwFs (
  IN  CHAR16*      FilePathName,
  OUT VOID         **FileBuffer,
  OUT UINTN        *FileSize
  )
{
  EFI_STATUS           Status;
  UINTN                Index;
  UINTN                HandleCount;
  EFI_HANDLE           *HandleBuffer;
  //EFI_HANDLE            ImageHandle;
  EFI_DEVICE_PATH_PROTOCOL  *LoadImageDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *FileSystemDevicePath;
  //EFI_BLOCK_IO_PROTOCOL     *BlkIo;

  //Locate handles for SimpleFileSystem protocol
  Status = gBS->LocateHandleBuffer (
                ByProtocol,
                &gEfiSimpleFileSystemProtocolGuid,
                NULL,
                &HandleCount,
                &HandleBuffer
                );
  if (EFI_ERROR(Status)) 
  {
    AsciiPrint("%a,Access simple file system failure\n",__FUNCTION__);
    return Status;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    //Get the device path
    FileSystemDevicePath = DevicePathFromHandle(HandleBuffer[Index]);
    if (FileSystemDevicePath == NULL) {
    AsciiPrint("get device path failure\n");
      continue;
    }

    LoadImageDevicePath = FileDevicePath(HandleBuffer[Index], FilePathName);
    *FileBuffer = AwGetFileBufferByFilePath (
                    LoadImageDevicePath,
                    FileSize
                    );
    if(*FileBuffer == NULL)
    {
      AsciiPrint("read file is NULL\n");
      continue;
    }
     
    return EFI_SUCCESS;
  }

  return EFI_LOAD_ERROR;  
}


EFI_STATUS
WriteFileToAwFs (
  IN  CHAR16*      FilePathName,
  IN  VOID         *FileBuffer,
  OUT UINTN        FileSize
  )
{
  EFI_STATUS           Status;
  UINTN                Index;
  UINTN                HandleCount;
  EFI_HANDLE           *HandleBuffer;
  //EFI_HANDLE            ImageHandle;
  EFI_DEVICE_PATH_PROTOCOL  *LoadImageDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *FileSystemDevicePath;
  EFI_BLOCK_IO_PROTOCOL     *BlkIo;

  //Locate handles for SimpleFileSystem protocol
  Status = gBS->LocateHandleBuffer (
                ByProtocol,
                &gEfiSimpleFileSystemProtocolGuid,
                NULL,
                &HandleCount,
                &HandleBuffer
                );
  if (EFI_ERROR(Status)) 
  {
    return Status;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    //Get the device path
    FileSystemDevicePath = DevicePathFromHandle(HandleBuffer[Index]);
    if (FileSystemDevicePath == NULL) {
      continue;
    }

    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiBlockIoProtocolGuid,
                    (VOID **) &BlkIo
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

    if(BlkIo->Media->MediaId != SIGNATURE_32('a','w','f','s'))
    {
      continue;
    }
     
    LoadImageDevicePath = FileDevicePath(HandleBuffer[Index], FilePathName);
    Status = AwWriteFileBufferByFilePath(
                  LoadImageDevicePath,
                  FileBuffer,
                  FileSize
                  );
    if(EFI_ERROR(Status))
    {
      return Status;
    }

    return EFI_SUCCESS;
  }

  AsciiPrint("Can't Find BlockIO Procotol AwFs\n");
  return Status;
}


