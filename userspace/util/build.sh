#!/usr/bin/env bash

D=$(dirname $(realpath "$0"))
FNAME=$1

ELFDIR=$D/../gen/bin/$FNAME.elf
HDIR=$D/../gen/inc/$FNAME.h
LDSCRIPT=$D/user.ld

i686-elf-gcc \
    -I"$D/../../libs/inc" \
    -m32 -nostdlib -static -ffreestanding \
    -T $LDSCRIPT \
    -o $ELFDIR \
    $D/../src/$FNAME.c \
    $D/../lib/main.c

xxd -i $ELFDIR > $HDIR