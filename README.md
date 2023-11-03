# LoRaWAN Tester Firmware

This repository contains firmware for a LoRaWAN tester device based on the Hardwario Tower kit.

# Build the Firmware

_Note: If you do not need to modify the firmware, you can also download a [pre-build binary image](https://github.com/irtlab/lora-tester/releases/)._

You will need the following tools to build the firmware:
  - cmake
  - ninja
  - GNU make
  - ARM GNU gcc toolchain

On Linux (Ubuntu) you can install all the packages via apt:
```
apt install cmake make ninja gcc-arm-none-eabi binutils-arm-none-eabi
```
Note: On Ubuntu 22.04 you also need to edit sdk/CMakeLists.txt and comment out the line which contains "no-warn-rwx-segments".

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
make release
```

If you want to re-build everything from scratch, run `make clean`.

# Flash the Firmware

You will need the Hardwario Tower firmware tool `bcf` to flash the firmware into the device. Install it if you don't have it:
```
python3 -m venv .venv
. .venv/bin/activate
pip install bcf
```
Now you should be able to run the command `bcf`.

Connect the tester device to your computer with a USB cable and flash the firmware to the device:
```
bcf flash --device <device> --unprotect firmware.bin
```
where `<device>` is the pathname to the device's special file under /dev. (something like /dev/tty.usbserial-xxxx)

# Join The Things Network

Follow [these instructions](https://github.com/hardwario/lora-modem/wiki/Connecting-to-The-Things-Network) to get the tester device connected to The Things Network (TTN).
