#ifndef _SMBIOS__H_
#define _SMBIOS__H_

#include <Base.h>
#include <Uefi.h>
#include "Ui/Menu.h"


EFI_STATUS  EFIAPI  InitSmbiosMenu(VOID  *Context);

EFI_STATUS  EFIAPI  ShowSmbusRegister(VOID *Context);


#endif