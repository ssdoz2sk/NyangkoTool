#ifndef _VARIABLE__H_
#define _VARIABLE__H_

#include <Base.h>
#include <Uefi.h>

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

#endif