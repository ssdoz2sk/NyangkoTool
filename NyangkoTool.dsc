[Defines]
  PLATFORM_NAME                  = NyangkoTool
  PLATFORM_GUID                  = 7971ac57-d363-4fba-9fbd-cf2f0be4ab7a
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  SUPPORTED_ARCHITECTURES        = X64|AARCH64
  OUTPUT_DIRECTORY               = Build/NyangkoTool-$(ARCH)
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT

[BuildOptions]
  GCC:RELEASE_*_*_CC_FLAGS             = -DMDEPKG_NDEBUG
  INTEL:RELEASE_*_*_CC_FLAGS           = /D MDEPKG_NDEBUG
  MSFT:RELEASE_*_*_CC_FLAGS            = /D MDEPKG_NDEBUG
  RVCT:RELEASE_*_*_CC_FLAGS            = -DMDEPKG_NDEBUG

[LibraryClasses]
  #
  # Entry Point Libraries
  #
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  #
  # Common Libraries
  #
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf  
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf


[LibraryClasses.ARM, LibraryClasses.AARCH64]
  NULL|ArmPkg/Library/CompilerIntrinsicsLib/CompilerIntrinsicsLib.inf
  NULL|MdePkg/Library/BaseStackCheckLib/BaseStackCheckLib.inf

[Components]
  NyangkoToolPkg/Application/NyangkoTool.inf {
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0xFF
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x8000004F
  }
