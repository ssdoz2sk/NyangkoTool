#include "Setting.h"
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>

SETTING *gSetting = NULL;

EFI_STATUS LoadSetting() {
    EFI_STATUS          Status = EFI_SUCCESS;

    if (gSetting == NULL) {
        gSetting = AllocatePool(sizeof(SETTING));
    }

    // Template setting.
    gSetting->DisplayMode.Col = 80;
    gSetting->DisplayMode.Row = 25;
    
    return Status;
}

EFI_STATUS SaveAndFreeSetting() {
    EFI_STATUS      Status = EFI_SUCCESS;
    if (gSetting != NULL) {
        FreePool(gSetting);
    }
    return Status;
}