#ifndef REGCC2430_HEADER
#define REGCC2430_HEADER

/* RADIO */

//#define -         0xDF00 // Reserved
//#define -         0xDF01
#define MDMCTRL0H 0xDF02 // Modem Control 0, high
#define MDMCTRL0L 0xDF03 // Modem Control 0, low
#define MDMCTRL1H 0xDF04 // Modem Control 1, high
#define MDMCTRL1L 0xDF05 // Modem Control 1, low
#define RSSIH     0xDF06 // RSSI and CCA Status and Control, high
#define RSSIL     0xDF07 // RSSI and CCA Status and Control, low
#define SYNCWORDH 0xDF08 // Synchronisation Word Control, high
#define SYNCWORDL 0xDF09 // Synchronisation Word Control, low
#define TXCTRLH   0xDF0A // Transmit Control, high
#define TXCTRLL   0xDF0B // Transmit Control, low
#define RXCTRL0H  0xDF0C // Receive Control 0, high
#define RXCTRL0L  0xDF0D // Receive Control 0, low
#define RXCTRL1H  0xDF0E // Receive Control 1, high
#define RXCTRL1L  0xDF0F // Receive Control 1, low
#define FSCTRLH   0xDF10 // Frequency Synthesizer Control and Status, high
#define FSCTRLL   0xDF11 // Frequency Synthesizer Control and Status, low
#define CSPX      0xDF12 // CSP X Data
#define CSPY      0xDF13 // CSP Y Data
#define CSPZ      0xDF14 // CSP Z Data
#define CSPCTRL   0xDF15 // CSP Control
#define CSPT      0xDF16 // CSP T Data
#define RFPWR     0xDF17 // RF Power Control
#define FSMTCH    0xDF20 // Finite State Machine Time Constants, high
#define FSMTCL    0xDF21 // Finite State Machine Time Constants, low
#define MANANDH   0xDF22 // Manual AND Override, high
#define MANANDL   0xDF23 // Manual AND Override, low
#define MANORH    0xDF24 // Manual OR Override, high
#define MANORL    0xDF25 // Manual OR Override, low
#define AGCCTRLH  0xDF26 // AGC Control, high
#define AGCCTRLL  0xDF27 // AGC Control, low
//#define -         0xDF28 // Reserved
//#define -         0xDF38 //
#define FSMSTATE   0xDF39 // Finite State Machine State Status
#define ADCTSTH    0xDF3A // ADC Test, high
#define ADCTSTL    0xDF3B // ADC Test, low
#define DACTSTH    0xDF3C // DAC Test, high
#define DACTSTL    0xDF3D // DAC Test, low
//#define -          0xDF3E // Reserved
//#define -          0xDF3F // Reserved
//#define -          0xDF40 // Reserved
//#define -          0xDF41 // Reserved
#define IEEE_ADDR0 0xDF43 // IEEE Address 0 (LSB)
#define IEEE_ADDR1 0xDF44 // IEEE Address 1
#define IEEE_ADDR2 0xDF45 // IEEE Address 2
#define IEEE_ADDR3 0xDF46 // IEEE Address 3
#define IEEE_ADDR4 0xDF47 // IEEE Address 4
#define IEEE_ADDR5 0xDF48 // IEEE Address 5
#define IEEE_ADDR6 0xDF49 // IEEE Address 6
#define IEEE_ADDR7 0xDF4A // IEEE Address 7 (MSB)
#define PANIDH     0xDF4B // PAN Identifier, high
#define PANIDL     0xDF4C // PAN Identifier, low
#define SHORTADDRH 0xDF4D // Short Address, high
#define SHORTADDRL 0xDF4E // Short Address, low
#define IOCFG0     0xDF4F // I/O Configuration 0
#define IOCFG1     0xDF50 // I/O Configuration 1
#define IOCFG2     0xDF51 // I/O Configuration 2
#define IOCFG3     0xDF52 // I/O Configuration 3
#define RXFIFOCNT  0xDF53 // RX FIFO Count
#define FSMTC1     0xDF54 // Finite State Machine Control
//#define -          0xDF55 // Reserved
//#define -          0xDF5F //
#define CHVER      0xDF60 // Chip Version
#define CHIPID     0xDF61 // Chip Identification
#define RFSTATUS   0xDF62 // RF Status
#define XOSC32M    0xDF63 // 32 MHz Crystal Oscillator Control
#define IRQSRC     0xDF64 // RF Interrupt Source

#endif

