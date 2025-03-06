#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Protocol/PciIo.h>
#include <Library/NyangkoMenuLib.h>

EFI_STATUS
GuidEditBox (
    CHAR16  **GuidStr,
    CHAR16  *Length,
    UINT16  CursorX,
    UINT16  CursorY,
    CHAR16  *DefaultGuidStr
);

BOOLEAN
EFIAPI
CompareNGuid (
  IN CONST GUID  *Guid1,
  IN CONST GUID  *Guid2,
  IN CONST UINT8 Length
  );

EFI_STATUS
EFIAPI
ShowHandleProtocol(
    VOID *Context
)
{
    EFI_HANDLE  Handle   = (EFI_HANDLE) Context;
    EFI_STATUS  Status;
    EFI_GUID    **ProtocolGuidArray = NULL;
    MENU        *HandleSubMenu      = AllocateZeroPool(sizeof(MENU));
    CHAR16      TitleTemp[]         = L"12345678-1234-4321-1234-123456789ABC";
    CHAR16      *Title;

    UINTN       ArrayCount;
    UINTN       ProtocolIndex;

    InitializeListHead (&HandleSubMenu->MenuItemList);
    HandleSubMenu->Title   = L"Protocols";
    HandleSubMenu->ColSplt = 4;
    HandleSubMenu->Ratio   = 3;
    HandleSubMenu->Order   = 1;

    Status = gBS->ProtocolsPerHandle(Handle,
                                     &ProtocolGuidArray,
                                     &ArrayCount);

    if (EFI_ERROR(Status)) {
        return Status;
    }

    for (ProtocolIndex = 0; ProtocolIndex < ArrayCount; ProtocolIndex += 1) {
        DEBUG((DEBUG_INFO, "  Protocol Guid: %g\n", ProtocolGuidArray[ProtocolIndex]));

        Title = AllocateZeroPool(sizeof(TitleTemp));
        UnicodeSPrint(Title, 
                      sizeof(TitleTemp), 
                      L"%g",
                      ProtocolGuidArray[ProtocolIndex]);

        RegisterMenuItem (HandleSubMenu,
                          Title,
                          NULL,
                          NULL);
    }
    RunMenuLoop(HandleSubMenu);

    FreePool(HandleSubMenu);
    FreePool(ProtocolGuidArray);
    return Status;
}

EFI_STATUS
EFIAPI
ShowAllHandle (
    VOID *Context
)
{
    UINTN       HandleCount;
    EFI_HANDLE  *HandleBuffer;
    UINTN       Index;
    MENU        *HandleMenu     = AllocateZeroPool(sizeof(MENU));
    CHAR16      TitleTemp[]     = L"0x00000000";
    CHAR16      *Title;

    InitializeListHead (&HandleMenu->MenuItemList);
    HandleMenu->Title   = L"Handle(%%p)";
    HandleMenu->ColSplt = 4;
    HandleMenu->Ratio   = 1;
    HandleMenu->Order   = 0;

    gBS->LocateHandleBuffer (AllHandles,
                             NULL,
                             NULL,
                             &HandleCount,
                             &HandleBuffer );

    for (Index = 0; Index < HandleCount; Index++) {
        DEBUG((DEBUG_INFO, "Handle: %p\n", HandleBuffer[Index]));

        Title = AllocateZeroPool(sizeof(TitleTemp));
        UnicodeSPrint(Title, 
                      sizeof(TitleTemp), 
                      L"0x%08p",
                      HandleBuffer[Index]);

        RegisterMenuItem (HandleMenu,
                          Title,
                          ShowHandleProtocol,
                          HandleBuffer[Index]);
    }
    RunMenuLoop(HandleMenu);

    FreePool(HandleMenu);
    FreePool(HandleBuffer);

    return EFI_SUCCESS;
}

STATIC
BOOLEAN
EFIAPI
GuidProtocolExist (
    IN EFI_HANDLE   Handle,
    IN EFI_GUID     *SearchGuid,
    IN UINT8        CompareFirstCharacterOfGuid
) {
    EFI_STATUS  Status;
    EFI_GUID    **ProtocolGuidArray = NULL;
    UINTN       ArrayCount;
    UINTN       ProtocolIndex;
    BOOLEAN     ProtocolExist = FALSE;

    Status = gBS->ProtocolsPerHandle(Handle,
                                     &ProtocolGuidArray,
                                     &ArrayCount);

    if (EFI_ERROR(Status)) {
        return FALSE;
    }

    for (ProtocolIndex = 0; ProtocolIndex < ArrayCount; ProtocolIndex += 1) {
        DEBUG((DEBUG_INFO, "  Protocol Guid: %g\n", ProtocolGuidArray[ProtocolIndex]));

        if (CompareNGuid(SearchGuid, ProtocolGuidArray[ProtocolIndex], CompareFirstCharacterOfGuid)) {
            ProtocolExist = TRUE;
        }
    }

    FreePool(ProtocolGuidArray);
    return ProtocolExist;
}

STATIC
EFI_STATUS
EFIAPI
Search (
    IN EFI_GUID     *SearchGuid,
    IN UINT8        CompareFirstCharacterOfGuid
) {
    UINTN       HandleCount;
    EFI_HANDLE  *HandleBuffer;
    UINTN       Index;
    MENU        *HandleMenu     = AllocateZeroPool(sizeof(MENU));
    CHAR16      TitleTemp[]     = L"0x00000000";
    CHAR16      *Title;

    InitializeListHead (&HandleMenu->MenuItemList);
    HandleMenu->Title   = L"Handle(%%p)";
    HandleMenu->ColSplt = 4;
    HandleMenu->Ratio   = 1;
    HandleMenu->Order   = 0;

    gBS->LocateHandleBuffer (AllHandles,
                             NULL,
                             NULL,
                             &HandleCount,
                             &HandleBuffer );

    for (Index = 0; Index < HandleCount; Index++) {
        DEBUG((DEBUG_INFO, "Handle: %p\n", HandleBuffer[Index]));

        if (!GuidProtocolExist(HandleBuffer[Index], SearchGuid, CompareFirstCharacterOfGuid)) {
            continue;
        }

        Title = AllocateZeroPool(sizeof(TitleTemp));
        UnicodeSPrint(Title, 
                      sizeof(TitleTemp), 
                      L"0x%08p",
                      HandleBuffer[Index]);

        RegisterMenuItem (HandleMenu,
                          Title,
                          ShowHandleProtocol,
                          HandleBuffer[Index]);
    }
    RunMenuLoop(HandleMenu);

    FreePool(HandleMenu);
    FreePool(HandleBuffer);

    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
InitSearchBox(
    VOID *Context
)
{
    EFI_STATUS      Status;
    CHAR16          *GuidEditStr;
    CHAR16          GuidStr[]= L"        -    -    -    -            ";
    EFI_GUID        Guid;
    UINT16          GuidLen = 0;
    UINT16          GuidHalfByteLen;
    UINT8           OptionSize  = 1; // Guid one input
    UINTN           Index;
    EFI_INPUT_KEY   Key;
    INT32           Attribute = gST->ConOut->Mode->Attribute;

// Location 0xff: OK, 0xfe: Cancel
    UINT8           Location = 0xff;


    gST->ConOut->ClearScreen(gST->ConOut);
    gST->ConOut->SetCursorPosition(gST->ConOut, 0, 3);
    Print(L"Guid: %s", GuidStr);

    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_RED);
    gST->ConOut->SetCursorPosition(gST->ConOut, 5, 4);
    Print(L"OK");
    gST->ConOut->SetAttribute(gST->ConOut, Attribute);

    gST->ConOut->SetCursorPosition(gST->ConOut, 10, 4);
    Print(L"Cancel");
    gST->ConOut->SetAttribute(gST->ConOut, Attribute);

    while (TRUE) {        
        Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);

        if (EFI_ERROR (Status)) {
            continue;
        }

        if (Key.ScanCode == SCAN_NULL && Key.UnicodeChar == L'\r') {
            if (Location == 0xff) { // Search
                for (Index = GuidLen; Index < StrLen(GuidStr); Index += 1) {
                    if (GuidStr[Index] == L' ') {
                        GuidStr[Index] = L'0';
                    }
                }
                if (GuidLen >= 24) {
                    GuidHalfByteLen = GuidLen - 4;
                } else if (GuidLen >= 19) {
                    GuidHalfByteLen = GuidLen - 3;
                } else if (GuidLen >= 14) {
                    GuidHalfByteLen = GuidLen - 2;
                } else if (GuidLen >= 9) {
                    GuidHalfByteLen = GuidLen - 1;
                } else {
                    GuidHalfByteLen = GuidLen;
                }
                DEBUG((DEBUG_INFO, "LEN: %d", GuidLen));
                StrToGuid(GuidStr, &Guid);
                Search(&Guid, (UINT8)GuidHalfByteLen);
                for (Index = GuidLen; Index < StrLen(GuidStr); Index += 1) {
                    if (GuidStr[Index] != L'-') {
                        GuidStr[Index] = L' ';
                    }
                }
            } else if (Location == 0xfe) {  // abort
                break;
            } else if (Location == 0) {
                Status = GuidEditBox(&GuidEditStr, &GuidLen, 6, 3, GuidStr);
                if (!EFI_ERROR(Status)) {
                    StrnCpyS(GuidStr, StrLen(GuidStr) + 1, GuidEditStr, StrLen(GuidEditStr));
                    DEBUG((DEBUG_INFO, "[Handle.c] GuidEditStr: %s\n", GuidEditStr));
                    FreePool(GuidEditStr);
                }
                DEBUG((DEBUG_INFO, "[Handle.c] VariableGuidStr: %s\n", GuidStr));
            }
        } else if (Key.ScanCode == SCAN_ESC) {
            gRedraw = TRUE;
            return EFI_ABORTED;
        } else if (Key.ScanCode == SCAN_UP) {
            if (Location == 0xff || Location == 0xfe) {
                Location = 0;
            }
        } else if (Key.ScanCode == SCAN_DOWN) {
            if (Location == OptionSize - 1) {
                Location = 0xff;
            }
        } else if (Key.ScanCode == SCAN_RIGHT || Key.ScanCode == SCAN_LEFT) {
            if (Location == 0xff) {
                Location = 0xfe;
            } else if (Location == 0xfe) {
                Location = 0xff;
            }
        }

        if (Location == 0) {
            gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_RED);
        }
        gST->ConOut->SetCursorPosition(gST->ConOut, 0, 3);
        Print(L"Guid: %s", GuidStr);
        gST->ConOut->SetAttribute(gST->ConOut, Attribute);


        if (Location == 0xff) {
            gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_RED);
        }
        gST->ConOut->SetCursorPosition(gST->ConOut, 5, 4);
        Print(L"OK");
        gST->ConOut->SetAttribute(gST->ConOut, Attribute);

        if (Location == 0xfe) {
            gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_RED);
        }
        gST->ConOut->SetCursorPosition(gST->ConOut, 10, 4);
        Print(L"Cancel");
        gST->ConOut->SetAttribute(gST->ConOut, Attribute);

    }
    gRedraw = TRUE;
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI InitHandleMenu(
  IN  VOID  *Context
)
{
    MENU            *Menu     = AllocateZeroPool(sizeof(MENU));

    InitializeListHead (&Menu->MenuItemList);
    Menu->Title   = L"Handle Menu";

    RegisterMenuItem (Menu,
                      L"Show all handle",
                      ShowAllHandle,
                      NULL);

    RegisterMenuItem (Menu,
                      L"Search Handle by Protocol Guid",
                      InitSearchBox,
                      NULL);
    
    RunMenuLoop(Menu);

    FreePool(Menu);

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HandleMenuItemLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  RegisterRootMenuItem(L"Handle",      InitHandleMenu,     NULL);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HandleMenuItemLibDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return EFI_SUCCESS;
}