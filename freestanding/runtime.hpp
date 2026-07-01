#ifndef __fs_runtime_hpp__
#define __fs_runtime_hpp__

class IRQ;

/// Point the C exception handlers at the IRQ controller kmain owns, so
/// irq_handler_c() can drive IRQ::dispatch(). Called once from kmain.
void set_active_irq(IRQ* irq);

/// Symbol exported by vectors.S — the 2 KB-aligned AArch64 vector table.
extern "C" unsigned char vector_table[];

/// C exception entry points invoked from vectors.S.
extern "C" void irq_handler_c();
extern "C" void bad_exception_c(unsigned long kind,
                                unsigned long esr,
                                unsigned long elr);

#endif /*__fs_runtime_hpp__*/
