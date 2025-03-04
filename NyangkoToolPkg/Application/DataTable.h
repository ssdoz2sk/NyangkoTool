#ifndef _DATA_TABLE__H_
#define _DATA_TABLE__H_

#include <Base.h>
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

#define BASE_POINT_X 0x4
#define BASE_POINT_Y 0x2

typedef struct _DATA_TABLE DATA_TABLE;

typedef
EFI_STATUS
(EFIAPI *DATA_TABLE_REFRESH_CALLBACK)(
  IN  DATA_TABLE  *Datatable
);

typedef
EFI_STATUS
(EFIAPI *DATA_TABLE_CHANGE_PAGE_CALLBACK)(
  IN  DATA_TABLE  *Datatable
);

typedef
EFI_STATUS
(EFIAPI *DATA_TABLE_EDIT_CELL_CALLBACK)(
  IN  DATA_TABLE  *Datatable,
  IN  UINT64      Offset,
  IN  UINT8       Data
);

#pragma pack(push, 1)

struct _DATA_TABLE{
    VOID                            *Context;
    CHAR16                          *Title;
    UINT16                          DataSize;
    UINT8                           Data[256];
    UINT8                           CursorX;
    UINT8                           CursorY;
    UINT32                          Page;
    UINT32                          MaxPages;
    BOOLEAN                         PauseRefresh;
    BOOLEAN                         ContentEditAble;
    DATA_TABLE_REFRESH_CALLBACK     RefreshCallback;
    DATA_TABLE_CHANGE_PAGE_CALLBACK PageUpCallback;
    DATA_TABLE_CHANGE_PAGE_CALLBACK PageDownCallback;
    DATA_TABLE_EDIT_CELL_CALLBACK   EditCellCallback;
};

#pragma pack(pop)

EFI_STATUS
RunDatatable (
    DATA_TABLE *DataTable
);


#endif