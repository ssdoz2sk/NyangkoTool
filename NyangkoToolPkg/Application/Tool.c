#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>

#include <Library/PcdLib.h>

#include <Protocol/HiiFont.h>
#include <Protocol/UgaDraw.h>

#include <Uefi.h>
#include "Setting.h"
#include "ScreenManager.h"
#include "Ui/Menu.h"
#include "Pci.h"
#include "Acpi.h"
#include "Variable.h"
#include "Handle.h"
#include "Smbios.h"

#define MAX_FILE_BUF 256

EFI_STATUS
EFIAPI 
ResetSystem (
  IN  VOID  *Context
)
{
    EFI_RESET_TYPE ResetType = (EFI_RESET_TYPE)(Context); 
    gRT->ResetSystem(ResetType, EFI_SUCCESS, 0, NULL);

    return EFI_SUCCESS;
}

/**
    This is entry point of the Shell Application.

    @param ImageHandle      Handle for the image of this driver
    @param SystemTable      Pointer to the EFI System Table

    @retval EFI_SUCCESS     Protocol successfully started and installed
**/
EFI_STATUS
EFIAPI
UefiMain (
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    // LoadSetting();
    gST->ConOut->EnableCursor (gST->ConOut, FALSE);

    LoadSetting();

    gMenu = AllocateZeroPool(sizeof(MENU));

    InitializeListHead (&gMenu->MenuItemList);
    gMenu->Title = L"Nyangko Tool v0.1.0 (25/02/29)";

    PushMenuItem (gMenu, L"Screen Size", InitScreenMenu,     NULL);
    PushMenuItem (gMenu, L"Pci",         InitPciMenu,        NULL);
    PushMenuItem (gMenu, L"Acpi Table",  InitAcpiMenu,       NULL);
    PushMenuItem (gMenu, L"Variables",   InitVariableMenu,   NULL);
    PushMenuItem (gMenu, L"Handle",      InitHandleMenu,     NULL);
    PushMenuItem (gMenu, L"Smbios",      InitSmbiosMenu,     NULL);
    PushMenuItem (gMenu, L"Warn Reset",  ResetSystem,        (VOID *)EfiResetWarm);
    PushMenuItem (gMenu, L"Cold Reset",  ResetSystem,        (VOID *)EfiResetCold);
    PushMenuItem (gMenu, L"Shutdown",    ResetSystem,        (VOID *)EfiResetShutdown);

    // Man loop;
    RunMenuLoop(gMenu);

    SaveAndFreeSetting(gSetting);
    FreePool(gMenu);

    // SaveAndFreeSetting();
    gST->ConOut->EnableCursor (gST->ConOut, TRUE);

   
    return EFI_SUCCESS;
}