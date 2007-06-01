//
// Register Declarations for Microchip 12F683 Processor
//
//
// This header file was automatically generated by:
//
//	inc2h.pl V4585
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
#ifndef P12F683_H
#define P12F683_H

//
// Register addresses.
//
#define INDF_ADDR	0x0000
#define TMR0_ADDR	0x0001
#define PCL_ADDR	0x0002
#define STATUS_ADDR	0x0003
#define FSR_ADDR	0x0004
#define GPIO_ADDR	0x0005
#define PCLATH_ADDR	0x000A
#define INTCON_ADDR	0x000B
#define PIR1_ADDR	0x000C
#define TMR1L_ADDR	0x000E
#define TMR1H_ADDR	0x000F
#define T1CON_ADDR	0x0010
#define TMR2_ADDR	0x0011
#define T2CON_ADDR	0x0012
#define CCPR1L_ADDR	0x0013
#define CCPR1H_ADDR	0x0014
#define CCP1CON_ADDR	0x0015
#define WDTCON_ADDR	0x0018
#define CMCON0_ADDR	0x0019
#define CMCON1_ADDR	0x001A
#define ADRESH_ADDR	0x001E
#define ADCON0_ADDR	0x001F
#define OPTION_REG_ADDR	0x0081
#define TRISIO_ADDR	0x0085
#define PIE1_ADDR	0x008C
#define PCON_ADDR	0x008E
#define OSCCON_ADDR	0x008F
#define OSCTUNE_ADDR	0x0090
#define PR2_ADDR	0x0092
#define WPU_ADDR	0x0095
#define WPUA_ADDR	0x0095
#define IOC_ADDR	0x0096
#define IOCA_ADDR	0x0096
#define VRCON_ADDR	0x0099
#define EEDATA_ADDR	0x009A
#define EEDAT_ADDR	0x009A
#define EEADR_ADDR	0x009B
#define EECON1_ADDR	0x009C
#define EECON2_ADDR	0x009D
#define ADRESL_ADDR	0x009E
#define ANSEL_ADDR	0x009F

//
// Memory organization.
//



//         LIST
// P12F683.INC  Standard Header File, Version 1.00    Microchip Technology, Inc.
//         NOLIST

// This header file defines configurations, registers, and other useful bits of
// information for the PIC12F683 microcontroller.  These names are taken to match 
// the data sheets as closely as possible.  

// Note that the processor must be selected before this file is 
// included.  The processor may be selected the following ways:

//       1. Command line switch:
//               C:\ MPASM MYFILE.ASM /PIC16F684
//       2. LIST directive in the source file
//               LIST   P=PIC12F683
//       3. Processor Type entry in the MPASM full-screen interface

//==========================================================================
//
//       Revision History
//
//==========================================================================
//1.01   01/05/07 GPIO Bits
//1.00   12/09/03 Original

//==========================================================================
//
//       Verify Processor
//
//==========================================================================

//        IFNDEF __12F683
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

extern __sfr  __at (INDF_ADDR)                    INDF;
extern __sfr  __at (TMR0_ADDR)                    TMR0;
extern __sfr  __at (PCL_ADDR)                     PCL;
extern __sfr  __at (STATUS_ADDR)                  STATUS;
extern __sfr  __at (FSR_ADDR)                     FSR;
extern __sfr  __at (GPIO_ADDR)                    GPIO;

extern __sfr  __at (PCLATH_ADDR)                  PCLATH;
extern __sfr  __at (INTCON_ADDR)                  INTCON;
extern __sfr  __at (PIR1_ADDR)                    PIR1;

extern __sfr  __at (TMR1L_ADDR)                   TMR1L;		
extern __sfr  __at (TMR1H_ADDR)                   TMR1H;		
extern __sfr  __at (T1CON_ADDR)                   T1CON;		
extern __sfr  __at (TMR2_ADDR)                    TMR2;
extern __sfr  __at (T2CON_ADDR)                   T2CON;
extern __sfr  __at (CCPR1L_ADDR)                  CCPR1L;
extern __sfr  __at (CCPR1H_ADDR)                  CCPR1H;
extern __sfr  __at (CCP1CON_ADDR)                 CCP1CON;

extern __sfr  __at (WDTCON_ADDR)                  WDTCON;
extern __sfr  __at (CMCON0_ADDR)                  CMCON0;		
extern __sfr  __at (CMCON1_ADDR)                  CMCON1;		

extern __sfr  __at (ADRESH_ADDR)                  ADRESH;		
extern __sfr  __at (ADCON0_ADDR)                  ADCON0;		

extern __sfr  __at (OPTION_REG_ADDR)              OPTION_REG;

extern __sfr  __at (TRISIO_ADDR)                  TRISIO;

extern __sfr  __at (PIE1_ADDR)                    PIE1;

extern __sfr  __at (PCON_ADDR)                    PCON;
extern __sfr  __at (OSCCON_ADDR)                  OSCCON;
extern __sfr  __at (OSCTUNE_ADDR)                 OSCTUNE;

extern __sfr  __at (PR2_ADDR)                     PR2;

extern __sfr  __at (WPU_ADDR)                     WPU;
extern __sfr  __at (WPUA_ADDR)                    WPUA;
extern __sfr  __at (IOC_ADDR)                     IOC;
extern __sfr  __at (IOCA_ADDR)                    IOCA;

extern __sfr  __at (VRCON_ADDR)                   VRCON;
extern __sfr  __at (EEDATA_ADDR)                  EEDATA; 
extern __sfr  __at (EEDAT_ADDR)                   EEDAT;	
extern __sfr  __at (EEADR_ADDR)                   EEADR;	
extern __sfr  __at (EECON1_ADDR)                  EECON1;
extern __sfr  __at (EECON2_ADDR)                  EECON2;
extern __sfr  __at (ADRESL_ADDR)                  ADRESL;		
extern __sfr  __at (ANSEL_ADDR)                   ANSEL;


//----- STATUS Bits --------------------------------------------------------


//----- INTCON Bits --------------------------------------------------------


//----- PIR1 Bits ----------------------------------------------------------


//----- T1CON Bits ---------------------------------------------------------


//----- T2CON Bits ---------------------------------------------------------


//----- CCP1CON Bits -------------------------------------------------------


//----- WDTCON Bits --------------------------------------------------------


//----- COMCON0 Bits -------------------------------------------------------


//----- COMCON1 Bits -------------------------------------------------------


//----- ADCON0 Bits --------------------------------------------------------


//----- OPTION Bits --------------------------------------------------------



//----- PIE1 Bits ----------------------------------------------------------


//----- PCON Bits ----------------------------------------------------------


//----- OSCCON Bits --------------------------------------------------------


//----- OSCTUNE Bits -------------------------------------------------------



//----- IOC --------------------------------------------------------------


//----- IOCA --------------------------------------------------------------


//----- VRCON Bits ---------------------------------------------------------


//----- EECON1 -------------------------------------------------------------


//----- ANSEL --------------------------------------------------------------


//==========================================================================
//
//       RAM Definition
//
//==========================================================================

//         __MAXRAM H'FF'
//         __BADRAM H'06', H'08'-H'09', H'0D', H'1B'-H'1D'
//         __BADRAM H'86', H'88'-H'89', H'8D', H'93'-H'94', H'97'-H'98', H'C0'-H'EF'

//==========================================================================
//
//       Configuration Bits
//
//==========================================================================

#define _FCMEN_ON            0x3FFF
#define _FCMEN_OFF           0x37FF
#define _IESO_ON             0x3FFF
#define _IESO_OFF            0x3BFF
#define _BOD_ON              0x3FFF
#define _BOD_NSLEEP          0x3EFF
#define _BOD_SBODEN          0x3DFF
#define _BOD_OFF             0x3CFF
#define _CPD_ON              0x3F7F
#define _CPD_OFF             0x3FFF
#define _CP_ON               0x3FBF
#define _CP_OFF              0x3FFF
#define _MCLRE_ON            0x3FFF
#define _MCLRE_OFF           0x3FDF
#define _PWRTE_OFF           0x3FFF
#define _PWRTE_ON            0x3FEF
#define _WDT_ON              0x3FFF
#define _WDT_OFF             0x3FF7
#define _LP_OSC              0x3FF8
#define _XT_OSC              0x3FF9
#define _HS_OSC              0x3FFA
#define _EC_OSC              0x3FFB
#define _INTRC_OSC_NOCLKOUT  0x3FFC
#define _INTOSCIO            0x3FFC
#define _INTRC_OSC_CLKOUT    0x3FFD
#define _INTOSC              0x3FFD
#define _EXTRC_OSC_NOCLKOUT  0x3FFE
#define _EXTRCIO             0x3FFE
#define _EXTRC_OSC_CLKOUT    0x3FFF
#define _EXTRC               0x3FFF

//         LIST

// ----- ADCON0 bits --------------------
typedef union {
  struct {
    unsigned char ADON:1;
    unsigned char GO:1;
    unsigned char CHS0:1;
    unsigned char CHS1:1;
    unsigned char CHS2:1;
    unsigned char :1;
    unsigned char VCFG:1;
    unsigned char ADFM:1;
  };
  struct {
    unsigned char :1;
    unsigned char NOT_DONE:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
  };
  struct {
    unsigned char :1;
    unsigned char GO_DONE:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
  };
} __ADCON0_bits_t;
extern volatile __ADCON0_bits_t __at(ADCON0_ADDR) ADCON0_bits;

#define ADON                 ADCON0_bits.ADON
#define GO                   ADCON0_bits.GO
#define NOT_DONE             ADCON0_bits.NOT_DONE
#define GO_DONE              ADCON0_bits.GO_DONE
#define CHS0                 ADCON0_bits.CHS0
#define CHS1                 ADCON0_bits.CHS1
#define CHS2                 ADCON0_bits.CHS2
#define VCFG                 ADCON0_bits.VCFG
#define ADFM                 ADCON0_bits.ADFM

// ----- ANSEL bits --------------------
typedef union {
  struct {
    unsigned char ANS0:1;
    unsigned char ANS1:1;
    unsigned char ANS2:1;
    unsigned char ANS3:1;
    unsigned char ADCS0:1;
    unsigned char ADCS1:1;
    unsigned char ADCS2:1;
    unsigned char :1;
  };
} __ANSEL_bits_t;
extern volatile __ANSEL_bits_t __at(ANSEL_ADDR) ANSEL_bits;

#define ANS0                 ANSEL_bits.ANS0
#define ANS1                 ANSEL_bits.ANS1
#define ANS2                 ANSEL_bits.ANS2
#define ANS3                 ANSEL_bits.ANS3
#define ADCS0                ANSEL_bits.ADCS0
#define ADCS1                ANSEL_bits.ADCS1
#define ADCS2                ANSEL_bits.ADCS2

// ----- CCP1CON bits --------------------
typedef union {
  struct {
    unsigned char CCP1M0:1;
    unsigned char CCP1M1:1;
    unsigned char CCP1M2:1;
    unsigned char CCP1M3:1;
    unsigned char DC1B0:1;
    unsigned char DC1B1:1;
    unsigned char :1;
    unsigned char :1;
  };
} __CCP1CON_bits_t;
extern volatile __CCP1CON_bits_t __at(CCP1CON_ADDR) CCP1CON_bits;

#define CCP1M0               CCP1CON_bits.CCP1M0
#define CCP1M1               CCP1CON_bits.CCP1M1
#define CCP1M2               CCP1CON_bits.CCP1M2
#define CCP1M3               CCP1CON_bits.CCP1M3
#define DC1B0                CCP1CON_bits.DC1B0
#define DC1B1                CCP1CON_bits.DC1B1

// ----- CMCON0 bits --------------------
typedef union {
  struct {
    unsigned char CM0:1;
    unsigned char CM1:1;
    unsigned char CM2:1;
    unsigned char CIS:1;
    unsigned char CINV:1;
    unsigned char :1;
    unsigned char COUT:1;
    unsigned char :1;
  };
} __CMCON0_bits_t;
extern volatile __CMCON0_bits_t __at(CMCON0_ADDR) CMCON0_bits;

#define CM0                  CMCON0_bits.CM0
#define CM1                  CMCON0_bits.CM1
#define CM2                  CMCON0_bits.CM2
#define CIS                  CMCON0_bits.CIS
#define CINV                 CMCON0_bits.CINV
#define COUT                 CMCON0_bits.COUT

// ----- CMCON1 bits --------------------
typedef union {
  struct {
    unsigned char CMSYNC:1;
    unsigned char T1GSS:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
  };
} __CMCON1_bits_t;
extern volatile __CMCON1_bits_t __at(CMCON1_ADDR) CMCON1_bits;

#define CMSYNC               CMCON1_bits.CMSYNC
#define T1GSS                CMCON1_bits.T1GSS

// ----- EECON1 bits --------------------
typedef union {
  struct {
    unsigned char RD:1;
    unsigned char WR:1;
    unsigned char WREN:1;
    unsigned char WRERR:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
  };
} __EECON1_bits_t;
extern volatile __EECON1_bits_t __at(EECON1_ADDR) EECON1_bits;

#define RD                   EECON1_bits.RD
#define WR                   EECON1_bits.WR
#define WREN                 EECON1_bits.WREN
#define WRERR                EECON1_bits.WRERR

// ----- GPIO bits --------------------
typedef union {
  struct {
    unsigned char GP0:1;
    unsigned char GP1:1;
    unsigned char GP2:1;
    unsigned char GP3:1;
    unsigned char GP4:1;
    unsigned char GP5:1;
    unsigned char :1;
    unsigned char :1;
  };
  struct {
    unsigned char GPIO0:1;
    unsigned char GPIO1:1;
    unsigned char GPIO2:1;
    unsigned char GPIO3:1;
    unsigned char GPIO4:1;
    unsigned char GPIO5:1;
    unsigned char :1;
    unsigned char :1;
  };
} __GPIO_bits_t;
extern volatile __GPIO_bits_t __at(GPIO_ADDR) GPIO_bits;

#define GP0                  GPIO_bits.GP0
#define GPIO0                GPIO_bits.GPIO0
#define GP1                  GPIO_bits.GP1
#define GPIO1                GPIO_bits.GPIO1
#define GP2                  GPIO_bits.GP2
#define GPIO2                GPIO_bits.GPIO2
#define GP3                  GPIO_bits.GP3
#define GPIO3                GPIO_bits.GPIO3
#define GP4                  GPIO_bits.GP4
#define GPIO4                GPIO_bits.GPIO4
#define GP5                  GPIO_bits.GP5
#define GPIO5                GPIO_bits.GPIO5

// ----- INTCON bits --------------------
typedef union {
  struct {
    unsigned char GPIF:1;
    unsigned char INTF:1;
    unsigned char T0IF:1;
    unsigned char GPIE:1;
    unsigned char INTE:1;
    unsigned char T0IE:1;
    unsigned char PEIE:1;
    unsigned char GIE:1;
  };
} __INTCON_bits_t;
extern volatile __INTCON_bits_t __at(INTCON_ADDR) INTCON_bits;

#define GPIF                 INTCON_bits.GPIF
#define INTF                 INTCON_bits.INTF
#define T0IF                 INTCON_bits.T0IF
#define GPIE                 INTCON_bits.GPIE
#define INTE                 INTCON_bits.INTE
#define T0IE                 INTCON_bits.T0IE
#define PEIE                 INTCON_bits.PEIE
#define GIE                  INTCON_bits.GIE

// ----- IOC bits --------------------
typedef union {
  struct {
    unsigned char IOC0:1;
    unsigned char IOC1:1;
    unsigned char IOC2:1;
    unsigned char IOC3:1;
    unsigned char IOC4:1;
    unsigned char IOC5:1;
    unsigned char :1;
    unsigned char :1;
  };
} __IOC_bits_t;
extern volatile __IOC_bits_t __at(IOC_ADDR) IOC_bits;

#define IOC0                 IOC_bits.IOC0
#define IOC1                 IOC_bits.IOC1
#define IOC2                 IOC_bits.IOC2
#define IOC3                 IOC_bits.IOC3
#define IOC4                 IOC_bits.IOC4
#define IOC5                 IOC_bits.IOC5

// ----- IOCA bits --------------------
typedef union {
  struct {
    unsigned char IOCA0:1;
    unsigned char IOCA1:1;
    unsigned char IOCA2:1;
    unsigned char IOCA3:1;
    unsigned char IOCA4:1;
    unsigned char IOCA5:1;
    unsigned char :1;
    unsigned char :1;
  };
} __IOCA_bits_t;
extern volatile __IOCA_bits_t __at(IOCA_ADDR) IOCA_bits;

#define IOCA0                IOCA_bits.IOCA0
#define IOCA1                IOCA_bits.IOCA1
#define IOCA2                IOCA_bits.IOCA2
#define IOCA3                IOCA_bits.IOCA3
#define IOCA4                IOCA_bits.IOCA4
#define IOCA5                IOCA_bits.IOCA5

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
    unsigned char NOT_GPPU:1;
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
#define NOT_GPPU             OPTION_REG_bits.NOT_GPPU

// ----- OSCCON bits --------------------
typedef union {
  struct {
    unsigned char SCS:1;
    unsigned char LTS:1;
    unsigned char HTS:1;
    unsigned char OSTS:1;
    unsigned char IRCF0:1;
    unsigned char IRCF1:1;
    unsigned char IRCF2:1;
    unsigned char :1;
  };
} __OSCCON_bits_t;
extern volatile __OSCCON_bits_t __at(OSCCON_ADDR) OSCCON_bits;

#define SCS                  OSCCON_bits.SCS
#define LTS                  OSCCON_bits.LTS
#define HTS                  OSCCON_bits.HTS
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
    unsigned char :1;
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

// ----- PCON bits --------------------
typedef union {
  struct {
    unsigned char NOT_BOD:1;
    unsigned char NOT_POR:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char SBODEN:1;
    unsigned char ULPWUE:1;
    unsigned char :1;
    unsigned char :1;
  };
} __PCON_bits_t;
extern volatile __PCON_bits_t __at(PCON_ADDR) PCON_bits;

#define NOT_BOD              PCON_bits.NOT_BOD
#define NOT_POR              PCON_bits.NOT_POR
#define SBODEN               PCON_bits.SBODEN
#define ULPWUE               PCON_bits.ULPWUE

// ----- PIE1 bits --------------------
typedef union {
  struct {
    unsigned char T1IE:1;
    unsigned char T2IE:1;
    unsigned char OSFIE:1;
    unsigned char CMIE:1;
    unsigned char :1;
    unsigned char CCP1IE:1;
    unsigned char ADIE:1;
    unsigned char EEIE:1;
  };
  struct {
    unsigned char TMR1IE:1;
    unsigned char TMR2IE:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
  };
} __PIE1_bits_t;
extern volatile __PIE1_bits_t __at(PIE1_ADDR) PIE1_bits;

#define T1IE                 PIE1_bits.T1IE
#define TMR1IE               PIE1_bits.TMR1IE
#define T2IE                 PIE1_bits.T2IE
#define TMR2IE               PIE1_bits.TMR2IE
#define OSFIE                PIE1_bits.OSFIE
#define CMIE                 PIE1_bits.CMIE
#define CCP1IE               PIE1_bits.CCP1IE
#define ADIE                 PIE1_bits.ADIE
#define EEIE                 PIE1_bits.EEIE

// ----- PIR1 bits --------------------
typedef union {
  struct {
    unsigned char T1IF:1;
    unsigned char T2IF:1;
    unsigned char OSFIF:1;
    unsigned char CMIF:1;
    unsigned char :1;
    unsigned char CCP1IF:1;
    unsigned char ADIF:1;
    unsigned char EEIF:1;
  };
  struct {
    unsigned char TMR1IF:1;
    unsigned char TMR2IF:1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
    unsigned char :1;
  };
} __PIR1_bits_t;
extern volatile __PIR1_bits_t __at(PIR1_ADDR) PIR1_bits;

#define T1IF                 PIR1_bits.T1IF
#define TMR1IF               PIR1_bits.TMR1IF
#define T2IF                 PIR1_bits.T2IF
#define TMR2IF               PIR1_bits.TMR2IF
#define OSFIF                PIR1_bits.OSFIF
#define CMIF                 PIR1_bits.CMIF
#define CCP1IF               PIR1_bits.CCP1IF
#define ADIF                 PIR1_bits.ADIF
#define EEIF                 PIR1_bits.EEIF

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
    unsigned char T1GE:1;
    unsigned char T1GINV:1;
  };
} __T1CON_bits_t;
extern volatile __T1CON_bits_t __at(T1CON_ADDR) T1CON_bits;

#define TMR1ON               T1CON_bits.TMR1ON
#define TMR1CS               T1CON_bits.TMR1CS
#define NOT_T1SYNC           T1CON_bits.NOT_T1SYNC
#define T1OSCEN              T1CON_bits.T1OSCEN
#define T1CKPS0              T1CON_bits.T1CKPS0
#define T1CKPS1              T1CON_bits.T1CKPS1
#define T1GE                 T1CON_bits.T1GE
#define T1GINV               T1CON_bits.T1GINV

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

// ----- VRCON bits --------------------
typedef union {
  struct {
    unsigned char VR0:1;
    unsigned char VR1:1;
    unsigned char VR2:1;
    unsigned char VR3:1;
    unsigned char :1;
    unsigned char VRR:1;
    unsigned char :1;
    unsigned char VREN:1;
  };
} __VRCON_bits_t;
extern volatile __VRCON_bits_t __at(VRCON_ADDR) VRCON_bits;

#define VR0                  VRCON_bits.VR0
#define VR1                  VRCON_bits.VR1
#define VR2                  VRCON_bits.VR2
#define VR3                  VRCON_bits.VR3
#define VRR                  VRCON_bits.VRR
#define VREN                 VRCON_bits.VREN

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
} __WDTCON_bits_t;
extern volatile __WDTCON_bits_t __at(WDTCON_ADDR) WDTCON_bits;

#define SWDTEN               WDTCON_bits.SWDTEN
#define WDTPS0               WDTCON_bits.WDTPS0
#define WDTPS1               WDTCON_bits.WDTPS1
#define WDTPS2               WDTCON_bits.WDTPS2
#define WDTPS3               WDTCON_bits.WDTPS3

#endif
