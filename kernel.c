#include <bmk/types.h>
#include <bmk/multiboot.h>
#include <bmk/kernel.h>
#include <bmk/memalloc.h>
#include <bmk/string.h>
#include <bmk/sched.h>
#include <bmk/app.h>

#include <bmk/machine/versatilepb.h>

static void puthex(char *name, unsigned int val) {
  bmk_cons_puts(name);
  bmk_cons_puts(": ");
  bmk_cons_puthex(val);
  bmk_cons_putc('\n');
}

unsigned long bmk_membase;
unsigned long bmk_memsize;

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

#if 0
static int
parsemem(uint32_t addr, uint32_t len)
{
	struct multiboot_mmap_entry *mbm;
	unsigned long memsize;
	unsigned long ossize, osbegin, osend;
	extern char _end[], _begin[];
	uint32_t off;

	/*
	 * Look for our memory.  We assume it's just in one chunk
	 * starting at MEMSTART.
	 */
	for (off = 0; off < len; off += mbm->size + sizeof(mbm->size)) {
		mbm = (void *)(addr + off);
		if (mbm->addr == MEMSTART
		    && mbm->type == MULTIBOOT_MEMORY_AVAILABLE) {
			break;
		}
	}
	assert(off < len);

	memsize = mbm->len;
	osbegin = (unsigned long)_begin;
	osend = round_page((unsigned long)_end);
	ossize = osend - osbegin;

	bmk_membase = mbm->addr + ossize;
	bmk_memsize = memsize - ossize;

	return 0;
}
#else
/*
 * qemu loads the kernel at 0x100000 (1mb) and 
 * we assume main memory starts at address 0.
 *
 * we assume a fixed size memory (for now) of 32 mb.
 *
 * We place bmk_membase after the os kernel.
 *
 */
static int
parsemem() {
	const unsigned long memsize = 32 * 1024 * 1024; // arbitrary 32 Mb

	extern char _end[], _begin[];
	unsigned long ossize, osbegin, osend;

	osbegin = (unsigned long)_begin;
	osend = round_page((unsigned long)_end);
	ossize = osend - osbegin;

  puthex("osbegin", osbegin);
  puthex("osend", osend);

	bmk_membase = osbegin + ossize;
	bmk_memsize = memsize - bmk_membase;

  puthex("bmk_membase", bmk_membase);
  puthex("bmk_memsize", bmk_memsize);

  return 0;
}
#endif

void
bmk_main(struct multiboot_info *mbi)
{

	bmk_cons_puts("rump kernel bare metal bootstrap\n\n");
#if 0
	if ((mbi->flags & MULTIBOOT_MEMORY_INFO) == 0) {
		bmk_cons_puts("multiboot memory info not available\n");
		return;
	}
	if (parsemem(mbi->mmap_addr, mbi->mmap_length))
		return;
#else
  if (parsemem()) return;
#endif

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

#if 0
/*
 * console.  quick, cheap, dirty, etc.
 * Should eventually keep an in-memory log.  printf-debugging is currently
 * a bit, hmm, limited.
 */
 
#define CONS_WIDTH 80
#define CONS_HEIGHT 25
#define CONS_MAGENTA 0x500
static volatile uint16_t *cons_buf = (volatile uint16_t *)0xb8000;

static void
cons_putat(int c, int x, int y)
{

	cons_buf[x + y*CONS_WIDTH] = CONS_MAGENTA|c;
}

/* display a character in the next available slot */
void
bmk_cons_putc(int c)
{
	static int cons_x;
	static int cons_y;
	int x;
	int doclear = 0;

	if (c == '\n') {
		cons_x = 0;
		cons_y++;
		doclear = 1;
	} else if (c == '\r') {
		cons_x = 0;
	} else {
		cons_putat(c, cons_x++, cons_y);
	}
	if (cons_x == CONS_WIDTH) {
		cons_x = 0;
		cons_y++;
		doclear = 1;
	}
	if (cons_y == CONS_HEIGHT) {
		cons_y--;
		/* scroll screen up one line */
		for (x = 0; x < (CONS_HEIGHT-1)*CONS_WIDTH; x++)
			cons_buf[x] = cons_buf[x+CONS_WIDTH];
	}
	if (doclear) {
		for (x = 0; x < CONS_WIDTH; x++)
			cons_putat(' ', x, cons_y);
	}
}
#else
/* Example code taken from
 *
 * https://balau82.wordpress.com/2010/02/28/hello-world-for-bare-metal-arm-using-qemu/
 *
 * works with the arm versatilepb qemu model 
 */
void 
bmk_cons_putc(int c) {
  // Check TXFF flag (not needed for qemu, but should be needed for hw)
  while ((read32(VPB_UART0, UARTFR) >> 5) & 1);
  write32(VPB_UART0, UARTDR, (unsigned int) c);
}
#endif

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

/*
 * init.  currently just clears the console.
 * rest is done in bmk_main()
 */
void
bmk_init(void)
{
#if 0
	int x;

	for (x = 0; x < CONS_HEIGHT * CONS_WIDTH; x++)
		cons_putat(' ', x % CONS_WIDTH, x / CONS_WIDTH);
#endif
}
