#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/NyangkoMenuLib.h>
#include <Library/NyangkoDataTableLib.h>
#include "Variable.h"

MENU_ITEM       *VariableMenuItem;

EFI_STATUS
GuidEditBox (
    CHAR16  **GuidStr,
    CHAR16  *Length,
    UINT16  CursorX,
    UINT16  CursorY,
    CHAR16  *DefaultGuidStr
);

EFI_STATUS
EditBox (
    CHAR16  **Buffer, 
    UINTN   Length,
    UINT16  CursorX,
    UINT16  CursorY,
    CHAR16  *DefaultString
);


BOOLEAN
EFIAPI
CompareNGuid (
  IN CONST GUID  *Guid1,
  IN CONST GUID  *Guid2,
  IN CONST UINT8 Length
  );


STATIC
EFI_STATUS
EFIAPI
VariableRefreshCallback (
    DATA_TABLE *DataTable
) {
    UINT16      Index;
    UINT16      EndOfDataTable;
    VARIABLE    *Variable = (VARIABLE *)DataTable->Context;
    UINT8       *Data = Variable->Data;
    UINTN       Length = Variable->Size;

    if (DataTable->Page + 1 < DataTable->MaxPages) {
        EndOfDataTable = 256;
    } else {
        EndOfDataTable = Length % 256;
    }

    for (Index = 0; Index < EndOfDataTable; Index += 1) {
        DataTable->Data[Index] = Data[DataTable->Page * 256 + Index];
    }
    DataTable->DataSize = EndOfDataTable;

    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
VariablePrePageCallback(
    DATA_TABLE *DataTable
)
{
    EFI_STATUS          Status = EFI_SUCCESS;

    if (DataTable->Page >= 1) {
        DataTable->Page -= 1;
        Status = VariableRefreshCallback(DataTable);
    }

    return Status;
}

STATIC
EFI_STATUS
EFIAPI
VariableNextPageCallback(
    DATA_TABLE *DataTable
)
{
    EFI_STATUS          Status = EFI_SUCCESS;

    if (DataTable->Page + 1 < DataTable->MaxPages) {
        DataTable->Page += 1;
        Status = VariableRefreshCallback(DataTable);
    }

    return Status;
}


EFI_STATUS
EFIAPI
ShowVariableTableDump (
    VOID *Context
)
{
    DATA_TABLE *DataTable;
    VARIABLE   *Variable = (VARIABLE *)Context;

    DataTable = AllocateZeroPool (sizeof (DATA_TABLE));
    DataTable->Context = Context;
    DataTable->Title   = Variable->Name;
    DataTable->CursorX = 0;
    DataTable->CursorY = 0;
    DataTable->Page    = 0;
    DataTable->MaxPages = Variable->Size % 256 == 0 ? (UINT32)(Variable->Size / 256) : (UINT32)(Variable->Size / 256 + 1);

    DataTable->DataSize         = 256;
    DataTable->ContentEditAble  = FALSE;
    DataTable->RefreshCallback  = VariableRefreshCallback;
    DataTable->PageUpCallback   = VariablePrePageCallback;
    DataTable->PageDownCallback = VariableNextPageCallback;
    // DataTable->EditCellCallback = AcpiEditCallback;

    RunDatatable(DataTable);

    FreePool(DataTable);
    gRedraw = TRUE;

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
VariableShowSelectItem(
    IN MENU         *Menu,
    IN MENU_ITEM    *MenuItem
)
{
    INT32       Attribute  = gST->ConOut->Mode->Attribute;
    VARIABLE    *Variable  = (VARIABLE *)MenuItem->Context;
    CHAR16      *PrintName;
    UINT8       MaxNameLen = 20;
    UINTN       Index;

    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_RED);
    PrintName = AllocateZeroPool ((MaxNameLen + 1) * sizeof(CHAR16));
    
    if (StrLen(Variable->Name) >= MaxNameLen) {
        StrnCpyS(PrintName, MaxNameLen, Variable->Name, MaxNameLen - 1);
        PrintName[MaxNameLen - 3] = L'.';
        PrintName[MaxNameLen - 2] = L'.';
        PrintName[MaxNameLen - 1] = L'.';
    } else {
        StrnCpyS(PrintName, MaxNameLen, Variable->Name, StrLen(Variable->Name));
        
        for (Index = StrLen(Variable->Name); Index < MaxNameLen; Index += 1) {
            PrintName[Index] = L' ';
        }
    }
    Print(L"%02d. %s", MenuItem->Index, 
                       PrintName);
    Print(L"  %g", &Variable->Guid);
    
    FreePool(PrintName);

    gST->ConOut->SetAttribute(gST->ConOut, Attribute);

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
VariableShowItem(
    IN MENU         *Menu,
    IN MENU_ITEM    *MenuItem
)
{
    INT32       Attribute  = gST->ConOut->Mode->Attribute;
    VARIABLE    *Variable  = (VARIABLE *)MenuItem->Context;
    CHAR16      *PrintName;
    UINT8       MaxNameLen = 20;
    UINTN       Index;

    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    PrintName = AllocateZeroPool ((MaxNameLen + 1) * sizeof(CHAR16));
    
    if (StrLen(Variable->Name) >= MaxNameLen) {
        StrnCpyS(PrintName, MaxNameLen, Variable->Name, MaxNameLen - 1);
        PrintName[MaxNameLen - 3] = L'.';
        PrintName[MaxNameLen - 2] = L'.';
        PrintName[MaxNameLen - 1] = L'.';
    } else {
        StrnCpyS(PrintName, MaxNameLen, Variable->Name, StrLen(Variable->Name));
        
        for (Index = StrLen(Variable->Name); Index < MaxNameLen; Index += 1) {
            PrintName[Index] = L' ';
        }
    }
    Print(L"%02d. %s", MenuItem->Index, 
                       PrintName);
    Print(L"  %g", &Variable->Guid);
    
    FreePool(PrintName);

    gST->ConOut->SetAttribute(gST->ConOut, Attribute);

    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
Search (
    IN EFI_STRING  SearchName,
    IN EFI_GUID    *SearchGuid,
    IN UINT16      CompareFistHalfByteOfGuid
) {
    EFI_STATUS          Status = EFI_SUCCESS;
    BOOLEAN             SearchByName = FALSE;
    BOOLEAN             SearchByGuid = FALSE;
    UINTN               SearchNameLen;
    UINT8               SearchGuidLen;
    CHAR16              *VariableName;
    UINTN               NameSize;
    UINTN               NameBufferSize;
    EFI_GUID            Guid;
    UINTN               VarSize;
    UINT32              Attributes;
    VARIABLE            *Variable;
    MENU                *VariableMenu    = AllocateZeroPool(sizeof(MENU));
    VOID                *VarData;

    InitializeListHead (&VariableMenu->MenuItemList);
    StrnCpyS(VariableMenu->Title, MAX_STRING_SIZE, L"Variables", sizeof(L"Variables")/sizeof(CHAR16));

    VariableMenu->ShowItem          = VariableShowItem;
    VariableMenu->ShowSelectItem    = VariableShowSelectItem;

    if (SearchName != NULL && StrLen(SearchName)) {
        SearchByName = TRUE;
        SearchNameLen = StrLen(SearchName);
    }

    if (SearchGuid != NULL) {
        SearchByGuid = TRUE;
        if (CompareFistHalfByteOfGuid >= 36) {
            SearchGuidLen = 36;
        } else {
            SearchGuidLen = (UINT8)CompareFistHalfByteOfGuid;
        }
    }

    DEBUG ((DEBUG_INFO, "Variable List:\n"));
    NameBufferSize = INIT_NAME_BUFFER_SIZE;
    VariableName = AllocateZeroPool(NameBufferSize);
    if (VariableName == NULL) {
        DEBUG((DEBUG_ERROR, "Out of resource\n"));
        return EFI_OUT_OF_RESOURCES;
    }
    *VariableName = CHAR_NULL;

    while (!EFI_ERROR(Status)) {
        // Get Name/Guid
        NameSize = NameBufferSize;
        Status = gRT->GetNextVariableName(&NameSize, VariableName, &Guid);
        if (Status == EFI_NOT_FOUND){
            Status = EFI_SUCCESS;
            break;
        } else if (Status == EFI_BUFFER_TOO_SMALL) {
            // Resize NameBufferSize and get again.
            NameBufferSize = NameSize > NameBufferSize * 2 ? NameSize : NameBufferSize * 2;
            if (VariableName != NULL) {
                FreePool (VariableName);
            }
            VariableName = AllocateZeroPool(NameBufferSize);
            if (VariableName == NULL) {
                Status = EFI_OUT_OF_RESOURCES;
                break;
            }
            NameSize = NameBufferSize;
            Status = gRT->GetNextVariableName(&NameSize, VariableName, &Guid);
        }

        if (EFI_ERROR(Status)) {
            break;
        }

        if (SearchByName && StrnCmp(SearchName, VariableName, SearchNameLen) != 0) {
            continue;;
        }


        if (SearchByGuid && !CompareNGuid(SearchGuid, &Guid, (UINT8)SearchGuidLen)) {
            continue;;
        }

        // Get Attributes/Size
        VarSize = 0;
        Attributes = 0;
        Status = gRT->GetVariable(VariableName, &Guid, &Attributes, &VarSize, NULL);
        if (Status == EFI_BUFFER_TOO_SMALL){
            VarData = AllocateZeroPool (VarSize);
            if (VarData == NULL) {
                break;
            }
            Status = gRT->GetVariable(VariableName, &Guid, &Attributes, &VarSize, VarData);
            if (!EFI_ERROR(Status)) {
                DEBUG ((DEBUG_INFO, "  Name=L\"%s\" Guid=%g Size=0x%x Attr=0x%x \n",
                                    VariableName,
                                    &Guid,
                                    VarSize,
                                    Attributes));
    
                Variable = AllocateZeroPool (sizeof(VARIABLE));
                Variable->Name          = AllocateCopyPool (StrSize(VariableName), VariableName);
                Variable->Size          = VarSize;
                Variable->Attributes    = Attributes;
                Variable->Data          = VarData;  
                gBS->CopyMem(&Variable->Guid, &Guid,         sizeof(EFI_GUID));

                RegisterMenuItem(VariableMenu,
                                 NULL,
                                 NULL,
                                 ShowVariableTableDump,
                                 Variable);

                Status = EFI_SUCCESS;
            } else {
                FreePool(VarData);
                FreePool(VariableName);
            }
        }
    }

    RunMenuLoop(VariableMenu);

    FreePool(VariableMenu);

    return Status;
}

STATIC
EFI_STATUS
EFIAPI
InitSearchBox(
    VOID *Context
)
{
    EFI_STATUS      Status;
    CHAR16          *VariableEditName;
    CHAR16          VariableName[100] = L"";
    CHAR16          *GuidEditStr;
    CHAR16          GuidStr[]= L"        -    -    -    -            ";
    EFI_GUID        Guid;
    UINT16          GuidLen = 0;
    UINT16          GuidHalfByteLen;
    UINT8           OptionSize  = 2; // Name & Guid, two input
    UINTN           Index;
    EFI_INPUT_KEY   Key;
    INT32           Attribute = gST->ConOut->Mode->Attribute;

// Location 0xff: OK, 0xfe: Cancel
    UINT8           Location = 0;


    gST->ConOut->ClearScreen(gST->ConOut);
    gST->ConOut->SetCursorPosition(gST->ConOut, 0, 2);
    Print(L"Name: %s", VariableName);

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
                Search(VariableName, &Guid, GuidHalfByteLen);
                for (Index = GuidLen; Index < StrLen(GuidStr); Index += 1) {
                    if (GuidStr[Index] != L'-') {
                        GuidStr[Index] = L' ';
                    }
                }
            } else if (Location == 0xfe) {  // abort
                break;
            } else if (Location == 0) {
                Status = EditBox(&VariableEditName, StrLen(VariableName), 6, 2, VariableName);
                if (!EFI_ERROR(Status)) {
                    StrnCpyS(VariableName, 100, VariableEditName, StrLen(VariableEditName));
                    FreePool(VariableEditName);
                }
                DEBUG((DEBUG_INFO, "[Variable.c] VariableName: %s\n", VariableName));
            } else if (Location == 1) {
                Status = GuidEditBox(&GuidEditStr, &GuidLen, 6, 3, GuidStr);
                if (!EFI_ERROR(Status)) {
                    StrnCpyS(GuidStr, StrLen(GuidStr) + 1, GuidEditStr, StrLen(GuidEditStr));
                    DEBUG((DEBUG_INFO, "[Variable.c] GuidEditStr: %s\n", GuidEditStr));
                    FreePool(GuidEditStr);
                }
                DEBUG((DEBUG_INFO, "[Variable.c] VariableGuidStr: %s\n", GuidStr));
            }
        } else if (Key.ScanCode == SCAN_ESC) {
            gRedraw = TRUE;
            return EFI_ABORTED;
        } else if (Key.ScanCode == SCAN_UP) {
            if (Location == 0xff || Location == 0xfe) {
                Location = 1;
            } else if (Location != 0) {
                Location -= 1;
            }
        } else if (Key.ScanCode == SCAN_DOWN) {
            if (Location == OptionSize - 1) {
                Location = 0xff;
            } else {
                Location += 1;
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
        gST->ConOut->SetCursorPosition(gST->ConOut, 0, 2);
        Print(L"Name: %s", VariableName);
        gST->ConOut->SetAttribute(gST->ConOut, Attribute);

        if (Location == 1) {
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

EFI_STATUS
EFIAPI
ShowAllVariable (
    VOID *Context
)
{
    return Search((CHAR16 *)NULL, (EFI_GUID *)NULL, 0);
}

EFI_STATUS
EFIAPI
InitVariableMenu(
    IN  VOID  *Context
)
{    
    // EFI_STATUS      Status;
    MENU            *VariableMenu     = AllocateZeroPool(sizeof(MENU));

    InitializeListHead (&VariableMenu->MenuItemList);
    StrnCpyS(VariableMenu->Title, MAX_STRING_SIZE, L"Variable Menu", sizeof(L"Variable Menu")/sizeof(CHAR16));

    RegisterMenuItem (VariableMenu,
                      L"Show all variables",
                      NULL,
                      ShowAllVariable,
                      NULL);

    RegisterMenuItem (VariableMenu,
                      L"Search variables",
                      NULL,
                      InitSearchBox,
                      NULL);
    
    RunMenuLoop(VariableMenu);

    FreePool(VariableMenu);

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
VariableMenuItemLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  RegisterRootMenuItem(L"Variables",    &VariableMenuItem,  InitVariableMenu,   NULL);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
VariableMenuItemLibDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
    UnregisterRootMenuItem(&VariableMenuItem);
    return EFI_SUCCESS;
}