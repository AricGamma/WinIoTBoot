#
#  Copyright (c) 2011, ARM Limited. All rights reserved.
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = ArmVExpressPkg-CTA5s
  PLATFORM_GUID                  = eb2bd5ff-2379-4a06-9c12-db905cdee9ea
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
!ifdef $(EDK2_OUT_DIR)
  OUTPUT_DIRECTORY               = $(EDK2_OUT_DIR)
!else
  OUTPUT_DIRECTORY               = Build/ArmVExpress-CTA5s
!endif
  SUPPORTED_ARCHITECTURES        = ARM
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = ArmPlatformPkg/ArmVExpressPkg/ArmVExpress-CTA5s.fdf
!ifndef $(EDK2_ARMVE_STANDALONE)
  DEFINE EDK2_ARMVE_STANDALONE=1
!endif

!include ArmPlatformPkg/ArmVExpressPkg/ArmVExpress.dsc.inc

[LibraryClasses.common]
  ArmLib|ArmPkg/Library/ArmLib/ArmV7/ArmV7Lib.inf
  ArmCpuLib|ArmPkg/Drivers/ArmCpuLib/ArmCortexA5Lib/ArmCortexA5Lib.inf
  ArmPlatformLib|ArmPlatformPkg/ArmVExpressPkg/Library/ArmVExpressLibCTA5s/ArmVExpressLib.inf

  # ARM PL310 L2 Cache Driver
  L2X0CacheLib|ArmPlatformPkg/Drivers/PL310L2Cache/PL310L2CacheSec.inf
  # ARM PL354 SMC Driver
  PL35xSmcLib|ArmPlatformPkg/Drivers/PL35xSmc/PL35xSmc.inf
  # ARM PL341 DMC Driver
  PL341DmcLib|ArmPlatformPkg/Drivers/PL34xDmc/PL341Dmc.inf
  # ARM PL301 Axi Driver
  PL301AxiLib|ArmPlatformPkg/Drivers/PL301Axi/PL301Axi.inf

  ArmPlatformSysConfigLib|ArmPlatformPkg/ArmVExpressPkg/Library/ArmVExpressSysConfigLib/ArmVExpressSysConfigLib.inf
  NorFlashPlatformLib|ArmPlatformPkg/ArmVExpressPkg/Library/NorFlashArmVExpressLib/NorFlashArmVExpressLib.inf
  #LcdPlatformLib|ArmPlatformPkg/ArmVExpressPkg/Library/PL111LcdArmVExpressLib/PL111LcdArmVExpressLib.inf

[LibraryClasses.common.SEC]
  ArmLib|ArmPkg/Library/ArmLib/ArmV7/ArmV7LibSec.inf
  ArmPlatformSecLib|ArmPlatformPkg/ArmVExpressPkg/Library/ArmVExpressLibCTA5s/ArmVExpressSecLib.inf

  # Uncomment to turn on GDB stub in SEC.
  #DebugAgentLib|EmbeddedPkg/Library/GdbDebugAgent/GdbDebugAgent.inf

[BuildOptions]
  RVCT:*_*_ARM_PLATFORM_FLAGS == --cpu Cortex-A5 -I$(WORKSPACE)/ArmPlatformPkg/ArmVExpressPkg/Include -I$(WORKSPACE)/ArmPlatformPkg/ArmVExpressPkg/Include/Platform/CTA5s

  GCC:*_*_ARM_PLATFORM_FLAGS == -mcpu=cortex-a5 -mfpu=neon -I$(WORKSPACE)/ArmPlatformPkg/ArmVExpressPkg/Include -I$(WORKSPACE)/ArmPlatformPkg/ArmVExpressPkg/Include/Platform/CTA5s

  XCODE:*_*_ARM_PLATFORM_FLAGS == -mcpu=cortex-a5 -mfpu=neon -I$(WORKSPACE)/ArmPlatformPkg/ArmVExpressPkg/Include -I$(WORKSPACE)/ArmPlatformPkg/ArmVExpressPkg/Include/Platform/CTA5s

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################

[PcdsFeatureFlag.common]
!ifdef $(EDK2_ARMVE_STANDALONE)
  gArmPlatformTokenSpaceGuid.PcdStandalone|TRUE
!else
  gArmPlatformTokenSpaceGuid.PcdStandalone|FALSE
  gArmPlatformTokenSpaceGuid.PcdSystemMemoryInitializeInSec|TRUE
  gArmPlatformTokenSpaceGuid.PcdSendSgiToBringUpSecondaryCores|TRUE
!endif

!ifdef $(EDK2_SKIP_PEICORE)
  gArmPlatformTokenSpaceGuid.PcdSystemMemoryInitializeInSec|TRUE
  gArmPlatformTokenSpaceGuid.PcdSendSgiToBringUpSecondaryCores|TRUE
!endif

  ## If TRUE, Graphics Output Protocol will be installed on virtual handle created by ConsplitterDxe.
  #  It could be set FALSE to save size.
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutGopSupport|TRUE

[PcdsFixedAtBuild.common]
  gArmPlatformTokenSpaceGuid.PcdFirmwareVendor|"ARM Versatile Express"
  gEmbeddedTokenSpaceGuid.PcdEmbeddedPrompt|"ArmVExpress"
  gArmPlatformTokenSpaceGuid.PcdCoreCount|2

  #
  # NV Storage PCDs. Use base of 0x0C000000 for NOR1
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableBase|0x0FFC0000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableSize|0x00010000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingBase|0x0FFD0000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingSize|0x00010000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareBase|0x0FFE0000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareSize|0x00010000

  gArmTokenSpaceGuid.PcdVFPEnabled|1

  # Stacks for MPCores in Secure World
  gArmPlatformTokenSpaceGuid.PcdCPUCoresSecStackBase|0x2E009000
  gArmPlatformTokenSpaceGuid.PcdCPUCoreSecPrimaryStackSize|0x1000

  # Stacks for MPCores in Monitor Mode
  gArmPlatformTokenSpaceGuid.PcdCPUCoresSecMonStackBase|0x2E008000
  gArmPlatformTokenSpaceGuid.PcdCPUCoreSecMonStackSize|0x100

  # Stacks for MPCores in Normal World
  gArmPlatformTokenSpaceGuid.PcdCPUCoresStackBase|0x2E000000
  gArmPlatformTokenSpaceGuid.PcdCPUCorePrimaryStackSize|0x4000

  # System Memory (1GB)
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x40000000

  #
  # ARM Pcds
  #
  gArmTokenSpaceGuid.PcdArmUncachedMemoryMask|0x0000000040000000

  #
  # ARM PrimeCell
  #

  ## SP804 Timer
  gEmbeddedTokenSpaceGuid.PcdEmbeddedPerformanceCounterFrequencyInHz|1000000
  gEmbeddedTokenSpaceGuid.PcdTimerPeriod|100000        # expressed in 100ns units, 100,000 x 100 ns = 10,000,000 ns = 10 ms
  gArmPlatformTokenSpaceGuid.PcdSP804TimerPeriodicInterruptNum|34
  gArmPlatformTokenSpaceGuid.PcdSP804TimerPeriodicBase|0x1c110000
  gArmPlatformTokenSpaceGuid.PcdSP804TimerPerformanceBase|0x1c110020
  gArmPlatformTokenSpaceGuid.PcdSP804TimerMetronomeBase|0x1c120020

  ## SP805 Watchdog - Motherboard Watchdog
  gArmPlatformTokenSpaceGuid.PcdSP805WatchdogBase|0x1C0F0000

  ## PL011 - Serial Terminal
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialRegisterBase|0x1c090000
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultBaudRate|38400

  ## PL031 RealTimeClock
  gArmPlatformTokenSpaceGuid.PcdPL031RtcBase|0x1C170000

  ## PL111 Versatile Express Motherboard controller
  #gArmPlatformTokenSpaceGuid.PcdPL111LcdBase|0x1C1F0000

  ## PL180 MMC/SD card controller
  gArmPlatformTokenSpaceGuid.PcdPL180SysMciRegAddress|0x1C010048
  gArmPlatformTokenSpaceGuid.PcdPL180MciBaseAddress|0x1C050000

  #
  # ARM PL390 General Interrupt Controller
  #
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x2C001000
  gArmTokenSpaceGuid.PcdGicInterruptInterfaceBase|0x2C000100

  #
  # ARM OS Loader
  #
  # Versatile Express machine type (ARM VERSATILE EXPRESS = 2272) required for ARM Linux:
  gArmTokenSpaceGuid.PcdArmMachineType|2272
  gArmPlatformTokenSpaceGuid.PcdDefaultBootDescription|L"Linaro image on SD card"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootDevicePath|L"VenHw(09831032-6FA3-4484-AF4F-0A000A8D3A82)/HD(1,MBR,0x00000000,0x3F,0x19FC0)/uImage"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootInitrdPath|L"VenHw(09831032-6FA3-4484-AF4F-0A000A8D3A82)/HD(1,MBR,0x00000000,0x3F,0x19FC0)/uInitrd"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootArgument|"console=ttyAMA0,38400n8 rootwait root=/dev/mmcblk0p2"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootType|2
  gArmPlatformTokenSpaceGuid.PcdFdtDevicePath|L"VenHw(09831032-6FA3-4484-AF4F-0A000A8D3A82)/HD(1,MBR,0x00000000,0x3F,0x19FC0)/v2p-ca5s.dtb"

  # Use the serial console (ConIn & ConOut) and the Graphic driver (ConOut)
  gArmPlatformTokenSpaceGuid.PcdDefaultConOutPaths|L"VenHw(D3987D4B-971A-435F-8CAF-4967EB627241)/Uart(38400,8,N,1)/VenPcAnsi();VenHw(407B4008-BF5B-11DF-9547-CF16E0D72085)"
  gArmPlatformTokenSpaceGuid.PcdDefaultConInPaths|L"VenHw(D3987D4B-971A-435F-8CAF-4967EB627241)/Uart(38400,8,N,1)/VenPcAnsi()"

  #
  # ARM L2x0 PCDs
  #
  gArmTokenSpaceGuid.PcdL2x0ControllerBase|0x1E00A000

################################################################################
#
# Components Section - list of all EDK II Modules needed by this Platform
#
################################################################################
[Components.common]

  #
  # SEC
  #
  ArmPlatformPkg/Sec/Sec.inf {
    <LibraryClasses>
      # Use the implementation which set the Secure bits
      ArmGicLib|ArmPkg/Drivers/ArmGic/ArmGicSecLib.inf
  }

  #
  # PEI Phase modules
  #
!ifdef $(EDK2_SKIP_PEICORE)
  ArmPlatformPkg/PrePi/PeiMPCore.inf {
    <LibraryClasses>
      ArmLib|ArmPkg/Library/ArmLib/ArmV7/ArmV7Lib.inf
      ArmPlatformLib|ArmPlatformPkg/ArmVExpressPkg/Library/ArmVExpressLibCTA5s/ArmVExpressLib.inf
      ArmPlatformGlobalVariableLib|ArmPlatformPkg/Library/ArmPlatformGlobalVariableLib/PrePi/PrePiArmPlatformGlobalVariableLib.inf
  }
!else
  ArmPlatformPkg/PrePeiCore/PrePeiCoreMPCore.inf {
    <LibraryClasses>
      ArmPlatformGlobalVariableLib|ArmPlatformPkg/Library/ArmPlatformGlobalVariableLib/Pei/PeiArmPlatformGlobalVariableLib.inf
  }
  MdeModulePkg/Core/Pei/PeiMain.inf
  MdeModulePkg/Universal/PCD/Pei/Pcd.inf  {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }
  ArmPlatformPkg/PlatformPei/PlatformPeim.inf
  ArmPlatformPkg/MemoryInitPei/MemoryInitPeim.inf
  ArmPkg/Drivers/CpuPei/CpuPei.inf
  IntelFrameworkModulePkg/Universal/StatusCode/Pei/StatusCodePei.inf
  Nt32Pkg/BootModePei/BootModePei.inf
  MdeModulePkg/Universal/Variable/Pei/VariablePei.inf
  MdeModulePkg/Core/DxeIplPeim/DxeIpl.inf {
    <LibraryClasses>
      NULL|IntelFrameworkModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
  }
!endif

  #
  # DXE
  #
  MdeModulePkg/Core/Dxe/DxeMain.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
      NULL|MdeModulePkg/Library/DxeCrc32GuidedSectionExtractLib/DxeCrc32GuidedSectionExtractLib.inf
  }

  #
  # Architectural Protocols
  #
  ArmPkg/Drivers/CpuDxe/CpuDxe.inf
  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf
  MdeModulePkg/Universal/FaultTolerantWriteDxe/FaultTolerantWriteDxe.inf
  MdeModulePkg/Universal/MonotonicCounterRuntimeDxe/MonotonicCounterRuntimeDxe.inf
  EmbeddedPkg/ResetRuntimeDxe/ResetRuntimeDxe.inf
  EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf
  EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf

  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
  MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  MdeModulePkg/Universal/Console/TerminalDxe/TerminalDxe.inf
  EmbeddedPkg/SerialDxe/SerialDxe.inf

  MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf

  ArmPkg/Drivers/ArmGic/ArmGicDxe.inf
  ArmPlatformPkg/Drivers/NorFlashDxe/NorFlashDxe.inf
  ArmPlatformPkg/Drivers/SP804TimerDxe/SP804TimerDxe.inf
  #ArmPlatformPkg/Drivers/LcdGraphicsOutputDxe/PL111LcdGraphicsOutputDxe.inf
  ArmPlatformPkg/Drivers/SP805WatchdogDxe/SP805WatchdogDxe.inf

  #
  # Filesystems
  #
  ArmPkg/Filesystem/SemihostFs/SemihostFs.inf

  #
  # Multimedia Card Interface
  #
  EmbeddedPkg/Universal/MmcDxe/MmcDxe.inf
  ArmPlatformPkg/Drivers/PL180MciDxe/PL180MciDxe.inf

  #
  # FAT filesystem + GPT/MBR partitioning
  #
  MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf

  #
  # Bds
  #
  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  ArmPlatformPkg/Bds/Bds.inf
