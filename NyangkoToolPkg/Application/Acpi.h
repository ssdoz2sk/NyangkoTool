#ifndef _ACPI__H_
#define _ACPI__H_

#include <Base.h>
#include <Uefi.h>
#include "Ui/Menu.h"

#define ACPI_MENU_GUID {0xECF9EE3F, 0x9B43, 0x49F1, {0xA2, 0x3D, 0xA7, 0xA7, 0x3E, 0x43, 0x12, 0xCB}};

extern EFI_GUID    AcpiMenuGuid;


EFI_STATUS  EFIAPI  InitAcpiMenu(VOID  *Context);

EFI_STATUS  EFIAPI  GetAcpiTableData(VOID *Context);

#endif