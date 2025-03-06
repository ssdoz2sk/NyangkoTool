#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/NyangkoDataTableLib.h>

EFI_STATUS
EditBox (
    CHAR16  **Buffer, 
    UINTN   Length,
    UINT16  CursorX,
    UINT16  CursorY,
    CHAR16  *DefaultString
);

STATIC VOID
TableInit ()
{
    UINT16   Index;
    UINT16   Index2;
    UINT16   LineRows[] = {2, 4, 21, 23};
    EFI_STATUS Status;
    UINTN    Col;
    UINTN    Row;

    Status = gST->ConOut->QueryMode (gST->ConOut,
                                     gST->ConOut->Mode->Mode,
                                     &Col,
                                     &Row);
    if (EFI_ERROR(Status)) {
        Col = 80;
    }

    // gST->ConOut->Reset(gST->ConOut, FALSE);
    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    for (Index = 0; Index < sizeof(LineRows) / sizeof(UINT16); Index += 1) {
        Index2 = 0;
        gST->ConOut->SetCursorPosition(gST->ConOut, 0, LineRows[Index]);
        while (Index2++ < Col) {
            gST->ConOut->OutputString(gST->ConOut, L"-");
        }
    }
    Index = LineRows[0] + 1;
    Index2 = 1;

    while (Index < LineRows[sizeof(LineRows) / sizeof(UINT16) - 1]) {
        gST->ConOut->SetCursorPosition(gST->ConOut, 3 * 17 + 2, Index);
        if (LineRows[Index2] == Index) {
            gST->ConOut->OutputString(gST->ConOut, L"+");
            Index2 += 1;
        } else {
            gST->ConOut->OutputString(gST->ConOut, L"|");
        }
        Index += 1;
    }

    gST->ConOut->SetAttribute(gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLACK);
    Index = 0;
    while (Index <= 0xF) {
        gST->ConOut->SetCursorPosition(gST->ConOut, 3 * Index + 5, 3);
        Print(L"%02X", Index);
        Index ++;
    }
    Index = 0;
    while (Index <= 0xF) {
        gST->ConOut->SetCursorPosition(gST->ConOut, 2, 5 + Index);
        Print(L"%02X", Index << 4);
        Index ++;
    }

    gST->ConOut->SetCursorPosition(gST->ConOut, 3 * 17 + 4 + 2, 3);
    Print(L"ASCII Code");
}

STATIC VOID
UpdateTablePage (
    DATA_TABLE  *DataTable
) {
    if (DataTable->MaxPages != 0) {
        gST->ConOut->SetAttribute(gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition(gST->ConOut, 3 * 17 + 4 + 2, 22);
        Print(L"%d / %d",DataTable->Page + 1, DataTable->MaxPages);
    }
}

STATIC VOID
SetTableTitle (
    DATA_TABLE  *DataTable
) {
    if (DataTable->Title != NULL && StrSize(DataTable->Title) != 0) {
        gST->ConOut->SetCursorPosition(gST->ConOut, 4, 1);
        gST->ConOut->SetAttribute(gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLACK);
        gST->ConOut->OutputString(gST->ConOut, L"Table: ");
        gST->ConOut->SetAttribute(gST->ConOut, EFI_RED | EFI_BACKGROUND_BLACK);
        Print(L"%S", DataTable->Title);
    }
}

STATIC VOID
TableContentEdit (
    DATA_TABLE  *DataTable,
    UINTN       Offset,
    UINT8       Size
)
{
    EFI_STATUS  Status;
    CHAR16      *RegStr;
    CHAR16      *RegEditStr;
    CHAR16      ByteStr[3];
    UINT16      Index = Offset % 256;
    UINT64      Data;

    if (Size > 8) {
        return;
    }
    DataTable->PauseRefresh = TRUE;

    RegStr = AllocateZeroPool(sizeof(CHAR16) * Size);

    for (; Index < Size; Index += 1) {
        UnicodeSPrint(ByteStr, sizeof(ByteStr), L"%02X", DataTable->Data[Index]);
        StrnCatS(RegStr, Size * 2, ByteStr, 2);
    }
        
    Status  = EditBox(&RegEditStr,              // Target String
                      Size * 2,                 // Edit Size
                      5 + 3 * (Offset % 16),    // OffsetX,
                      5 + Offset % 256 / 16,    // OffsetY
                      RegStr);                  // Default String
    if (!EFI_ERROR(Status)) {
        Data    = StrHexToUint64(RegEditStr);
        DEBUG((DEBUG_INFO, "func: TableContentEdit, Offset %2X Edit to %2X\n", Offset, Data));

        for (Index = 0; Index < Size; Index += 1) {
            DataTable->EditCellCallback(DataTable,
                                        Offset + Index,
                                        (UINT8)(Data >> (Index * 8)));
        }
    }
    if (RegStr != NULL) {
        FreePool(RegStr);
    }
    if (RegEditStr != NULL) {
        FreePool(RegEditStr);
    }
    DataTable->PauseRefresh = FALSE;

}

VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
    DATA_TABLE      *DataTable;
    UINT16          Index;
    UINT16          OffsetX;
    UINT16          OffsetY;

    DataTable = (DATA_TABLE *)Context;
    if (DataTable->PauseRefresh) {
        return;
    }

    DataTable->RefreshCallback(DataTable);

    // Print data (hex)
    gST->ConOut->SetAttribute(gST->ConOut, EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK);
    for (Index = 0; Index < DataTable->DataSize; Index += 1) {
        OffsetX = Index % 16;
        OffsetY = Index / 16;

        gST->ConOut->SetCursorPosition(gST->ConOut,
                                        5 + OffsetX * 3,
                                        5 + OffsetY);
        if (DataTable->CursorX == OffsetX && DataTable->CursorY == OffsetY) {
            gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_RED);
            Print(L"%02X",
                    DataTable->Data[Index]);
            gST->ConOut->SetAttribute(gST->ConOut, EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK);
        } else {
            Print(L"%02X",
                    DataTable->Data[Index]);
        }
    }

    for (Index = DataTable->DataSize; Index < 256; Index += 1) {
        OffsetX = Index % 16;
        OffsetY = Index / 16;

        gST->ConOut->SetCursorPosition(gST->ConOut,
                                        5 + OffsetX * 3,
                                        5 + OffsetY);
        Print(L"  ");
    }

    // Print data (ASCII character)
    gST->ConOut->SetAttribute(gST->ConOut, EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK);
    for (Index = 0; Index < DataTable->DataSize; Index += 1) {
        OffsetX = Index % 16;
        OffsetY = Index / 16;

        gST->ConOut->SetCursorPosition(gST->ConOut,
                                        3 * 17 + 4 + OffsetX,
                                        5 + OffsetY);
        if (' ' <= DataTable->Data[Index] && DataTable->Data[Index] <= '~') {
            Print(L"%c", DataTable->Data[Index]);
        } else {
            Print(L"%c", '.');
        }
    }

    for (Index = DataTable->DataSize; Index < 256; Index += 1) {
        OffsetX = Index % 16;
        OffsetY = Index / 16;

        gST->ConOut->SetCursorPosition(gST->ConOut,
                                        3 * 17 + 4 + OffsetX,
                                        5 + OffsetY);
        Print(L" ");
    }
}

EFI_STATUS
RunDatatable(
    DATA_TABLE *DataTable
){
    EFI_STATUS      Status;
    EFI_INPUT_KEY   Key;
    EFI_EVENT       TimerEvent;

    gST->ConOut->ClearScreen (gST->ConOut);
    TableInit();
    SetTableTitle(DataTable);
    UpdateTablePage(DataTable);
    DataTable->RefreshCallback(DataTable);
    Status = gBS->CreateEvent ( EVT_TIMER | EVT_NOTIFY_SIGNAL,
                                TPL_NOTIFY,
                                TimerCallback,
                                (VOID *)DataTable,
                                &TimerEvent);

    if (!EFI_ERROR(Status)) {
        Status = gBS->SetTimer (TimerEvent,
                                TimerPeriodic,
                                EFI_TIMER_PERIOD_MILLISECONDS (300));
    }

    while (TRUE)
    {        
        Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
        if (Status == EFI_DEVICE_ERROR) {
            DEBUG((DEBUG_ERROR, "The keystroke information was not returned due to hardware errors."));
            break;
        }

        if (EFI_ERROR (Status))  {
            continue;
        }

        if (Key.ScanCode == SCAN_NULL && Key.UnicodeChar == L'\r') {
            DEBUG((DEBUG_INFO, "Enter ?\n"));
            if (DataTable->ContentEditAble) {
                TableContentEdit(DataTable,
                                 DataTable->Page * 256 + DataTable->CursorY * 16 + DataTable->CursorX,
                                 1);
            }
        }

        if (Key.ScanCode == SCAN_LEFT) {
            if (DataTable->CursorX == 0) {
                if (DataTable->CursorY == (DataTable->DataSize - 1) / 16) {
                    DataTable->CursorX = (DataTable->DataSize - 1) % 16;
                } else {
                    DataTable->CursorX = 15;
                }
            } else {
                DataTable->CursorX -= 1;
            }
        }
        
        if (Key.ScanCode == SCAN_RIGHT) {
            if (DataTable->CursorY == (DataTable->DataSize - 1) / 16) {
                if (DataTable->CursorX == (DataTable->DataSize - 1) % 16) {
                    DataTable->CursorX = 0;
                } else {
                    DataTable->CursorX += 1;
                }
            } else {
                if (DataTable->CursorX == 15) {
                    DataTable->CursorX = 0;
                } else {
                    DataTable->CursorX += 1;
                }
            }
        }

        if (Key.ScanCode ==  SCAN_UP) {
            if (DataTable->CursorY == 0) {
                if (DataTable->CursorX < DataTable->DataSize % 16) {
                    DataTable->CursorY = (UINT8)(DataTable->DataSize / 16);
                } else {
                    DataTable->CursorY = (UINT8)(DataTable->DataSize / 16 - 1);
                }
            } else {
                DataTable->CursorY -= 1;
            }
        }

        if (Key.ScanCode == SCAN_DOWN) {
            if (DataTable->DataSize % 16 == 0) {
                if (DataTable->CursorY == (DataTable->DataSize - 1) / 16) {
                    DataTable->CursorY = 0;
                } else {
                    DataTable->CursorY += 1;
                }
            } else {
                if (DataTable->CursorX < DataTable->DataSize % 16 && 
                    DataTable->CursorY == (DataTable->DataSize - 1) / 16 - 1) {
                    DataTable->CursorY += 1;
                } else if (DataTable->CursorY >= (DataTable->DataSize - 1) / 16 - 1) {
                    DataTable->CursorY = 0;
                } else {
                    DataTable->CursorY += 1;
                }
            }
        }

        if (Key.ScanCode == SCAN_PAGE_DOWN) {
            DataTable->PageDownCallback(DataTable);
            UpdateTablePage(DataTable);
        }
        
        if (Key.ScanCode == SCAN_PAGE_UP) {
            DataTable->PageUpCallback(DataTable);
            UpdateTablePage(DataTable);
        }
        

        if (Key.ScanCode == SCAN_ESC) {
            break;
        }
    }

    if (TimerEvent != NULL) {
        gBS->CloseEvent(TimerEvent);
    }
    
    return EFI_SUCCESS;
}