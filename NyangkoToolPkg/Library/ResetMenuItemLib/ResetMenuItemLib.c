#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/NyangkoMenuLib.h>

MENU_ITEM       *ResetMenuItem[3];

EFI_STATUS
EFIAPI 
ResetSystem (
  IN  VOID  *Context
)
{
    EFI_RESET_TYPE ResetType = (EFI_RESET_TYPE)(Context); 
    gRT->ResetSystem(ResetType, EFI_SUCCESS, 0, NULL);

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
ResetMenuItemLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  RegisterRootMenuItem(L"Warn Reset",  &ResetMenuItem[0], ResetSystem,        (VOID *)EfiResetWarm);
  RegisterRootMenuItem(L"Cold Reset",  &ResetMenuItem[1], ResetSystem,        (VOID *)EfiResetCold);
  RegisterRootMenuItem(L"Shutdown",    &ResetMenuItem[2], ResetSystem,        (VOID *)EfiResetShutdown);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ResetMenuItemLibDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UnregisterRootMenuItem(&ResetMenuItem[0]);
  UnregisterRootMenuItem(&ResetMenuItem[1]);
  UnregisterRootMenuItem(&ResetMenuItem[2]);

  return EFI_SUCCESS;
}