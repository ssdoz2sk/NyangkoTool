#ifndef _SCREEN_MANAGER__H_
#define _SCREEN_MANAGER__H_

#include <Base.h>
#include <Uefi.h>

#define SCREEN_MENU_GUID {0x54332298, 0x7f17, 0x4835, {0x87, 0x1f, 0xfb, 0xb0, 0xf1, 0xce, 0xdf, 0x96}}

extern EFI_GUID ScreenMenuGuid;

typedef struct {
    UINTN       Mode;
    UINTN       Col;
    UINTN       Row;
} SCREEN_MODE;

EFI_STATUS  EFIAPI  InitScreenMenu(VOID  *Context);

EFI_STATUS  EFIAPI  SetScreenMode(VOID *Context);

#endif