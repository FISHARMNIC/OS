#!/usr/bin/env bash
# Requires: i686-elf-gcc, i686-elf-grub, mtools, qemu  (all via brew)
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${BUILD_DIR:-build-host}"
COMPILED="${ROOT_DIR}/${BUILD_DIR}/compiled"
KERNEL_BIN="${COMPILED}/MyOS.bin"
DISK_IMG="${COMPILED}/disk.img"
ISO_PATH="${COMPILED}/MyOS.iso"
ISO_ROOT="${ROOT_DIR}/${BUILD_DIR}/isodir-host"
FAT32_IMG_SIZE_MB="${FAT32_IMG_SIZE_MB:-64}"
NO_QEMU="${NO_QEMU:-0}"
MODE="fat32"

die()  { echo "error: $*" >&2; exit 1; }
info() { echo "$*"; }

need_cmd() { command -v "$1" >/dev/null 2>&1 || die "missing: $1 — $2"; }

usage() { cat <<EOF
Usage: $(basename "$0") [--iso|--fat32] [--no-qemu] [--help]

Modes:
  --fat32   FAT32 disk image, macOS-native tools only (default)
  --iso     GRUB rescue ISO via grub-mkrescue + xorriso

Options:
  --no-qemu   Build only, skip QEMU
  --help      Show this help

Env vars:
  BUILD_DIR            build directory        (default: build-host)
  FAT32_IMG_SIZE_MB    disk image size in MB  (default: 64)
  NO_QEMU=1            skip QEMU launch
EOF
}

for arg in "$@"; do
    case "${arg}" in
        --fat32)    MODE="fat32" ;;
        --iso)      MODE="iso"   ;;
        --no-qemu)  NO_QEMU=1   ;;
        --help|-h)  usage; exit 0 ;;
        *) die "unknown argument '${arg}'"; ;;
    esac
done

# cross-compiler
if   command -v /opt/homebrew/bin/i686-elf-gcc >/dev/null 2>&1; then
    CROSS="/opt/homebrew/bin/i686-elf-"
elif command -v i686-elf-gcc >/dev/null 2>&1; then
    CROSS="i686-elf-"
else
    die "i686-elf-gcc not found — brew install i686-elf-gcc i686-elf-binutils"
fi

# QEMU
if   [[ -x /opt/homebrew/bin/qemu-system-i386 ]]; then
    QEMU="/opt/homebrew/bin/qemu-system-i386"
elif command -v qemu-system-i386 >/dev/null 2>&1; then
    QEMU="$(command -v qemu-system-i386)"
else
    die "qemu-system-i386 not found — brew install qemu"
fi

# i686-elf-grub helper
grub_tool() {
    local t="$1"
    if   [[ -x "/opt/homebrew/bin/i686-elf-${t}" ]]; then echo "/opt/homebrew/bin/i686-elf-${t}"
    elif command -v "i686-elf-${t}" >/dev/null 2>&1;  then command -v "i686-elf-${t}"
    else echo ""; fi
}

# build kernel

cd "${ROOT_DIR}"
info "[1/4] Building kernel..."
cmake -S . -B "${BUILD_DIR}" --fresh -DCROSS_COMPILE="${CROSS}"
cmake --build "${BUILD_DIR}" --target kernel

[[ -f "${KERNEL_BIN}" ]] || die "kernel not found at ${KERNEL_BIN}"

GRUB_FILE="$(grub_tool grub-file)"
if [[ -n "${GRUB_FILE}" ]]; then
    "${GRUB_FILE}" --is-x86-multiboot "${KERNEL_BIN}" \
        || die "${KERNEL_BIN} is not a valid multiboot binary"
fi

mkdir -p "${COMPILED}"

# iso mode

if [[ "${MODE}" == "iso" ]]; then
    GRUB_MKRESCUE="$(grub_tool grub-mkrescue)"
    [[ -n "${GRUB_MKRESCUE}" ]] || die "i686-elf-grub-mkrescue not found — brew install i686-elf-grub"
    need_cmd xorriso "brew install xorriso"

    info "[2/4] Building ISO..."
    rm -rf "${ISO_ROOT}"
    mkdir -p "${ISO_ROOT}/boot/grub"
    cp "${KERNEL_BIN}"             "${ISO_ROOT}/boot/MyOS.bin"
    cp "${ROOT_DIR}/boot/FONT.F16" "${ISO_ROOT}/boot/FONT.F16"
    cp "${ROOT_DIR}/boot/grub.cfg" "${ISO_ROOT}/boot/grub/grub.cfg"
    "${GRUB_MKRESCUE}" -o "${ISO_PATH}" "${ISO_ROOT}"
    [[ -f "${ISO_PATH}" ]] || die "ISO not found at ${ISO_PATH}"

    info "[3/4] ISO ready: ${ISO_PATH}"
    [[ "${NO_QEMU}" == "1" ]] && { info "[4/4] Skipping QEMU (NO_QEMU=1)."; exit 0; }
    info "[4/4] Launching QEMU..."
    exec "${QEMU}" -machine pc -drive file="${ISO_PATH}",index=0,media=disk,format=raw
fi

# FAT32 mode (default)

GRUB_MKIMAGE="$(grub_tool grub-mkimage)"
[[ -n "${GRUB_MKIMAGE}" ]] || die "i686-elf-grub-mkimage not found — brew install i686-elf-grub"
need_cmd mformat "brew install mtools"
need_cmd mcopy   "brew install mtools"
need_cmd mmd     "brew install mtools"

# Find GRUB i386-pc lib

GRUB_I386_LIB=""
while IFS= read -r candidate; do
    [[ -f "${candidate}/boot.img" ]] && { GRUB_I386_LIB="${candidate}"; break; }
done < <(find /opt/homebrew /usr/local -name "boot.img" -path "*/i386-pc/*" 2>/dev/null \
         | xargs -I{} dirname {} | sort -u)
[[ -n "${GRUB_I386_LIB}" ]] || die "GRUB i386-pc lib not found — brew install i686-elf-grub"

info "[2/4] Building FAT32 disk image..."

PART_OFFSET=1048576   # 1 MiB (sector 2048)
PART_START_LBA=2048
TOTAL_SECTORS=$(( FAT32_IMG_SIZE_MB * 2048 ))
PART_SECTORS=$(( TOTAL_SECTORS - PART_START_LBA ))

# Create blank disk
dd if=/dev/zero of="${DISK_IMG}" bs=1M count="${FAT32_IMG_SIZE_MB}" 2>/dev/null

# Write MBR partition table
python3 - "${DISK_IMG}" "${FAT32_IMG_SIZE_MB}" <<'PYEOF'
import struct, sys
img, size_mb = sys.argv[1], int(sys.argv[2])
start_lba = 2048
part_sectors = size_mb * 2048 - start_lba

entry = struct.pack('<BBBBBBBBII',
    0x80,
    0xFE, 0xFF, 0xFF,
    0x0C,
    0xFE, 0xFF, 0xFF,
    start_lba, part_sectors,
)

with open(img, 'r+b') as f:
    f.seek(446); f.write(entry); f.write(b'\x00' * 48)
    f.seek(510); f.write(b'\x55\xAA')
PYEOF

# Create mtools config
MTOOLSRC="${COMPILED}/mtools.conf"
cat > "${MTOOLSRC}" <<EOF
drive z:
  file="${DISK_IMG}"
  offset=${PART_OFFSET}
EOF

M() { MTOOLSRC="${MTOOLSRC}" MTOOLS_SKIP_CHECK=1 "$@"; }

# Format + populate filesystem
M mformat -F -v MYOS z:
M mmd z:/boot
M mmd z:/boot/grub

M mcopy "${KERNEL_BIN}"             z:/boot/MyOS.bin
M mcopy "${ROOT_DIR}/boot/grub.cfg" z:/boot/grub/grub.cfg
M mcopy "${ROOT_DIR}/boot/FONT.F16" z:/boot/FONT.F16

# Build GRUB core image
CORE_IMG="${COMPILED}/core.img"
"${GRUB_MKIMAGE}" \
    --format=i386-pc \
    --output="${CORE_IMG}" \
    --prefix="(hd0,msdos1)/boot/grub" \
    part_msdos biosdisk fat multiboot normal configfile echo ls

# Install bootloader
dd if="${GRUB_I386_LIB}/boot.img" of="${DISK_IMG}" bs=1   count=440 conv=notrunc 2>/dev/null
dd if="${CORE_IMG}"               of="${DISK_IMG}" bs=512 seek=1    conv=notrunc 2>/dev/null

[[ -f "${DISK_IMG}" ]] || die "disk image not found at ${DISK_IMG}"
info "[3/4] FAT32 disk ready: ${DISK_IMG}"

[[ "${NO_QEMU}" == "1" ]] && { info "[4/4] Skipping QEMU."; exit 0; }

info "[4/4] Launching QEMU..."
exec "${QEMU}" -machine pc -drive file="${DISK_IMG}",index=0,media=disk,format=raw