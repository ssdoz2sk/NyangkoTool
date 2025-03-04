#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include "ScreenManager.h"
#include "Ui/Menu.h"
#include "Setting.h"

EFI_GUID ScreenMenuGuid = SCREEN_MENU_GUID;


EFI_STATUS
EFIAPI
InitScreenMenu(
    IN  VOID  *Context
) {
    EFI_STATUS          Status          = EFI_SUCCESS;
    UINTN               Mode;
    UINTN               Col;
    UINTN               Row;
    SCREEN_MODE         *ScreenMode;
    CHAR16              TitleTemp[] = L"Mode: 00, Col: 000, Row: 000";
    CHAR16              *Title;
    MENU                *ScreenMenu     = AllocateZeroPool(sizeof(MENU));

    InitializeListHead (&ScreenMenu->MenuItemList);
    ScreenMenu->Title = L"Screen Menu";

    // Set mirror resolution
    for (Mode = 0; Mode < gST->ConOut->Mode->MaxMode; Mode++) {
        Status = gST->ConOut->QueryMode(gST->ConOut, Mode, &Col, &Row);
        if (EFI_ERROR(Status))
            continue;

        ScreenMode = AllocateZeroPool(sizeof(SCREEN_MODE));
        ScreenMode->Mode = Mode;
        ScreenMode->Col = Col;
        ScreenMode->Row = Row;

        Title = AllocateZeroPool(sizeof(TitleTemp));
        UnicodeSPrint(Title, 
                      sizeof(TitleTemp), 
                      L"Mode: %2d, Col: %3d, Row: %3d",
                      Mode,
                      Col,
                      Row);

        PushMenuItem (ScreenMenu,
                      Title,
                      SetScreenMode,
                      ScreenMode);

    }

    RunMenuLoop(ScreenMenu);

    FreePool(ScreenMenu);

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
SetScreenMode(VOID *Context) {
    EFI_STATUS      Status;
    SCREEN_MODE     *ScreenMode = (SCREEN_MODE *)Context;
    Status = gST->ConOut->SetMode(gST->ConOut, ScreenMode->Mode);

    gRedraw = TRUE; // Redraw menu

    if (gSetting != NULL) {
        gSetting->DisplayMode.Mode = ScreenMode->Mode;
        gSetting->DisplayMode.Col  = ScreenMode->Col;
        gSetting->DisplayMode.Row  = ScreenMode->Row;
    }

    return Status;
}
