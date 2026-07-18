#!/usr/bin/env bash
set -e

ROOTPATH="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD="${1:-chk}"

OUTPATH="$ROOTPATH/out/$BUILD"
LIMINEPATH="$ROOTPATH/vendor/limine"
NTOSKRNLPATH="$OUTPATH/system32/htoskrnl.exe"
LIMINECONF="$ROOTPATH/limine.conf"
DISKIMG="$OUTPATH/htos.img"

if [ ! -f "$NTOSKRNLPATH" ]; then
    echo "kernel not found at $NTOSKRNLPATH, run make $BUILD first"
    exit 1
fi

rm -f "$DISKIMG"
truncate -s 64M "$DISKIMG"

sgdisk -n 1:2048:0 -t 1:ef00 -c 1:"EFI System" "$DISKIMG"

mformat -i "$DISKIMG@@1M" -F ::
mmd -i "$DISKIMG@@1M" ::/EFI
mmd -i "$DISKIMG@@1M" ::/EFI/BOOT
mmd -i "$DISKIMG@@1M" ::/system32

mcopy -i "$DISKIMG@@1M" "$LIMINEPATH/BOOTX64.EFI" ::/EFI/BOOT/BOOTX64.EFI
mcopy -i "$DISKIMG@@1M" "$NTOSKRNLPATH" ::/system32/htoskrnl.exe
mcopy -i "$DISKIMG@@1M" "$LIMINECONF" ::/limine.conf

if [ -d "$OUTPATH/system32/drivers" ] && [ -n "$(ls -A "$OUTPATH/system32/drivers" 2>/dev/null)" ]; then
    mmd -i "$DISKIMG@@1M" ::/system32/drivers
    mcopy -i "$DISKIMG@@1M" "$OUTPATH/system32/drivers/"*.sys ::/system32/drivers/
fi

echo "disk image ready at $DISKIMG"