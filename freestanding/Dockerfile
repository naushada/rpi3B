# Builder image for the freestanding aarch64 kernel image + QEMU boot test.
# Bind-mount the repo and run freestanding/build-and-run.sh inside:
#
#   podman build -t bcm2837-bm-builder -f freestanding/Dockerfile freestanding
#   podman run --rm -v "$PWD":/src:Z -w /src bcm2837-bm-builder \
#       freestanding/build-and-run.sh
FROM debian:bookworm-slim

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
        cmake make \
        gcc-aarch64-linux-gnu g++-aarch64-linux-gnu \
        qemu-system-arm \
 && rm -rf /var/lib/apt/lists/*

# qemu-system-arm provides qemu-system-aarch64 (with the raspi3b machine).
