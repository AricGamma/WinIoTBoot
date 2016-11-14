/** @file
  Static SMBIOS Table for ARM platform
  Derived from EmulatorPkg package

  Note SMBIOS 2.7.1 Required structures:
    BIOS Information (Type 0)
    System Information (Type 1)
    Board Information (Type 2)
    System Enclosure (Type 3)
    Processor Information (Type 4) - CPU Driver
    Cache Information (Type 7) - For cache that is external to processor
    System Slots (Type 9) - If system has slots
    Physical Memory Array (Type 16)
    Memory Device (Type 17) - For each socketed system-memory Device
    Memory Array Mapped Address (Type 19) - One per contiguous block per Physical Memroy Array
    System Boot Information (Type 32)

  Copyright (c) 2012, Apple Inc. All rights reserved.<BR>
  Copyright (c) 2013 Linaro.org
  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

//#include <PiDxe.h>
#include <Protocol/Smbios.h>
#include <IndustryStandard/SmBios.h>
#include <Guid/SmBios.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/SysConfigLib.h>
#include <Library/HobLib.h>
#include <Guid/SunxiScriptParseHob.h>


/***********************************************************************
  SMBIOS data definition  TYPE0  BIOS Information
************************************************************************/
/* BIOS Info String Index */
UINTN  BiosInfoVendorNameStringNum = 1;
UINTN  BiosInfoVersionStringNum = 2;
UINTN  BiosInfoReleaseDateStringNum = 3;

/* System Info String Index */
UINTN  SystemInfoManufactureStringNum = 1;
UINTN  SystemInfoProductNameStringNum = 2;
UINTN  SystemInfoVersionStringNum = 3;
UINTN  SystemInfoSerialNumStringNum = 4;
UINTN  SystemInfoSkuNumStringNum = 5;
UINTN  SystemInfoFamilyStringNum = 6;

/* Board Info String Index */
UINTN  BoardInfoManufactureStringNum = 1;
UINTN  BoardInfoProductNameStringNum = 2;
UINTN  BoardInfoVersionStringNum = 3;
UINTN  BoardInfoSerialNumStringNum = 4;
UINTN  BoardInfoAssetTagStringNum = 5;
UINTN  BoardInfoChassisStringNum = 6;

/* Enclosure Info String Index */
UINTN  EnclosureInfoManufactureStringNum = 1;
UINTN  EnclosureInfoVersionStringNum = 2;
UINTN  EnclosureInfoSerialNumStringNum = 3;
UINTN  EnclosureInfoAssetTagStringNum = 4;

/* Process Info String Index */
UINTN  ProcessorInfoSocketStringNum = 1;
UINTN  ProcessorInfoManufactureStringNum = 2;
UINTN  ProcessorInfoVersionStringNum = 3;
UINTN  ProcessorInfoSerialNumStringNum = 4;
UINTN  ProcessorInfoAssetTagStringNum = 5;
UINTN  ProcessorInfoPartNumStringNum = 6;

/* Cache Info String Index */
UINTN  CacheInfoSocketStringNum = 1;

/* Slot Info String Index */
UINTN  SlotInfoSlotStringNum = 1;

/* Memory Device Info String Index */
UINTN  MemDevInfoDeviceLocatorStringNum = 1;
UINTN  MemDevInfoBankLocatorStringNum = 2;
UINTN  MemDevInfoManufactureStringNum = 3;


SMBIOS_TABLE_TYPE0 mBIOSInfoType0 = {
  { EFI_SMBIOS_TYPE_BIOS_INFORMATION, sizeof (SMBIOS_TABLE_TYPE0), 0 },
  1,                    // Vendor String
  2,                    // BiosVersion String
  0x0,               // BiosSegment
  3,                    // BiosReleaseDate String
  0x0,                 // BiosSize
  {                     // BiosCharacteristics
    0,    //  Reserved                          :2;  ///< Bits 0-1.
    0,    //  Unknown                           :1;
    0,    //  BiosCharacteristicsNotSupported   :1;
    0,    //  IsaIsSupported                    :1;
    0,    //  McaIsSupported                    :1;
    0,    //  EisaIsSupported                   :1;
    0,    //  PciIsSupported                    :1;
    0,    //  PcmciaIsSupported                 :1;
    0,    //  PlugAndPlayIsSupported            :1;
    0,    //  ApmIsSupported                    :1;
    1,    //  BiosIsUpgradable                  :1;
    1,    //  BiosShadowingAllowed              :1;
    0,    //  VlVesaIsSupported                 :1;
    0,    //  EscdSupportIsAvailable            :1;
    0,    //  BootFromCdIsSupported             :1;
    1,    //  SelectableBootIsSupported         :1;
    0,    //  RomBiosIsSocketed                 :1;
    0,    //  BootFromPcmciaIsSupported         :1;
    0,    //  EDDSpecificationIsSupported       :1;
    0,    //  JapaneseNecFloppyIsSupported      :1;
    0,    //  JapaneseToshibaFloppyIsSupported  :1;
    0,    //  Floppy525_360IsSupported          :1;
    0,    //  Floppy525_12IsSupported           :1;
    0,    //  Floppy35_720IsSupported           :1;
    0,    //  Floppy35_288IsSupported           :1;
    0,    //  PrintScreenIsSupported            :1;
    0,    //  Keyboard8042IsSupported           :1;
    0,    //  SerialIsSupported                 :1;
    0,    //  PrinterIsSupported                :1;
    0,    //  CgaMonoIsSupported                :1;
    0,    //  NecPc98                           :1;
    0     //  ReservedForVendor                 :32; ///< Bits 32-63. Bits 32-47 reserved for BIOS vendor
                                                 ///< and bits 48-63 reserved for System Vendor.
  },
  {       // BIOSCharacteristicsExtensionBytes[]
    0x81, //  AcpiIsSupported                   :1;
          //  UsbLegacyIsSupported              :1;
          //  AgpIsSupported                    :1;
          //  I2OBootIsSupported                :1;
          //  Ls120BootIsSupported              :1;
          //  AtapiZipDriveBootIsSupported      :1;
          //  Boot1394IsSupported               :1;
          //  SmartBatteryIsSupported           :1;
                  //  BIOSCharacteristicsExtensionBytes[1]
    0x0e, //  BiosBootSpecIsSupported              :1;
          //  FunctionKeyNetworkBootIsSupported    :1;
          //  TargetContentDistributionEnabled     :1;
          //  UefiSpecificationSupported           :1;
          //  VirtualMachineSupported              :1;
          //  ExtensionByte2Reserved               :3;
  },
  0xFF,                    // SystemBiosMajorRelease
  0xFF,                    // SystemBiosMinorRelease
  0xFF,                    // EmbeddedControllerFirmwareMajorRelease
  0xFF,                    // EmbeddedControllerFirmwareMinorRelease
};

CHAR8 *mBIOSInfoType0Strings[] = {
  "edk2.sourceforge.net",  // Vendor String
  __TIME__,                // BiosVersion String
  __DATE__,                // BiosReleaseDate String
  NULL
};

/***********************************************************************
  SMBIOS data definition  TYPE1  System Information
************************************************************************/
SMBIOS_TABLE_TYPE1 mSysInfoType1 = {
  { EFI_SMBIOS_TYPE_SYSTEM_INFORMATION, sizeof (SMBIOS_TABLE_TYPE1), 0 },
  1,    // Manufacturer String
  2,    // ProductName String
  3,    // Version String
  4,    // SerialNumber String
  { 0x25EF0280, 0xEC82, 0x42B0, { 0x8F, 0xB6, 0x10, 0xAD, 0xCC, 0xC6, 0x7C, 0x02 } },
  SystemWakeupTypePowerSwitch,
  5,    // SKUNumber String
  6,    // Family String
};
CHAR8  *mSysInfoType1Strings[] = {
  "AllWinner",
  "A64",
  "1.0",
  "System Serial#",
  "System SKU#",
  "A64",
  NULL
};

/***********************************************************************
  SMBIOS data definition  TYPE2  Board Information
************************************************************************/
SMBIOS_TABLE_TYPE2  mBoardInfoType2 = {
  { EFI_SMBIOS_TYPE_BASEBOARD_INFORMATION, sizeof (SMBIOS_TABLE_TYPE2), 0 },
  1,    // Manufacturer String
  2,    // ProductName String
  3,    // Version String
  4,    // SerialNumber String
  5,    // AssetTag String
  {     // FeatureFlag
    1,    //  Motherboard           :1;
    0,    //  RequiresDaughterCard  :1;
    0,    //  Removable             :1;
    0,    //  Replaceable           :1;
    0,    //  HotSwappable          :1;
    0,    //  Reserved              :3;
  },
  6,    // LocationInChassis String
  0,                        // ChassisHandle;
  BaseBoardTypeMotherBoard, // BoardType;
  0,                        // NumberOfContainedObjectHandles;
  { 0 }                     // ContainedObjectHandles[1];
};
CHAR8  *mBoardInfoType2Strings[] = {
  "AllWinner",
  "A64",
  "1.0",
  "Base Board Serial#",
  "Base Board Asset Tag#",
  "Part Component",
  NULL
};

/***********************************************************************
  SMBIOS data definition  TYPE3  Enclosure Information
************************************************************************/
SMBIOS_TABLE_TYPE3  mEnclosureInfoType3 = {
  { EFI_SMBIOS_TYPE_SYSTEM_ENCLOSURE, sizeof (SMBIOS_TABLE_TYPE3), 0 },
  1,                        // Manufacturer String
  MiscChassisTypeLapTop,    // Type;
  2,                        // Version String
  3,                        // SerialNumber String
  4,                        // AssetTag String
  ChassisStateSafe,         // BootupState;
  ChassisStateSafe,         // PowerSupplyState;
  ChassisStateSafe,         // ThermalState;
  ChassisSecurityStatusNone,// SecurityStatus;
  { 0, 0, 0, 0 },           // OemDefined[4];
  0,    // Height;
  0,    // NumberofPowerCords;
  0,    // ContainedElementCount;
  0,    // ContainedElementRecordLength;
  { 0 },    // ContainedElements[1];
};
CHAR8  *mEnclosureInfoType3Strings[] = {
  "AllWinner",
  "1.0",
  "Chassis Board Serial#",
  "Chassis Board Asset Tag#",
  NULL
};

/***********************************************************************
  SMBIOS data definition  TYPE4  Processor Information
************************************************************************/
SMBIOS_TABLE_TYPE4 mProcessorInfoType4 = {
  { EFI_SMBIOS_TYPE_PROCESSOR_INFORMATION, sizeof (SMBIOS_TABLE_TYPE4), 0},
  1,                    // Socket String
  CentralProcessor,       // ProcessorType;             ///< The enumeration value from PROCESSOR_TYPE_DATA.
  ProcessorFamilyIndicatorFamily2, // ProcessorFamily;        ///< The enumeration value from PROCESSOR_FAMILY2_DATA.
  2,                    // ProcessorManufacture String;
  {                     // ProcessorId;
    {  // PROCESSOR_SIGNATURE
      0, //  ProcessorSteppingId:4;
      0, //  ProcessorModel:     4;
      0, //  ProcessorFamily:    4;
      0, //  ProcessorType:      2;
      0, //  ProcessorReserved1: 2;
      0, //  ProcessorXModel:    4;
      0, //  ProcessorXFamily:   8;
      0, //  ProcessorReserved2: 4;
    },

    {  // PROCESSOR_FEATURE_FLAGS
      0, //  ProcessorFpu       :1;
      0, //  ProcessorVme       :1;
      0, //  ProcessorDe        :1;
      0, //  ProcessorPse       :1;
      0, //  ProcessorTsc       :1;
      0, //  ProcessorMsr       :1;
      0, //  ProcessorPae       :1;
      0, //  ProcessorMce       :1;
      0, //  ProcessorCx8       :1;
      0, //  ProcessorApic      :1;
      0, //  ProcessorReserved1 :1;
      0, //  ProcessorSep       :1;
      0, //  ProcessorMtrr      :1;
      0, //  ProcessorPge       :1;
      0, //  ProcessorMca       :1;
      0, //  ProcessorCmov      :1;
      0, //  ProcessorPat       :1;
      0, //  ProcessorPse36     :1;
      0, //  ProcessorPsn       :1;
      0, //  ProcessorClfsh     :1;
      0, //  ProcessorReserved2 :1;
      0, //  ProcessorDs        :1;
      0, //  ProcessorAcpi      :1;
      0, //  ProcessorMmx       :1;
      0, //  ProcessorFxsr      :1;
      0, //  ProcessorSse       :1;
      0, //  ProcessorSse2      :1;
      0, //  ProcessorSs        :1;
      0, //  ProcessorReserved3 :1;
      0, //  ProcessorTm        :1;
      0, //  ProcessorReserved4 :2;
    }
  },
  3,                      // ProcessorVersion String;
  {           // Voltage;
  1,  // ProcessorVoltageCapability5V      :1;
  1,  // ProcessorVoltageCapability3_3V    :1;
  1,  // ProcessorVoltageCapability2_9V    :1;
  0,  // ProcessorVoltageCapabilityReserved  :1; ///< Bit 3, must be zero.
  0,  // ProcessorVoltageReserved        :3; ///< Bits 4-6, must be zero.
    0 // ProcessorVoltageIndicateLegacy    :1;
  },
  0,                      // ExternalClock;
  1088,                   // MaxSpeed;
  1088,                   // CurrentSpeed;
  0x41,                   // Status;
  ProcessorUpgradeNone,   // ProcessorUpgrade;      ///< The enumeration value from PROCESSOR_UPGRADE.
  0xFFFF,                 // L1CacheHandle;
  0xFFFF,                 // L2CacheHandle;
  0xFFFF,                 // L3CacheHandle;
  4,                      // SerialNumber;
  5,                      // AssetTag;
  6,                      // PartNumber;
  4,                      // CoreCount;
  4,                      // EnabledCoreCount;
  0,                      // ThreadCount;
  0,                      // ProcessorCharacteristics;
  ProcessorFamilyARM      // ARM Processor Family;
};

CHAR8 *mProcessorInfoType4Strings[] = {
  "Socket",
  "ARM",
  "v7",
  "0",
  "0",
  "1.0",
  NULL
};

/***********************************************************************
  SMBIOS data definition  TYPE7  Cache Information
************************************************************************/
SMBIOS_TABLE_TYPE7  mL1InstructionCacheInfoType7 = {
  { EFI_SMBIOS_TYPE_CACHE_INFORMATION, sizeof (SMBIOS_TABLE_TYPE7), 0 },
  1,                        // SocketDesignation String
  0x200,          // Cache Configuration
  0x20,         // Maximum Size 256k
  0x20,         // Install Size 256k
  {             // Supported SRAM Type
  0,  //Other       :1
  0,  //Unknown     :1
  0,  //NonBurst      :1
  1,  //Burst       :1
  0,  //PiplelineBurst  :1
  1,  //Synchronous   :1
  0,  //Asynchronous    :1
  0 //Reserved      :9
  },
  {             // Current SRAM Type
  0,  //Other       :1
  0,  //Unknown     :1
  0,  //NonBurst      :1
  1,  //Burst       :1
  0,  //PiplelineBurst  :1
  1,  //Synchronous   :1
  0,  //Asynchronous    :1
  0 //Reserved      :9
  },
  0,            // Cache Speed unknown
  CacheErrorParity,       // Error Correction Multi
  CacheTypeInstruction,     // System Cache Type
  CacheAssociativity16Way // Associativity
};
CHAR8  *mL1InstructionCacheInfoType7Strings[] = {
  "L1 Instruction Cache",
  NULL
};

SMBIOS_TABLE_TYPE7  mL1DataCacheInfoType7 = {
  { EFI_SMBIOS_TYPE_CACHE_INFORMATION, sizeof (SMBIOS_TABLE_TYPE7), 0 },
  1,                        // SocketDesignation String
  0x200,          // Cache Configuration
  0x20,             // Maximum Size 256k
  0x20,             // Install Size 256k
  {             // Supported SRAM Type
  0,  //Other       :1
  0,  //Unknown     :1
  0,  //NonBurst      :1
  1,  //Burst       :1
  0,  //PiplelineBurst  :1
  1,  //Synchronous   :1
  0,  //Asynchronous    :1
  0 //Reserved      :9
  },
  {             // Current SRAM Type
  0,  //Other       :1
  0,  //Unknown     :1
  0,  //NonBurst      :1
  1,  //Burst       :1
  0,  //PiplelineBurst  :1
  1,  //Synchronous   :1
  0,  //Asynchronous    :1
  0 //Reserved      :9
  },
  0,            // Cache Speed unknown
  CacheErrorParity,       // Error Correction Multi
  CacheTypeData,      // System Cache Type
  CacheAssociativity16Way // Associativity
};
CHAR8  *mL1DataCacheInfoType7Strings[] = {
  "L1 Data Cache",
  NULL
};

SMBIOS_TABLE_TYPE7  mL2CacheInfoType7 = {
  { EFI_SMBIOS_TYPE_CACHE_INFORMATION, sizeof (SMBIOS_TABLE_TYPE7), 0 },
  1,                        // SocketDesignation String
  0x201,          // Cache Configuration
  0x8008,         // Maximum Size 256k
  0x8008,         // Install Size 256k
  {             // Supported SRAM Type
  0,  //Other       :1
  0,  //Unknown     :1
  0,  //NonBurst      :1
  1,  //Burst       :1
  0,  //PiplelineBurst  :1
  1,  //Synchronous   :1
  0,  //Asynchronous    :1
  0 //Reserved      :9
  },
  {             // Current SRAM Type
  0,  //Other       :1
  0,  //Unknown     :1
  0,  //NonBurst      :1
  1,  //Burst       :1
  0,  //PiplelineBurst  :1
  1,  //Synchronous   :1
  0,  //Asynchronous    :1
  0 //Reserved      :9
  },
  0,            // Cache Speed unknown
  CacheErrorParity,       // Error Correction Multi
  CacheTypeUnified,     // System Cache Type
  CacheAssociativity8Way  // Associativity
};
CHAR8  *mL2CacheInfoType7Strings[] = {
  "L2 Cache",
  NULL
};

/***********************************************************************
  SMBIOS data definition  TYPE9  System Slot Information
************************************************************************/
SMBIOS_TABLE_TYPE9  mSysSlotInfoType9 = {
  { EFI_SMBIOS_TYPE_SYSTEM_SLOTS, sizeof (SMBIOS_TABLE_TYPE9), 0 },
  1,    // SlotDesignation String
  SlotTypeOther,          // SlotType;                 ///< The enumeration value from MISC_SLOT_TYPE.
  SlotDataBusWidthOther,  // SlotDataBusWidth;         ///< The enumeration value from MISC_SLOT_DATA_BUS_WIDTH.
  SlotUsageAvailable,    // CurrentUsage;             ///< The enumeration value from MISC_SLOT_USAGE.
  SlotLengthOther,    // SlotLength;               ///< The enumeration value from MISC_SLOT_LENGTH.
  0,    // SlotID;
  {    // SlotCharacteristics1;
    1,  // CharacteristicsUnknown  :1;
    0,  // Provides50Volts         :1;
    0,  // Provides33Volts         :1;
    0,  // SharedSlot              :1;
    0,  // PcCard16Supported       :1;
    0,  // CardBusSupported        :1;
    0,  // ZoomVideoSupported      :1;
    0,  // ModemRingResumeSupported:1;
  },
  {     // SlotCharacteristics2;
    0,  // PmeSignalSupported      :1;
    0,  // HotPlugDevicesSupported :1;
    0,  // SmbusSignalSupported    :1;
    0,  // Reserved                :5;  ///< Set to 0.
  },
  0,    // SegmentGroupNum;
  0,    // BusNum;
  0,    // DevFuncNum;
};
CHAR8  *mSysSlotInfoType9Strings[] = {
  "EMMC",
  NULL
};

/***********************************************************************
  SMBIOS data definition  TYPE16  Physical Memory ArrayInformation
************************************************************************/
SMBIOS_TABLE_TYPE16 mPhyMemArrayInfoType16 = {
  { EFI_SMBIOS_TYPE_PHYSICAL_MEMORY_ARRAY, sizeof (SMBIOS_TABLE_TYPE16), 0 },
  MemoryArrayLocationSystemBoard, // Location;                       ///< The enumeration value from MEMORY_ARRAY_LOCATION.
  MemoryArrayUseSystemMemory,     // Use;                            ///< The enumeration value from MEMORY_ARRAY_USE.
  MemoryErrorCorrectionUnknown,   // MemoryErrorCorrection;          ///< The enumeration value from MEMORY_ERROR_CORRECTION.
  0x40000000,                     // MaximumCapacity;
  0xFFFE,                         // MemoryErrorInformationHandle;
  1,                              // NumberOfMemoryDevices;
  0x3fffffffffffffffULL,          // ExtendedMaximumCapacity;
};
CHAR8 *mPhyMemArrayInfoType16Strings[] = {
  NULL
};

/***********************************************************************
  SMBIOS data definition  TYPE17  Memory Device Information
************************************************************************/
SMBIOS_TABLE_TYPE17 mMemDevInfoType17 = {
  { EFI_SMBIOS_TYPE_MEMORY_DEVICE, sizeof (SMBIOS_TABLE_TYPE17), 0 },
  0,          // MemoryArrayHandle;
  0xFFFE,     // MemoryErrorInformationHandle;
  0xFFFF,     // TotalWidth;
  0xFFFF,     // DataWidth;
  0xFFFF,     // Size;
  MemoryFormFactorTsop, // FormFactor;                     ///< The enumeration value from MEMORY_FORM_FACTOR.
  0x0,       // DeviceSet;
  1,          // DeviceLocator String
  2,          // BankLocator String
  MemoryTypeDdr3,         // MemoryType;                     ///< The enumeration value from MEMORY_DEVICE_TYPE.
  {           // TypeDetail;
    0,  // Reserved        :1;
    0,  // Other           :1;
    0,  // Unknown         :1;
    0,  // FastPaged       :1;
    0,  // StaticColumn    :1;
    0,  // PseudoStatic    :1;
    0,  // Rambus          :1;
    0,  // Synchronous     :1;
    0,  // Cmos            :1;
    0,  // Edo             :1;
    0,  // WindowDram      :1;
    0,  // CacheDram       :1;
    0,  // Nonvolatile     :1;
    0,  // Registered      :1;
    0,  // Unbuffered      :1;
    0,  // Reserved1       :1;
  },
  400,          // Speed;
  3,          // Manufacturer String
  0,          // SerialNumber String
  0,          // AssetTag String
  0,          // PartNumber String
  0,          // Attributes;
  0,          // ExtendedSize;
  0,          // ConfiguredMemoryClockSpeed;
};
CHAR8 *mMemDevInfoType17Strings[] = {
  "OS Virtual Memory",
  "malloc",
  "OSV",
  NULL
};

/***********************************************************************
  SMBIOS data definition  TYPE19  Memory Array Mapped Address Information
************************************************************************/
SMBIOS_TABLE_TYPE19 mMemArrMapInfoType19 = {
  { EFI_SMBIOS_TYPE_MEMORY_ARRAY_MAPPED_ADDRESS, sizeof (SMBIOS_TABLE_TYPE19), 0 },
  0x80000000, // StartingAddress;
  0xbfffffff, // EndingAddress;
  0,          // MemoryArrayHandle;
  1,          // PartitionWidth;
  0,          // ExtendedStartingAddress;
  0,          // ExtendedEndingAddress;
};
CHAR8 *mMemArrMapInfoType19Strings[] = {
  NULL
};

/***********************************************************************
  SMBIOS data definition  TYPE32  Boot Information
************************************************************************/
SMBIOS_TABLE_TYPE32 mBootInfoType32 = {
  { EFI_SMBIOS_TYPE_SYSTEM_BOOT_INFORMATION, sizeof (SMBIOS_TABLE_TYPE32), 0 },
  { 0, 0, 0, 0, 0, 0 },         // Reserved[6];
  BootInformationStatusNoError  // BootStatus
};

CHAR8 *mBootInfoType32Strings[] = {
  NULL
};


/**

  Create SMBIOS record.

  Converts a fixed SMBIOS structure and an array of pointers to strings into
  an SMBIOS record where the strings are cat'ed on the end of the fixed record
  and terminated via a double NULL and add to SMBIOS table.

  SMBIOS_TABLE_TYPE32 gSmbiosType12 = {
    { EFI_SMBIOS_TYPE_SYSTEM_CONFIGURATION_OPTIONS, sizeof (SMBIOS_TABLE_TYPE12), 0 },
    1 // StringCount
  };

  CHAR8 *gSmbiosType12Strings[] = {
    "Not Found",
    NULL
  };

  ...

  LogSmbiosData (
    (EFI_SMBIOS_TABLE_HEADER*)&gSmbiosType12,
    gSmbiosType12Strings
    );

  @param  Template    Fixed SMBIOS structure, required.
  @param  StringArray Array of strings to convert to an SMBIOS string pack.
                      NULL is OK.
**/

EFI_STATUS
EFIAPI
LogSmbiosData (
  IN  EFI_SMBIOS_TABLE_HEADER *Template,
  IN  CHAR8                   **StringPack,
  OUT EFI_SMBIOS_HANDLE        *Handle
  )
{
  EFI_STATUS                Status;
  EFI_SMBIOS_PROTOCOL       *Smbios;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  EFI_SMBIOS_TABLE_HEADER   *Record;
  UINTN                     Index;
  UINTN                     StringSize;
  UINTN                     Size;
  CHAR8                     *Str;

  //
  // Locate Smbios protocol.
  //
  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID **)&Smbios);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Calculate the size of the fixed record and optional string pack

  Size = Template->Length;
  if (StringPack == NULL) {
    // At least a double null is required
    Size += 2;
  } else {
    for (Index = 0; StringPack[Index] != NULL; Index++) {
      StringSize = AsciiStrSize (StringPack[Index]);
      Size += StringSize;
    }
    if (StringPack[0] == NULL) {
    // At least a double null is required
      Size += 1;
    }
 
    // Don't forget the terminating double null
    Size += 1;
  }

  // Copy over Template
  Record = (EFI_SMBIOS_TABLE_HEADER *)AllocateZeroPool (Size);
  if (Record == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  CopyMem (Record, Template, Template->Length);

  // Append string pack
  Str = ((CHAR8 *)Record) + Record->Length;

  for (Index = 0; StringPack[Index] != NULL; Index++) {
    StringSize = AsciiStrSize (StringPack[Index]);
    CopyMem (Str, StringPack[Index], StringSize);
    Str += StringSize;
  }

  *Str = 0;
  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  Status = Smbios->Add (
                     Smbios,
                     gImageHandle,
                     &SmbiosHandle,
                     Record
                     );

  if(Handle != NULL)
    *Handle = SmbiosHandle;
  ASSERT_EFI_ERROR (Status);
  FreePool (Record);
  return Status;
}

/*
*  Install Type 0 default info
*  And update tyep 0 info by system configuration file
*/
EFI_STATUS
EFIAPI
SunxiUpdateSmbiosType0 (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SMBIOS_PROTOCOL       *Smbios;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  CHAR8 *Str = NULL;

  //
  // Locate Smbios protocol.
  //
  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mBIOSInfoType0, mBIOSInfoType0Strings, &SmbiosHandle);
  
  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID **)&Smbios);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Str = AllocateZeroPool (128);
  if (Str == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  // Update Type 0 BIOS Info 
  Status = script_parser_fetch("type0", "vendor", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 0 vendor is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                  &SmbiosHandle,  
                  &BiosInfoVendorNameStringNum, 
                  Str);
    
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }
  
  SetMem(Str,128,0);    
  Status = script_parser_fetch("type0", "version", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 0 version is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                                &SmbiosHandle,  
                                &BiosInfoVersionStringNum, 
                                Str);
  
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }
  
  SetMem(Str,128,0);    
  Status = script_parser_fetch("type0", "release_date", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 0 release_date is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                                &SmbiosHandle,  
                                &BiosInfoReleaseDateStringNum, 
                                Str);
  
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }
  
update_fail:
  FreePool(Str);
  
  return Status;
}

/*
*  Install Type 1 default info
*  And update tyep 1 info by system configuration file
*/

EFI_STATUS
EFIAPI
SunxiUpdateSmbiosType1 (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SMBIOS_PROTOCOL       *Smbios;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  CHAR8 *Str = NULL;

  //
  // Locate Smbios protocol.
  //
  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mSysInfoType1, mSysInfoType1Strings, &SmbiosHandle);
  
  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID **)&Smbios);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Str = AllocateZeroPool (128);
  if (Str == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  // Update Type 1 System Info 
  Status = script_parser_fetch("type1", "manufacture", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 1 manufacture is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                    &SmbiosHandle,  
                    &SystemInfoManufactureStringNum, 
                    Str);
      
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }
  
  SetMem(Str,128,0);    
  Status = script_parser_fetch("type1", "product_name", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 1 product name is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                                &SmbiosHandle,  
                                &SystemInfoProductNameStringNum, 
                                Str);
  
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }
  
  SetMem(Str,128,0);    
  Status = script_parser_fetch("type1", "version", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 1 version is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                                &SmbiosHandle,  
                                &SystemInfoVersionStringNum, 
                                Str);
  
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }

  SetMem(Str,128,0);    
  Status = script_parser_fetch("type1", "serial_num", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 1 serail number is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                                &SmbiosHandle,  
                                &SystemInfoSerialNumStringNum, 
                                Str);
  
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }

  SetMem(Str,128,0);    
  Status = script_parser_fetch("type1", "sku_num", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 1 sku number is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                                &SmbiosHandle,  
                                &SystemInfoSkuNumStringNum, 
                                Str);
  
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }

  SetMem(Str,128,0);    
  Status = script_parser_fetch("type1", "family", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 1 family is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                                &SmbiosHandle,  
                                &SystemInfoFamilyStringNum, 
                                Str);
  
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }
  
update_fail:
  FreePool(Str);
  
  return Status;
}


/*
*  Install Type 2 default info
*  And update tyep 2 info by system configuration file
*/
EFI_STATUS
EFIAPI
SunxiUpdateSmbiosType2 (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SMBIOS_PROTOCOL       *Smbios;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  CHAR8 *Str = NULL;

  //
  // Locate Smbios protocol.
  //
  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mBoardInfoType2, mBoardInfoType2Strings, &SmbiosHandle);
  
  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID **)&Smbios);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Str = AllocateZeroPool (128);
  if (Str == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  // Update Type 2 Board Info 
  Status = script_parser_fetch("type2", "manufacture", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 2 manufacture is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                    &SmbiosHandle,  
                    &BoardInfoManufactureStringNum, 
                    Str);
      
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }
  
  SetMem(Str,128,0);    
  Status = script_parser_fetch("type2", "product_name", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 2 product name is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                                &SmbiosHandle,  
                                &BoardInfoProductNameStringNum, 
                                Str);
  
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }
  
  SetMem(Str,128,0);    
  Status = script_parser_fetch("type2", "version", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 2 version is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                                &SmbiosHandle,  
                                &BoardInfoVersionStringNum, 
                                Str);
  
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }

  SetMem(Str,128,0);    
  Status = script_parser_fetch("type2", "serial_num", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 2 serail number is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                                &SmbiosHandle,  
                                &BoardInfoSerialNumStringNum, 
                                Str);
  
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }

  SetMem(Str,128,0);    
  Status = script_parser_fetch("type2", "asset_tag", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 2 asset tag is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                                &SmbiosHandle,  
                                &BoardInfoAssetTagStringNum, 
                                Str);
  
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }

  SetMem(Str,128,0);    
  Status = script_parser_fetch("type2", "chassis", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 2 chassis is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                                &SmbiosHandle,  
                                &BoardInfoChassisStringNum, 
                                Str);
  
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }
  
update_fail:
  FreePool(Str);
  
  return Status;
}


/*
*  Install Type 3 default info
*  And update tyep 3 info by system configuration file
*/
EFI_STATUS
EFIAPI
SunxiUpdateSmbiosType3 (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SMBIOS_PROTOCOL       *Smbios;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  CHAR8 *Str = NULL;

  //
  // Locate Smbios protocol.
  //
  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mEnclosureInfoType3, mEnclosureInfoType3Strings, &SmbiosHandle);
  
  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID **)&Smbios);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Str = AllocateZeroPool (128);
  if (Str == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  // Update Type 3 Enclosure Info 
  Status = script_parser_fetch("type3", "manufacture", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 3 manufacture is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                    &SmbiosHandle,  
                    &EnclosureInfoManufactureStringNum, 
                    Str);
      
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }
  
  SetMem(Str,128,0);    
  Status = script_parser_fetch("type3", "version", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 3 version is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                                &SmbiosHandle,  
                                &EnclosureInfoVersionStringNum, 
                                Str);
  
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }

  SetMem(Str,128,0);    
  Status = script_parser_fetch("type3", "serial_num", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 3 serail number is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                                &SmbiosHandle,  
                                &EnclosureInfoSerialNumStringNum, 
                                Str);
  
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }

  SetMem(Str,128,0);    
  Status = script_parser_fetch("type3", "asset_tag", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 3 asset tag is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                                &SmbiosHandle,  
                                &EnclosureInfoAssetTagStringNum, 
                                Str);
  
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }
  
update_fail:
  FreePool(Str);
  
  return Status;
}


/*
*  Install Type 4 default info
*  And update tyep 4 info by system configuration file
*/
EFI_STATUS
EFIAPI
SunxiUpdateSmbiosType4 (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SMBIOS_PROTOCOL       *Smbios;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  CHAR8 *Str = NULL;

  //
  // Locate Smbios protocol.
  //
  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mProcessorInfoType4, mProcessorInfoType4Strings, &SmbiosHandle);
  
  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID **)&Smbios);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Str = AllocateZeroPool (128);
  if (Str == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  // Update Type 4 Processor Info 
  Status = script_parser_fetch("type4", "socket", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 4 socket is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                    &SmbiosHandle,  
                    &ProcessorInfoSocketStringNum, 
                    Str);
      
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }
  
  SetMem(Str,128,0);
  Status = script_parser_fetch("type4", "manufacture", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 4 manufacture is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                    &SmbiosHandle,  
                    &ProcessorInfoManufactureStringNum, 
                    Str);
      
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }
  
  SetMem(Str,128,0);    
  Status = script_parser_fetch("type4", "version", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 4 version is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                                &SmbiosHandle,  
                                &ProcessorInfoVersionStringNum, 
                                Str);
  
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }

  SetMem(Str,128,0);    
  Status = script_parser_fetch("type4", "serial_num", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 4 serail number is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                                &SmbiosHandle,  
                                &ProcessorInfoSerialNumStringNum, 
                                Str);
  
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }

  SetMem(Str,128,0);    
  Status = script_parser_fetch("type4", "asset_tag", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 4 asset tag is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                                &SmbiosHandle,  
                                &ProcessorInfoAssetTagStringNum, 
                                Str);
  
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }

  SetMem(Str,128,0);    
  Status = script_parser_fetch("type4", "part_num", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 4 part number is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                                &SmbiosHandle,  
                                &ProcessorInfoPartNumStringNum, 
                                Str);
  
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }
  
update_fail:
  FreePool(Str);
  
  return Status;
}


/*
*  Install Type 7 default info
*  And update tyep 7 info by system configuration file
*/
EFI_STATUS
EFIAPI
SunxiUpdateSmbiosType7 (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SMBIOS_PROTOCOL       *Smbios;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  CHAR8 *Str = NULL;

  //
  // Locate Smbios protocol.
  //
  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mL1InstructionCacheInfoType7, mL1InstructionCacheInfoType7Strings, &SmbiosHandle);
  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mL1DataCacheInfoType7,   mL1DataCacheInfoType7Strings, &SmbiosHandle);
  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mL2CacheInfoType7, mL2CacheInfoType7Strings, &SmbiosHandle);
  
  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID **)&Smbios);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Str = AllocateZeroPool (128);
  if (Str == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }


  // Update Type 7 Cache Info 
  Status = script_parser_fetch("type7", "socket", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 7 socket is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                    &SmbiosHandle,  
                    &CacheInfoSocketStringNum, 
                    Str);
      
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }
  
update_fail:
  FreePool(Str);
  
  return Status;
}


/*
*  Install Type 9 default info
*  And update tyep 9 info by system configuration file
*/
EFI_STATUS
EFIAPI
SunxiUpdateSmbiosType9 (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SMBIOS_PROTOCOL       *Smbios;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  CHAR8 *Str = NULL;

  //
  // Locate Smbios protocol.
  //
  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mSysSlotInfoType9, mSysSlotInfoType9Strings, &SmbiosHandle);
  
  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID **)&Smbios);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Str = AllocateZeroPool (128);
  if (Str == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  // Update Type 9 Slot Info 
  Status = script_parser_fetch("type9", "slot", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 9 slot is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                    &SmbiosHandle,  
                    &SlotInfoSlotStringNum, 
                    Str);
      
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }
  
update_fail:
  FreePool(Str);
  
  return Status;
}


/*
*  Install Type 0 default info
*  
*/
EFI_STATUS
EFIAPI
SunxiUpdateSmbiosType16 (
  VOID
  )
{
  EFI_STATUS                Status;

  //
  // Locate Smbios protocol.
  //
  Status = LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mPhyMemArrayInfoType16, mPhyMemArrayInfoType16Strings, NULL);
  
  return Status;
}


/*
*  Install Type 17 default info
*  And update tyep 17 info by system configuration file
*/
EFI_STATUS
EFIAPI
SunxiUpdateSmbiosType17 (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SMBIOS_PROTOCOL       *Smbios;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  CHAR8 *Str = NULL;

  //
  // Locate Smbios protocol.
  //
  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mMemDevInfoType17, mMemDevInfoType17Strings, &SmbiosHandle);
  
  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID **)&Smbios);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Str = AllocateZeroPool (128);
  if (Str == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  // Update Type 17 Memory Device Info 
  Status = script_parser_fetch("type17", "device_locator", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 17 device locator is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                    &SmbiosHandle,  
                    &MemDevInfoDeviceLocatorStringNum, 
                    Str);
      
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }
  
  SetMem(Str,128,0);
  Status = script_parser_fetch("type17", "bank_locator", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 17 bank locator is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                    &SmbiosHandle,  
                    &MemDevInfoBankLocatorStringNum, 
                    Str);
      
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }
  
  SetMem(Str,128,0);    
  Status = script_parser_fetch("type17", "manufacture", (INT32 *)Str, 128);
  if(Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO,"Get SMBIOS Info: Type 17 manufacture is null\n"));
  }
  else
  {
    Status = Smbios->UpdateString(Smbios,  
                                &SmbiosHandle,  
                                &MemDevInfoManufactureStringNum, 
                                Str);
  
    if(Status != EFI_SUCCESS)
    {
      goto update_fail;
    }
  }
  
update_fail:
  FreePool(Str);
  
  return Status;
}


/*
*  Install Type 19 default info
*  
*/
EFI_STATUS
EFIAPI
SunxiUpdateSmbiosType19 (
  VOID
  )
{
  EFI_STATUS                Status;

  //
  // Locate Smbios protocol.
  //
  Status = LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mMemArrMapInfoType19, mMemArrMapInfoType19Strings, NULL);
  
  return Status;
}


/*
*  Install Type 32 default info
*  
*/
EFI_STATUS
EFIAPI
SunxiUpdateSmbiosType32 (
  VOID
  )
{
  EFI_STATUS                Status;

  //
  // Locate Smbios protocol.
  //
  Status = LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mBootInfoType32, mBootInfoType32Strings, NULL);
  
  return Status;
}


/***********************************************************************
  Driver Entry
************************************************************************/
EFI_STATUS
EFIAPI
PlatfomrSmbiosDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{

  SunxiUpdateSmbiosType0();
  SunxiUpdateSmbiosType1();
  SunxiUpdateSmbiosType2();
  SunxiUpdateSmbiosType3();
  SunxiUpdateSmbiosType4();
  SunxiUpdateSmbiosType7();
  SunxiUpdateSmbiosType9();
  SunxiUpdateSmbiosType16();
  SunxiUpdateSmbiosType17();
  SunxiUpdateSmbiosType19();
  SunxiUpdateSmbiosType32();
  
  return EFI_SUCCESS;
}
