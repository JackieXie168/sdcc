//
// Register Declarations for Microchip 16F737 Processor
//
//
// This header file was automatically generated by:
//
//	inc2h.pl V1.6
//
//	Copyright (c) 2002, Kevin L. Pauba, All Rights Reserved
//
//	SDCC is licensed under the GNU Public license (GPL) v2.  Note that
//	this license covers the code to the compiler and other executables,
//	but explicitly does not cover any code or objects generated by sdcc.
//	We have not yet decided on a license for the run time libraries, but
//	it will not put any requirements on code linked against it. See:
// 
//	http://www.gnu.org/copyleft/gpl/html
//
//	See http://sdcc.sourceforge.net/ for the latest information on sdcc.
//
// 
#ifndef P16F737_H
#define P16F737_H

//
// Register addresses.
//
#define INDF_ADDR	0x0000
#define TMR0_ADDR	0x0001
#define PCL_ADDR	0x0002
#define STATUS_ADDR	0x0003
#define FSR_ADDR	0x0004
#define PORTA_ADDR	0x0005
#define PORTB_ADDR	0x0006
#define PORTC_ADDR	0x0007
#define PORTD_ADDR	0x0008
#define PORTE_ADDR	0x0009
#define PCLATH_ADDR	0x000A
#define INTCON_ADDR	0x000B
#define PIR1_ADDR	0x000C
#define PIR2_ADDR	0x000D
#define TMR1L_ADDR	0x000E
#define TMR1H_ADDR	0x000F
#define T1CON_ADDR	0x0010
#define TMR2_ADDR	0x0011
#define T2CON_ADDR	0x0012
#define SSPBUF_ADDR	0x0013
#define SSPCON_ADDR	0x0014
#define CCPR1L_ADDR	0x0015
#define CCPR1H_ADDR	0x0016
#define CCP1CON_ADDR	0x0017
#define RCSTA_ADDR	0x0018
#define TXREG_ADDR	0x0019
#define RCREG_ADDR	0x001A
#define CCPR2L_ADDR	0x001B
#define CCPR2H_ADDR	0x001C
#define CCP2CON_ADDR	0x001D
#define ADRESH_ADDR	0x001E
#define ADCON0_ADDR	0x001F
#define OPTION_REG_ADDR	0x0081
#define TRISA_ADDR	0x0085
#define TRISB_ADDR	0x0086
#define TRISC_ADDR	0x0087
#define TRISD_ADDR	0x0088
#define TRISE_ADDR	0x0089
#define PIE1_ADDR	0x008C
#define PIE2_ADDR	0x008D
#define PCON_ADDR	0x008E
#define OSCCON_ADDR	0x008F
#define OSCTUNE_ADDR	0x0090
#define SSPCON2_ADDR	0x0091
#define PR2_ADDR	0x0092
#define SSPADD_ADDR	0x0093
#define SSPSTAT_ADDR	0x0094
#define CCPR3L_ADDR	0x0095
#define CCPR3H_ADDR	0x0096
#define CCP3CON_ADDR	0x0097
#define TXSTA_ADDR	0x0098
#define SPBRG_ADDR	0x0099
#define ADCON2_ADDR	0x009B
#define CMCON_ADDR	0x009C
#define CVRCON_ADDR	0x009D
#define ADRESL_ADDR	0x009E
#define ADCON1_ADDR	0x009F
#define WDTCON_ADDR	0x0105
#define LVDCON_ADDR	0x0109
#define PMDATA_ADDR	0x010C
#define PMADR_ADDR	0x010D
#define PMDATH_ADDR	0x010E
#define PMADRH_ADDR	0x010F
#define PMCON1_ADDR	0x018C

//
// Memory organization.
//



//         LIST
// P16F737.INC  Standard Header File, Version 1.00    Microchip Technology, Inc.
//         NOLIST

// This header file defines configurations, registers, and other useful bits of
// information for the PIC16F737 microcontroller.  These names are taken to match 
// the data sheets as closely as possible.  

// Note that the processor must be selected before this file is 
// included.  The processor may be selected the following ways:

//       1. Command line switch:
//               C:\ MPASM MYFILE.ASM /PIC16F737
//       2. LIST directive in the source file
//               LIST   P=PIC16F737
//       3. Processor Type entry in the MPASM full-screen interface

//==========================================================================
//
//       Revision History
//
//==========================================================================

//Rev:   Date:    Reason:
//1.00   05/05/03 Initial Release
//1.01	10/21/03 Made changes to Program Memory register names. 
//1.02	04/07/04 Added INT0IE & INT0IF  bit names.

//==========================================================================
//
//       Verify Processor
//
//==========================================================================

//        IFNDEF __16F737
//            MESSG "Processor-header file mismatch.  Verify selected processor."
//         ENDIF

//==========================================================================
//
//       Register Definitions
//
//==========================================================================

#define W                    0x0000
#define F                    0x0001

//----- Register Files------------------------------------------------------

extern __data __at (INDF_ADDR) volatile char      INDF;
extern __sfr  __at (TMR0_ADDR)                    TMR0;
extern __data __at (PCL_ADDR) volatile char       PCL;
extern __sfr  __at (STATUS_ADDR)                  STATUS;
extern __sfr  __at (FSR_ADDR)                     FSR;
extern __sfr  __at (PORTA_ADDR)                   PORTA;
extern __sfr  __at (PORTB_ADDR)                   PORTB;
extern __sfr  __at (PORTC_ADDR)                   PORTC;
extern __sfr  __at (PORTD_ADDR)                   PORTD;
extern __sfr  __at (PORTE_ADDR)                   PORTE;
extern __sfr  __at (PCLATH_ADDR)                  PCLATH;
extern __sfr  __at (INTCON_ADDR)                  INTCON;
extern __sfr  __at (PIR1_ADDR)                    PIR1;
extern __sfr  __at (PIR2_ADDR)                    PIR2;
extern __sfr  __at (TMR1L_ADDR)                   TMR1L;
extern __sfr  __at (TMR1H_ADDR)                   TMR1H;
extern __sfr  __at (T1CON_ADDR)                   T1CON;
extern __sfr  __at (TMR2_ADDR)                    TMR2;
extern __sfr  __at (T2CON_ADDR)                   T2CON;
extern __sfr  __at (SSPBUF_ADDR)                  SSPBUF;
extern __sfr  __at (SSPCON_ADDR)                  SSPCON;
extern __sfr  __at (CCPR1L_ADDR)                  CCPR1L;
extern __sfr  __at (CCPR1H_ADDR)                  CCPR1H;
extern __sfr  __at (CCP1CON_ADDR)                 CCP1CON;
extern __sfr  __at (RCSTA_ADDR)                   RCSTA;
extern __sfr  __at (TXREG_ADDR)                   TXREG;
extern __sfr  __at (RCREG_ADDR)                   RCREG;
extern __sfr  __at (CCPR2L_ADDR)                  CCPR2L;
extern __sfr  __at (CCPR2H_ADDR)                  CCPR2H;
extern __sfr  __at (CCP2CON_ADDR)                 CCP2CON;
extern __sfr  __at (ADRESH_ADDR)                  ADRESH;
extern __sfr  __at (ADCON0_ADDR)                  ADCON0;

extern __sfr  __at (OPTION_REG_ADDR)              OPTION_REG;
extern __sfr  __at (TRISA_ADDR)                   TRISA;
extern __sfr  __at (TRISB_ADDR)                   TRISB;
extern __sfr  __at (TRISC_ADDR)                   TRISC;
extern __sfr  __at (TRISD_ADDR)                   TRISD;
extern __sfr  __at (TRISE_ADDR)                   TRISE;
extern __sfr  __at (PIE1_ADDR)                    PIE1;
extern __sfr  __at (PIE2_ADDR)                    PIE2;
extern __sfr  __at (PCON_ADDR)                    PCON;
extern __sfr  __at (OSCCON_ADDR)                  OSCCON;
extern __sfr  __at (OSCTUNE_ADDR)                 OSCTUNE;
extern __sfr  __at (SSPCON2_ADDR)                 SSPCON2;
extern __sfr  __at (PR2_ADDR)                     PR2;
extern __sfr  __at (SSPADD_ADDR)                  SSPADD;
extern __sfr  __at (SSPSTAT_ADDR)                 SSPSTAT;
extern __sfr  __at (CCPR3L_ADDR)                  CCPR3L;
extern __sfr  __at (CCPR3H_ADDR)                  CCPR3H;
extern __sfr  __at (CCP3CON_ADDR)                 CCP3CON;
extern __sfr  __at (TXSTA_ADDR)                   TXSTA;
extern __sfr  __at (SPBRG_ADDR)                   SPBRG;
extern __sfr  __at (ADCON2_ADDR)                  ADCON2;
extern __sfr  __at (CMCON_ADDR)                   CMCON;
extern __sfr  __at (CVRCON_ADDR)                  CVRCON;
extern __sfr  __at (ADRESL_ADDR)                  ADRESL;
extern __sfr  __at (ADCON1_ADDR)                  ADCON1;

extern __sfr  __at (WDTCON_ADDR)                  WDTCON;
extern __sfr  __at (LVDCON_ADDR)                  LVDCON;
extern __sfr  __at (PMDATA_ADDR)                  PMDATA;
extern __sfr  __at (PMADR_ADDR)                   PMADR;
extern __sfr  __at (PMDATH_ADDR)                  PMDATH;
extern __sfr  __at (PMADRH_ADDR)                  PMADRH;

extern __sfr  __at (PMCON1_ADDR)                  PMCON1;

//----- STATUS Bits --------------------------------------------------------


//----- INTCON Bits --------------------------------------------------------


//----- PIR1 Bits ----------------------------------------------------------


//----- PIR2 Bits ----------------------------------------------------------


//----- T1CON Bits ---------------------------------------------------------


//----- T2CON Bits ---------------------------------------------------------


//----- SSPCON Bits --------------------------------------------------------


//----- CCP1CON Bits -------------------------------------------------------


//----- RCSTA Bits ---------------------------------------------------------


//----- CCP2CON Bits -------------------------------------------------------


//----- ADCON0 Bits --------------------------------------------------------


//----- OPTION Bits -----------------------------------------------------


//----- TRISE Bits ---------------------------------------------------------


//----- PIE1 Bits ----------------------------------------------------------


//----- PIE2 Bits ----------------------------------------------------------


//----- PCON Bits ----------------------------------------------------------


//----- OSCCON Bits -------------------------------------------------------

//----- OSCTUNE Bits -------------------------------------------------------

//----- SSPCON2 Bits --------------------------------------------------------


//----- SSPSTAT Bits -------------------------------------------------------


//----- CCP3CON Bits -------------------------------------------------------


//----- TXSTA Bits ---------------------------------------------------------


//----- ADCON2 Bits ---------------------------------------------------------


//----- CMCON Bits ---------------------------------------------------------


//----- CVRCON Bits --------------------------------------------------------


//----- ADCON1 Bits --------------------------------------------------------


//----- WDTCON Bits --------------------------------------------------------


//----- LVDCON Bits --------------------------------------------------------


//----- PMCON1 Bits --------------------------------------------------------



//==========================================================================
//
//       RAM Definition
//
//==========================================================================

//         __MAXRAM H'1FF'
//         __BADRAM H'08'
//         __BADRAM H'88', H'9A'
//         __BADRAM H'107'-H'108'
//         __BADRAM H'185', H'187'-H'189', H'18D'-H'18F'

//==========================================================================
//
//       Configuration Bits
//
//==========================================================================

#define _CONFIG1             0x2007
#define _CONFIG2             0x2008

//Configuration Byte 1 Options
#define _CP_ALL              0x1FFF
#define _CP_OFF              0x3FFF
#define _CCP2_RC1            0x3FFF
#define _CCP2_RB3            0x2FFF
#define _DEBUG_OFF           0x3FFF
#define _DEBUG_ON            0x37FF
#define _VBOR_2_0            0x3FFF
#define _VBOR_2_7            0x3F7F
#define _VBOR_4_2            0x3EFF
#define _VBOR_4_5            0x3E7F
#define _BOREN_1             0x3FFF	//MUST BE CONFIGURED IN CONJUCTION W/ BORSEN (CONFIG2)
#define _BOREN_0             0x3FBF	//MUST BE CONFIGURED IN CONJUCTION W/ BORSEN (CONFIG2)
#define _MCLR_ON             0x3FFF
#define _MCLR_OFF            0x3FDF
#define _PWRTE_OFF           0x3FFF
#define _PWRTE_ON            0x3FF7
#define _WDT_ON              0x3FFF
#define _WDT_OFF             0x3FFB
#define _EXTRC_CLKOUT        0x3FFF
#define _EXTRC_IO            0x3FFE
#define _INTRC_CLKOUT        0x3FFD
#define _INTRC_IO            0x3FFC
#define _EXTCLK              0x3FEF
#define _HS_OSC              0x3FEE
#define _XT_OSC              0x3FED
#define _LP_OSC              0x3FEC

//Configuration Byte 2 Options
#define _BORSEN_1            0x3FFF	//MUST BE CONFIGURED IN CONJUCTION W/ BOREN (CONFIG1)
#define _BORSEN_0            0x3FBF	//MUST BE CONFIGURED IN CONJUCTION W/ BOREN (CONFIG1)
#define _IESO_ON             0x3FFF
#define _IESO_OFF            0x3FFD
#define _FCMEN_ON            0x3FFF
#define _FCMEN_OFF           0x3FFE


//**** Brown-out Reset configurations **** (Refer to the 16F7x7 Data Sheet for more details)
//BOREN_1 & BORSEN_1	=	BOR enabled and always on
//BOREN_1 & BORSEN_0 = 	BOR enabled during operation and disabled during sleep by hardware
//BOREN_0 & BORSEN_1 = 	BOR controlled by software bit SBOREN (PCON,2)
//BOREN_0 & BORSEN_0 =   BOR disabled


// To use the Configuration Bits, place the following lines in your source code
//  in the following format, and change the configuration value to the desired 
//  setting (such as CP_OFF to CP_ALL).  These are currently commented out here
//  and each __CONFIG line should have the preceding semicolon removed when
//  pasted into your source code.

//Program Configuration Register 1
//		__CONFIG    _CONFIG1, _CP_OFF & _CCP2_RC1 & _DEBUG_OFF & _VBOR_2_0 & BOREN_1 & _MCLR_OFF & _PWRTE_OFF & _WDT_OFF & _HS_OSC

//Program Configuration Register 2
//		__CONFIG    _CONFIG2, _BORSEN_1 & _IESO_OFF & _FCMEN_OFF


//         LIST

// ----- ADCON0 bits --------------------
typedef union {
  struct {
    unsigned char ADON:1;
    unsigned char CHS3:1;
    unsigned char GO:1;
    unsigned char CHS0:1;
    unsigned char CHS1:1;
    unsigned char CHS2:1;
    unsigned char ADCS0:1;
    unsigned char ADCS1:1;
  };
  struct {
    unsigned char :1;
    unsigned char :1;
    unsigned char NOT_DONE:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
  };
  struct {
    unsigned char :1;
    unsigned char :1;
    unsigned char GO_DONE:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
  };
} __ADCON0_bits_t;
extern volatile __ADCON0_bits_t __at(ADCON0_ADDR) ADCON0_bits;

#define ADON                 ADCON0_bits.ADON
#define CHS3                 ADCON0_bits.CHS3
#define GO                   ADCON0_bits.GO
#define NOT_DONE             ADCON0_bits.NOT_DONE
#define GO_DONE              ADCON0_bits.GO_DONE
#define CHS0                 ADCON0_bits.CHS0
#define CHS1                 ADCON0_bits.CHS1
#define CHS2                 ADCON0_bits.CHS2
#define ADCS0                ADCON0_bits.ADCS0
#define ADCS1                ADCON0_bits.ADCS1

// ----- ADCON1 bits --------------------
typedef union {
  struct {
    unsigned char PCFG0:1;
    unsigned char PCFG1:1;
    unsigned char PCFG2:1;
    unsigned char PCFG3:1;
    unsigned char VCFG0:1;
    unsigned char VCFG1:1;
    unsigned char ADCS2:1;
    unsigned char ADFM:1;
  };
} __ADCON1_bits_t;
extern volatile __ADCON1_bits_t __at(ADCON1_ADDR) ADCON1_bits;

#define PCFG0                ADCON1_bits.PCFG0
#define PCFG1                ADCON1_bits.PCFG1
#define PCFG2                ADCON1_bits.PCFG2
#define PCFG3                ADCON1_bits.PCFG3
#define VCFG0                ADCON1_bits.VCFG0
#define VCFG1                ADCON1_bits.VCFG1
#define ADCS2                ADCON1_bits.ADCS2
#define ADFM                 ADCON1_bits.ADFM

// ----- ADCON2 bits --------------------
typedef union {
  struct {
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char ACQT0:1;
    unsigned char ACQT1:1;
    unsigned char ACQT2:1;
    unsigned char :1;
    unsigned char :1;
  };
} __ADCON2_bits_t;
extern volatile __ADCON2_bits_t __at(ADCON2_ADDR) ADCON2_bits;

#define ACQT0                ADCON2_bits.ACQT0
#define ACQT1                ADCON2_bits.ACQT1
#define ACQT2                ADCON2_bits.ACQT2

// ----- CCP1CON bits --------------------
typedef union {
  struct {
    unsigned char CCP1M0:1;
    unsigned char CCP1M1:1;
    unsigned char CCP1M2:1;
    unsigned char CCP1M3:1;
    unsigned char CCP1Y:1;
    unsigned char CCP1X:1;
    unsigned char :1;
    unsigned char :1;
  };
} __CCP1CON_bits_t;
extern volatile __CCP1CON_bits_t __at(CCP1CON_ADDR) CCP1CON_bits;

#define CCP1M0               CCP1CON_bits.CCP1M0
#define CCP1M1               CCP1CON_bits.CCP1M1
#define CCP1M2               CCP1CON_bits.CCP1M2
#define CCP1M3               CCP1CON_bits.CCP1M3
#define CCP1Y                CCP1CON_bits.CCP1Y
#define CCP1X                CCP1CON_bits.CCP1X

// ----- CCP2CON bits --------------------
typedef union {
  struct {
    unsigned char CCP2M0:1;
    unsigned char CCP2M1:1;
    unsigned char CCP2M2:1;
    unsigned char CCP2M3:1;
    unsigned char CCP2Y:1;
    unsigned char CCP2X:1;
    unsigned char :1;
    unsigned char :1;
  };
} __CCP2CON_bits_t;
extern volatile __CCP2CON_bits_t __at(CCP2CON_ADDR) CCP2CON_bits;

#define CCP2M0               CCP2CON_bits.CCP2M0
#define CCP2M1               CCP2CON_bits.CCP2M1
#define CCP2M2               CCP2CON_bits.CCP2M2
#define CCP2M3               CCP2CON_bits.CCP2M3
#define CCP2Y                CCP2CON_bits.CCP2Y
#define CCP2X                CCP2CON_bits.CCP2X

// ----- CCP3CON bits --------------------
typedef union {
  struct {
    unsigned char CCP3M0:1;
    unsigned char CCP3M1:1;
    unsigned char CCP3M2:1;
    unsigned char CCP3M3:1;
    unsigned char CCP3Y:1;
    unsigned char CCP3X:1;
    unsigned char :1;
    unsigned char :1;
  };
} __CCP3CON_bits_t;
extern volatile __CCP3CON_bits_t __at(CCP3CON_ADDR) CCP3CON_bits;

#define CCP3M0               CCP3CON_bits.CCP3M0
#define CCP3M1               CCP3CON_bits.CCP3M1
#define CCP3M2               CCP3CON_bits.CCP3M2
#define CCP3M3               CCP3CON_bits.CCP3M3
#define CCP3Y                CCP3CON_bits.CCP3Y
#define CCP3X                CCP3CON_bits.CCP3X

// ----- CMCON bits --------------------
typedef union {
  struct {
    unsigned char CM0:1;
    unsigned char CM1:1;
    unsigned char CM2:1;
    unsigned char CIS:1;
    unsigned char C1INV:1;
    unsigned char C2INV:1;
    unsigned char C1OUT:1;
    unsigned char C2OUT:1;
  };
} __CMCON_bits_t;
extern volatile __CMCON_bits_t __at(CMCON_ADDR) CMCON_bits;

#define CM0                  CMCON_bits.CM0
#define CM1                  CMCON_bits.CM1
#define CM2                  CMCON_bits.CM2
#define CIS                  CMCON_bits.CIS
#define C1INV                CMCON_bits.C1INV
#define C2INV                CMCON_bits.C2INV
#define C1OUT                CMCON_bits.C1OUT
#define C2OUT                CMCON_bits.C2OUT

// ----- CVRCON bits --------------------
typedef union {
  struct {
    unsigned char CVR0:1;
    unsigned char CVR1:1;
    unsigned char CVR2:1;
    unsigned char CVR3:1;
    unsigned char :1;
    unsigned char CVRR:1;
    unsigned char CVROE:1;
    unsigned char CVREN:1;
  };
} __CVRCON_bits_t;
extern volatile __CVRCON_bits_t __at(CVRCON_ADDR) CVRCON_bits;

#define CVR0                 CVRCON_bits.CVR0
#define CVR1                 CVRCON_bits.CVR1
#define CVR2                 CVRCON_bits.CVR2
#define CVR3                 CVRCON_bits.CVR3
#define CVRR                 CVRCON_bits.CVRR
#define CVROE                CVRCON_bits.CVROE
#define CVREN                CVRCON_bits.CVREN

// ----- INTCON bits --------------------
typedef union {
  struct {
    unsigned char RBIF:1;
    unsigned char INTF:1;
    unsigned char T0IF:1;
    unsigned char RBIE:1;
    unsigned char INTE:1;
    unsigned char T0IE:1;
    unsigned char PEIE:1;
    unsigned char GIE:1;
  };
  struct {
    unsigned char :1;
    unsigned char INT0IF:1;
    unsigned char TMR0IF:1;
    unsigned char :1;
    unsigned char INT0IE:1;
    unsigned char TMR0IE:1;
    unsigned char :1;
    unsigned char :1;
  };
} __INTCON_bits_t;
extern volatile __INTCON_bits_t __at(INTCON_ADDR) INTCON_bits;

#define RBIF                 INTCON_bits.RBIF
#define INTF                 INTCON_bits.INTF
#define INT0IF               INTCON_bits.INT0IF
#define T0IF                 INTCON_bits.T0IF
#define TMR0IF               INTCON_bits.TMR0IF
#define RBIE                 INTCON_bits.RBIE
#define INTE                 INTCON_bits.INTE
#define INT0IE               INTCON_bits.INT0IE
#define T0IE                 INTCON_bits.T0IE
#define TMR0IE               INTCON_bits.TMR0IE
#define PEIE                 INTCON_bits.PEIE
#define GIE                  INTCON_bits.GIE

// ----- LVDCON bits --------------------
typedef union {
  struct {
    unsigned char LVDL0:1;
    unsigned char LVDL1:1;
    unsigned char LVDL2:1;
    unsigned char LVDL3:1;
    unsigned char LVDEN:1;
    unsigned char IRVST:1;
    unsigned char :1;
    unsigned char :1;
  };
} __LVDCON_bits_t;
extern volatile __LVDCON_bits_t __at(LVDCON_ADDR) LVDCON_bits;

#define LVDL0                LVDCON_bits.LVDL0
#define LVDL1                LVDCON_bits.LVDL1
#define LVDL2                LVDCON_bits.LVDL2
#define LVDL3                LVDCON_bits.LVDL3
#define LVDEN                LVDCON_bits.LVDEN
#define IRVST                LVDCON_bits.IRVST

// ----- OPTION_REG bits --------------------
typedef union {
  struct {
    unsigned char PS0:1;
    unsigned char PS1:1;
    unsigned char PS2:1;
    unsigned char PSA:1;
    unsigned char T0SE:1;
    unsigned char T0CS:1;
    unsigned char INTEDG:1;
    unsigned char NOT_RBPU:1;
  };
} __OPTION_REG_bits_t;
extern volatile __OPTION_REG_bits_t __at(OPTION_REG_ADDR) OPTION_REG_bits;

#define PS0                  OPTION_REG_bits.PS0
#define PS1                  OPTION_REG_bits.PS1
#define PS2                  OPTION_REG_bits.PS2
#define PSA                  OPTION_REG_bits.PSA
#define T0SE                 OPTION_REG_bits.T0SE
#define T0CS                 OPTION_REG_bits.T0CS
#define INTEDG               OPTION_REG_bits.INTEDG
#define NOT_RBPU             OPTION_REG_bits.NOT_RBPU

// ----- OSCCON bits --------------------
typedef union {
  struct {
    unsigned char SCS0:1;
    unsigned char SCS1:1;
    unsigned char IOFS:1;
    unsigned char OSTS:1;
    unsigned char IRCF0:1;
    unsigned char IRCF1:1;
    unsigned char IRCF2:1;
    unsigned char :1;
  };
} __OSCCON_bits_t;
extern volatile __OSCCON_bits_t __at(OSCCON_ADDR) OSCCON_bits;

#define SCS0                 OSCCON_bits.SCS0
#define SCS1                 OSCCON_bits.SCS1
#define IOFS                 OSCCON_bits.IOFS
#define OSTS                 OSCCON_bits.OSTS
#define IRCF0                OSCCON_bits.IRCF0
#define IRCF1                OSCCON_bits.IRCF1
#define IRCF2                OSCCON_bits.IRCF2

// ----- OSCTUNE bits --------------------
typedef union {
  struct {
    unsigned char TUN0:1;
    unsigned char TUN1:1;
    unsigned char TUN2:1;
    unsigned char TUN3:1;
    unsigned char TUN4:1;
    unsigned char TUN5:1;
    unsigned char :1;
    unsigned char :1;
  };
} __OSCTUNE_bits_t;
extern volatile __OSCTUNE_bits_t __at(OSCTUNE_ADDR) OSCTUNE_bits;

#define TUN0                 OSCTUNE_bits.TUN0
#define TUN1                 OSCTUNE_bits.TUN1
#define TUN2                 OSCTUNE_bits.TUN2
#define TUN3                 OSCTUNE_bits.TUN3
#define TUN4                 OSCTUNE_bits.TUN4
#define TUN5                 OSCTUNE_bits.TUN5

// ----- PCON bits --------------------
typedef union {
  struct {
    unsigned char NOT_BO:1;
    unsigned char NOT_POR:1;
    unsigned char SBOREN:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
  };
  struct {
    unsigned char NOT_BOR:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
  };
} __PCON_bits_t;
extern volatile __PCON_bits_t __at(PCON_ADDR) PCON_bits;

#define NOT_BO               PCON_bits.NOT_BO
#define NOT_BOR              PCON_bits.NOT_BOR
#define NOT_POR              PCON_bits.NOT_POR
#define SBOREN               PCON_bits.SBOREN

// ----- PIE1 bits --------------------
typedef union {
  struct {
    unsigned char TMR1IE:1;
    unsigned char TMR2IE:1;
    unsigned char CCP1IE:1;
    unsigned char SSPIE:1;
    unsigned char TXIE:1;
    unsigned char RCIE:1;
    unsigned char ADIE:1;
    unsigned char PSPIE:1;
  };
} __PIE1_bits_t;
extern volatile __PIE1_bits_t __at(PIE1_ADDR) PIE1_bits;

#define TMR1IE               PIE1_bits.TMR1IE
#define TMR2IE               PIE1_bits.TMR2IE
#define CCP1IE               PIE1_bits.CCP1IE
#define SSPIE                PIE1_bits.SSPIE
#define TXIE                 PIE1_bits.TXIE
#define RCIE                 PIE1_bits.RCIE
#define ADIE                 PIE1_bits.ADIE
#define PSPIE                PIE1_bits.PSPIE

// ----- PIE2 bits --------------------
typedef union {
  struct {
    unsigned char CCP2IE:1;
    unsigned char CCP3IE:1;
    unsigned char :1;
    unsigned char BCLIE:1;
    unsigned char :1;
    unsigned char LVDIE:1;
    unsigned char CMIE:1;
    unsigned char OSFIE:1;
  };
} __PIE2_bits_t;
extern volatile __PIE2_bits_t __at(PIE2_ADDR) PIE2_bits;

#define CCP2IE               PIE2_bits.CCP2IE
#define CCP3IE               PIE2_bits.CCP3IE
#define BCLIE                PIE2_bits.BCLIE
#define LVDIE                PIE2_bits.LVDIE
#define CMIE                 PIE2_bits.CMIE
#define OSFIE                PIE2_bits.OSFIE

// ----- PIR1 bits --------------------
typedef union {
  struct {
    unsigned char TMR1IF:1;
    unsigned char TMR2IF:1;
    unsigned char CCP1IF:1;
    unsigned char SSPIF:1;
    unsigned char TXIF:1;
    unsigned char RCIF:1;
    unsigned char ADIF:1;
    unsigned char PSPIF:1;
  };
} __PIR1_bits_t;
extern volatile __PIR1_bits_t __at(PIR1_ADDR) PIR1_bits;

#define TMR1IF               PIR1_bits.TMR1IF
#define TMR2IF               PIR1_bits.TMR2IF
#define CCP1IF               PIR1_bits.CCP1IF
#define SSPIF                PIR1_bits.SSPIF
#define TXIF                 PIR1_bits.TXIF
#define RCIF                 PIR1_bits.RCIF
#define ADIF                 PIR1_bits.ADIF
#define PSPIF                PIR1_bits.PSPIF

// ----- PIR2 bits --------------------
typedef union {
  struct {
    unsigned char CCP2IF:1;
    unsigned char CCP3IF:1;
    unsigned char :1;
    unsigned char BCLIF:1;
    unsigned char :1;
    unsigned char LVDIF:1;
    unsigned char CMIF:1;
    unsigned char OSFIF:1;
  };
} __PIR2_bits_t;
extern volatile __PIR2_bits_t __at(PIR2_ADDR) PIR2_bits;

#define CCP2IF               PIR2_bits.CCP2IF
#define CCP3IF               PIR2_bits.CCP3IF
#define BCLIF                PIR2_bits.BCLIF
#define LVDIF                PIR2_bits.LVDIF
#define CMIF                 PIR2_bits.CMIF
#define OSFIF                PIR2_bits.OSFIF

// ----- PMCON1 bits --------------------
typedef union {
  struct {
    unsigned char RD:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
  };
} __PMCON1_bits_t;
extern volatile __PMCON1_bits_t __at(PMCON1_ADDR) PMCON1_bits;

#define RD                   PMCON1_bits.RD

// ----- RCSTA bits --------------------
typedef union {
  struct {
    unsigned char RX9D:1;
    unsigned char OERR:1;
    unsigned char FERR:1;
    unsigned char ADDEN:1;
    unsigned char CREN:1;
    unsigned char SREN:1;
    unsigned char RX9:1;
    unsigned char SPEN:1;
  };
  struct {
    unsigned char RCD8:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char RC9:1;
    unsigned char :1;
  };
  struct {
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char NOT_RC8:1;
    unsigned char :1;
  };
  struct {
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char RC8_9:1;
    unsigned char :1;
  };
} __RCSTA_bits_t;
extern volatile __RCSTA_bits_t __at(RCSTA_ADDR) RCSTA_bits;

#define RX9D                 RCSTA_bits.RX9D
#define RCD8                 RCSTA_bits.RCD8
#define OERR                 RCSTA_bits.OERR
#define FERR                 RCSTA_bits.FERR
#define ADDEN                RCSTA_bits.ADDEN
#define CREN                 RCSTA_bits.CREN
#define SREN                 RCSTA_bits.SREN
#define RX9                  RCSTA_bits.RX9
#define RC9                  RCSTA_bits.RC9
#define NOT_RC8              RCSTA_bits.NOT_RC8
#define RC8_9                RCSTA_bits.RC8_9
#define SPEN                 RCSTA_bits.SPEN

// ----- SSPCON bits --------------------
typedef union {
  struct {
    unsigned char SSPM0:1;
    unsigned char SSPM1:1;
    unsigned char SSPM2:1;
    unsigned char SSPM3:1;
    unsigned char CKP:1;
    unsigned char SSPEN:1;
    unsigned char SSPOV:1;
    unsigned char WCOL:1;
  };
} __SSPCON_bits_t;
extern volatile __SSPCON_bits_t __at(SSPCON_ADDR) SSPCON_bits;

#define SSPM0                SSPCON_bits.SSPM0
#define SSPM1                SSPCON_bits.SSPM1
#define SSPM2                SSPCON_bits.SSPM2
#define SSPM3                SSPCON_bits.SSPM3
#define CKP                  SSPCON_bits.CKP
#define SSPEN                SSPCON_bits.SSPEN
#define SSPOV                SSPCON_bits.SSPOV
#define WCOL                 SSPCON_bits.WCOL

// ----- SSPCON2 bits --------------------
typedef union {
  struct {
    unsigned char SEN:1;
    unsigned char RSEN:1;
    unsigned char PEN:1;
    unsigned char RCEN:1;
    unsigned char ACKEN:1;
    unsigned char ACKDT:1;
    unsigned char ACKSTAT:1;
    unsigned char GCEN:1;
  };
} __SSPCON2_bits_t;
extern volatile __SSPCON2_bits_t __at(SSPCON2_ADDR) SSPCON2_bits;

#define SEN                  SSPCON2_bits.SEN
#define RSEN                 SSPCON2_bits.RSEN
#define PEN                  SSPCON2_bits.PEN
#define RCEN                 SSPCON2_bits.RCEN
#define ACKEN                SSPCON2_bits.ACKEN
#define ACKDT                SSPCON2_bits.ACKDT
#define ACKSTAT              SSPCON2_bits.ACKSTAT
#define GCEN                 SSPCON2_bits.GCEN

// ----- SSPSTAT bits --------------------
typedef union {
  struct {
    unsigned char BF:1;
    unsigned char UA:1;
    unsigned char R:1;
    unsigned char S:1;
    unsigned char P:1;
    unsigned char D:1;
    unsigned char CKE:1;
    unsigned char SMP:1;
  };
  struct {
    unsigned char :1;
    unsigned char :1;
    unsigned char I2C_READ:1;
    unsigned char I2C_START:1;
    unsigned char I2C_STOP:1;
    unsigned char I2C_DATA:1;
    unsigned char :1;
    unsigned char :1;
  };
  struct {
    unsigned char :1;
    unsigned char :1;
    unsigned char NOT_W:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char NOT_A:1;
    unsigned char :1;
    unsigned char :1;
  };
  struct {
    unsigned char :1;
    unsigned char :1;
    unsigned char NOT_WRITE:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char NOT_ADDRESS:1;
    unsigned char :1;
    unsigned char :1;
  };
  struct {
    unsigned char :1;
    unsigned char :1;
    unsigned char R_W:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char D_A:1;
    unsigned char :1;
    unsigned char :1;
  };
  struct {
    unsigned char :1;
    unsigned char :1;
    unsigned char READ_WRITE:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char DATA_ADDRESS:1;
    unsigned char :1;
    unsigned char :1;
  };
} __SSPSTAT_bits_t;
extern volatile __SSPSTAT_bits_t __at(SSPSTAT_ADDR) SSPSTAT_bits;

#define BF                   SSPSTAT_bits.BF
#define UA                   SSPSTAT_bits.UA
#define R                    SSPSTAT_bits.R
#define I2C_READ             SSPSTAT_bits.I2C_READ
#define NOT_W                SSPSTAT_bits.NOT_W
#define NOT_WRITE            SSPSTAT_bits.NOT_WRITE
#define R_W                  SSPSTAT_bits.R_W
#define READ_WRITE           SSPSTAT_bits.READ_WRITE
#define S                    SSPSTAT_bits.S
#define I2C_START            SSPSTAT_bits.I2C_START
#define P                    SSPSTAT_bits.P
#define I2C_STOP             SSPSTAT_bits.I2C_STOP
#define D                    SSPSTAT_bits.D
#define I2C_DATA             SSPSTAT_bits.I2C_DATA
#define NOT_A                SSPSTAT_bits.NOT_A
#define NOT_ADDRESS          SSPSTAT_bits.NOT_ADDRESS
#define D_A                  SSPSTAT_bits.D_A
#define DATA_ADDRESS         SSPSTAT_bits.DATA_ADDRESS
#define CKE                  SSPSTAT_bits.CKE
#define SMP                  SSPSTAT_bits.SMP

// ----- STATUS bits --------------------
typedef union {
  struct {
    unsigned char C:1;
    unsigned char DC:1;
    unsigned char Z:1;
    unsigned char NOT_PD:1;
    unsigned char NOT_TO:1;
    unsigned char RP0:1;
    unsigned char RP1:1;
    unsigned char IRP:1;
  };
} __STATUS_bits_t;
extern volatile __STATUS_bits_t __at(STATUS_ADDR) STATUS_bits;

#define C                    STATUS_bits.C
#define DC                   STATUS_bits.DC
#define Z                    STATUS_bits.Z
#define NOT_PD               STATUS_bits.NOT_PD
#define NOT_TO               STATUS_bits.NOT_TO
#define RP0                  STATUS_bits.RP0
#define RP1                  STATUS_bits.RP1
#define IRP                  STATUS_bits.IRP

// ----- T1CON bits --------------------
typedef union {
  struct {
    unsigned char TMR1ON:1;
    unsigned char TMR1CS:1;
    unsigned char NOT_T1SYNC:1;
    unsigned char T1OSCEN:1;
    unsigned char T1CKPS0:1;
    unsigned char T1CKPS1:1;
    unsigned char T1RUN:1;
    unsigned char :1;
  };
  struct {
    unsigned char :1;
    unsigned char :1;
    unsigned char T1INSYNC:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
  };
  struct {
    unsigned char :1;
    unsigned char :1;
    unsigned char T1SYNC:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
  };
} __T1CON_bits_t;
extern volatile __T1CON_bits_t __at(T1CON_ADDR) T1CON_bits;

#define TMR1ON               T1CON_bits.TMR1ON
#define TMR1CS               T1CON_bits.TMR1CS
#define NOT_T1SYNC           T1CON_bits.NOT_T1SYNC
#define T1INSYNC             T1CON_bits.T1INSYNC
#define T1SYNC               T1CON_bits.T1SYNC
#define T1OSCEN              T1CON_bits.T1OSCEN
#define T1CKPS0              T1CON_bits.T1CKPS0
#define T1CKPS1              T1CON_bits.T1CKPS1
#define T1RUN                T1CON_bits.T1RUN

// ----- T2CON bits --------------------
typedef union {
  struct {
    unsigned char T2CKPS0:1;
    unsigned char T2CKPS1:1;
    unsigned char TMR2ON:1;
    unsigned char TOUTPS0:1;
    unsigned char TOUTPS1:1;
    unsigned char TOUTPS2:1;
    unsigned char TOUTPS3:1;
    unsigned char :1;
  };
} __T2CON_bits_t;
extern volatile __T2CON_bits_t __at(T2CON_ADDR) T2CON_bits;

#define T2CKPS0              T2CON_bits.T2CKPS0
#define T2CKPS1              T2CON_bits.T2CKPS1
#define TMR2ON               T2CON_bits.TMR2ON
#define TOUTPS0              T2CON_bits.TOUTPS0
#define TOUTPS1              T2CON_bits.TOUTPS1
#define TOUTPS2              T2CON_bits.TOUTPS2
#define TOUTPS3              T2CON_bits.TOUTPS3

// ----- TRISE bits --------------------
typedef union {
  struct {
    unsigned char TRISE0:1;
    unsigned char TRISE1:1;
    unsigned char TRISE2:1;
    unsigned char TRISE3:1;
    unsigned char PSPMODE:1;
    unsigned char IBOV:1;
    unsigned char OBF:1;
    unsigned char IBF:1;
  };
} __TRISE_bits_t;
extern volatile __TRISE_bits_t __at(TRISE_ADDR) TRISE_bits;

#define TRISE0               TRISE_bits.TRISE0
#define TRISE1               TRISE_bits.TRISE1
#define TRISE2               TRISE_bits.TRISE2
#define TRISE3               TRISE_bits.TRISE3
#define PSPMODE              TRISE_bits.PSPMODE
#define IBOV                 TRISE_bits.IBOV
#define OBF                  TRISE_bits.OBF
#define IBF                  TRISE_bits.IBF

// ----- TXSTA bits --------------------
typedef union {
  struct {
    unsigned char TX9D:1;
    unsigned char TRMT:1;
    unsigned char BRGH:1;
    unsigned char :1;
    unsigned char SYNC:1;
    unsigned char TXEN:1;
    unsigned char TX9:1;
    unsigned char CSRC:1;
  };
  struct {
    unsigned char TXD8:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char NOT_TX8:1;
    unsigned char :1;
  };
  struct {
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char TX8_9:1;
    unsigned char :1;
  };
} __TXSTA_bits_t;
extern volatile __TXSTA_bits_t __at(TXSTA_ADDR) TXSTA_bits;

#define TX9D                 TXSTA_bits.TX9D
#define TXD8                 TXSTA_bits.TXD8
#define TRMT                 TXSTA_bits.TRMT
#define BRGH                 TXSTA_bits.BRGH
#define SYNC                 TXSTA_bits.SYNC
#define TXEN                 TXSTA_bits.TXEN
#define TX9                  TXSTA_bits.TX9
#define NOT_TX8              TXSTA_bits.NOT_TX8
#define TX8_9                TXSTA_bits.TX8_9
#define CSRC                 TXSTA_bits.CSRC

// ----- WDTCON bits --------------------
typedef union {
  struct {
    unsigned char SWDTEN:1;
    unsigned char WDTPS0:1;
    unsigned char WDTPS1:1;
    unsigned char WDTPS2:1;
    unsigned char WDTPS3:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
  };
  struct {
    unsigned char SWDTE:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
  };
} __WDTCON_bits_t;
extern volatile __WDTCON_bits_t __at(WDTCON_ADDR) WDTCON_bits;

#define SWDTEN               WDTCON_bits.SWDTEN
#define SWDTE                WDTCON_bits.SWDTE
#define WDTPS0               WDTCON_bits.WDTPS0
#define WDTPS1               WDTCON_bits.WDTPS1
#define WDTPS2               WDTCON_bits.WDTPS2
#define WDTPS3               WDTCON_bits.WDTPS3

#endif
