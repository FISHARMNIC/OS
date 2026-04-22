#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${BUILD_DIR:-build-host}"
ISO_PATH="${ROOT_DIR}/${BUILD_DIR}/compiled/MyOS.iso"
ISO_ROOT="${ROOT_DIR}/${BUILD_DIR}/isodir-host"
KERNEL_BIN="${ROOT_DIR}/${BUILD_DIR}/compiled/MyOS.bin"

cd "${ROOT_DIR}"

if command -v /opt/homebrew/bin/i686-elf-gcc >/dev/null 2>&1; then
    CROSS_COMPILE_PREFIX="/opt/homebrew/bin/i686-elf-"
elif command -v i686-elf-gcc >/dev/null 2>&1; then
    CROSS_COMPILE_PREFIX="i686-elf-"
else
    echo "error: i686-elf-gcc not found on host." >&2
    echo "hint: brew install i686-elf-gcc i686-elf-binutils" >&2
    exit 1
fi

if command -v /opt/homebrew/bin/i686-elf-grub-mkrescue >/dev/null 2>&1; then
    GRUB_MKRESCUE="/opt/homebrew/bin/i686-elf-grub-mkrescue"
elif command -v i686-elf-grub-mkrescue >/dev/null 2>&1; then
    GRUB_MKRESCUE="$(command -v i686-elf-grub-mkrescue)"
else
    echo "error: i686-elf-grub-mkrescue not found on host." >&2
    echo "hint: brew install i686-elf-grub xorriso" >&2
    exit 1
fi

if command -v /opt/homebrew/bin/i686-elf-grub-file >/dev/null 2>&1; then
    GRUB_FILE="/opt/homebrew/bin/i686-elf-grub-file"
elif command -v i686-elf-grub-file >/dev/null 2>&1; then
    GRUB_FILE="$(command -v i686-elf-grub-file)"
else
    GRUB_FILE=""
fi

if [[ -x /opt/homebrew/bin/qemu-system-i386 ]]; then
    QEMU_BIN="/opt/homebrew/bin/qemu-system-i386"
elif command -v qemu-system-i386 >/dev/null 2>&1; then
    QEMU_BIN="$(command -v qemu-system-i386)"
else
    echo "error: qemu-system-i386 was not found on host PATH." >&2
    exit 1
fi

echo "[1/4] Building kernel on host..."
cmake -S . -B "${BUILD_DIR}" --fresh -DCROSS_COMPILE="${CROSS_COMPILE_PREFIX}"
cmake --build "${BUILD_DIR}" --target kernel

if [[ ! -f "${KERNEL_BIN}" ]]; then
    echo "error: kernel binary not found at ${KERNEL_BIN}" >&2
    exit 1
fi

if [[ -n "${GRUB_FILE}" ]]; then
    if ! "${GRUB_FILE}" --is-x86-multiboot "${KERNEL_BIN}"; then
        echo "error: ${KERNEL_BIN} is not multiboot compatible" >&2
        exit 1
    fi
fi

echo "[2/4] Creating BIOS ISO on host..."
rm -rf "${ISO_ROOT}"
mkdir -p "${ISO_ROOT}/boot/grub"
cp "${KERNEL_BIN}" "${ISO_ROOT}/boot/MyOS.bin"
cp "${ROOT_DIR}/boot/FONT.F16" "${ISO_ROOT}/boot/FONT.F16"
cp "${ROOT_DIR}/boot/grub.cfg" "${ISO_ROOT}/boot/grub/grub.cfg"
"${GRUB_MKRESCUE}" -o "${ISO_PATH}" "${ISO_ROOT}"

if [[ ! -f "${ISO_PATH}" ]]; then
    echo "error: ISO not found at ${ISO_PATH}" >&2
    exit 1
fi

echo "[3/4] ISO ready: ${ISO_PATH}"
if [[ "${NO_QEMU:-0}" == "1" ]]; then
    echo "[4/4] Skipping QEMU launch (NO_QEMU=1)."
    exit 0
fi

echo "[4/4] Launching QEMU on host via ${QEMU_BIN}"
exec "${QEMU_BIN}" -machine pc -drive file="${ISO_PATH}",index=0,media=disk,format=raw
