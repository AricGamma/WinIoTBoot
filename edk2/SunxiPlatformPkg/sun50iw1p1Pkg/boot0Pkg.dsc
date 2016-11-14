#  
#  Copyright (c) 2007-2015, Allwinner Technology Co., Ltd. All rights reserved.
#  http://www.allwinnertech.com
#
#  tangmanliang <tangmanliang@allwinnertech.com>
#  
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#  
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = sun50iw1p1Pkg
  PLATFORM_GUID                  = A0936893-AEE0-48B2-B3D5-C2A295297D91
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/SunxiPlatform/$(PLATFORM_NAME)
  SUPPORTED_ARCHITECTURES        = ARM|AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = SunxiPlatformPkg/$(PLATFORM_NAME)/boot0Pkg.fdf
  


[LibraryClasses.common]
  SplLib | SunxiPlatformPkg/sun50iw1p1Pkg/Library/SplLib/SplLib.inf
  SunxiSpl | SunxiPlatformPkg/Library/SunxiSpl/SunxiSpl.inf
  Boot0SdMmcLib | SunxiPlatformPkg/sun50iw1p1Pkg/Library/Boot0SdMmcLib/Boot0SdMmcLib.inf
  #Sun50iW1P1DramLib | SunxiPlatformPkg/sun50iw1p1Pkg/Library/Sun50iW1P1DramLib/Sun50iW1P1Dram.inf
  

!if $(TOOLCHAIN) == "ARMLINUXGCC"
  DramLib|SunxiPlatformPkg/sun50iw1p1Pkg/Library/DramLib/DramLib_GCC.inf
!else
  DramLib|SunxiPlatformPkg/sun50iw1p1Pkg/Library/DramLib/DramLib_RVCT.inf

!endif

  #NandExtLib|SunxiPlatformPkg/sun50iw1p1Pkg/Driver/Nand/NandExtLib.inf

[LibraryClasses.ARM]
  ArmLib|ArmPkg/Library/ArmLib/ArmV7/ArmV7Lib.inf

[LibraryClasses.common.SEC]


[LibraryClasses.common.DXE_CORE]


[LibraryClasses.common.DXE_DRIVER]

[LibraryClasses.common.UEFI_APPLICATION]

[LibraryClasses.common.UEFI_DRIVER]

[LibraryClasses.common.DXE_RUNTIME_DRIVER]

[LibraryClasses.ARM]
  #
  # It is not possible to prevent the ARM compiler for generic intrinsic functions.
  # This library provides the instrinsic functions generate by a given compiler.
  # [LibraryClasses.ARM] and NULL mean link this library into all ARM images.
  #
  NULL|ArmPkg/Library/CompilerIntrinsicsLib/CompilerIntrinsicsLib.inf


[BuildOptions]
  XCODE:*_*_ARM_PLATFORM_FLAGS == -arch armv7 -I$(WORKSPACE)/SunxiPlatformPkg/$(PLATFORM_NAME)/Include/Sun50iW1P1

  GCC:*_*_ARM_PLATFORM_FLAGS == -march=armv7-a -I$(WORKSPACE)/SunxiPlatformPkg/$(PLATFORM_NAME)/Include/Sun50iW1P1 

  RVCT:*_*_ARM_PLATFORM_FLAGS == --cpu Cortex-A7 -I$(WORKSPACE)/SunxiPlatformPkg/$(PLATFORM_NAME)/Include/Sun50iW1P1 --licretry --diag_suppress=9931,9933 
  

  XCODE:*_*_ARM_ARCHCC_FLAGS = -DCONFIG_ARCH_SUN50IW1P1 
  
  GCC:*_*_ARM_ARCHCC_FLAGS = -DCONFIG_ARCH_SUN50IW1P1 -fno-builtin -Os
  
  RVCT:*_*_ARM_ARCHCC_FLAGS = -DCONFIG_ARCH_SUN50IW1P1 

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################

[PcdsFeatureFlag.common]
[PcdsFixedAtBuild.common]
################################################################################
#
# Components Section - list of all EDK II Modules needed by this Platform
#
################################################################################
[Components.common]
  SunxiPlatformPkg/SunxiSpl/Boot0/Boot0.inf
  
