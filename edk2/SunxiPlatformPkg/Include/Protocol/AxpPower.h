/** @file

  Copyright (c) 2007 - 2013, Allwinner Technology Co., Ltd. <www.allwinnertech.com>

  Martin.zheng <zhengjiewen@allwinnertech.com>
  
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __AXP_POWER_H__
#define __AXP_POWER_H__

#define PMU_SUPPLY_DCDC_TYPE    (0x00010000)
#define PMU_SUPPLY_DCDC1        (0x00010001)
#define PMU_SUPPLY_DCDC2        (0x00010002)
#define PMU_SUPPLY_DCDC3        (0x00010003)
#define PMU_SUPPLY_DCDC4        (0x00010004)
#define PMU_SUPPLY_DCDC5        (0x00010005)
#define PMU_SUPPLY_DCDC6        (0x00010006)

#define PMU_SUPPLY_ALDO_TYPE    (0x00020000)
#define PMU_SUPPLY_ALDO1        (0x00020001)
#define PMU_SUPPLY_ALDO2    (0x00020002)
#define PMU_SUPPLY_ALDO3    (0x00020003)

#define PMU_SUPPLY_BLDO_TYPE    (0x00030000)

#define PMU_SUPPLY_CLDO_TYPE    (0x00040000)

#define PMU_SUPPLY_DLDO_TYPE    (0x00050000)
#define PMU_SUPPLY_DLDO1    (0x00050001)
#define PMU_SUPPLY_DLDO2    (0x00050002)
#define PMU_SUPPLY_DLDO3    (0x00050003)
#define PMU_SUPPLY_DLDO4    (0x00050004)


#define PMU_SUPPLY_ELDO_TYPE    (0x00060000)
#define PMU_SUPPLY_ELDO1    (0x00060001)
#define PMU_SUPPLY_ELDO2    (0x00060002)
#define PMU_SUPPLY_ELDO3    (0x00060003)

#define PMU_SUPPLY_GPIOLDO_TYPE (0x00070000)
#define PMU_SUPPLY_GPIO0LDO   (0x00070000)
#define PMU_SUPPLY_GPIO1LDO   (0x00070001)

#define PMU_SUPPLY_MISC_TYPE    (0x00080000)
#define PMU_SUPPLY_DC5LDO   (0x00080001)
#define PMU_SUPPLY_DC1SW      (0x00080002)
#define PMU_SUPPLY_VBUSEN     (0x00080003)

#define PMU_SUPPLY_GPIO_TYPE  (0x00090000)
#define PMU_SUPPLY_GPIO0    (0x00090000)
#define PMU_SUPPLY_GPIO1    (0x00090001)



typedef enum
{
    AXP_BUS_TYPE_I2C,
    AXP_BUS_TYPE_P2WI,
    AXP_BUS_TYPE_RSB,
} PM_BUS_TYPE;

typedef enum 
{
  AXP_POWER_ID_NULL   =0,
  AXP_POWER_ID_AXP152, 
  AXP_POWER_ID_AXP209,
  AXP_POWER_ID_AXP221,
  AXP_POWER_ID_AXP221S,
  AXP_POWER_ID_AXP229,
  AXP_POWER_ID_AXP806,
  AXP_POWER_ID_AXP808,
  AXP_POWER_ID_AXP809,
  AXP_POWER_ID_AXP81X
}AXP_POWER_ID;

typedef enum
{
    AXP_POWER_SYS_MODE  = 0x0e,
    AXP_POWER_BOOT_MODE = 0x0f,
}AXP_POWER_MODE;

typedef enum
{
    AXP_POWER_KEY_MASK_PRESS_LONG = 0x00000001,
    AXP_POWER_KEY_MASK_PRESS_SHORT= 0x00000002,
}AXP_POWER_KEY_PRESS;

typedef enum
{
    AXP_POWER_UP_TRIGGER_BY_KEY =0x00,
    AXP_POWER_UP_TRIGGER_BY_POWER =0x01,
}AXP_POWER_UP_TRIGGER;

typedef enum
{
    BATTERY_RATIO_DEFAULT,
    BATTERY_RATIO_TOO_LOW_WITHOUT_DCIN,
    BATTERY_RATIO_TOO_LOW_WITH_DCIN,
    BATTERY_RATIO_ENOUGH,
    BATTERY_RATIO_TOO_LOW_WITH_DCIN_VOL_TOO_LOW,
}AXP_POWER_LEVEL;

typedef enum
{
    AXP_INT_MASK_AC_REMOVE          = 0x00000001,
    AXP_INT_MASK_AC_INSERT          = 0x00000002,
    AXP_INT_MASK_VBUS_REMOVE        = 0x00000004,
    AXP_INT_MASK_VBUS_INSERT        = 0x00000008,
    AXP_INT_MASK_CHARGE_DONE        = 0x00000010,
    AXP_INT_MASK_LONG_KEY_PRESS     = 0x00000020,
    AXP_INT_MASK_SHORT_KEY_PRESS    = 0x00000040,
}AXP_POWER_INT_MASK;

typedef enum
{
  AXP_POWER_VBUS_EXIST      = 0X00000001,
  AXP_POWER_AC_BUS_EXIST      = 0X00000002,
}AXP_POWER_BUS_EXISTANCE_MASK;

typedef enum
{
    PMU_PRE_FASTBOOT_MODE =     (0x0c),
    PMU_PRE_RECOVERY_MODE =     (0x10),
    PMU_PRE_FACTORY_MODE  =     (0x0d),
    PMU_PRE_SYS_MODE      =     (0x0e),
    PMU_PRE_BOOT_MODE     =     (0x0f),
}PMU_PRE_MODE_VALUE;

typedef enum
{
    AXP_POWER_ON_BY_POWER_KEY     = 0,
    AXP_POWER_ON_BY_POWER_TRIGGER = 1,
}AXP_POWER_ON_CAUSE;


typedef 
EFI_STATUS
(EFIAPI *AXP_PM_BUS_READ) (
  IN UINT8                      DeviceRegister,
  IN UINT8                       *Data
  );

typedef 
EFI_STATUS
(EFIAPI *AXP_PM_BUS_WRITE) (
  IN UINT8                      DeviceRegister,
  IN UINT8                      Data
  );
  
struct _AXP_PM_BUS_OPS {

  AXP_PM_BUS_READ     AxpPmBusRead;
  AXP_PM_BUS_WRITE    AxpPmBusWrite;

};

typedef struct _AXP_PM_BUS_OPS AXP_PM_BUS_OPS;
//
// Protocol interface structure
//
typedef struct _AXP_POWER_PROTOCOL AXP_POWER_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *PROBE) (
    VOID
  );


typedef
EFI_STATUS
(EFIAPI *SET_SUPPLY_STATUS) (
  IN UINTN      VoltageIndex,
  IN UINTN      Voltage,
  IN INTN     OnOff
  );

typedef
EFI_STATUS
(EFIAPI *SET_SUPPLY_STATUS_BY_NAME) (
  IN CHAR8      *VoltageName,
  IN UINTN       Voltage,
  IN INTN      OnOff
  );

typedef
EFI_STATUS
(EFIAPI *PROBE_SUPPLY_STATUS) (
  IN UINTN      VoltageIndex,
  IN UINTN      *Voltage,
  IN INTN     *OnOff
  );

typedef
EFI_STATUS
(EFIAPI *PROBE_SUPPLY_STATUS_BY_NAME) (
  IN CHAR8      *VoltageName,
  IN UINTN      *Voltage
  );


typedef
EFI_STATUS
(EFIAPI *SET_NEXT_SYSTEM_MODE) (
  IN UINTN      Status
  );

typedef
EFI_STATUS
(EFIAPI *PROBE_PRE_SYSTEM_MODE) (
  OUT UINTN           *Status
  );

typedef
EFI_STATUS
(EFIAPI *PROBE_THIS_POWER_ON_CAUSE) (
  OUT UINTN           *Status
  );

typedef
EFI_STATUS
(EFIAPI *PROBE_POWER_BUS_EXISTANCE) (
  OUT UINTN           *Status
  );

typedef
EFI_STATUS
(EFIAPI *PROBE_BATTERY_VOLTAGE) (
  OUT UINTN           *Voltage
 );

typedef
EFI_STATUS
(EFIAPI *PROBE_BATTERY_RATIO) (
  OUT UINTN           *Ratio
  );

typedef
EFI_STATUS
(EFIAPI *PROBE_BATTERY_EXISTANCE) (
  OUT UINTN           *Status
  );

typedef
EFI_STATUS
(EFIAPI *PROBE_POWER_KEY) (
  OUT UINTN           *Pressed
  );

typedef
EFI_STATUS
(EFIAPI *SET_POWER_OFF) (
  VOID
  );

typedef
EFI_STATUS
(EFIAPI *SET_POWER_ONOFF_VOLTAGE) (
  IN UINTN Voltage,
  IN UINTN Stage
  );

typedef
EFI_STATUS
(EFIAPI *SET_CHARGER_ON_OFF) (
  IN UINTN OnOff
  );

typedef
EFI_STATUS
(EFIAPI *SET_VBUS_CURRENT_LIMIT) (
   IN UINTN Current
  );

typedef
EFI_STATUS
(EFIAPI *PROBE_VBUS_CURRENT_LIMIT) (
   IN UINTN *Current
  );

typedef
EFI_STATUS
(EFIAPI *SET_VBUS_VOLTAGE_LIMIT) (
   IN UINTN Voltage
  );

typedef
EFI_STATUS
(EFIAPI *SET_CHARGE_CURRENT) (
   IN UINTN Current
  );

typedef
EFI_STATUS
(EFIAPI *PROBE_CHARGE_CURRENT) (
   OUT UINTN *Current
  );

typedef
EFI_STATUS
(EFIAPI *PROBE_INT_PENDDING) (
   OUT UINT64 *IntMask
  );

typedef
EFI_STATUS
(EFIAPI *PROBE_INT_ENABLE) (
  IN UINT64 *IntMask
  );

typedef
EFI_STATUS
(EFIAPI *SET_INT_ENABLE) (
  IN UINT64 IntMask
  );

typedef
EFI_STATUS
(EFIAPI *SET_INT_DISABLE) (
  IN UINT64 IntMask
);

typedef
EFI_STATUS
(EFIAPI *SET_COULOMBMETER_ON_OFF) (
  IN UINTN OnOff
);


///
/// This protocol allows parse the sunxi script to config the system.
///
struct _AXP_POWER_PROTOCOL {
  AXP_POWER_ID              AxpPowerId;

  PROBE                 Probe;
  
  SET_SUPPLY_STATUS               SetSupplyStatus;
  SET_SUPPLY_STATUS_BY_NAME           SetSupplyStatusByName;
  PROBE_SUPPLY_STATUS                 ProbeSupplyStatus;
  PROBE_SUPPLY_STATUS_BY_NAME         ProbeSupplyStatusByName;
  
  SET_NEXT_SYSTEM_MODE            SetNextSystemMode;
  PROBE_PRE_SYSTEM_MODE               ProbePreSystemMode;
  PROBE_THIS_POWER_ON_CAUSE         ProbeThisPowerOnCause;
  
  PROBE_POWER_BUS_EXISTANCE           ProbePowerBusExistance;
  
  PROBE_BATTERY_VOLTAGE           ProbeBatteryVoltage;
  PROBE_BATTERY_RATIO         ProbeBatteryRatio;
  PROBE_BATTERY_EXISTANCE         ProbeBatteryExistance;
  PROBE_POWER_KEY           ProbePowerKey;
  
  SET_POWER_OFF               SetPowerOff;
  SET_POWER_ONOFF_VOLTAGE       SetPowerOnOffVoltage;
  
  SET_CHARGER_ON_OFF            SetChargerOnOff;
  SET_VBUS_CURRENT_LIMIT        SetVbusCurrentLimit;
  PROBE_VBUS_CURRENT_LIMIT            ProbeVbusCurrentLimit;
  SET_VBUS_VOLTAGE_LIMIT          SetVbusVoltageLimit;
  SET_CHARGE_CURRENT          SetChargeCurrent;
  PROBE_CHARGE_CURRENT          ProbeChargeCurrent;
  
  PROBE_INT_PENDDING          ProbeIntPendding;
  PROBE_INT_ENABLE          ProbeIntEnable;
  SET_INT_ENABLE            SetIntEnable;
  SET_INT_DISABLE           SetIntDisable;
  SET_COULOMBMETER_ON_OFF             SetCoulombmeterOnOff;
};


extern EFI_GUID gAxpPowerProtocolGuid;

#endif
