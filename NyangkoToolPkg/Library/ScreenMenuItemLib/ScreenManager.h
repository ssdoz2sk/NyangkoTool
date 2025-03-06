#ifndef _SCREEN_MANAGER__H_
#define _SCREEN_MANAGER__H_

#include <Base.h>
#include <Uefi.h>

#pragma pack(push, 1)
typedef struct {
    UINTN       Mode;
    UINTN       Col;
    UINTN       Row;
} SCREEN_MODE;
#pragma pack(pop)

#endif