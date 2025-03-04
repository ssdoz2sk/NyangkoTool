#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Protocol/PciIo.h>
#include "Pci.h"
#include "DataTable.h"

EFI_GUID PciMenuGuid = PCI_MENU_GUID;

EFI_STATUS 
EFIAPI
ShowHeader(
    IN MENU     *Menu
)
{
    Print(L" Pci Devices");

    return EFI_SUCCESS;
}

CHAR16*
EFIAPI
GetPciDescription(
    EFI_PCI_IO_PROTOCOL *PciDevice
)
{
    EFI_STATUS  Status;
    UINT8       Data[256];
    UINT8       Index;

    Status = PciDevice->Pci.Read (
        PciDevice,
        EfiPciIoWidthUint8,
        0,
        256,
        Data
    );
    if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_INFO, "Read PCI Space error: %r", Status));
        return L"";
    }

    for (Index = 0; Index < sizeof(gPciClassCodeMap) / sizeof(PCI_CLASS_CODE_MAP); Index += 1) {
        if (gPciClassCodeMap[Index].ClassCode == Data[0x0B] &&
            gPciClassCodeMap[Index].SubClassCode == Data[0x0A] &&
            (gPciClassCodeMap[Index].ProgrammingInterface == Data[0x09] ||
             gPciClassCodeMap[Index].ProgrammingInterface == 0xFF)) {

            return gPciClassCodeMap[Index].Description;
        }
        if (gPciClassCodeMap[Index].ClassCode == 0xFF) {
            return gPciClassCodeMap[Index].Description;
        }
    }
    // It should never be executed.
    return L"";
}

EFI_STATUS
EFIAPI
ShowItem(
    IN MENU         *Menu,
    IN MENU_ITEM    *MenuItem
)
{
    EFI_STATUS          Status;
    INT32               Attribute = gST->ConOut->Mode->Attribute;
    EFI_PCI_IO_PROTOCOL *PciDevice;
    UINTN               PciSeg;
    UINTN               PciBus;
    UINTN               PciDev;
    UINTN               PciFun;
    CHAR16              *Description;

    PciDevice   = (EFI_PCI_IO_PROTOCOL *)MenuItem->Context;
    Description = GetPciDescription(PciDevice);
    
    // Get PCI Device Bus/Device/Function Numbers
    Status = PciDevice->GetLocation(PciDevice, 
                                    &PciSeg, 
                                    &PciBus, 
                                    &PciDev, 
                                    &PciFun);
    
    if (EFI_ERROR(Status)) {
        Print(L"%02d. %s", MenuItem->Index, 
                           MenuItem->Title);
        gST->ConOut->SetAttribute(gST->ConOut, Attribute);
        return Status;
    }
    Print(L"%02d. B: 0x%02X, D: 0x%02X, F: 0x%02X, %s",
          MenuItem->Index,
          PciBus,
          PciDev,
          PciFun,
          Description);

    gST->ConOut->SetAttribute(gST->ConOut, Attribute);
    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ShowSelectItem(
    IN MENU         *Menu,
    IN MENU_ITEM    *MenuItem
)
{
    EFI_STATUS          Status;
    INT32               Attribute = gST->ConOut->Mode->Attribute;
    EFI_PCI_IO_PROTOCOL *PciDevice;
    UINTN               PciSeg;
    UINTN               PciBus;
    UINTN               PciDev;
    UINTN               PciFun;
    CHAR16              *Description;

    PciDevice   = (EFI_PCI_IO_PROTOCOL *)MenuItem->Context;
    Description = GetPciDescription(PciDevice);
    
    // Get PCI Device Bus/Device/Function Numbers
    Status = PciDevice->GetLocation(PciDevice, 
                                    &PciSeg, 
                                    &PciBus, 
                                    &PciDev, 
                                    &PciFun);
    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_RED);
    
    if (EFI_ERROR(Status)) {
        Print(L"%02d. %s", MenuItem->Index, 
                           MenuItem->Title);
        gST->ConOut->SetAttribute(gST->ConOut, Attribute);
        return Status;
    }
    Print(L"%02d. B: 0x%02X, D: 0x%02X, F: 0x%02X, %s",
          MenuItem->Index,
          PciBus,
          PciDev,
          PciFun,
          Description);

    gST->ConOut->SetAttribute(gST->ConOut, Attribute);
    return EFI_SUCCESS;
}

EFI_STATUS  EFIAPI  InitPciMenu(
    IN  VOID  *Context
)
{
    EFI_STATUS                  Status;
    UINTN                       HandleCount = 0;
    EFI_HANDLE                  *HandleBuffer = NULL;
    EFI_PCI_IO_PROTOCOL         *PciDevice;
    UINTN                       PciSeg;
    UINTN                       PciBus;
    UINTN                       PciDev;
    UINTN                       PciFun;
    UINTN                       Index;
    CHAR16                      TitleTemp[] = L"Bus: 00, Device: 00, function: 00";
    CHAR16                      *Title;
    MENU                        *PciMenu     = AllocateZeroPool(sizeof(MENU));

    InitializeListHead (&PciMenu->MenuItemList);
    PciMenu->ShowHeader     = ShowHeader;
    PciMenu->ShowItem       = ShowItem;
    PciMenu->ShowSelectItem = ShowSelectItem;
    
    Status = gBS->LocateHandleBuffer(ByProtocol,
                                     &gEfiPciIoProtocolGuid,
                                     NULL,
                                     &HandleCount,
                                     &HandleBuffer);

    if (EFI_ERROR(Status)) return Status;
    
    // Check PCI devices
    for (Index = 0; Index < HandleCount; Index++) {
        // Collect Onboard PCI devices information
        Status = gBS->HandleProtocol (HandleBuffer[Index], 
                                      &gEfiPciIoProtocolGuid, 
                                      (VOID **) &PciDevice);
        
        if (!EFI_ERROR (Status)) {
            // Get PCI Device Bus/Device/Function Numbers
            Status = PciDevice->GetLocation(PciDevice, 
                                            &PciSeg, 
                                            &PciBus, 
                                            &PciDev, 
                                            &PciFun);
            
            if (EFI_ERROR(Status)) continue;

            
            Title = AllocateZeroPool(sizeof(TitleTemp));
            UnicodeSPrint(Title, 
                          sizeof(TitleTemp), 
                          L"Bus: %02d, Device: %02d, function: %02d",
                          PciBus,
                          PciDev,
                          PciFun);

            DEBUG((DEBUG_INFO, "PCI Seg: %02d, Bus: %02d, Device: %02d, function: %02d\n", 
                    PciSeg,
                    PciBus,
                    PciDev,
                    PciFun));
            
            PushMenuItem (PciMenu,
                          Title,
                          ShowPciRegister,
                          PciDevice);
        }
    }

    RunMenuLoop(PciMenu);

    FreePool(PciMenu);
    FreePool(HandleBuffer);

    return Status;
}

EFI_STATUS
EFIAPI
RefreshCallback (
    DATA_TABLE *DataTable
)
{
    EFI_STATUS          Status;
    EFI_PCI_IO_PROTOCOL *PciDevice;

    PciDevice = (EFI_PCI_IO_PROTOCOL *)DataTable->Context;
    
    Status = PciDevice->Pci.Read (
        PciDevice,
        EfiPciIoWidthUint8,
        256 * DataTable->Page,
        256,
        &DataTable->Data
    );
    
    return Status;
}

EFI_STATUS
EFIAPI
PrePageCallback(
    DATA_TABLE *DataTable
)
{
    EFI_STATUS          Status = EFI_SUCCESS;

    if (DataTable->Page >= 1) {
        DataTable->Page -= 1;
        Status = RefreshCallback(DataTable);
    }

    return Status;
}

EFI_STATUS
EFIAPI
NextPageCallback(
    DATA_TABLE *DataTable
)
{
    EFI_STATUS          Status = EFI_SUCCESS;

    if (DataTable->Page + 1 < DataTable->MaxPages) {
        DataTable->Page += 1;
        Status = RefreshCallback(DataTable);
    }

    return Status;
}

EFI_STATUS
EFIAPI
EditCallback (
    DATA_TABLE *DataTable,
    UINT64     Offset,
    UINT8      Data
) 
{
    EFI_STATUS          Status;
    EFI_PCI_IO_PROTOCOL *PciDevice;

    PciDevice = (EFI_PCI_IO_PROTOCOL *)DataTable->Context;

    Status = PciDevice->Pci.Write (
        PciDevice,
        EfiPciIoWidthUint8,
        (UINT32)Offset,
        1,
        &Data
    );

    DEBUG((DEBUG_INFO, "Write %2X to Offset %2X, Status: %r", Data, Offset, Status));

    return Status;
}

BOOLEAN
EFIAPI
IsPcieDevice (
    EFI_PCI_IO_PROTOCOL *PciDevice
) {
    EFI_STATUS  Status;
    UINT8       Data[256];
    UINT8       CapabilityPointer;

    Status = PciDevice->Pci.Read (
        PciDevice,
        EfiPciIoWidthUint8,
        0,
        256,
        Data
    );

    if (!EFI_ERROR(Status)) {
        CapabilityPointer = Data[0x34];
        while (CapabilityPointer != 0x0) {
            if (Data[CapabilityPointer] == 0x10) {
                return TRUE;
            }
            CapabilityPointer = Data[CapabilityPointer + 1];
        }
    }
    return FALSE;
}

EFI_STATUS
EFIAPI
ShowPciRegister (
    VOID *Context
)
{
    DATA_TABLE          *DataTable;
    EFI_PCI_IO_PROTOCOL *PciIo  = (EFI_PCI_IO_PROTOCOL *) Context;
    
    DataTable = AllocateZeroPool (sizeof (DATA_TABLE));
    DataTable->Context = Context;
    DataTable->Title   = GetPciDescription(PciIo);
    DataTable->CursorX = 0;
    DataTable->CursorY = 0;
    DataTable->Page    = 0;
    
    if (IsPcieDevice(PciIo)) {
        DataTable->MaxPages = 16;
    } else {
        DataTable->MaxPages= 1;
    }
    DataTable->DataSize         = 256;
    DataTable->ContentEditAble  = TRUE;
    DataTable->RefreshCallback  = RefreshCallback;
    DataTable->PageUpCallback   = PrePageCallback;
    DataTable->PageDownCallback = NextPageCallback;
    DataTable->EditCellCallback = EditCallback;

    RunDatatable(DataTable);

    FreePool(DataTable);
    gRedraw = TRUE;

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PciMenuDestroy() 
{
    return EFI_SUCCESS;
}
