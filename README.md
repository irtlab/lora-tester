# LoRaWAN Tester Firmware

This repository contains firmware for a LoRaWAN tester device based on the Hardwario Tower kit.

# Build the Firmware
You will need the following tools to build the firmware:
  - cmake
  - ninja
  - GNU make
  - ARM GNU gcc toolchain

On Linux (Ubuntu) you can install all the packages via apt:
```
apt install cmake make ninja gcc-arm-none-eabi binutils-arm-none-eabi
```

On MacOS you can install everything via Homebrew:
```
brew install gcc-arm-embedded cmake make ninja
```

Glone the git repository with submodules and enter it:
```
git clone --recurse-submodules ssh://git@github.com/irtlab/lora-tester
cd lora-tester
```
Build the firmware. The following steps should generate a file called `firmware.bin` in your current working directory:
```
make config
make
```
The `make config` step only needs to be run once. It generates Makefile(s) out of cmake templates. The `make` step builds the binary firmware image out of the source code.

If you want to re-build everything from scratch, run `make clean`. You need to run `make config` again after this.