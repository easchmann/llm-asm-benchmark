#!/usr/bin/env bash
# shared config for the run scripts


ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
KERNELS_DIR="$ROOT/kernels/polybench"
RESULTS_DIR="$ROOT/results"
FREEDOS_DIR="$ROOT/freedos"
WORK="$ROOT/.work"

# toolchain
IA16_GCC="${IA16_GCC:-ia16-elf-gcc}"
GCC="${GCC:-gcc}"
QEMU="${QEMU:-qemu-system-i386}"
MCOPY="${MCOPY:-mcopy}"

FREEDOS_IMG="$FREEDOS_DIR/FD14BOOT.img"

# which kernels to run
KERNELS=(atax bicg mvt)

mkdir -p "$RESULTS_DIR" "$WORK"

log() {
    echo "[$(date +%H:%M:%S)] $*" >&2
}

check_toolchain() {
    ok=1
    command -v "$GCC" >/dev/null 2>&1 || { log "no $GCC found"; ok=0; }
    command -v "$IA16_GCC" >/dev/null 2>&1 || { log "no $IA16_GCC found"; ok=0; }
    command -v "$QEMU" >/dev/null 2>&1 || { log "no $QEMU found"; ok=0; }
    command -v "$MCOPY" >/dev/null 2>&1 || { log "no mtools (mcopy) found"; ok=0; }
    [ -f "$FREEDOS_IMG" ] || { log "missing freedos image at $FREEDOS_IMG"; ok=0; }

    if [ "$ok" -eq 0 ]; then
        log "environment setup incomplete"
        exit 1
    fi
}
