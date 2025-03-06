#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/NyangkoMenuLib.h>
#include <Library/NyangkoDataTableLib.h>

STATIC
EFI_STATUS
EFIAPI
AcpiRefreshCallback (
    DATA_TABLE *DataTable
) {
    UINT16      Index;
    UINT16      EndOfDataTable;
    VOID        *Context = DataTable->Context;
    UINT8       *Data = (UINT8 *)Context;
    UINT16      Length = *(UINT16 *)(Data + 4);

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
AcpiPrePageCallback(
    DATA_TABLE *DataTable
)
{
    EFI_STATUS          Status = EFI_SUCCESS;

    if (DataTable->Page >= 1) {
        DataTable->Page -= 1;
        Status = AcpiRefreshCallback(DataTable);
    }

    return Status;
}

STATIC
EFI_STATUS
EFIAPI
AcpiNextPageCallback(
    DATA_TABLE *DataTable
)
{
    EFI_STATUS          Status = EFI_SUCCESS;

    if (DataTable->Page + 1 < DataTable->MaxPages) {
        DataTable->Page += 1;
        Status = AcpiRefreshCallback(DataTable);
    }

    return Status;
}


EFI_STATUS
EFIAPI
ShowAcpiTableDump (
    VOID *Context
)
{

    DATA_TABLE *DataTable;
    EFI_ACPI_DESCRIPTION_HEADER  *Sdt = (EFI_ACPI_DESCRIPTION_HEADER *)Context;
    UINT32     Length                 = Sdt->Length;
    CHAR16     *Signature             = AllocateZeroPool(sizeof(CHAR16) * 5);
    UnicodeSPrint(Signature,
                  sizeof(CHAR16) * 5,
                  L"%c%c%c%c",
                  (UINT8)(Sdt->Signature),
                  (UINT8)(Sdt->Signature >> 8),
                  (UINT8)(Sdt->Signature >> 16),
                  (UINT8)(Sdt->Signature >> 24));

    DataTable = AllocateZeroPool (sizeof (DATA_TABLE));
    DataTable->Context = Context;
    DataTable->Title   = Signature;
    DataTable->CursorX = 0;
    DataTable->CursorY = 0;
    DataTable->Page    = 0;
    DataTable->MaxPages = Length % 256 == 0 ? Length / 256 : Length / 256 + 1;

    DataTable->DataSize         = 256;
    DataTable->ContentEditAble  = FALSE;
    DataTable->RefreshCallback  = AcpiRefreshCallback;
    DataTable->PageUpCallback   = AcpiPrePageCallback;
    DataTable->PageDownCallback = AcpiNextPageCallback;
    // DataTable->EditCellCallback = AcpiEditCallback;

    RunDatatable(DataTable);

    FreePool(DataTable);
    FreePool(Signature);
    gRedraw = TRUE;

    return EFI_SUCCESS;
}

/**
    Get RSDP, XSDT, RSDT, and the number of table under XSDT and RSDT.

    @param Rsdp                           The root system description pointer
    @param Xsdt                           Extended System Descript Table pointer
    @param Rsdt                           Root System Description Table pointer
    @param XsdtPointerToOtherSdtSize      The number of table under XSDT
    @param RsdtPointerToOtherSdtSize      The number of table under RSDT

    @retval                               Return Status based on errors
**/
EFI_STATUS
EFIAPI
GetXsdtRsdtTable(
    OUT EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER    **Rsdp,
    OUT EFI_ACPI_DESCRIPTION_HEADER                     **Xsdt,
    OUT EFI_ACPI_DESCRIPTION_HEADER                     **Rsdt
)
{
    EFI_STATUS                                      Status = EFI_SUCCESS;

    // get RSDP
    Status = EfiGetSystemConfigurationTable (&gEfiAcpiTableGuid, (VOID *) Rsdp);
    if (EFI_ERROR (Status) || *Rsdp == NULL) {
        DEBUG((DEBUG_ERROR, "Fail to locate Rsdp.\n"));
        return Status;
    }
    
    // Get XSDT
    *Xsdt = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN) (*Rsdp)->XsdtAddress;
    if (*Xsdt == NULL || (*Xsdt)->Signature != EFI_ACPI_5_0_EXTENDED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
        DEBUG((DEBUG_ERROR, "XSDT == NULL or wrong signature\n"));
        *Xsdt = NULL;
    }
    DEBUG((DEBUG_INFO, "Xsdt: %p\n", *Xsdt));

    // Get RSDT
    *Rsdt = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN) (*Rsdp)->RsdtAddress;
    if (*Rsdt == NULL || (*Rsdt)->Signature != EFI_ACPI_5_0_ROOT_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
        DEBUG((DEBUG_ERROR, "RSDT == NULL or wrong signature\n"));
        *Rsdt = NULL;
    }
    DEBUG((DEBUG_INFO, "Rsdt: %p\n", *Rsdt));
    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
InitSdtMenu (
    IN  VOID     *Context
) {
    EFI_ACPI_DESCRIPTION_HEADER     *Sdt;
    UINT32                          Data32;
    UINT64                          Data64;
    CHAR16                          TitleTemp[] = L"  Address: PPPPPPPPPPPPPPPP, Signature: XXXX";
    CHAR16                          *Title;
    UINTN                           Offset;
    MENU                            *SdtMenu = AllocateZeroPool(sizeof(MENU));
    EFI_ACPI_DESCRIPTION_HEADER     *Header = (EFI_ACPI_DESCRIPTION_HEADER     *) Context;

    InitializeListHead (&SdtMenu->MenuItemList);

    Title = AllocateZeroPool(sizeof(TitleTemp));
    UnicodeSPrint(Title,
                  sizeof(TitleTemp),
                  L"%c%c%c%c Table(0x%p):",
                  (UINT8)(Header->Signature),
                  (UINT8)(Header->Signature >> 8),
                  (UINT8)(Header->Signature >> 16),
                  (UINT8)(Header->Signature >> 24),
                  (UINTN *)Header);

    DEBUG((DEBUG_INFO, "%s\n", Title));
    
    RegisterMenuItem (SdtMenu,
                      Title,
                      NULL,
                      NULL);

    if (Header->Signature == EFI_ACPI_5_0_ROOT_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
        SdtMenu->Title = L"RSDT Table";

        for (Offset = sizeof(EFI_ACPI_DESCRIPTION_HEADER);
             Offset < Header->Length; 
             Offset += sizeof(UINT32)) {

            Data32  = *(UINT32 *)((UINT8 *)Header + Offset);
            Sdt     = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN)Data32;

            Title = AllocateZeroPool(sizeof(TitleTemp));
            UnicodeSPrint(Title,
                          sizeof(TitleTemp),
                          L"  Address: %8p, Signature: %c%c%c%c", 
                          Sdt,
                          (UINT8)(Sdt->Signature),
                          (UINT8)(Sdt->Signature >> 8),
                          (UINT8)(Sdt->Signature >> 16),
                          (UINT8)(Sdt->Signature >> 24));

            DEBUG((DEBUG_INFO, "%s\n", Title));

            RegisterMenuItem (SdtMenu,
                              Title,
                              ShowAcpiTableDump,
                              Sdt);
        }
    } else if (Header->Signature == EFI_ACPI_5_0_EXTENDED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
        SdtMenu->Title = L"XSDT Table";

        for (Offset = sizeof(EFI_ACPI_DESCRIPTION_HEADER);
             Offset < Header->Length; 
             Offset += sizeof(UINT64)) {

            Data64  = *(UINT64 *)((UINT8 *)Header + Offset);
            Sdt     = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN)Data64;

            Title = AllocateZeroPool(sizeof(TitleTemp));
            UnicodeSPrint(Title,
                          sizeof(TitleTemp),
                          L"  Address: %16p, Signature: %c%c%c%c", 
                          Sdt,
                          (UINT8)(Sdt->Signature),
                          (UINT8)(Sdt->Signature >> 8),
                          (UINT8)(Sdt->Signature >> 16),
                          (UINT8)(Sdt->Signature >> 24));
            
            DEBUG((DEBUG_INFO, "%s\n", Title));

            RegisterMenuItem (SdtMenu,
                              Title,
                              ShowAcpiTableDump,
                              Sdt);
        }
    }
    RunMenuLoop(SdtMenu);
    FreePool(SdtMenu);
    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
InitAcpiMenu(
    IN  VOID  *Context
)
{    
    EFI_STATUS                                      Status;
    EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER    *Rsdp;
    EFI_ACPI_DESCRIPTION_HEADER                     *Xsdt;
    EFI_ACPI_DESCRIPTION_HEADER                     *Rsdt;
    MENU                                            *SdtMenu = AllocateZeroPool(sizeof(MENU));
    
    InitializeListHead (&SdtMenu->MenuItemList);
    SdtMenu->Title = L"ACPI Tables";

    Status = GetXsdtRsdtTable(&Rsdp,
                              &Xsdt, 
                              &Rsdt);

    if (EFI_ERROR(Status)) {
        return Status;
    }

    if (Xsdt != NULL) {
        RegisterMenuItem (SdtMenu, L"Xsdt",    InitSdtMenu,    Xsdt);
    }
#ifdef MDE_CPU_X64
    if (Rsdt != NULL) {
        RegisterMenuItem (SdtMenu, L"Rsdt",    InitSdtMenu,    Rsdt);
    }
#endif 
    RunMenuLoop(SdtMenu);

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
AcpiMenuItemLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  RegisterRootMenuItem(L"Acpi Table",  InitAcpiMenu,       NULL);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
AcpiMenuItemLibDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return EFI_SUCCESS;
}