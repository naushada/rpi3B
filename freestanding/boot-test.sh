#!/usr/bin/env bash
# Build kernel8.img, boot it in QEMU raspi3b, and ASSERT the AArch64 interrupt
# path actually ran: reached EL1, installed VBAR_EL1, and took timer IRQs through
# IRQ::dispatch(). Exit non-zero on failure. Used by CI and runnable locally
# (needs the aarch64 cross toolchain + qemu-system-aarch64, or run it inside the
# freestanding/Dockerfile builder).
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

BUILD="${1:-build-bm}"
SECS="${QEMU_SECONDS:-8}"        # wall-clock boot window
MIN_TICKS="${MIN_TICKS:-3}"      # timer IRQs that must arrive to pass

echo "=== build (aarch64 cross) ==="
cmake -S freestanding -B "$BUILD" \
      -DCMAKE_TOOLCHAIN_FILE="$REPO_ROOT/freestanding/toolchain-aarch64.cmake" >/dev/null
cmake --build "$BUILD" -j

echo "=== boot QEMU raspi3b (${SECS}s) ==="
LOG="$(mktemp)"
timeout -s KILL "$SECS" \
    qemu-system-aarch64 -M raspi3b -kernel "$BUILD/kernel8.img" \
        -display none -monitor none -serial stdio > "$LOG" 2>&1 || true

echo "----- captured UART -----"
cat "$LOG"
echo "-------------------------"

fail=0
grep -q "running at EL1"            "$LOG" || { echo "FAIL: never reached EL1 (EL drop broken)";        fail=1; }
grep -q "VBAR_EL1 -> vector_table"  "$LOG" || { echo "FAIL: VBAR_EL1 not installed";                    fail=1; }
grep -q "\[EXCEPTION\]"             "$LOG" && { echo "FAIL: an unhandled exception fired";               fail=1; }

ticks="$(grep -c "timer tick #" "$LOG" || true)"
if [ "${ticks:-0}" -lt "$MIN_TICKS" ]; then
    echo "FAIL: only ${ticks:-0} timer IRQs reached the handler (need >= $MIN_TICKS)"
    fail=1
fi

rm -f "$LOG"
if [ "$fail" -eq 0 ]; then
    echo "PASS: EL1 drop + VBAR_EL1 + $ticks timer IRQs via IRQ::dispatch()"
fi
exit "$fail"
