#include <bmk/types.h>
#include <bmk/kernel.h>
#include <bmk/memalloc.h>
#include <bmk/string.h>
#include <bmk/sched.h>
#include <bmk/app.h>

/*
static void puthex(char *name, unsigned int val) {
  bmk_cons_puts(name);
  bmk_cons_puts(": ");
  bmk_cons_puthex(val);
  bmk_cons_putc('\n');
}
*/

/*
 * we don't need freepg
 * (for the humour impaired: it was a joke, on the TODO)
 */
void *
bmk_allocpg(size_t howmany)
{
	static size_t current = 0;
	unsigned long rv;

	rv = bmk_membase + PAGE_SIZE*current;
	current += howmany;
	if (current*PAGE_SIZE > bmk_memsize)
		return NULL;

	return (void *)rv;
}

void
bmk_main(void *mbi)
{

	bmk_cons_puts("rump kernel bare metal bootstrap\n\n");

	if (bmk_parsemem(mbi)) return;

/*
  puthex("sysid", read32(VPB_SYS_REG, 0));

  write32(VPB_VIC, VIC_INT_ENABLE, 1 << VPB_INTR_TIMER01); 
  puthex("intr status", read32(VPB_VIC, VIC_RAW_STATUS));
  puthex("intr enable", read32(VPB_VIC, VIC_INT_ENABLE));

  puthex("timer1load", read32(VPB_TIMER01, Timer1Load));
  puthex("timer1value", read32(VPB_TIMER01, Timer1Value));
  puthex("timer1control", read32(VPB_TIMER01, Timer1Control));
  write32(VPB_TIMER01, Timer1Load, 0xF);
  write32(VPB_TIMER01, Timer1Control, TimerCtrlEnable | TimerCtrlIntEnable | TimerCtrlOneShot);
  puthex("timer1control", read32(VPB_TIMER01, Timer1Control));
  puthex("timer1value", read32(VPB_TIMER01, Timer1Value));
  puthex("timer1value", read32(VPB_TIMER01, Timer1Value));
*/
  bmk_cons_puts("cpu init...\n");
	bmk_cpu_init();
  bmk_cons_puts("sched init...\n");
	bmk_sched_init();
  bmk_cons_puts("isr init...\n");
	bmk_isr_init();

#ifdef BMK_APP
	/* run 'em if you got 'em */
  bmk_cons_puts("run app...\n");
	bmk_app_main();
#endif
}

/*
 * Console printing utilities
 */

/* display a string */
void
bmk_cons_puts(const char *str)
{

	for (; *str; str++)
		bmk_cons_putc(*str);
}

/* print 32 bit hex value */
void 
bmk_cons_puthex(int value) {
  int i;
  for (i=0; i<8; i++) {
    int v = (value >> 28) & 0xf;
    if (v < 10) bmk_cons_putc('0' + v);
    else bmk_cons_putc('a' + v - 10);
    value <<= 4;
  }
}

void 
bmk_cons_putint(int value) {
  static char s[11]; // ~ 10^9 
  if (0 == value) bmk_cons_putc('0');
  else {
    int pos=0;
    // convert negative to positive (except -2^31, which doesn't work)
    if (value < 0) {
      value = - value;
      bmk_cons_putc('-');
    }
    // construct the string backwards
    while (value) {
      s[pos] = '0' + (value % 10);
      value /= 10;
      pos++;;
    }
    // print string from the end
    while (pos) 
      bmk_cons_putc(s[--pos]);
  }
}
