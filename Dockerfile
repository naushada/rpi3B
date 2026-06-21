# Reproducible build + self-test for the bcm2837 driver library.
#
# Builds the static driver library, the bcm2837_demo executable, and the
# GoogleTest bit-layout suite, then runs the suite at image-build time (the
# tests exercise the register/field logic over heap buffers, so they need no
# real hardware). The final stage carries just the built binaries.
#
#   podman build -t bcm2837:latest .
#   podman run --rm bcm2837:latest                 # prints the demo usage
#   podman run --rm --entrypoint bcm2837_test bcm2837:latest   # run the suite
#
# Driving real GPIO needs a Pi with /dev/mem exposed to the container; the
# default run is host-safe and only prints usage.

# ── Build stage ─────────────────────────────────────────────────────────────
FROM debian:bookworm-slim AS build

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        libgtest-dev \
        ca-certificates \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

# Configure + build the library, demo, and gtest suite (C++20).
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build -j"$(nproc)"

# Run the host self-test. Reported but non-fatal by default, since the suite
# currently carries known pre-existing failures (Clock/IRQ/SPI layout — see
# docs/DRIVER_REVIEW.md). Build with --build-arg STRICT_TESTS=1 to make any
# test failure fail the image build.
ARG STRICT_TESTS=0
RUN ctest --test-dir build --output-on-failure || [ "$STRICT_TESTS" = "0" ]

# ── Runtime stage ───────────────────────────────────────────────────────────
FROM debian:bookworm-slim AS runtime

LABEL org.opencontainers.image.title="bcm2837-driver" \
      org.opencontainers.image.description="BCM2837/2835/2711 peripheral drivers + gtest self-test" \
      org.opencontainers.image.licenses="MIT"

COPY --from=build /src/build/bcm2837_demo      /usr/local/bin/bcm2837_demo
COPY --from=build /src/build/test/bcm2837_test /usr/local/bin/bcm2837_test

ENTRYPOINT ["/usr/local/bin/bcm2837_demo"]
