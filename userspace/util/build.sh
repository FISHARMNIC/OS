#!/usr/bin/env bash

D=$(dirname $(realpath "$0"))
LDSCRIPT=$D/user.ld
BINDIR=$D/../bin

mkdir -p "$BINDIR"

while IFS= read -r -d '' SRC; do
    REL=${SRC#$D/../src/}
    FNAME=$(basename "$REL" .c)
    SUBDIR=$(dirname "$REL")

    OUTDIR=$BINDIR/$SUBDIR
    mkdir -p "$OUTDIR"

    ELFDIR=$OUTDIR/$FNAME.elf

    echo "[BUILD] $REL"

    i686-elf-gcc \
        -I"$D/../../libs/inc" \
        -m32 -nostdlib -static -ffreestanding \
        -T $LDSCRIPT \
        -o $ELFDIR \
        $SRC \
        $D/lib/main.c

    if [ $? -ne 0 ]; then
        echo "[ERROR] Failed to build $REL"
        exit 1
    fi

    echo "[DONE] $REL -> $ELFDIR"
done < <(find "$D/../src" -name "*.c" -print0)