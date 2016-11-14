/** @file
*
*  Copyright (c) 2011-2012, ARM Limited. All rights reserved.
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

#include "PrePi.h"

#include <Library/ArmGicLib.h>

#include <Ppi/ArmMpCoreInfo.h>

VOID
PrimaryMain (
  IN  UINTN                     UefiMemoryBase,
  IN  UINTN                     StacksBase,
  IN  UINTN                     GlobalVariableBase,
  IN  UINT64                    StartTimeStamp
  )
{
  // Enable the GIC Distributor
  ArmGicEnableDistributor(PcdGet32(PcdGicDistributorBase));

  // In some cases, the secondary cores are waiting for an SGI from the next stage boot loader to resume their initialization
  if (!FixedPcdGet32(PcdSendSgiToBringUpSecondaryCores)) {
    // Sending SGI to all the Secondary CPU interfaces
    ArmGicSendSgiTo (PcdGet32(PcdGicDistributorBase), ARM_GIC_ICDSGIR_FILTER_EVERYONEELSE, 0x0E, PcdGet32 (PcdGicSgiIntId));
  }

  PrePiMain (UefiMemoryBase, StacksBase, GlobalVariableBase, StartTimeStamp);

  // We must never return
  ASSERT(FALSE);
}

#pragma pack(1)

typedef struct {
  UINT32 ProcessorId; // processor ID
  UINT32 RESERVED;
  UINT32 JumpAddress; // processor jump address
} EFI_PROCESSOR_MAILBOX;

#pragma pack()

VOID
SecondaryMain (
  IN  UINTN                     MpId
  )
{
  EFI_STATUS              Status;
  ARM_MP_CORE_INFO_PPI    *ArmMpCoreInfoPpi;
  UINTN                   Index;
  UINTN                   ArmCoreCount;
  ARM_CORE_INFO           *ArmCoreInfoTable;
  UINT32                  ClusterId;
  UINT32                  CoreId;
  UINT32          CorePosition;
  VOID                    (*SecondaryStart)(VOID*);
  UINTN                   SecondaryEntryAddr;
  EFI_PROCESSOR_MAILBOX *PMailBox;//set the mailbox to sram.

  ClusterId = GET_CLUSTER_ID(MpId);
  CoreId    = GET_CORE_ID(MpId);
  CorePosition = CoreId+ClusterId*4;
  
  PMailBox = (EFI_PROCESSOR_MAILBOX *)(UINTN)(PcdGet64(PcdMpParkSharedBase)+(CorePosition<<12));

  // Clear Secondary cores MailBox
  PMailBox->JumpAddress =0;
  PMailBox->ProcessorId =0xffffffff;

  //enable the cpuif in gic for each cpu
  ArmGicEnableInterruptInterface (PcdGet32(PcdGicInterruptInterfaceBase));

  MmioWrite32(PcdGet32(PcdGicInterruptInterfaceBase)+0x4,0xf0);
  
  // On MP Core Platform we must implement the ARM MP Core Info PPI (gArmMpCoreInfoPpiGuid)
  Status = GetPlatformPpi (&gArmMpCoreInfoPpiGuid, (VOID**)&ArmMpCoreInfoPpi);
  ASSERT_EFI_ERROR (Status);

  ArmCoreCount = 0;
  Status = ArmMpCoreInfoPpi->GetMpCoreInfo (&ArmCoreCount, &ArmCoreInfoTable);
  ASSERT_EFI_ERROR (Status);

  // Find the core in the ArmCoreTable
  for (Index = 0; Index < ArmCoreCount; Index++) {
    if ((ArmCoreInfoTable[Index].ClusterId == ClusterId) && (ArmCoreInfoTable[Index].CoreId == CoreId)) {
      break;
    }
  }

  // The ARM Core Info Table must define every core
  ASSERT (Index != ArmCoreCount);


  do { 
    ArmDataSyncronizationBarrier();
  
    ArmCallWFI ();
  
    ArmDataSyncronizationBarrier();
    // Read the Mailbox
    SecondaryEntryAddr = (UINTN)(PMailBox->JumpAddress);
    if((PMailBox->ProcessorId&0xffff) ==CorePosition)
    {
      if(SecondaryEntryAddr)
      {
        PMailBox->JumpAddress =0x00000000;//write zero to acknowledge
        ArmGicAcknowledgeInterrupt (PcdGet32(PcdGicDistributorBase), PcdGet32(PcdGicInterruptInterfaceBase), NULL, NULL);
        break;  //let's break and prepare to jump to secondery core entry point.
      }
    }

    // Acknowledge the interrupt and send End of Interrupt signal.
    ArmGicAcknowledgeInterrupt (PcdGet32(PcdGicDistributorBase), PcdGet32(PcdGicInterruptInterfaceBase), NULL, NULL);
  } while (1);
    // Disable Instruction Caches on all cores.
  ArmDisableInstructionCache ();
  // Data Cache enabled on Primary core when MMU is enabled.
  ArmDisableDataCache ();
  // Invalidate Data cache
  ArmCleanInvalidateDataCache ();
  // Invalidate instruction cache
  ArmInvalidateInstructionCache ();
  
  ArmDataSyncronizationBarrier();
  // Jump to secondary core entry point.
  SecondaryStart = (VOID (*)())SecondaryEntryAddr;

  SecondaryStart(PMailBox);

  // The secondaries shouldn't reach here
  ASSERT(FALSE);
}
