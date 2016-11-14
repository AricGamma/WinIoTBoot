/** @file
*
*  Copyright (c) 2011-2013, ARM Limited. All rights reserved.
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

#include <Uefi.h>
#include <Chipset/AArch64.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include "AArch64Lib.h"
#include "ArmLibPrivate.h"
#include <Library/ArmArchTimerLib.h>

VOID
EFIAPI
ArmArchTimerReadReg (
    IN   ARM_ARCH_TIMER_REGS   Reg,
    OUT  VOID                  *DstBuf
    )
{
  // Check if the Generic/Architecture timer is implemented
  if (ArmIsArchTimerImplemented ()) {

    switch (Reg) {

    case CntFrq:
      *((UINTN *)DstBuf) = ArmReadCntFrq ();
      break;

    case CntPct:
      *((UINT64 *)DstBuf) = ArmReadCntPct ();
      break;

    case CntkCtl:
      *((UINTN *)DstBuf) = ArmReadCntkCtl();
      break;

    case CntpTval:
      *((UINTN *)DstBuf) = ArmReadCntpTval ();
      break;

    case CntpCtl:
      *((UINTN *)DstBuf) = ArmReadCntpCtl ();
      break;

    case CntvTval:
      *((UINTN *)DstBuf) = ArmReadCntvTval ();
      break;

    case CntvCtl:
      *((UINTN *)DstBuf) = ArmReadCntvCtl ();
      break;

    case CntvCt:
      *((UINT64 *)DstBuf) = ArmReadCntvCt ();
      break;

    case CntpCval:
      *((UINT64 *)DstBuf) = ArmReadCntpCval ();
      break;

    case CntvCval:
      *((UINT64 *)DstBuf) = ArmReadCntvCval ();
      break;

    case CntvOff:
      *((UINT64 *)DstBuf) = ArmReadCntvOff ();
      break;

    case CnthCtl:
    case CnthpTval:
    case CnthpCtl:
    case CnthpCval:
    DEBUG ((EFI_D_ERROR, "The register is related to Hypervisor Mode. Can't perform requested operation\n "));
      break;

    default:
      DEBUG ((EFI_D_ERROR, "Unknown ARM Generic Timer register %x. \n ", Reg));
    }
  } else {
    DEBUG ((EFI_D_ERROR, "Attempt to read ARM Generic Timer registers. But ARM Generic Timer extension is not implemented \n "));
    ASSERT (0);
  }
}

VOID
EFIAPI
ArmArchTimerWriteReg (
    IN   ARM_ARCH_TIMER_REGS   Reg,
    IN   VOID                  *SrcBuf
    )
{
  // Check if the Generic/Architecture timer is implemented
  if (ArmIsArchTimerImplemented ()) {

    switch (Reg) {

    case CntFrq:
      ArmWriteCntFrq (*((UINTN *)SrcBuf));
      break;

    case CntPct:
      DEBUG ((EFI_D_ERROR, "Can't write to Read Only Register: CNTPCT \n"));
      break;

    case CntkCtl:
      ArmWriteCntkCtl (*((UINTN *)SrcBuf));
      break;

    case CntpTval:
      ArmWriteCntpTval (*((UINTN *)SrcBuf));
      break;

    case CntpCtl:
      ArmWriteCntpCtl (*((UINTN *)SrcBuf));
      break;

    case CntvTval:
      ArmWriteCntvTval (*((UINTN *)SrcBuf));
      break;

    case CntvCtl:
      ArmWriteCntvCtl (*((UINTN *)SrcBuf));
      break;

    case CntvCt:
      DEBUG ((EFI_D_ERROR, "Can't write to Read Only Register: CNTVCT \n"));
      break;

    case CntpCval:
      ArmWriteCntpCval (*((UINT64 *)SrcBuf) );
      break;

    case CntvCval:
      ArmWriteCntvCval (*((UINT64 *)SrcBuf) );
      break;

    case CntvOff:
      ArmWriteCntvOff (*((UINT64 *)SrcBuf));
      break;

    case CnthCtl:
    case CnthpTval:
    case CnthpCtl:
    case CnthpCval:
      DEBUG ((EFI_D_ERROR, "The register is related to Hypervisor Mode. Can't perform requested operation\n "));
      break;

    default:
      DEBUG ((EFI_D_ERROR, "Unknown ARM Generic Timer register %x. \n ", Reg));
    }
  } else {
    DEBUG ((EFI_D_ERROR, "Attempt to write to ARM Generic Timer registers. But ARM Generic Timer extension is not implemented \n "));
    ASSERT (0);
  }
}

VOID
EFIAPI
ArmArchTimerEnableTimer (
    VOID
    )
{
  UINTN TimerCtrlReg;

  ArmArchTimerReadReg (CntpCtl, (VOID *)&TimerCtrlReg);
  TimerCtrlReg |= ARM_ARCH_TIMER_ENABLE;
  ArmArchTimerWriteReg (CntpCtl, (VOID *)&TimerCtrlReg);
}

VOID
EFIAPI
ArmArchTimerDisableTimer (
    VOID
    )
{
  UINTN TimerCtrlReg;

  ArmArchTimerReadReg (CntpCtl, (VOID *)&TimerCtrlReg);
  TimerCtrlReg &= ~ARM_ARCH_TIMER_ENABLE;
  ArmArchTimerWriteReg (CntpCtl, (VOID *)&TimerCtrlReg);
}

VOID
EFIAPI
ArmArchTimerSetTimerFreq (
    IN   UINTN  FreqInHz
    )
{
  ArmArchTimerWriteReg (CntFrq, (VOID *)&FreqInHz);
}

UINTN
EFIAPI
ArmArchTimerGetTimerFreq (
    VOID
    )
{
  UINTN ArchTimerFreq = 0;
  ArmArchTimerReadReg (CntFrq, (VOID *)&ArchTimerFreq);
  return ArchTimerFreq;
}

UINTN
EFIAPI
ArmArchTimerGetTimerVal (
    VOID
    )
{
  UINTN ArchTimerVal;
  ArmArchTimerReadReg (CntpTval, (VOID *)&ArchTimerVal);
  return ArchTimerVal;
}


VOID
EFIAPI
ArmArchTimerSetTimerVal (
    IN   UINTN   Val
    )
{
  ArmArchTimerWriteReg (CntpTval, (VOID *)&Val);
}

UINT64
EFIAPI
ArmArchTimerGetSystemCount (
    VOID
    )
{
  UINT64 SystemCount;
  ArmArchTimerReadReg (CntPct, (VOID *)&SystemCount);
  return SystemCount;
}

UINTN
EFIAPI
ArmArchTimerGetTimerCtrlReg (
    VOID
    )
{
  UINTN  Val;
  ArmArchTimerReadReg (CntpCtl, (VOID *)&Val);
  return Val;
}

VOID
EFIAPI
ArmArchTimerSetTimerCtrlReg (
    UINTN Val
    )
{
  ArmArchTimerWriteReg (CntpCtl, (VOID *)&Val);
}

VOID
EFIAPI
ArmArchTimerSetCompareVal (
    IN   UINT64   Val
    )
{
  ArmArchTimerWriteReg (CntpCval, (VOID *)&Val);
}
