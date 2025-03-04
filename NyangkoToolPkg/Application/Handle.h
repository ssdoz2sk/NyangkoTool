#ifndef _HANDLE__H_
#define _HANDLE__H_

#define HANDLE_MENU_GUID  {0x80500F67, 0x7DBD, 0x46C7, {0xB3, 0xDF, 0x68, 0x5E, 0xE5, 0xB9, 0x72, 0x92}}

extern EFI_GUID    HandleMenuGuid;


EFI_STATUS
EFIAPI
ShowAllHandle();

EFI_STATUS
EFIAPI
ShowHandleProtocol(
    EFI_HANDLE Handle
);

EFI_STATUS
EFIAPI
InitHandleMenu(
    VOID  *Context
);

#endif