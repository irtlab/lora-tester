all:
	ninja -C obj/debug

config:
	cmake -B obj/debug . -G Ninja -DTYPE=debug -DCMAKE_TOOLCHAIN_FILE=sdk/toolchain/toolchain.cmake

.PHONY: clean
clean:
	rm -rf obj
	rm -rf out
	rm firmware.bin

