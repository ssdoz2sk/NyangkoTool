#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/NyangkoMenuLib.h>

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
  RegisterRootMenuItem(L"Warn Reset",  ResetSystem,        (VOID *)EfiResetWarm);
  RegisterRootMenuItem(L"Cold Reset",  ResetSystem,        (VOID *)EfiResetCold);
  RegisterRootMenuItem(L"Shutdown",    ResetSystem,        (VOID *)EfiResetShutdown);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ResetMenuItemLibDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return EFI_SUCCESS;
}