# Minimal OS Build (CMake)

This project mirrors the `boot + kernel` flow from `FISHARMNIC/osWIP`, but replaces shell build scripts with CMake.

## What is included

- Multiboot-compatible entry (`boot/boot.S`)
- Linker script (`boot/linker.ld`)
- GRUB config (`boot/grub.cfg`)
- Minimal `kernel_entry` in C (`kernel/main.c`)
- CMake targets for kernel binary, ISO image, and QEMU run

## Build

```sh
cmake -S . -B build
cmake --build build
```

That produces:

- `build/compiled/MyOS.bin`

## Optional targets

If `grub-mkrescue` is installed:

```sh
cmake --build build --target iso
```

If `qemu-system-i386` is installed too:

```sh
cmake --build build --target run
```

## Toolchain notes

By default, CMake looks for `i686-elf-gcc`.

If you want to override the compiler prefix:

```sh
cmake -S . -B build -DCROSS_COMPILE=<prefix>
```

Example: `-DCROSS_COMPILE=i686-elf-`
