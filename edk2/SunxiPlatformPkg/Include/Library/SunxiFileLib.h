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

#ifndef _EFI_SUNXIFILE_LIB_H_
#define _EFI_SUNXIFILE_LIB_H_

/*
 FileName        --file path name 
 **FileBuffer     -- a buffer which has file data 
*FileSize          -- the size of a file

*/
EFI_STATUS
ReadFileFromAwFs (
  IN  CHAR16*      FileName,
  OUT VOID         **FileBuffer,
  OUT UINTN        *FileSize
  );
  
  

EFI_STATUS
WriteFileToAwFs (
  IN  CHAR16*      FilePathName,
  IN VOID         *FileBuffer,
  IN UINTN        FileSize
  );

#endif
