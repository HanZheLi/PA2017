#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  rtl_push(&cpu.EFLAGS);
  rtl_li(&t0,cpu.cs);
  rtl_push(&t0);
  rtl_push(&ret_addr);
  uint32_t low=vaddr_read(cpu.idtr.base+NO*8,4)&0xffff;
  uint32_t high=vaddr_read(cpu.idtr.base+4+NO*8,4)&0xffff0000;
  uint32_t offset=low|high;
  decoding.jmp_eip=offset;
  decoding.is_jmp=1;
}

void dev_raise_intr() {
}
