#ifndef _VARIABLE__H_
#define _VARIABLE__H_

#include <Base.h>
#include <Uefi.h>
#include "Ui/Menu.h"

#define VARIABLE_MENU_GUID {0x965896B2, 0x394F, 0x42DD, {0xAF, 0x8F, 0xF0, 0x51, 0xD3, 0x61, 0x5C, 0x0D}}
#define VARIABLE_LIST_MENU_GUID {0xC8D6DE03, 0x71CE, 0x4C21, {0x9F, 0x37, 0x57, 0x37, 0x94, 0xDB, 0x68, 0x3C}}

#define INIT_NAME_BUFFER_SIZE  128

typedef struct _VARIABLE VARIABLE;

#pragma pack(push, 1)
struct _VARIABLE
{
    CHAR16      *Name;
    EFI_GUID    Guid;
    UINTN       Size;
    UINT32      Attributes;
    VOID        *Data;
};
#pragma pack(pop)

extern EFI_GUID VariableMenuGuid;

EFI_STATUS  EFIAPI  InitVariableMenu(VOID  *Context);

EFI_STATUS  EFIAPI  SearchVariable(VOID *Context);

#endif