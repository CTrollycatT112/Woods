#!/usr/bin/env bash
set -e

ROOTPATH="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD="${1:-chk}"

OUTPATH="$ROOTPATH/out/$BUILD"
DISKIMG="$OUTPATH/htos.img"

OVMFCODE="/usr/share/OVMF/OVMF_CODE_4M.fd"
OVMFVARS="/usr/share/OVMF/OVMF_VARS_4M.fd"
OVMFVARSLOCAL="$OUTPATH/OVMF_VARS_4M.fd"

if [ ! -f "$DISKIMG" ]; then
    echo "disk image not found at $DISKIMG, run make disk first"
    exit 1
fi

if [ ! -f "$OVMFVARSLOCAL" ]; then
    cp "$OVMFVARS" "$OVMFVARSLOCAL"
fi

qemu-system-x86_64 \
    -machine q35 \
    -cpu qemu64 \
    -m 2G \
    -drive if=pflash,format=raw,unit=0,file="$OVMFCODE",readonly=on \
    -drive if=pflash,format=raw,unit=1,file="$OVMFVARSLOCAL" \
    -drive format=raw,file="$DISKIMG" \
    -serial stdio \
    -no-reboot \
    -no-shutdown