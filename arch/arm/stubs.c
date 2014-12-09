#define STUB(name) int name(void); int name(void) {for(;;);}

extern void bmk_cons_puts(char*);
#define VECFN(name) \
void name(void); \
void name(void) { \
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

void bmk_cpu_vector_table(void);
void bmk_cpu_vector_table(void) {
  // Set up the vector table(s)
  // see http://www.state-machine.com/arm/Building_bare-metal_ARM_with_GNU.pdf
  
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

int bmk_cpu_init(void);
int bmk_cpu_init(void) { return 0; }

STUB(bmk_cpu_intr_init);
STUB(bmk_cpu_intr_ack);
STUB(bmk_cpu_clock_now);
STUB(bmk_cpu_nanohlt);
STUB(bmk_cpu_sched_create);
STUB(bmk_cpu_sched_bouncer);
STUB(bmk_cpu_sched_switch);
