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

#include "OSAL_Power.h"
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/AxpPower.h>


#ifndef __OSAL_POWER_MASK__


void Str_Split(char* src, char flag, char* des1, char* des2)
{
     UINT32 i = 0;
     
     for(i = 0; i < AsciiStrLen(src); i++)
    {
        if(src[i] == flag)
        {
            break;
        }

    }
    if(i < AsciiStrLen(src))
    {
        CopyMem(des1, src, i);
        AsciiStrCpy(des2, src+i+1);
    }
  
    
}

#define AXP_809_NAME  ("axp809")
#define AXP_806_NAME  ("axp806")

EFI_STATUS
GetAxpInstanceByName(char* AxpName,  AXP_POWER_PROTOCOL  **Instance)
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    *HandleBuffer;
  UINTN                         NumberOfHandles;
  UINTN                         Index;
  AXP_POWER_ID                  AxpID;
  AXP_POWER_ID                  AxpID_Need;
  AXP_POWER_PROTOCOL  *AxpInstance = NULL;

  

  if(!AsciiStrCmp(AxpName,AXP_809_NAME))
  {
    AxpID_Need = AXP_POWER_ID_AXP809;
  }
  else if(!AsciiStrCmp(AxpName,AXP_806_NAME))
  {
    AxpID_Need = AXP_POWER_ID_AXP806;
  }
  else 
  {
    return EFI_DEVICE_ERROR;
  }

  //
  // Locate protocol.
  //
  Status = gBS->LocateHandleBuffer (
                   ByProtocol,
                   &gAxpPowerProtocolGuid,
                   NULL,
                   &NumberOfHandles,
                   &HandleBuffer
                   );
  if (EFI_ERROR (Status)) {
    //
    // Defined errors at this time are not found and out of resources.
    //
    return Status;
  }

  DEBUG((DEBUG_INFO,"AXP NumberOfHandles = %d\n",NumberOfHandles));
  
  for (Index = 0; Index < NumberOfHandles; Index++) {
    //
    // Get the protocol on this handle
    // This should not fail because of LocateHandleBuffer
    //
    Status = gBS->HandleProtocol (
                     HandleBuffer[Index],
                     &gAxpPowerProtocolGuid,
                     (VOID**) &AxpInstance
                     );
    ASSERT_EFI_ERROR (Status);

 
    AxpID = AxpInstance->AxpPowerId;
    if(AxpID == AxpID_Need)
    {
        *Instance = AxpInstance;
        break;
    }

  }

  if(*Instance==NULL) Status = EFI_DEVICE_ERROR;
  //
  // Our exit status is determined by the success of the previous operations
  // If the protocol was found, Instance already points to it.
  //

  //
  // Free any allocated buffers
  //
  gBS->FreePool (HandleBuffer);

  return Status;
}


int OSAL_Power_Enable(char *name)
{

    EFI_STATUS Status;
  AXP_POWER_PROTOCOL  *AxpPowerProtocol = NULL;

  char power_name[20], pmu_type[20], vol_type[20];
   

    ZeroMem(pmu_type,sizeof(pmu_type));
    ZeroMem(vol_type,sizeof(vol_type));
    ZeroMem(power_name,sizeof(power_name));
    
  AsciiStrCpy(power_name, name);
    Str_Split(power_name,'_', pmu_type,vol_type);

#if defined(CONFIG_ARCH_SUN9IW1P1)
  if(!AsciiStrCmp("axp15", pmu_type))
    AsciiStrCpy(pmu_type, AXP_806_NAME);
  else if(!AsciiStrCmp("axp22", pmu_type))
    AsciiStrCpy(pmu_type, AXP_809_NAME);
  else
    __wrn("unknown pmu type");
#else
#if 0
  if(!strcmp("dc1sw", vol_type)) {
    __wrn("enable dcdc1");
    axp_set_supply_status_byname("axp22", "dcdc1", 0, 1);
  }
#endif
#endif

  if(!AsciiStrCmp("ldoio0", vol_type))
    AsciiStrCpy(vol_type, "gpio0ldo");
  else if(!AsciiStrCmp("ldoio1", vol_type))
    AsciiStrCpy(vol_type, "gpio1ldo");

  //axp_set_supply_status_byname(pmu_type, vol_type, 0, 1);

    //Status = gBS->LocateProtocol (&gAxpPowerProtocolGuid, NULL, (VOID **)&AxpPowerProtocol);
    //ASSERT_EFI_ERROR(Status);

    Status = GetAxpInstanceByName(pmu_type,&AxpPowerProtocol);
    if (EFI_ERROR (Status))
    {
        __wrn("GetAxpInstanceByName fail: pmu name is %a\n", pmu_type);
        return Status;
    }
    Status = AxpPowerProtocol->SetSupplyStatusByName(vol_type, 0, 1);
    if (EFI_ERROR (Status))
    {
        __wrn("SetSupplyStatusByName fail: vol_type name is %a\n", vol_type);
        return Status;
    }

    __wrn("OSAL_Power_Enable <%a, %a> ok\n", pmu_type, vol_type);

  return 0;
}

int OSAL_Power_Disable(char *name)
{
    EFI_STATUS Status;
  AXP_POWER_PROTOCOL  *AxpPowerProtocol = NULL;
  char power_name[20], pmu_type[20], vol_type[20];

    ZeroMem(pmu_type,sizeof(pmu_type));
    ZeroMem(vol_type,sizeof(vol_type));
    ZeroMem(power_name,sizeof(power_name));
  AsciiStrCpy(power_name, name);
    Str_Split(power_name,'_', pmu_type,vol_type);

#if defined(CONFIG_ARCH_SUN9IW1P1)
  if(!AsciiStrCmp("axp15", pmu_type))
    AsciiStrCpy(pmu_type, "axp806");
  else if(!AsciiStrCmp("axp22", pmu_type))
    AsciiStrCpy(pmu_type, "axp809");
  else
    __wrn("unknown pmu type");
#else
#endif

  if(!AsciiStrCmp("ldoio0", vol_type))
    AsciiStrCpy(vol_type, "gpio0ldo");
  else if(!AsciiStrCmp("ldoio1", vol_type))
    AsciiStrCpy(vol_type, "gpio1ldo");

  
  //axp_set_supply_status_byname(pmu_type, vol_type, 0, 0);
    //Status = gBS->LocateProtocol (&gAxpPowerProtocolGuid, NULL, (VOID **)&AxpPowerProtocol);
    //ASSERT_EFI_ERROR(Status);
    Status = GetAxpInstanceByName(pmu_type,&AxpPowerProtocol);
    if (EFI_ERROR (Status))
    {
        __wrn("GetAxpInstanceByName fail: pmu name is %a\n", pmu_type);
        return Status;
    }
    
    Status = AxpPowerProtocol->SetSupplyStatusByName( vol_type, 0, 0);
    if (EFI_ERROR (Status))
    {
        __wrn("SetSupplyStatusByName fail: vol_type name is %a\n", vol_type);
        return Status;
    }

    __wrn("OSAL_Power_Disable <%a, %a> ok\n", pmu_type, vol_type);

#if defined(CONFIG_ARCH_SUN8IW5P1)
#if 0
  if(!strcmp("dc1sw", vol_type))
    axp_set_supply_status_byname("axp22", "dcdc1", 0 , 0);
#endif
#endif

  return 0;
}

#else

int OSAL_Power_Enable(char *name)
{
  int ret = 0;

  return ret;
}

int OSAL_Power_Disable(char *name)
{
  int ret = 0;

  return ret;
}

#endif
