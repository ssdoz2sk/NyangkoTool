#ifndef _MENU__H_
#define _MENU__H_

#include <Base.h>
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

#define MENU_SIGNATURE  SIGNATURE_32 ('M', 'E', 'N', 'U')
#define ROOT_GUID { 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}
#define TAIL_GUID { 0xFFFFFFFF, 0xFFFF, 0xFFFF, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }}
#define DEFAULT_CURSOR  3

typedef struct _MENU MENU;
typedef struct _MENU_ITEM MENU_ITEM;

typedef
EFI_STATUS
(EFIAPI *SHOW_MENU_HEADER)(
  IN  MENU      *Menu
);

typedef
EFI_STATUS
(EFIAPI *SHOW_MENU_ITEM)(
  IN  MENU      *Menu,
  IN  MENU_ITEM *Item
);

typedef
EFI_STATUS
(EFIAPI *MENU_CALLBACK)(
  IN  VOID                     *Context
);

#pragma pack(push, 1)

struct _MENU {
    LIST_ENTRY        MenuItemList;
    UINT32            MenuItemSize;
    UINT16            ColSplt;
    UINT16            Ratio;
    UINT16            Order;
    EFI_STRING        Title;
    SHOW_MENU_HEADER  ShowHeader;
    SHOW_MENU_ITEM    ShowItem;
    SHOW_MENU_ITEM    ShowSelectItem;
};

struct _MENU_ITEM {
    UINT32          Signature;
    UINT32          Index;
    EFI_STRING      Title;
    MENU_CALLBACK   Func;
    VOID            *Context;
    LIST_ENTRY      MenuEntry;
};

#pragma pack(pop)

#define MENU_ITEM_FROM_ENTRY(a)          CR (a, MENU_ITEM, MenuEntry, MENU_SIGNATURE)

//
// Global Variables
//
extern EFI_GUID gTailMenuGuid;
extern EFI_GUID gRootMenuGuid;
extern BOOLEAN  gRedraw;
extern MENU     *gMenu;
extern UINTN    DisplayMode;
extern UINTN    DisplayCol;
extern UINTN    DisplayRow;


VOID
EFIAPI
RunMenuLoop (
    IN MENU             *Menu
);

EFI_STATUS
EFIAPI
RegisterMenuItem (
    IN OUT  MENU             *Menu,
    IN      EFI_STRING       Title,
    IN      MENU_CALLBACK    Func        OPTIONAL,
    IN      VOID*            Context     OPTIONAL
);

EFI_STATUS
EFIAPI
RegisterRootMenuItem (
    IN EFI_STRING       Title,
    IN MENU_CALLBACK    Func        OPTIONAL,
    IN VOID*            Context     OPTIONAL
);


#endif