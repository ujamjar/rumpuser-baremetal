Rump kernels for "bare metal" [![Build Status](https://travis-ci.org/rumpkernel/rumpuser-baremetal.svg?branch=master)](https://travis-ci.org/rumpkernel/rumpuser-baremetal)
=======================================

This repository contains a simple, and highly experimental, "bare
metal kernel" and a hypercall implementation which enable running [rump
kernels](http://rumpkernel.org/) directly on bare metal.  By default, the
produced image includes a TCP/IP stack, a driver for the i82540 PCI NIC
and of course system calls -- enough be able to use TCP/IP via sockets.

Testing has been done with QEMU, VirtualBox and a few laptops.

See the [wiki page](http://wiki.rumpkernel.org/Repo:-rumpuser-baremetal)
for information on building and running.

Future directions
-----------------

This repo is meant as the base experiment for running rump kernels on
various virtualization platforms such as VMware, VirtualBox and Hyper-V.
When we add support for easy running of POSIX applications, most likely
the contents of this repository will migrate to a `rumprun-X` repository
(cf. http://repo.rumpkernel.org/).


ARM port notes
--------------

_In Progess_

The initial target is the [ARM versatile-pb](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0224i/index.html) i
running under `qemu-system-arm`.

### Building and running

```
$ make IWANTARM=1
$ ./qemu-arm-vpb.sh
$ $GBD # in another shell...
```

### Status

1. Comes up in qemu (I assume the bss is already set) and switches to SYSTEM mode.
2. Sets the stack pointer.  See the `bootstack` section in locore.S (shamelessly ripped
out of the i386 code).  This gives a 64K stack.
3. Initialize the interrupt vector tables in `bmk_cpu_vector_table`.  All bar `IRQ`
are mapped to stubs.  `IRQ` calls `bmk_cpu_isr`.
4. Turns on interrupts
5. Call `bmk_init` (doesn't do anything - could logically do the interrupt init here probably).
6. Call `bmk_main`

Currently `bmk_main` calls the various init procedures (in `arch/arm/stubs.c`) and also
has some debug printing stuff and testcode related to triggering a timer interrupt.
It then stops somewhere in `isr_init`.

Note; the current code is not very well organised yet - the i386 code is just ifdef'd out.  
A bunch of stuff needs to be reorganised in the `arch` dirs.

### Interrupts

The assembly interrupt handler (`bmk_cpu_isr`) was taken from 
[here](http://www.state-machine.com/arm/Building_bare-metal_ARM_with_GNU.pdf).
I think the basic idea is to avoid use of multiple stacks (different CPU modes have
different stacks), keep FIQ mode disabled for as little time as possible, and call
a c-code interrupt request handler (`bmk_isr`).

It seems a little complex to me, but seems to work and is very well described in 
the paper.

The actual requirements for interrupt delivery to the rump kernel might require a
different scheme.

