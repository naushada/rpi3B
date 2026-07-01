# AArch64 (ARMv8-A) interrupt model — Raspberry Pi 3B / BCM2837

This documents the interrupt layer in `inc/interrupt.hpp` +
`src/interrupt/interrupt.cpp` (with `BCM2837::ArmLocalRegisterAddress` in
`inc/memory_map.hpp`). Like the rest of the repo it is a **host-testable model**:
placement-`new` overlays the register blocks and the vector table either on live
MMIO or on a `std::vector<uint32_t>` for gtest — no hardware required.

It replaces the previous **AArch32** vector model (8 word-sized vectors at
`0x00..0x1C`), which was wrong for the Pi 3's ARMv8-A cores. See the
[Runtime phase (deferred)](#runtime-phase-deferred) section for what is modeled
vs. what a real freestanding boot still needs.

---

## 1. The AArch64 exception model

On ARMv8-A, exceptions are taken through **`VBAR_ELn`** (we target `VBAR_EL1`,
the level a kernel runs at). `VBAR` points at a **2 KB-aligned table of 16
entries**, each entry a **`0x80`-byte slot** (`0x800` = 2 KB total). Each slot
holds up to 32 instructions — normally a single `b <handler>`. The 16 entries are
four groups of four:

| Group (by taken-from state)   | Sync | IRQ | FIQ | SError | Slot offsets |
|-------------------------------|:----:|:---:|:---:|:------:|--------------|
| Current EL, SP0               |  ✓   |  ✓  |  ✓  |   ✓    | `0x000..0x180` |
| Current EL, SPx               |  ✓   |  ✓  |  ✓  |   ✓    | `0x200..0x380` |
| Lower EL, AArch64             |  ✓   |  ✓  |  ✓  |   ✓    | `0x400..0x580` |
| Lower EL, AArch32             |  ✓   |  ✓  |  ✓  |   ✓    | `0x600..0x780` |

`IVT::Vector` enumerates these 16 in order; `IVT::byte_offset(v)` returns
`v * 0x80`. `IVT::Number::IRQ`/`FIQ` are a compatibility alias resolving to the
**Current EL, SPx** group (where a kernel taking its own interrupts lands).

Geometry constants: `IVT::kEntryStride = 0x80`, `kTableBytes = 0x800`,
`kTableAlign = 0x800`.

## 2. Interrupt routing on the Pi 3

Two controllers matter, and the old model omitted the second entirely:

1. **Legacy interrupt controller** — `InterruptRegisterAddress` at
   `0x3F00B200`. Handles the **peripheral** IRQs (I2C = 53, SPI = 54, GPIO,
   UART, …) via the `Enable/Disable/Pending_IRQs_{1,2}` banks (bank 1 = IRQ
   0–31, bank 2 = IRQ 32–63). Its outputs are OR'd into a **single combined
   line**.

2. **Per-core ARM local peripherals ("QA7")** — `ArmLocalRegisterAddress` at
   `BCM_LOCAL_BASE` (`0x40000000` on BCM2836/7; `0xFF800000` on BCM2711; absent
   on single-core BCM2835). This is where interrupts are routed **to a specific
   core**. `CORE_IRQ_SOURCE0..3` decode what is pending for each core:
   - bits 0–3: the four **generic-timer** lines (`CNTPSIRQ`, `CNTPNSIRQ`,
     `CNTHPIRQ`, `CNTVIRQ`) — the AArch64 EL1 kernel tick is `CNTPNSIRQ`;
   - bits 4–7: mailboxes; bit 8: the combined **GPU** line (the legacy
     controller above); bit 9: PMU; bit 11: local timer.
   `CORE_TIMER_IRQCNTL0..3` enable/route the timer lines per core.

The generic-timer interrupt — the natural "hello world" of an AArch64 IVT — is
**not** in the legacy controller at all; it only appears through the local block.
That is why this phase models both.

## 3. Dispatch: one IRQ vector → many sources

The architectural table has a **single** IRQ slot, so real code demuxes. `IRQ`
keeps a software dispatch table (handlers keyed by IRQ number 0–63, plus a small
local-source range at `IVT::kLocalBase`) and:

- `register_handler(num, cb)` / `handler_for(num)` — manage the table.
- `install_IRQHandler(num, cb)` — registers `cb` for `num` **and** points the
  architectural IRQ vector at the dispatch trampoline. (The `num` argument used
  to be ignored — see `DRIVER_REVIEW.md` §2.4; it is now meaningful.)
- `install_FIQHandler(num, cb)` — FIQ is single-source, so `cb` goes straight
  into the FIQ vector slot.
- `dispatch(core)` — the body the IRQ vector branches to:
  1. read `CORE_IRQ_SOURCE[core]`;
  2. if a **generic-timer** bit is set, call `handler_for(kLocalBase + line)`;
  3. else if the **GPU** bit is set, read the legacy `IRQ_Pending1/2` banks and
     call the handler for the **lowest-numbered** pending peripheral IRQ.
- `enable/disable/isEnabled(num)` — the legacy-controller bank operations
  (correct bank/shift per `DRIVER_REVIEW.md` §2.4).

`install_vector_table(table)` points `VBAR_EL1` at the (2 KB-aligned) table. It
emits the real `msr VBAR_EL1, x; isb` **only** under `-DINTERRUPT_BAREMETAL`; on
the host it is a no-op that records the pointer so tests can read it back (the
same gating idea as the WFI in the IRQ-driven I²C transport).

## 4. Host layout & tests

The `IRQTest` fixture overlays one region:
`[interrupt registers][IVT (16 vectors + dispatch table)][ARM-local registers]`.
`IRQ::region_words()` sizes it; the ARM-local block begins at
`IRQ::kLocalWordOffset`. Tests script `CORE_IRQ_SOURCE` and the legacy
`IRQ_Pending` banks over the buffer and assert the right handler fires. Cases
live in `test/interrupt_test.cpp` (vector geometry, per-IRQ dispatch,
lowest-numbered arbitration, generic-timer routing, local-timer control bit,
VBAR record, plus the §2.4 regressions).

---

## Freestanding runtime — implemented (`freestanding/`)

The model above is now driven by a real bootable image under
[`freestanding/`](../freestanding), built with the aarch64 cross toolchain and
**verified booting in QEMU `raspi3b`**. It enables the MMU and brings up **all
four cores**; each core takes its *own* generic-timer IRQ through the vector
table → `IRQ::dispatch(core)` → the registered handler, printing per-core ticks
over the UART. The same `IRQ`/`IVT` C++ types drive it unchanged; the only build
difference is `-DINTERRUPT_BAREMETAL` and the freestanding flags.

| Piece | File | What it does |
|-------|------|--------------|
| Boot + EL2→EL1 drop | `boot.S` | Each core gets its own stack (`_start − N·0x10000`) and drops EL3/EL2→EL1, setting `CNTHCTL_EL2.EL1PC{T,}EN` so EL1 can use the physical timer; **core 0** clears `.bss` and calls `kmain`, cores 1–3 branch to `ksecondary`. |
| MMU bring-up | `mmu.cpp` | Identity map via `TTBR0_EL1` (4 KB granule, 39-bit VA): RAM Normal cacheable **inner-shareable**, peripherals + ARM-local Device-nGnRnE. Core 0 builds the shared tables once (MMU off); every core calls `mmu_enable()`. |
| SMP bring-up | `smp.cpp` | Core 0 releases cores 1–3 via the Pi 3 spin-table mailboxes (`0xe0/0xe8/0xf0`); each secondary enables the MMU, sets its own `VBAR_EL1`, routes `CORE_TIMER_IRQCNTL[core]`, and lands in `ksecondary()`. A single shared `timer_isr()` services all four. |
| Vector table | `vectors.S` | The 2 KB-aligned, 16 × `0x80` `VBAR_EL1` table; context save/restore (`x0..x30`, `ELR_EL1`, `SPSR_EL1`); `el1_irq` → `irq_handler_c`, others → `bad_exception_c`. |
| Exception glue | `exceptions.cpp` | `irq_handler_c()` → `IRQ::dispatch(core_id())` (each core reads its own `CORE_IRQ_SOURCE`); `bad_exception_c()` prints `ESR/ELR` and halts. |
| Demo | `kmain.cpp` | Core 0: enable MMU; `install_vector_table(vector_table)` → `VBAR_EL1`; register `CNTPNSIRQ`; release the secondaries; start its own timer; `wfi`. |
| Console / libc | `uart.cpp`, `libc_min.cpp` | Minimal PL011 + `printf`/`mem*`, plus a **cross-core console lock** (cacheable exclusives) so the four cores' output stays legible. |
| Build | `linker.ld`, `toolchain-aarch64.cmake`, `CMakeLists.txt` | `kernel8.elf` at `0x80000` → `objcopy` → `kernel8.img`. `-mno-outline-atomics` inlines the spinlock's exclusives (no libgcc). |
| Reproducible run | `Dockerfile`, `build-and-run.sh` | Debian builder (aarch64 cross + QEMU) that builds and boots the image. |

```bash
podman build -t bcm2837-bm-builder -f freestanding/Dockerfile freestanding
podman run --rm -v "$PWD":/src:Z -w /src bcm2837-bm-builder freestanding/build-and-run.sh
# core 0 running at EL1 / MMU enabled ... / VBAR_EL1 -> vector_table @ 0x80800
# [core 0] online / [core 1] online / ... / [core 0] timer tick #1 / [core 1] timer tick #1 ...
```

### MMU + SMP

The four cores share one set of translation tables and one dispatch table, so
the interrupt model scales from one core to four with no per-core copies:

- **MMU (why it's a prerequisite for SMP).** The RAM mapping is *Normal cacheable
  inner-shareable*; that shareability attribute is what makes the cores'
  data caches **coherent**, so the shared dispatch table, the tick counters, and
  the console lock behave the same on every core. MMIO stays *Device-nGnRnE* to
  keep register accesses strongly-ordered. Core 0 writes the tables with the MMU
  off (the stores reach RAM), then each core enables translation.
- **SMP release.** QEMU `raspi3b` (like a real Pi 3) holds cores 1–3 in the
  armstub spin-table, each polling a release mailbox. Core 0 writes the image
  entry (`_start`) into `0xe0/0xe8/0xf0` and `SEV`s — cleaning the cache line to
  the point of coherency first, since its caches are on but the secondaries poll
  with the MMU off. Each secondary re-enters `boot.S`, drops to EL1, and runs
  `ksecondary()`.
- **Per-core dispatch.** Every core arms its own physical timer via
  `CORE_TIMER_IRQCNTL[core]` and sets its own `VBAR_EL1`. When a timer fires, the
  IRQ is taken on that core; `irq_handler_c()` calls `IRQ::dispatch(core_id())`,
  which reads that core's `CORE_IRQ_SOURCE[core]` (§3) and invokes the one shared
  handler — so a single `register_handler(CNTPNSIRQ, …)` on core 0 services all
  four.

On real hardware the image is a `kernel8.img` for the SD card (`arm_64bit=1`);
the EL the firmware hands over at depends on `armstub` — the boot code drops from
EL3/EL2/EL1 as found. Still intentionally minimal beyond this: no scheduler or
user tasks, and the console is PL011-only.
