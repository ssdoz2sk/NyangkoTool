#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Protocol/Smbios.h>
#include <Library/NyangkoMenuLib.h>
#include <Library/NyangkoDataTableLib.h>

MENU_ITEM       *SmbiosMenuItem;

EFI_STATUS
EFIAPI
SmbiosRefreshCallback (
    DATA_TABLE *DataTable
)
{
    EFI_SMBIOS_TABLE_HEADER *Record;
    CHAR8                   *StringPtr;
    CHAR8                   PreChar;
    CHAR8                   CurChar;
    UINT16                  TableSize;
    UINT16                  EndOfDataTable;
    UINT16                  Index;

    Record      = (EFI_SMBIOS_TABLE_HEADER *)DataTable->Context;
    StringPtr   = (CHAR8 *)Record + Record->Length;
    PreChar     = *StringPtr;
    CurChar     = *(++StringPtr);
    
    while (TRUE) {
        if (PreChar == '\0' && CurChar == '\0') {
            break;
        }

        PreChar = CurChar;
        CurChar = *(++StringPtr);
    }
    
    TableSize = (UINT16)((UINT8 *)StringPtr - (UINT8 *)Record + 1);
    EndOfDataTable = TableSize < (DataTable->Page + 1) * 256 ? TableSize % 256 : 256;

    for (Index = 0; Index < EndOfDataTable; Index += 1) {
        DataTable->Data[Index] = *((UINT8 *)Record + DataTable->Page * 256 + Index);
    }
    DataTable->DataSize = EndOfDataTable;

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SmbiosPrePageCallback(
    DATA_TABLE *DataTable
)
{
    EFI_STATUS          Status = EFI_SUCCESS;

    if (DataTable->Page >= 1) {
        DataTable->Page -= 1;
        Status = SmbiosRefreshCallback(DataTable);
    }

    return Status;
}

EFI_STATUS
EFIAPI
SmbiosNextPageCallback(
    DATA_TABLE *DataTable
)
{
    EFI_STATUS          Status = EFI_SUCCESS;

    if (DataTable->Page + 1 < DataTable->MaxPages) {
        DataTable->Page += 1;
        Status = SmbiosRefreshCallback(DataTable);
    }

    return Status;
}

EFI_STATUS
EFIAPI
SmbiosEditCallback (
    DATA_TABLE *DataTable,
    UINT64     Offset,
    UINT8      Data
) 
{
    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ShowSmbiosRegister (
    VOID *Context
)
{
    DATA_TABLE *DataTable;
    CHAR16     TitleTemp[]  = L"Type: 0x00, Handle: 0x0000";
    CHAR16     *Title       = AllocateZeroPool(sizeof(TitleTemp));
    EFI_SMBIOS_TABLE_HEADER *Record = (EFI_SMBIOS_TABLE_HEADER *)Context;

    UnicodeSPrint(Title,
                  sizeof(TitleTemp), 
                  L"Type: 0x%2X, Handle: 0x%4p",
                  Record->Type,
                  Record->Handle);

    DataTable = AllocateZeroPool (sizeof (DATA_TABLE));
    DataTable->Context = Context;
    DataTable->Title   = Title;
    DataTable->CursorX = 0;
    DataTable->CursorY = 0;
    DataTable->Page    = 0;
    
    DataTable->ContentEditAble  = TRUE;
    DataTable->RefreshCallback  = SmbiosRefreshCallback;
    DataTable->PageUpCallback   = SmbiosPrePageCallback;
    DataTable->PageDownCallback = SmbiosNextPageCallback;
    DataTable->EditCellCallback = SmbiosEditCallback;

    RunDatatable(DataTable);

    FreePool(DataTable);
    FreePool(Title);
    gRedraw = TRUE;

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
InitSmbiosMenu(
  IN  VOID  *Context
)
{
    EFI_STATUS              Status;
    EFI_SMBIOS_PROTOCOL     *SmbiosProtocol;
    EFI_SMBIOS_HANDLE       SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
    EFI_SMBIOS_TABLE_HEADER *Record;
    CHAR16                  TitleTemp[]  = L"Type: 0x00, Handle: 0x0000";
    CHAR16                  *Title;
    MENU                    *SmbiosMenu  = AllocateZeroPool(sizeof(MENU));

    InitializeListHead (&SmbiosMenu->MenuItemList);
    StrnCpyS(SmbiosMenu->Title, MAX_STRING_SIZE, L"Smbios Menu", sizeof(L"Smbios Menu")/sizeof(CHAR16));

    Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid,
                                  NULL,
                                  (VOID **)&SmbiosProtocol);

    if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR, "Locate Smbios Protocol error: %r\n", Status));
    }

    while (TRUE) {
        Status = SmbiosProtocol->GetNext (SmbiosProtocol,
                                          &SmbiosHandle,
                                          NULL,
                                          &Record,
                                          NULL);

        if (EFI_ERROR(Status)) {
            if (Status == EFI_NOT_FOUND) {
                break;
            }
            DEBUG((DEBUG_ERROR, "Smbios get next error: %r\n", Status));
            return Status;
        }


        Title = AllocateZeroPool(sizeof(TitleTemp));
        UnicodeSPrint(Title, 
                      sizeof(TitleTemp), 
                      L"Type: 0x%2X, Handle: 0x%4p",
                      Record->Type,
                      Record->Handle);

        RegisterMenuItem (SmbiosMenu,
                          Title,
                          NULL,
                          ShowSmbiosRegister,
                          Record);
    }

    RunMenuLoop(SmbiosMenu);

    FreePool(SmbiosMenu);

    return Status;
}

EFI_STATUS
EFIAPI
SmbiosMenuItemLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  RegisterRootMenuItem(L"Smbios",   &SmbiosMenuItem,    InitSmbiosMenu,     NULL);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SmbiosMenuItemLibDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
    UnregisterRootMenuItem(&SmbiosMenuItem);
    return EFI_SUCCESS;
}