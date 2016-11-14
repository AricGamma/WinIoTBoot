/** @file

  Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>
  
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __SM_BUS_LIB_H__
#define __SM_BUS_LIB_H__

extern EFI_STATUS
SmBusInit (
    VOID
    );
    
extern EFI_STATUS
SmbusBlockRead (
  OUT UINT8       *Buffer,
  IN  UINTN       Length
  );

extern EFI_STATUS
SmbusBlockWrite (
  IN UINT8       *Buffer,
  IN UINTN       Length
  );
  
#endif
