#include <bmk/types.h>
#include <bmk/kernel.h>
#include <bmk/machine/versatilepb.h>

unsigned long bmk_membase;
unsigned long bmk_memsize;

/* Pushes characters to the serial port.
 *
 * In qemu you dont need to check the tx fifo level.
 *
 * In reality you would either need to run interrupt driven or 
 * check the fifo level and also configure serial port
 * parameters.
 */
void 
bmk_cons_putc(int c) {
  // Check TXFF flag (not needed for qemu, but should be needed for hw)
  // while ((read32(VPB_UART0, UARTFR) >> 5) & 1);
  write32(VPB_UART0, UARTDR, (unsigned int) c);
}

#define VECFN(name) \
static void name(void) { \
  bmk_cons_puts(#name " handler\n"); \
  while (1); \
}
VECFN(reset);
VECFN(undef);
VECFN(swint);
VECFN(prefetch);
VECFN(dataabort);
VECFN(reserved);
VECFN(fiq);

extern void bmk_cpu_isr(void);

/* 
 * Set up the vector table 
 */
void bmk_init(void) {
  unsigned int LDR_PC_PC = 0xE59FF000U;
  unsigned int MAGIC = 0xDEADBEEFU; 

  // primary table
  *(unsigned int volatile *)(0x00) = (LDR_PC_PC | 0x18);
  *(unsigned int volatile *)(0x04) = (LDR_PC_PC | 0x18);
  *(unsigned int volatile *)(0x08) = (LDR_PC_PC | 0x18);
  *(unsigned int volatile *)(0x0C) = (LDR_PC_PC | 0x18);
  *(unsigned int volatile *)(0x10) = (LDR_PC_PC | 0x18);
  *(unsigned int volatile *)(0x14) = MAGIC;
  *(unsigned int volatile *)(0x18) = (LDR_PC_PC | 0x18);
  // secondary table
  *(unsigned int volatile *)(0x20) = (unsigned int) reset; // reset
  *(unsigned int volatile *)(0x24) = (unsigned int) undef; // undef'd instr
  *(unsigned int volatile *)(0x28) = (unsigned int) swint; // sw int
  *(unsigned int volatile *)(0x2C) = (unsigned int) prefetch; // prefetch abort
  *(unsigned int volatile *)(0x30) = (unsigned int) dataabort; // data abort
  *(unsigned int volatile *)(0x34) = (unsigned int) reserved; // reserved
  *(unsigned int volatile *)(0x38) = (unsigned int) bmk_cpu_isr; // irq
  *(unsigned int volatile *)(0x3C) = (unsigned int) fiq; // fiq
}

/*
 * qemu loads the kernel at 0x100000 (1mb) and 
 *
 * Assume main memory starts at address 0 and is 32Mb.
 *
 * We place bmk_membase after the os kernel.
 *
 */
int bmk_parsemem(void *_p) {
	const unsigned long memsize = 32 * 1024 * 1024; // arbitrary 32 Mb

	extern char _end[], _begin[];
	unsigned long ossize, osbegin, osend;

	osbegin = (unsigned long)_begin;
	osend = round_page((unsigned long)_end);
	ossize = osend - osbegin;

	bmk_membase = osbegin + ossize;
	bmk_memsize = memsize - bmk_membase;

  return 0;
}

void bmk_cpu_init(void) { 
}

void bmk_isr(int which)
{
  bmk_cons_puts("\n\n******** interrupt occured\n\n");
  // for now, just disable the timer interrupt
  write32(VPB_TIMER01, Timer1IntClr, 1);
  write32(VPB_VIC, VIC_INT_ENABLE_CLEAR, 1 << VPB_INTR_TIMER01);
}

