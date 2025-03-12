# Nyangko Tool

## How to setup the enviroment and build the tool
### Create the workspace and clone the code
1. `mkdir edk2_workspace && cd edk2_workspace`
2. `git clone git@github.com:tianocore/edk2.git`
3. `git clone git@github.com:ssdoz2sk/NyangkoTool.git`

### Init the submodule and build the basetool
1. `cd edk2`
2. `git submodule update --init`
3. `cd BaseTool`
4. `make`

### Build the tool
1. `cd edk2_workspace`
2. `export WORKSPACE=$(pwd)`
3. `export EDK_TOOLS_PATH=$WORKSPACE/edk2/BaseTools`
4. `export PACKAGES_PATH=$WORKSPACE/edk2:$WORKSPACE/NyangkoTool`
5. `build -n $(nproc --all) -a [X64|AARCH64] -t GCC5 -b [DEBUG|RELEASE] -p NyangkoTool/NyangkoTool.dsc`

### Binary
The tool can be found in `$(WORKSPACE)/Build/NyangkoTool-$(ARCH)/$(BUILD_TARGETS)_$(Toolchain)/$(ARCH)/`

## Test the tool with QEMU
### X64
1. `export BIOS_IMG="Build/Ovmf3264/DEBUG_GCC5/FV/OVMF.fd"`
2. `build -n $(nproc --all) -a IA32 -a X64 -t GCC5 -b DEBUG -p OvmfPkg/OvmfPkgIa32X64.dsc`
3. `mkdir -p disk/ && cp Build/NyangkoTool-X64/DEBUG_GCC5/X64/NyangkoTool.efi disk`
4. Run the virtual machine 
    ```sh
    qemu-system-x86_64 -m 4096M -machine q35,smm=on,accel=kvm \
        -debugcon file:debug.log \
        -global isa-debugcon.iobase=0x402 \
        -net none \
        -pflash  $BIOS_IMG \
        -serial telnet::4444,server \
        -drive file=fat:rw:$(pwd)/disk/,id=fat32,format=raw,if=none \
        -device ide-hd,serial=0x2,drive=fat32
    ```

### AARCH64
1. `export BIOS_IMG="Build/ArmVirtQemu-AARCH64/DEBUG_GCC5/FV/QEMU_EFI.fd"`
2. `build -n $(nproc --all) -a $ARCH -t GCC5 -b $BUILD_TARGET -p ArmVirtPkg/ArmVirtQemu.dsc`
3. `mkdir -p disk/ && cp Build/NyangkoTool-AARCH64/DEBUG_GCC5/AARCH64/NyangkoTool.efi disk`
4. Run the virtual machine
    ```sh
    qemu-system-aarch64 -m 4096M -cpu cortex-a57 -M virt -nographic \
        -bios $BIOS_IMG \
        -serial telnet::4444,server \
        -drive file=fat:rw:$(pwd)/disk/,id=fat32,format=raw,if=none \
        -device virtio-blk-device,drive=fat32
    ```