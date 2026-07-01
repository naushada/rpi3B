#!/usr/bin/env bash
# Cross-build the freestanding kernel image and boot it in QEMU raspi3b.
# Meant to run inside the freestanding/Dockerfile builder (aarch64 cross + QEMU),
# with the repo bind-mounted at the working directory.
set -euo pipefail

BUILD="${1:-build-bm}"
SECONDS_TO_RUN="${QEMU_SECONDS:-6}"
REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

echo "=== configure + build (aarch64 cross) ==="
cmake -S freestanding -B "$BUILD" \
      -DCMAKE_TOOLCHAIN_FILE="$REPO_ROOT/freestanding/toolchain-aarch64.cmake" >/dev/null
cmake --build "$BUILD" -j

echo
echo "=== boot kernel8.img in QEMU raspi3b (${SECONDS_TO_RUN}s, PL011 on -serial stdio) ==="
timeout -s KILL "${SECONDS_TO_RUN}" \
    qemu-system-aarch64 -M raspi3b -kernel "$BUILD/kernel8.img" \
        -display none -monitor none -serial stdio || true

echo
echo "=== qemu exited ==="
