#define STUB(name) int name(void); int name(void) { return 0; }

STUB(bmk_cpu_intr_init);
STUB(bmk_cpu_intr_ack);
STUB(bmk_cpu_clock_now);
STUB(bmk_cpu_nanohlt);
STUB(bmk_cpu_sched_create);
STUB(bmk_cpu_sched_bouncer);
STUB(bmk_cpu_sched_switch);
