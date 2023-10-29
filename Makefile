debug:
	cmake -B obj/debug . -G Ninja -DTYPE=debug -DCMAKE_TOOLCHAIN_FILE=sdk/toolchain/toolchain.cmake
	ninja -C obj/debug

release:
	cmake -B obj/release . -G Ninja -DTYPE=release -DCMAKE_TOOLCHAIN_FILE=sdk/toolchain/toolchain.cmake
	ninja -C obj/release

.PHONY: clean
clean:
	rm -rf obj
	rm -rf out
	rm -f firmware.bin

