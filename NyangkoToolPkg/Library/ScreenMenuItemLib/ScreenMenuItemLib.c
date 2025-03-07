#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/NyangkoMenuLib.h>
#include "ScreenManager.h"

MENU_ITEM       *ScreenMenuItem;

EFI_STATUS
EFIAPI
SetScreenMode(VOID *Context) {
    EFI_STATUS      Status;
    SCREEN_MODE     *ScreenMode = (SCREEN_MODE *)Context;
    Status = gST->ConOut->SetMode(gST->ConOut, ScreenMode->Mode);

    gRedraw = TRUE; // Redraw menu

    return Status;
}


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
    StrnCpyS(ScreenMenu->Title, MAX_STRING_SIZE, L"Screen Menu", sizeof(L"Screen Menu")/sizeof(CHAR16));

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

        RegisterMenuItem (ScreenMenu,
                          Title,
                          NULL,
                          SetScreenMode,
                          ScreenMode);

    }

    RunMenuLoop(ScreenMenu);

    FreePool(ScreenMenu);

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ScreenMenuItemLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  RegisterRootMenuItem(L"Screen Size", &ScreenMenuItem, InitScreenMenu,     NULL);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ScreenMenuItemLibDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
    UnregisterRootMenuItem(&ScreenMenuItem);
    return EFI_SUCCESS;
}