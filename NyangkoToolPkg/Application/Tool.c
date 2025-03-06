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
// #include "Setting.h"
// #include "ScreenManager.h"
// #include "Ui/Menu.h"
// #include "Pci.h"
// #include "Acpi.h"
// #include "Variable.h"
// #include "Handle.h"
// #include "Smbios.h"

#include <Library/NyangkoMenuLib.h>

#define MAX_FILE_BUF 256

// extern MENU gMenu;

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

    // Man loop;
    RunMenuLoop(gMenu);
    // SaveAndFreeSetting();
    gST->ConOut->EnableCursor (gST->ConOut, TRUE);

   
    return EFI_SUCCESS;
}