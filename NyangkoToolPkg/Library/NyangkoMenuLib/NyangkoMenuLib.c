#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/NyangkoMenuLib.h>

BOOLEAN         gRedraw = FALSE;
MENU            *gMenu  = NULL;
EFI_GUID        gTailMenuGuid = TAIL_GUID;
EFI_GUID        gRootMenuGuid = ROOT_GUID;
UINTN           DisplayMode;
UINTN           DisplayCol;
UINTN           DisplayRow;

EFI_STATUS
EFIAPI
NyangkoMenuLibConstructor (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
) {
    EFI_STATUS Status;
    DisplayMode = gST->ConOut->Mode->Mode;

    Status = gST->ConOut->QueryMode (gST->ConOut,
                                     DisplayMode,
                                     &DisplayCol,
                                     &DisplayRow);

    if (EFI_ERROR(Status)) {
        DisplayMode = 0;
        DisplayCol = 80;
        DisplayRow = 25;
    }

    gMenu = AllocateZeroPool(sizeof(MENU));
    InitializeListHead (&gMenu->MenuItemList);
    gMenu->Title = L"Nyangko Tool v0.1.0 (25/02/29)";

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
NyangkoMenuLibDestructor (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable    
) {
    EFI_STATUS Status;
    
    Status = gBS->FreePool(gMenu);

    return Status;
}

EFI_STATUS
EFIAPI
RegisterMenuItem (
    IN OUT  MENU             *Menu,
    IN      EFI_STRING       Title,
    IN      MENU_CALLBACK    Func        OPTIONAL,
    IN      VOID*            Context     OPTIONAL
) {
    MENU_ITEM           *MenuItem;
    MenuItem = AllocateZeroPool (sizeof (MENU_ITEM));
    MenuItem->Signature = MENU_SIGNATURE;
    MenuItem->Index     = Menu->MenuItemSize + 1;
    MenuItem->Title     = Title;
    MenuItem->Func      = Func;
    MenuItem->Context   = Context;

    Menu->MenuItemSize += 1;
    InsertTailList(&Menu->MenuItemList, &MenuItem->MenuEntry);

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RegisterRootMenuItem (
    IN EFI_STRING       Title,
    IN MENU_CALLBACK    Func        OPTIONAL,
    IN VOID*            Context     OPTIONAL
) {
    return RegisterMenuItem(gMenu,
                            Title,
                            Func,
                            Context);
}



STATIC
EFI_STATUS
EFIAPI
ShowItem(
    IN MENU         *Menu,
    IN MENU_ITEM    *MenuItem
)
{
    INT32 Attribute = gST->ConOut->Mode->Attribute;
    
    Print(L"%02d. %s", MenuItem->Index, 
                       MenuItem->Title);

    gST->ConOut->SetAttribute(gST->ConOut, Attribute);
    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ShowSelectItem(
    IN MENU         *Menu,
    IN MENU_ITEM    *MenuItem
)
{
    INT32 Attribute = gST->ConOut->Mode->Attribute;
    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_RED);
    Print(L"%02d. %s", MenuItem->Index, 
                       MenuItem->Title);

    gST->ConOut->SetAttribute(gST->ConOut, Attribute);

    return EFI_SUCCESS;
}

STATIC
VOID
ClearScreenBetweenCols (
    UINT16  Col1,
    UINT16  Col2
) {
    UINT16  Index1;
    UINT16  Index2;
    UINT16  Temp;

    if (Col1 > Col2) {
        Temp = Col2;
        Col2 = Col1;
        Col1 = Temp;
    }

    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);

    for (Index1 = 0; Index1 < DisplayRow; Index1 += 1) {
        Index2 = Col1;
        gST->ConOut->SetCursorPosition(gST->ConOut, Col1, Index1);
        while (Index2++ < Col2) {
            Print(L" ");
        }
    }
    
}


STATIC
VOID
MenuInit (
    IN MENU                         *Menu
) 
{
    UINT16  Index;
    UINT16  Index2;
    UINT16  CursorIndex;
    UINT16  LineRows[]  = {0, 2, 0xFFFF};
    UINT16  LineCols[]  = {0, 0xFFFF};
    UINT16  ColSplt     = Menu->ColSplt;
    UINT16  Order       = Menu->Order;
    UINT16  Ratio       = Menu->Ratio;

    // Last Row
    LineRows[sizeof(LineRows) / sizeof (UINT16) - 1]  = (UINT16)DisplayRow - 1;

    if (ColSplt == 0 || ColSplt == 1 || ColSplt >= DisplayCol / 3) {
        ColSplt = 0;
        Order = 0;
        LineCols[0] = 0;
        LineCols[1] = (UINT16)DisplayCol - 1;
    } else {
        if (Order >= ColSplt) {
            Order = 0;
        }
        if (Ratio >= ColSplt) {
            Ratio = 1;
        }

        LineCols[0] = ((UINT16)DisplayCol - 1) / ColSplt * Order;
        if (Ratio + Order == ColSplt) {
            LineCols[1] = (UINT16)DisplayCol - 1;
        } else {
            LineCols[1] = ((UINT16)DisplayCol - 1) / ColSplt * (Ratio + Order) - 1;
        }
    }
    ClearScreenBetweenCols(LineCols[0], LineCols[1]);

    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    for (Index = 0; Index < sizeof(LineRows) / sizeof (UINT16); Index += 1) {
        Index2 = LineCols[0];
        gST->ConOut->SetCursorPosition(gST->ConOut, LineCols[0], LineRows[Index]);
        while (Index2++ < LineCols[1]) {
            gST->ConOut->OutputString(gST->ConOut, L"-");
        }
    }

    for (Index = 0; Index < sizeof(LineCols) / sizeof (UINT16); Index += 1) {
        Index2 = 0;
        CursorIndex = LineRows[0] + 1;
        while (CursorIndex < LineRows[sizeof(LineRows) / sizeof(UINT16) - 1]) {
            gST->ConOut->SetCursorPosition(gST->ConOut, LineCols[Index], CursorIndex);

            if (LineRows[Index2] == CursorIndex) {
                gST->ConOut->OutputString(gST->ConOut, L"+");
                Index2 += 1;
            } else {
                gST->ConOut->OutputString(gST->ConOut, L"|");
            }
            CursorIndex += 1;
        } 
    }
}

VOID
EFIAPI
RunMenuLoop (
    IN MENU                         *Menu
) 
{
    UINT16                          Row;
    UINT16                          Cursor      = DEFAULT_CURSOR;
    UINT16                          PreCursor   = DEFAULT_CURSOR;
    EFI_STATUS                      Status;
    LIST_ENTRY                      *Entry;
    LIST_ENTRY                      *PreEntry;
    LIST_ENTRY                      *List;
    MENU_ITEM                       *MenuItem;
    MENU_ITEM                       *SelectMenuItem = NULL;
    EFI_INPUT_KEY                   Key;
    BOOLEAN                         Redraw = TRUE;
    UINTN                           LineCols[]  = {0, 0xFF};
    UINTN                           LineRows[]  = {3, 0xFF};
    UINT16                          ColSplt     = Menu->ColSplt;
    UINT16                          Order       = Menu->Order;
    UINT16                          Ratio       = Menu->Ratio;
    UINT16                          ItemPrePage = (UINT16)DisplayRow - 4;
    // UINT16                          MenuPages   = (Menu->MenuItemSize - 1) / ItemPrePage + 1;
    UINT16                          Page        = 0;
    UINT16                          Counter;
    UINT16                          SkipCounter;

    if (ColSplt == 0 || ColSplt == 1 || ColSplt >= DisplayCol / 3) {
        ColSplt = 0;
        Order = 0;
        LineCols[0] = 0;
        LineCols[1] = DisplayCol - 1;
        LineRows[1] = DisplayRow - 2;
    } else {
        if (Order >= ColSplt) {
            Order = 0;
        }

        if (Ratio >= ColSplt) {
            Ratio = 1;
        }

        LineCols[0] = (DisplayCol - 1) / ColSplt * Order;
        if (Ratio + Order == ColSplt) {
            LineCols[1] = DisplayCol - 1;
        } else {
            LineCols[1] = (DisplayCol - 1) / ColSplt * (Ratio + Order) - 1;
        }
        LineRows[1] = DisplayRow - 2;
    }
    List = &Menu->MenuItemList;
    
    while (TRUE) {
        if (Redraw || gRedraw) {
            Row         = DEFAULT_CURSOR;
            Counter     = 0;
            SkipCounter = 0;

            MenuInit(Menu);
            gST->ConOut->SetCursorPosition(gST->ConOut, LineCols[0] + 1, 1);
            if (Menu->ShowHeader != NULL) {
                Menu->ShowHeader(Menu);
            } else if (Menu->Title != NULL) {
                Print(L" %s", Menu->Title);
            }

            for (Entry = GetFirstNode (&Menu->MenuItemList); 
                 !IsNull (List, Entry) && Counter < ItemPrePage; 
                 Entry = GetNextNode (List, Entry)) {

                if (SkipCounter < Page * ItemPrePage) {
                    SkipCounter ++;
                    continue;
                }
                Counter ++;
                gST->ConOut->SetCursorPosition(gST->ConOut, LineCols[0] + 2, Row++);
                MenuItem = MENU_ITEM_FROM_ENTRY (Entry);
                if (SelectMenuItem == NULL) {
                    SelectMenuItem = MenuItem;
                    if (Menu->ShowSelectItem != NULL) {
                        Menu->ShowSelectItem(Menu, MenuItem);
                    } else {
                        ShowSelectItem(Menu, MenuItem);
                    }
                } else {
                    if (SelectMenuItem == MenuItem) {
                        if (Menu->ShowSelectItem != NULL) {
                            Menu->ShowSelectItem(Menu, MenuItem);
                        } else {
                            ShowSelectItem(Menu, MenuItem);
                        }
                    } else {
                        if (Menu->ShowItem == NULL) {
                            ShowItem(Menu, MenuItem);
                        } else {
                            Menu->ShowItem(Menu, MenuItem);
                        }
                    }
                }
            }
            
            Redraw = FALSE;
            gRedraw = FALSE;
        }
        if (SelectMenuItem == NULL) {
            DEBUG((DEBUG_ERROR, "No subitem in this menu\n"));
            break;
        }

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
            if (SelectMenuItem->Func == NULL) {
                DEBUG((DEBUG_INFO, "Run item %s, but func is NULL. SKIP!\n", SelectMenuItem->Title));
            } else {
                DEBUG((DEBUG_INFO, "Run item %s, Func: %p\n", SelectMenuItem->Title, SelectMenuItem->Func));
                Status = SelectMenuItem->Func(SelectMenuItem->Context);
                DEBUG((DEBUG_INFO, "Run item %s: %r\n", SelectMenuItem->Title, Status));
            }
        }

        if (Key.ScanCode == SCAN_UP) {
            Entry = &SelectMenuItem->MenuEntry;
            while (TRUE) {
                PreEntry = Entry;
                Entry = GetPreviousNode (List, Entry);
                if (IsNull (List, Entry)) {
                    break;
                }

                MenuItem = MENU_ITEM_FROM_ENTRY (PreEntry);
                SelectMenuItem = MENU_ITEM_FROM_ENTRY (Entry);
                PreCursor = Cursor;
                Cursor -= 1;

                if (Cursor < LineRows[0]) {
                    Cursor = (UINT16)LineRows[1];
                    Page   -= 1;
                    Redraw = TRUE;
                }

                gST->ConOut->SetCursorPosition(gST->ConOut, LineCols[0] + 2, Cursor);
                if (Menu->ShowSelectItem != NULL) {
                    Menu->ShowSelectItem(Menu, SelectMenuItem);
                } else {
                    ShowSelectItem(Menu, SelectMenuItem);
                }

                gST->ConOut->SetCursorPosition(gST->ConOut, LineCols[0] + 2, PreCursor);
                if (Menu->ShowItem == NULL) {
                    ShowItem(Menu, MenuItem);
                } else {
                    Menu->ShowItem(Menu, MenuItem);
                }
                break;
            }
        }
        
        if (Key.ScanCode == SCAN_DOWN) {
            Entry = &SelectMenuItem->MenuEntry;
            while (TRUE) {
                PreEntry = Entry;
                Entry = GetNextNode (List, Entry);
                if (IsNull (List, Entry)) {
                    break;
                }

                MenuItem = MENU_ITEM_FROM_ENTRY (PreEntry);
                SelectMenuItem = MENU_ITEM_FROM_ENTRY (Entry);
                PreCursor = Cursor;
                Cursor += 1;

                if (Cursor > LineRows[1]) {
                    Cursor = (UINT16)LineRows[0];
                    Page   += 1;
                    Redraw = TRUE;
                }

                gST->ConOut->SetCursorPosition(gST->ConOut, LineCols[0] + 2, Cursor);
                if (Menu->ShowSelectItem != NULL) {
                    Menu->ShowSelectItem(Menu, SelectMenuItem);
                } else {
                    ShowSelectItem(Menu, SelectMenuItem);
                }

                gST->ConOut->SetCursorPosition(gST->ConOut, LineCols[0] + 2, PreCursor);
                if (Menu->ShowItem == NULL) {
                    ShowItem(Menu, MenuItem);
                } else {
                    Menu->ShowItem(Menu, MenuItem);
                }
                break;
            }
        }

        if (Key.ScanCode == SCAN_ESC) {
            DEBUG((DEBUG_INFO, "Esc ?\n"));
            gRedraw = TRUE;
            break;
        }
    }
    
    gST->ConOut->ClearScreen (gST->ConOut);
}