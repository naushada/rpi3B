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

## Runtime phase (deferred)

This phase is the **model**. A real freestanding boot additionally needs (none of
which is host-testable, so it is intentionally out of scope here):

- **Asm vector stub** — a 2 KB-aligned `.vectors` section of 16 `0x80`-byte slots,
  each `b <handler>`, with a **context save/restore** macro (push/pop `x0..x30`,
  `SP`, `ELR_EL1`, `SPSR_EL1`) around the C handler that calls
  `IRQ::dispatch()`.
- **`VBAR_EL1` install** at boot via `install_vector_table(&vectors)` built with
  `-DINTERRUPT_BAREMETAL`.
- **EL2 → EL1 drop** — the Pi's GPU firmware (`armstub8.S`) starts the cores in
  **EL2**. Before installing `VBAR_EL1` the runtime must set `HCR_EL2.RW` (EL1 is
  AArch64), stage `SPSR_EL2`/`ELR_EL2`, and `eret` to EL1.
- **`_start` + linker script** placing `.vectors`, the stack, and `.bss`, and a
  cross **`aarch64-none-elf` toolchain** — none of which exists in this repo yet
  (see the README *Status* section).

Once those land, the same `IRQ`/`IVT` C++ types drive real silicon unchanged; the
only build difference is `-DINTERRUPT_BAREMETAL` and the freestanding flags.
