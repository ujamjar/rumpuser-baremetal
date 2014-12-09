#ifndef _VERSATILE_PB_H_
#define _VERSATILE_PB_H_

// versatilepb board
// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0224i/index.html

// Memory mapped registers
enum {
  VPB_SYS_REG     = 0x10000000, /* System registers.  */
  VPB_PCI_CFG     = 0x10001000, /* PCI controller config registers.  */
  VPB_SERIAL      = 0x10002000, /* Serial bus interface.  */
  VPB_INTR2       = 0x10003000, /* Secondary interrupt controller.  */
  VPB_AACI        = 0x10004000, /* AACI (audio). pl041 */
  VPB_MMCIO       = 0x10005000, /* MMCI0. pl181 */
  VPB_KMIO        = 0x10006000, /* KMI0 (keyboard). pl050 */
  VPB_KMI1        = 0x10007000, /* KMI1 (mouse).  pl050 */
  VPB_LCD         = 0x10008000, /* Character LCD Interface.  */
  VPB_UART3       = 0x10009000, /* UART3. pl011 */
  VPB_SMART_CARD1 = 0x1000a000, /* Smart card 1.  */
  VPB_MMCI1       = 0x1000b000, /* MMCI1. pl181 */
  VPB_ETHERNET    = 0x10010000, /* Ethernet. smc91c111 (or rtl8139 on pci bus) */
  VPB_USB         = 0x10020000, /* USB.  */
  VPB_SSMC        = 0x10100000, /* SSMC.  */
  VPB_MPMC        = 0x10110000, /* MPMC.  */
  VPB_CLCD        = 0x10120000, /* CLCD Controller.  */
  VPB_DMA         = 0x10130000, /* DMA Controller. pl080 */
  VPB_VIC         = 0x10140000, /* Vectored interrupt controller. pl190 */
  VPB_AHB_MON     = 0x101d0000, /* AHB Monitor Interface.  */
  VPB_SYS_CTL     = 0x101e0000, /* System Controller.  */
  VPB_WATCH_DOG   = 0x101e1000, /* Watchdog Interface.  */
  VPB_TIMER01     = 0x101e2000, /* Timer 0/1. sp804 */
  VPB_TIMER23     = 0x101e3000, /* Timer 2/3. sp804 */
  VPB_GPIO0       = 0x101e4000, /* GPIO port 0. pl061 */
  VPB_GPIO1       = 0x101e5000, /* GPIO port 1. pl061 */
  VPB_GPIO2       = 0x101e6000, /* GPIO port 2. pl061 */
  VPB_GPIO3       = 0x101e7000, /* GPIO port 3. pl061 */
  VPB_RTC         = 0x101e8000, /* RTC. pl031 */
  VPB_SMART_CARD0 = 0x101f0000, /* Smart card 0.  */
  VPB_UART0       = 0x101f1000, /* UART0. pl011 */
  VPB_UART1       = 0x101f2000, /* UART1. pl011 */
  VPB_UART2       = 0x101f3000, /* UART2. pl011 */
  VPB_SSPI        = 0x101f4000, /* SSPI.  */
  VPB_NOR_FLASH   = 0x34000000 /* NOR Flash */
};

// VIC (primary interrupt controller) interrupt assignments
enum {
  VPB_INTR_WATCHDOG = 0,
  VPB_INTR_SW,
  VPB_INTR_COMMS_RX,
  VPB_INTR_COMMS_TX,
  VPB_INTR_TIMER01,
  VPB_INTR_TIMER23,
  VPB_INTR_GPIO0,
  VPB_INTR_GPIO1,
  VPB_INTR_GPIO2,
  VPB_INTR_GPIO3,
  VPB_INTR_RTC,
  VPB_INTR_SSP,
  VPB_INTR_UART0,
  VPB_INTR_UART1,
  VPB_INTR_UART2,
  VPB_INTR_SCIO,
  VPB_INTR_CLCDC,
  VPB_INTR_DMA,
  VPB_INTR_PWRFAIL,
  VPB_INTR_MBX,
  VPB_INTR_GND,
  VPB_INTR_21,
  VPB_INTR_22,
  VPB_INTR_23,
  VPB_INTR_24,
  VPB_INTR_25,
  VPB_INTR_26,
  VPB_INTR_27,
  VPB_INTR_28,
  VPB_INTR_29,
  VPB_INTR_30,
  VPB_INTR_31_SND
};

// SIC (fpga secondary interrupt controller)
enum {
  VPB_INTR2_SOFTINTX=0,
  VPB_INTR2_MMCI0B,
  VPB_INTR2_MMCI1B,
  VPB_INTR2_KMI0,
  VPB_INTR2_KMI1,
  VPB_INTR2_SCI1,
  VPB_INTR2_UART3,
  VPB_INTR2_CHARLCD,
  VPB_INTR2_TOUCH_PEN,
  VPB_INTR2_TOUCH_KEYPAD,
  VPB_INTR2_DISKONCHIP=21, // these may also map to the vic
  VPB_INTR2_MMCI0A,
  VPB_INTR2_MMCI1A,
  VPB_INTR2_AACI,
  VPB_INTR2_ETHERNET,
  VPB_INTR2_USB,
  VPB_INTR2_PCI0,
  VPB_INTR2_PCI1,
  VPB_INTR2_PCI2,
  VPB_INTR2_PCI3
};

// VIC registers
enum {
  VIC_IRQ_STATUS                = 0x00/4,
  VIC_FIQ_STATUS                = 0x04/4,
  VIC_RAW_STATUS                = 0x08/4,
  VIC_INT_SELECT                = 0x0c/4,    // 1 = FIQ/4, 0 = IRQ 
  VIC_INT_ENABLE                = 0x10/4,    // 1 = enable/4, 0 = disable 
  VIC_INT_ENABLE_CLEAR          = 0x14/4,
  VIC_INT_SOFT                  = 0x18/4,
  VIC_INT_SOFT_CLEAR            = 0x1c/4,
  VIC_PROTECT                   = 0x20/4,
  VIC_PL190_VECT_ADDR           = 0x30/4,    // PL190 only 
  VIC_PL190_DEF_VECT_ADDR       = 0x34/4,    // PL190 only 
  VIC_VECT_ADDR0                = 0x100/4,   // 0 to 15 (0..31 PL192) 
  VIC_VECT_CNTL0                = 0x200/4,   // 0 to 15 (0..31 PL192) 
  VIC_ITCR                      = 0x300/4,   // VIC test control register 
  VIC_PL192_VECT_ADDR           = 0xF00/4,
  VIC_PERIPH_ID                 = 0xFE0/4,   // 0..3
  VIC_CELL_ID                   = 0xFF0/4,   // 0..3
};

// SIC registers
enum {
  SIC_STATUS      = 0x00/4,   // [Read] Status of interrupt (after mask)
  SIC_RAWSTAT     = 0x04/4,   // [Read] Status of interrupt (before mask)
  SIC_ENABLE      = 0x08/4,   // [Read] Interrupt mask
                              // [Write] Set bits HIGH to enable the corresponding interrupt signals
  SIC_ENCLR       = 0x0C/4,   // [Write] Set bits HIGH to mask the corresponding interrupt signals
  SIC_SOFTINTSET  = 0x10/4,   // [Read/write] Set software interrupt
  SIC_SOFTINTCLR  = 0x14/4,   // [Write] Clear software interrupt
  SIC_PICENABLE   = 0x20/4,   // [Read] Read status of pass-through mask 
                              //        (allows interrupt to pass directly 
                              //         to the primary interrupt controller)
                              // [Write] Set bits HIGH to set the corresponding interrupt 
                              // pass-through mask bits
  SIC_PICENCLR    = 0x24/4,   // [Write] Set bits HIGH to clear the corresponding 
                              //         interrupt pass-through mask bits
};

// pl011 UART
enum {
  UARTDR        = 0x000/4, // [RW] [0x---] [12/8] Data Register, UARTDR
  UARTRSR       = 0x004/4, // [UARTECR] [RW] [0x0] 4/0 Receive Status Register / Error 
                           // Clear Register, UARTRSR/UARTECR
  UARTFR        = 0x018/4, // [RO] [0b-10010---] [9] Flag Register/4, UARTFR
  UARTILPR      = 0x020/4, // [RW] [0x00] [8] IrDA Low-Power Counter Register, UARTILPR
  UARTIBRD      = 0x024/4, // [RW] [0x0000] [16] Integer Baud Rate Register, UARTIBRD
  UARTFBRD      = 0x028/4, // [RW] [0x00] [6] Fractional Baud Rate Register, UARTFBRD
  UARTLCR_H     = 0x02C/4, // [RW] [0x00] [8] Line Control Register, UARTLCR_H
  UARTCR        = 0x030/4, // [RW] [0x0300] [16] Control Register, UARTCR
  UARTIFLS      = 0x034/4, // [RW] [0x12] [6] Interrupt FIFO Level Select Register, UARTIFLS
  UARTIMSC      = 0x038/4, // [RW] [0x000] [11] Interrupt Mask Set/Clear Register, UARTIMSC
  UARTRIS       = 0x03C/4, // [RO] [0x00-] [11] Raw Interrupt Status Register, UARTRIS
  UARTMIS       = 0x040/4, // [RO] [0x00-] [11] Masked Interrupt Status Register, UARTMIS
  UARTICR       = 0x044/4, // [WO] [-] [11] Interrupt Clear Register, UARTICR
  UARTDMACR     = 0x048/4, // [RW] [0x00] [3] DMA Control Register, UARTDMACR
  UARTPeriphID0 = 0xFE0/4, // [RO] [0x11] [8] UARTPeriphID0 Register
  UARTPeriphID1 = 0xFE4/4, // [RO] [0x10] [8] UARTPeriphID1 Register
  UARTPeriphID2 = 0xFE8/4, // [RO] [0x_4[1]] [8] UARTPeriphID2 Register
  UARTPeriphID3 = 0xFEC/4, // [RO] [0x00] [8] UARTPeriphID3 Register
  UARTPCellID0  = 0xFF0/4, // [RO] [0x0D] [8] UARTPCellID0 Register
  UARTPCellID1  = 0xFF4/4, // [RO] [0xF0] [8] UARTPCellID1 Register
  UARTPCellID2  = 0xFF8/4, // [RO] [0x05] [8] UARTPCellID2 Register
  UARTPCellID3  = 0xFFC/4, // [RO] [0xB1] [8] UARTPCellID3 Register
};

// sp804 timer
enum {
  Timer1Load     = 0x00/4, // Read/write 32 0x00000000 See Load Register, TimerXLoad
  Timer1Value    = 0x04/4, // Read 32 0xFFFFFFFF See Current Value Register, TimerXValue
  Timer1Control  = 0x08/4, // Read/write 8 0x20 See Control Register, TimerXControl
  Timer1IntClr   = 0x0C/4, // Write - - See Interrupt Clear Register. TimerXIntClr
  Timer1RIS	     = 0x10/4, // Read 1 0x0 See Raw Interrupt Status Register, TimerXRIS
  Timer1MIS	     = 0x14/4, // Read 1 0x0 See Masked Interrupt Status Register, TimerXMIS
  Timer1BGLoad   = 0x18/4, // Read/write 32 0x00000000 See Background Load Register, TimerXBGLoad
  Timer2Load     = 0x20/4, // Read/write 32 0x00000000 See Load Register, TimerXLoad
  Timer2Value    = 0x24/4, // Read 32 0xFFFFFFFF See Current Value Register, TimerXValue
  Timer2Control  = 0x28/4, // Read/write 8 0x20 See Control Register, TimerXControl
  Timer2IntClr   = 0x2C/4, // Write - - See Interrupt Clear Register. TimerXIntClr
  Timer2RIS	     = 0x30/4, // Read 1 0x0 See Raw Interrupt Status Register, TimerXRIS
  Timer2MIS	     = 0x34/4, // Read 1 0x0 See Masked Interrupt Status Register, TimerXMIS
  Timer2BGLoad   = 0x38/4, // Read/write 32 0x00000000 See Background Load Register, TimerXBGLoad
  TimerITCR	     = 0xF00/4, // Read/write 1 0x0 See Integration Test Control Register, TimerITCR
  TimerITOP      = 0xF04/4, // Write 2 0x0 See Integration Test Control Register, TimerITCR
  TimerPeriphID0 = 0xFE0/4, // Read-only 8 0x04 See Timer Peripheral ID0 Register, TimerPeriphID0
  TimerPeriphID1 = 0xFE4/4, // Read-only 8 0x18 See Timer Peripheral ID1 Register, TimerPeriphID1
  TimerPeriphID2 = 0xFE8/4, // Read-only 8 0x14 See Timer Peripheral ID2 Register, TimerPeriphID2
  TimerPeriphID3 = 0xFEC/4, // Read-only 8 0x00 See Timer Peripheral ID3 Register, TimerPeriphID3
  TimerPCellID0  = 0xFF0/4, // Read-only 8 0x0D See PrimeCell ID0 Register, TimerPCellID0
  TimerPCellID1  = 0xFF4/4, // Read-only 8 0xF0 See PrimeCell ID1 Register, TimerPCellID1
  TimerPCellID2  = 0xFF8/4, // Read-only 8 0x05 See PrimeCell ID2 Register, TimerPCellID2
  TimerPCellID3  = 0xFFC/4, // Read-only 8 0xB1 See PrimeCell ID3 Register, TimerPCellID3
};

// Timer control register flags
enum {
  TimerCtrlWrap       = 0,
  TimerCtrlOneShot    = 1,

  TimerCtrlSize16     = 0,
  TimerCtrlSize32     = 1 << 1,
  
  TimerCtrlPrescale0  = 0<<2,
  TimerCtrlPrescale4  = 1<<2,
  TimerCtrlPrescale8  = 2<<2,
  TimerCtrlIntEnable  = 1<<5,
  
  TimerCtrlFree       = 0,
  TimerCtrlPeriodic   = 1<<6,

  TimerCtrlEnable     = 1<<7
};

static inline unsigned int read32(unsigned int base, int offset) {
  volatile unsigned int *p = (volatile unsigned int *) base;
  return p[offset];
}

static inline void write32(unsigned int base, int offset, unsigned int value) {
  volatile unsigned int *p = (volatile unsigned int *) base;
  p[offset] = value;
}

#endif//_VERSATILE_PB_H_

