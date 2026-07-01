# rpi3B — bare-metal peripheral drivers for the Raspberry Pi 3B (BCM2837)

[![CI](https://github.com/naushada/rpi3B/actions/workflows/ci.yml/badge.svg)](https://github.com/naushada/rpi3B/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A small, header-light C++ driver layer for the Broadcom **BCM2837** SoC on the
Raspberry Pi 3B. Each peripheral is modelled as a memory-mapped register block
that the driver classes read/modify/write through typed, field-level accessors.

Peripherals: **GPIO**, **Clock manager (CM_GPn)**, **Interrupt controller**,
**I2C (BSC1)**, **SPI0**.

> See [`docs/DRIVER_REVIEW.md`](docs/DRIVER_REVIEW.md) for the design review,
> the per-peripheral issue log, and what is fixed vs. still open.

---

## Overview

This is a **dependency-light, register-level driver library** for the BCM2837's
on-chip peripherals — the kind of thing you'd reach for in a freestanding /
RTOS firmware, a teaching project, or (as it's actually used) a userspace driver
on a booted Pi. It is deliberately small: no HAL framework, no RTOS coupling,
just typed register blocks and field accessors you compose yourself.

What makes it practical is a single **placement-`new` seam** that lets the *same*
driver code run two ways:

- **On hardware** — the register block is overlaid on the peripheral's physical
  MMIO address, so reads/writes hit real silicon.
- **On a host (no hardware)** — the same block is overlaid on an ordinary
  `std::vector<uint32_t>`, so the bit-layout logic is exercised by GoogleTest
  with zero hardware. Every peripheral ships such a test.

On a *booted Linux* Pi, [`inc/mmio.hpp`](inc/mmio.hpp) bridges the two: it
`mmap`s a peripheral block through `/dev/mem` (or `/dev/gpiomem` for GPIO) and
hands the live pointer to the very same region constructor the tests use — so
the host-verified code drives actual pins.

This library is **vendored into** the [`naushada/iot`](https://github.com/naushada/iot)
LwM2M gateway (under `modules/bcm2837`), where its I²C driver backs the mangOH
Yellow sensor integration and its gtest suite runs as a boot-time self-test.

**What it is not:** a complete bare-metal runtime. There is no cross toolchain,
linker script, or `_start` in this repo — see [Status](#status) and
`docs/DRIVER_REVIEW.md` for the precise boundaries.

---

## Hardware requirements

**Primary SoC — Broadcom BCM2837** (quad-core ARM Cortex-A53), but the driver is
**multi-SoC**: the peripheral register *offsets* are identical across the
BCM283x and BCM2711 families, so only the peripheral **base** differs. On a
booted Pi the base is **auto-detected at runtime** (see [Supported boards](#supported-boards-auto-detected)),
so one binary runs on every board below:

| Board | SoC | Peripheral base | Supported |
|-------|-----|-----------------|-----------|
| Pi **1** (A/B/A+/B+) / **Zero** / **Zero W** | BCM2835 | `0x20000000` | ✅ auto-detected |
| Raspberry Pi **2 Model B** (v1.1) | BCM2836 | `0x3F000000` | ✅ auto-detected |
| Raspberry Pi **2 Model B** (v1.2) | BCM2837 | `0x3F000000` | ✅ auto-detected |
| Raspberry Pi **3 Model B / 3B+** | BCM2837 / BCM2837B0 | `0x3F000000` | ✅ primary target |
| **Zero 2 W** / Compute Module **3 / 3+** | BCM2837 | `0x3F000000` | ✅ auto-detected |
| Pi **4** / **400** / **CM4** | BCM2711 | `0xFE000000` | ✅ auto-detected |

> The **Linux MMIO path** picks the base from `/proc/device-tree/soc/ranges` at
> first use (override via `BCM2837::periph_base(...)`). A **bare-metal** build
> instead selects it at compile time with `-DBCM_PERIPH_BASE=0x20000000` /
> `0x3F000000` / `0xFE000000` (see `inc/memory_map.hpp`). Verified on the host
> for all three bases; on-silicon coverage is still BCM2837.

**Pin/peripheral mapping** (BCM GPIO numbers, not 40-pin header positions):

| Peripheral | Pins (ALT0) | Notes |
|------------|-------------|-------|
| GPIO       | any of GPIO0–53 | direct |
| I²C1 (BSC1) | SDA1 = GPIO2, SCL1 = GPIO3 | mux to ALT0 + set clock divider |
| SPI0       | GPIO7–GPIO11 (CE1/CE0/MISO/MOSI/SCLK) | mux to ALT0 + set clock divider |
| Clock mgr  | (internal) | GP clock generators |
| Interrupt  | (internal) | AArch64 `VBAR_EL1` IVT + IRQ dispatch (bare-metal) |

**To run on real hardware you also need:**

- A **booted OS on the Pi** (this is not a standalone bootable image — see Status).
- For the Linux MMIO path: `root` / `CAP_SYS_RAWIO` for `/dev/mem`, or membership
  of the `gpio` group for the unprivileged `/dev/gpiomem` (GPIO only).
- I²C / SPI pins **muxed to ALT0** and the peripheral **clock divider** programmed
  — the register writes alone do not configure the pin mux.
- For a true bare-metal build: an **`aarch64` cross toolchain** + linker script +
  startup — **not provided here**.

**To build and test without any hardware:** just a host with a **C++20 compiler**
(GCC ≥ 9 / Clang ≥ 10), **CMake ≥ 3.16**, and **GoogleTest** (for the suite). The
register/bit-layout tests run anywhere.

---

## Framework

Every peripheral follows the same three layers:

| Layer | File | Responsibility |
|-------|------|----------------|
| Register block | `inc/memory_map.hpp` | `enum Register` (word offsets), field enums, and a placement `operator new` returning the peripheral's **physical base** on hardware (or a caller-supplied buffer in tests). |
| Driver class | `inc/<peri>.hpp` | Holds a reference to the register block; exposes field `set/clr/get` + helpers. |
| Implementation | `src/<peri>/<peri>.cpp` | Bit manipulation against `m_register[...]`. |

The placement-`new` trick lets the **same code** run two ways:

```cpp
GPIO gpio;                         // hardware: registers overlay MMIO at 0x3F200000
                                   // (real Pi only — see "Status" below)

std::vector<std::uint32_t> buf(BCM2837::GPIORegistersAddress::BCM2837_MAX);
GPIO gpio(buf.data());             // test/host: registers overlay a heap buffer
```

Physical bases: GPIO `0x3F200000`, Clock `0x3F101070`*, IRQ `0x3F00B200`,
I2C/BSC1 `0x3F804000`, SPI0 `0x3F204000`.
<sub>*Clock base is tracked as an open issue — see DRIVER_REVIEW §2.3.</sub>

---

## Build

Requires CMake ≥ 3.16 and a C++20 compiler. GoogleTest is only needed for the
test suite.

```bash
cmake -S . -B build
cmake --build build
```

Targets / options:

| Target / option | Default | Purpose |
|-----------------|---------|---------|
| `bcm2837_driver` (alias `bcm2837::driver`) | always | STATIC driver library (`src/*/*.cpp`). |
| `bcm2837_demo` | top-level only | Standalone demo executable (`src/main.cpp`). |
| `bcm2837_test` | top-level only | GoogleTest suite. |
| `-DBCM2837_BUILD_APP=ON/OFF` | top-level | Build the demo executable. |
| `-DBCM2837_BUILD_TESTS=ON/OFF` | top-level | Build + register the gtest suite. |

When pulled in via `add_subdirectory(...)`, only `bcm2837_driver` is built by
default (app/tests off), so a parent project just links the library:

```cmake
add_subdirectory(bcm2837)
target_link_libraries(my_app PRIVATE bcm2837_driver)   # headers come transitively
```

The public headers are C++17-consumable (the `auto`-param ctors are written as
explicit templates), so a C++17 parent can link the C++20-built library.

### Tests

```bash
cmake -S . -B build -DBCM2837_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build            # or: ./build/test/bcm2837_test
```

Tests construct each driver over a `std::vector<uint32_t>` and assert the
register **bit-layout** — no hardware required. The suite passes under both
`-O0` and `-O2`/`-O3` (the fixtures explicitly re-zero the overlay storage; see
[`docs/DRIVER_REVIEW.md`](docs/DRIVER_REVIEW.md) §2.1 for why).

### Containerised build (podman/docker)

A [`Dockerfile`](Dockerfile) builds the library, demo, and gtest suite in a
pinned Debian image and runs the suite at build time (fatal by default):

```bash
podman build -t bcm2837:latest .                 # builds + runs the 97-case suite
podman run --rm bcm2837:latest                   # demo usage
podman run --rm --entrypoint bcm2837_test bcm2837:latest   # run the suite
```

---

## Usage sketches

```cpp
// GPIO — drive GPIO17 as an output, set it high
GPIO gpio;
gpio.output(17);
gpio.GPSETn(17);

// I2C (BSC1) — read a device register via the transaction layer
I2C i2c; GPIO gpio;
Bcm2837I2cTransport bus(i2c, gpio);
bus.bus_init();                          // GPIO2/3 → ALT0, clock divider, enable
std::uint8_t whoami = 0;
bus.read_reg(/*addr=*/0x68, /*reg=*/0x00, &whoami, 1);   // write reg ptr, then read
// (the raw I2C register API — i2c.slave_address()/write_byte()/start_write() —
//  is still there if you want to drive the BSC registers directly.)

// SPI0 — mode 0, CE0, one byte
SPI spi;
spi.mode(/*cpol=*/0, /*cpha=*/0);
spi.chip_select(0);
spi.clear_fifo();
spi.begin_transfer();
spi.write_byte(0x55);
```

> Register addressing is by **BCM GPIO number** (not 40-pin header pin). I2C/SPI
> additionally require the relevant pins muxed to ALT0 (GPIO2/3 for I2C1,
> GPIO7–11 for SPI0) and the peripheral clock divided appropriately.

---

## I²C transaction layers

The raw `I2C` driver is a register model — it sets the address/DLEN/FIFO but does
not *sequence* a transfer. On top of it sits a small abstract seam,
**`I2cTransport`** (`inc/i2c_bus.hpp`), with `read` / `write` / `write_read` plus
`read_reg` / `write_reg` helpers. Device/sensor drivers depend on the seam, so
they are host-unit-testable against a fake. Three implementations ship:

| Transport | Header | Use it for |
|-----------|--------|-----------|
| `Bcm2837I2cTransport` | `i2c_bus.hpp` | **Polled** BSC1 register transfer — the bare-metal default / no-`i2c-dev` fallback. `bus_init()` muxes GPIO2/3→ALT0, sets the divider, enables BSC1; the FIFO is pumped against `S.DONE`/`ERR`/`CLKT`. |
| `I2cDevTransport` | `i2c_dev.hpp` | **Linux `/dev/i2c-N`** via `ioctl(I2C_RDWR)` — **preferred on a booted Pi**: needs only r/w on the node (group `i2c`, no `CAP_SYS_RAWIO`/`/dev/mem`) and gives a real combined repeated-START. Header-only, Linux-only. |
| `Bcm2837I2cIrqTransport` | `i2c_irq.hpp` | **Interrupt-driven** bare-metal variant — DONE/TXW/RXR ISR + watchdog + async callback. `WFI` is gated behind `-DI2C_IRQ_BAREMETAL`. Design: [`docs/i2c-irq-transport-spec.md`](docs/i2c-irq-transport-spec.md). |

```cpp
// Booted Pi, unprivileged: read a register over /dev/i2c-1
#include "i2c_dev.hpp"
I2cDevTransport bus("/dev/i2c-1");
std::uint8_t id = 0;
if (bus.read_reg(0x68, 0x00, &id, 1) == I2cResult::Ok) { /* id == chip id */ }
```

`I2cResult` is `Ok` / `BadArg` / `Timeout` / `Nack` / `ClockTimeout`. All three
transports share the seam, so swapping one for another is a one-line change, and
each is covered by host gtests (a scripted status/ISR seam, or a
`FakeI2cTransport`) — no hardware required.

---

## Real hardware (Linux MMIO)

On a booted Pi the drivers can attach to live registers through
[`inc/mmio.hpp`](inc/mmio.hpp), which `mmap`s a peripheral block and hands the
virtual pointer to the same region constructor the tests use:

```cpp
#include "mmio.hpp"

auto gpio = BCM2837::map_gpio();   // maps the GPIO block via /dev/mem (root)
gpio->output(17);
gpio->GPSETn(17);                // drive GPIO17 high
```

`map_gpio()` / `map_clock()` / `map_i2c()` / `map_spi()` use `/dev/mem` (needs
root / `CAP_SYS_RAWIO`); `map_gpiomem()` maps **GPIO only** via the unprivileged
`/dev/gpiomem` (group `gpio`). Each returns an RAII `Mapped<Driver>` that owns
the mapping and unmaps on scope exit. The demo binary does this under
`bcm2837_demo --blink`.

### Supported boards (auto-detected)

The peripheral *offsets* are identical across the BCM283x and BCM2711 families;
only the peripheral base differs. The `map_*` factories detect the running
board's base at runtime from `/proc/device-tree/soc/ranges`, so **one Linux
binary targets every supported Pi**:

| SoC | Peripheral base | Boards |
|-----|-----------------|--------|
| BCM2835   | `0x20000000` | Pi 1, Pi Zero, Pi Zero W |
| BCM2836/7 | `0x3F000000` | Pi 2, Pi 3, Pi Zero 2 W |
| BCM2711   | `0xFE000000` | Pi 4, Pi 400, CM4 |

Detection falls back to BCM2836/7 (`0x3F000000`) if the device tree is
unreadable; pass a nonzero base to `BCM2837::periph_base(...)` to force one
(e.g. `BCM2837::periph_base(static_cast<std::uintptr_t>(BCM2837::SocPeriphBase::BCM2711))`).
A *bare-metal* build instead selects the base at compile time with
`-DBCM_PERIPH_BASE=0x20000000` (see [`inc/memory_map.hpp`](inc/memory_map.hpp)).

This is safe because `BCM2837::mmio_reg` (the register cell type) is *trivially
default-constructible*: placement-new'ing a register block over live MMIO
**overlays** the registers instead of zeroing them. `IRQ` is **not** exposed
here — its AArch64 `VBAR_EL1` vector/exception model is bare-metal-only, not a
Linux userspace concept (see [`docs/aarch64-interrupt-model.md`](docs/aarch64-interrupt-model.md)).

## Status

The register/field logic is verified on the host (gtest over heap buffers) and
the same code drives real silicon via the `mmap` path above. The `IRQ`/IVT layer
is an **AArch64-correct model** (`VBAR_EL1` 16-slot vector table + a top-level IRQ
dispatcher over the legacy controller and the per-core ARM local block),
host-tested over a buffer **and** exercised by a real freestanding image (see the
next section). Driving real pins also needs the usual pin-mux (ALT0) +
clock-divider setup, not just the register writes.

### Bare-metal runtime (`freestanding/`)

A small freestanding aarch64 image makes the interrupt model real: it drops to
EL1, installs the 16-entry vector table into `VBAR_EL1`, wires the ARM generic
timer, and takes `CNTPNSIRQ` through `IRQ::dispatch()` to a handler that prints
tick counts over the UART. It is **verified booting in QEMU `raspi3b`** via a
containerised aarch64 cross toolchain — no host setup required:

```bash
podman build -t bcm2837-bm-builder -f freestanding/Dockerfile freestanding
podman run --rm -v "$PWD":/src:Z -w /src bcm2837-bm-builder freestanding/build-and-run.sh
#   running at EL1
#   VBAR_EL1 -> vector_table @ 0x0000000000080800
#   [irq] timer tick #1
#   [irq] timer tick #2  ...
```

`freestanding/` holds the boot + EL2→EL1 drop (`boot.S`), the vector table +
context save/restore (`vectors.S`), a minimal PL011 console + `printf`, and the
`kmain` demo; `interrupt.cpp` is compiled `-DINTERRUPT_BAREMETAL` so
`IRQ::install_vector_table` emits `msr VBAR_EL1`. Design + layout:
[`docs/aarch64-interrupt-model.md`](docs/aarch64-interrupt-model.md). The image
also builds as a `kernel8.img` for a real Pi 3 (`arm_64bit=1`). MMU/caches and
SMP are intentionally out of scope for this demo.

## Use in the iot Yocto image

This repo is vendored into [`naushada/iot`](https://github.com/naushada/iot)
under `modules/bcm2837` and built as the `bcm2837_driver` library. Its
gtest suite ships as the **`iot-bcm2837-selftest`** systemd oneshot, which runs the
bit-layout suite once at boot and records pass/fail in the journal (gated by the
recipe's `bcm2837-selftest` PACKAGECONFIG).

## Documentation

In-repo design and reference docs under [`docs/`](docs/):

| Doc | What it covers |
|-----|----------------|
| [`docs/DRIVER_REVIEW.md`](docs/DRIVER_REVIEW.md) | Per-peripheral design review and the open/fixed issue log. |
| [`docs/aarch64-interrupt-model.md`](docs/aarch64-interrupt-model.md) | AArch64 `VBAR_EL1` vector-table + IRQ-dispatch model (legacy controller + per-core ARM local block); what's modeled vs. the deferred freestanding runtime. |
| [`docs/i2c-irq-transport-spec.md`](docs/i2c-irq-transport-spec.md) | Spec for the interrupt-driven I²C transport (`Bcm2837I2cIrqTransport`). |
| [`docs/HARDWARE_PROPOSAL.md`](docs/HARDWARE_PROPOSAL.md) | Proposal for a minimal battery-powered IoT gateway + WiFi AP running `device-iot`. |

## License

Released under the [MIT License](LICENSE) — © 2026 Mohd Naushad Ahmed.
