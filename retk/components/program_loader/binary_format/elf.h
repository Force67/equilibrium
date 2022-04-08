//< Copyright (C) 2021 Force67 <github.com/Force67>.
//< For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <memory>
#include <base/arch.h>
#include <program_loader/file_classifier.h>

namespace program_loader {
// https://github.com/llvm/llvm-project/blob/d480f968ad8b56d3ee4a6b6df5532d485b0ad01e/llvm/include/llvm/BinaryFormat/ELF.h
static constexpr const char kElfMagic[] = {0x7f, 'E', 'L', 'F', '\0'};

enum class ElfMachineType {
  kNONE = 0,             //< No machine
  kM32 = 1,              //< AT&T WE 32100
  kSPARC = 2,            //< SPARC
  k386 = 3,              //< Intel 386
  k68K = 4,              //< Motorola 68000
  k88K = 5,              //< Motorola 88000
  kIAMCU = 6,            //< Intel MCU
  k860 = 7,              //< Intel 80860
  kMIPS = 8,             //< MIPS R3000
  kS370 = 9,             //< IBM System/370
  kMIPS_RS3_LE = 10,     //< MIPS RS3000 Little-endian
  kPARISC = 15,          //< Hewlett-Packard PA-RISC
  kVPP500 = 17,          //< Fujitsu VPP500
  kSPARC32PLUS = 18,     //< Enhanced instruction set SPARC
  k960 = 19,             //< Intel 80960
  kPPC = 20,             //< PowerPC
  kPPC64 = 21,           //< PowerPC64
  kS390 = 22,            //< IBM System/390
  kSPU = 23,             //< IBM SPU/SPC
  kV800 = 36,            //< NEC V800
  kFR20 = 37,            //< Fujitsu FR20
  kRH32 = 38,            //< TRW RH-32
  kRCE = 39,             //< Motorola RCE
  kARM = 40,             //< ARM
  kALPHA = 41,           //< DEC Alpha
  kSH = 42,              //< Hitachi SH
  kSPARCV9 = 43,         //< SPARC V9
  kTRICORE = 44,         //< Siemens TriCore
  kARC = 45,             //< Argonaut RISC Core
  kH8_300 = 46,          //< Hitachi H8/300
  kH8_300H = 47,         //< Hitachi H8/300H
  kH8S = 48,             //< Hitachi H8S
  kH8_500 = 49,          //< Hitachi H8/500
  kIA_64 = 50,           //< Intel IA-64 processor architecture
  kMIPS_X = 51,          //< Stanford MIPS-X
  kCOLDFIRE = 52,        //< Motorola ColdFire
  k68HC12 = 53,          //< Motorola M68HC12
  kMMA = 54,             //< Fujitsu MMA Multimedia Accelerator
  kPCP = 55,             //< Siemens PCP
  kNCPU = 56,            //< Sony nCPU embedded RISC processor
  kNDR1 = 57,            //< Denso NDR1 microprocessor
  kSTARCORE = 58,        //< Motorola Star*Core processor
  kME16 = 59,            //< Toyota ME16 processor
  kST100 = 60,           //< STMicroelectronics ST100 processor
  kTINYJ = 61,           //< Advanced Logic Corp. TinyJ embedded processor family
  kX86_64 = 62,          //< AMD x86-64 architecture
  kPDSP = 63,            //< Sony DSP Processor
  kPDP10 = 64,           //< Digital Equipment Corp. PDP-10
  kPDP11 = 65,           //< Digital Equipment Corp. PDP-11
  kFX66 = 66,            //< Siemens FX66 microcontroller
  kST9PLUS = 67,         //< STMicroelectronics ST9+ 8/16 bit microcontroller
  kST7 = 68,             //< STMicroelectronics ST7 8-bit microcontroller
  k68HC16 = 69,          //< Motorola MC68HC16 Microcontroller
  k68HC11 = 70,          //< Motorola MC68HC11 Microcontroller
  k68HC08 = 71,          //< Motorola MC68HC08 Microcontroller
  k68HC05 = 72,          //< Motorola MC68HC05 Microcontroller
  kSVX = 73,             //< Silicon Graphics SVx
  kST19 = 74,            //< STMicroelectronics ST19 8-bit microcontroller
  kVAX = 75,             //< Digital VAX
  kCRIS = 76,            //< Axis Communications 32-bit embedded processor
  kJAVELIN = 77,         //< Infineon Technologies 32-bit embedded processor
  kFIREPATH = 78,        //< Element 14 64-bit DSP Processor
  kZSP = 79,             //< LSI Logic 16-bit DSP Processor
  kMMIX = 80,            //< Donald Knuth's educational 64-bit processor
  kHUANY = 81,           //< Harvard University machine-independent object files
  kPRISM = 82,           //< SiTera Prism
  kAVR = 83,             //< Atmel AVR 8-bit microcontroller
  kFR30 = 84,            //< Fujitsu FR30
  kD10V = 85,            //< Mitsubishi D10V
  kD30V = 86,            //< Mitsubishi D30V
  kV850 = 87,            //< NEC v850
  kM32R = 88,            //< Mitsubishi M32R
  kMN10300 = 89,         //< Matsushita MN10300
  kMN10200 = 90,         //< Matsushita MN10200
  kPJ = 91,              //< picoJava
  kOPENRISC = 92,        //< OpenRISC 32-bit embedded processor
  kARC_COMPACT = 93,     //< ARC International ARCompact processor (old
                           //< spelling/synonym: kARC_A5)
  kXTENSA = 94,          //< Tensilica Xtensa Architecture
  kVIDEOCORE = 95,       //< Alphamosaic VideoCore processor
  kTMM_GPP = 96,         //< Thompson Multimedia General Purpose Processor
  kNS32K = 97,           //< National Semiconductor 32000 series
  kTPC = 98,             //< Tenor Network TPC processor
  kSNP1K = 99,           //< Trebia SNP 1000 processor
  kST200 = 100,          //< STMicroelectronics (www.st.com) ST200
  kIP2K = 101,           //< Ubicom IP2xxx microcontroller family
  kMAX = 102,            //< MAX Processor
  kCR = 103,             //< National Semiconductor CompactRISC microprocessor
  kF2MC16 = 104,         //< Fujitsu F2MC16
  kMSP430 = 105,         //< Texas Instruments embedded microcontroller msp430
  kBLACKFIN = 106,       //< Analog Devices Blackfin (DSP) processor
  kSE_C33 = 107,         //< S1C33 Family of Seiko Epson processors
  kSEP = 108,            //< Sharp embedded microprocessor
  kARCA = 109,           //< Arca RISC Microprocessor
  kUNICORE = 110,        //< Microprocessor series from PKU-Unity Ltd. and MPRC
                           //< of Peking University
  kEXCESS = 111,         //< eXcess: 16/32/64-bit configurable embedded CPU
  kDXP = 112,            //< Icera Semiconductor Inc. Deep Execution Processor
  kALTERA_NIOS2 = 113,   //< Altera Nios II soft-core processor
  kCRX = 114,            //< National Semiconductor CompactRISC CRX
  kXGATE = 115,          //< Motorola XGATE embedded processor
  kC166 = 116,           //< Infineon C16x/XC16x processor
  kM16C = 117,           //< Renesas M16C series microprocessors
  kDSPIC30F = 118,       //< Microchip Technology dsPIC30F Digital Signal
                           //< Controller
  kCE = 119,             //< Freescale Communication Engine RISC core
  kM32C = 120,           //< Renesas M32C series microprocessors
  kTSK3000 = 131,        //< Altium TSK3000 core
  kRS08 = 132,           //< Freescale RS08 embedded processor
  kSHARC = 133,          //< Analog Devices SHARC family of 32-bit DSP
                           //< processors
  kECOG2 = 134,          //< Cyan Technology eCOG2 microprocessor
  kSCORE7 = 135,         //< Sunplus S+core7 RISC processor
  kDSP24 = 136,          //< New Japan Radio (NJR) 24-bit DSP Processor
  kVIDEOCORE3 = 137,     //< Broadcom VideoCore III processor
  kLATTICEMICO32 = 138,  //< RISC processor for Lattice FPGA architecture
  kSE_C17 = 139,         //< Seiko Epson C17 family
  kTI_C6000 = 140,       //< The Texas Instruments TMS320C6000 DSP family
  kTI_C2000 = 141,       //< The Texas Instruments TMS320C2000 DSP family
  kTI_C5500 = 142,       //< The Texas Instruments TMS320C55x DSP family
  kMMDSP_PLUS = 160,     //< STMicroelectronics 64bit VLIW Data Signal Processor
  kCYPRESS_M8C = 161,    //< Cypress M8C microprocessor
  kR32C = 162,           //< Renesas R32C series microprocessors
  kTRIMEDIA = 163,       //< NXP Semiconductors TriMedia architecture family
  kHEXAGON = 164,        //< Qualcomm Hexagon processor
  k8051 = 165,           //< Intel 8051 and variants
  kSTXP7X = 166,         //< STMicroelectronics STxP7x family of configurable
                           //< and extensible RISC processors
  kNDS32 = 167,          //< Andes Technology compact code size embedded RISC
                           //< processor family
  kECOG1 = 168,          //< Cyan Technology eCOG1X family
  kECOG1X = 168,         //< Cyan Technology eCOG1X family
  kMAXQ30 = 169,         //< Dallas Semiconductor MAXQ30 Core Micro-controllers
  kXIMO16 = 170,         //< New Japan Radio (NJR) 16-bit DSP Processor
  kMANIK = 171,          //< M2000 Reconfigurable RISC Microprocessor
  kCRAYNV2 = 172,        //< Cray Inc. NV2 vector architecture
  kRX = 173,             //< Renesas RX family
  kMETAG = 174,          //< Imagination Technologies META processor
                           //< architecture
  kMCST_ELBRUS = 175,    //< MCST Elbrus general purpose hardware architecture
  kECOG16 = 176,         //< Cyan Technology eCOG16 family
  kCR16 = 177,           //< National Semiconductor CompactRISC CR16 16-bit
                           //< microprocessor
  kETPU = 178,           //< Freescale Extended Time Processing Unit
  kSLE9X = 179,          //< Infineon Technologies SLE9X core
  kL10M = 180,           //< Intel L10M
  kK10M = 181,           //< Intel K10M
  kAARCH64 = 183,        //< ARM AArch64
  kAVR32 = 185,          //< Atmel Corporation 32-bit microprocessor family
  kSTM8 = 186,           //< STMicroeletronics STM8 8-bit microcontroller
  kTILE64 = 187,         //< Tilera TILE64 multicore architecture family
  kTILEPRO = 188,        //< Tilera TILEPro multicore architecture family
  kCUDA = 190,           //< NVIDIA CUDA architecture
  kTILEGX = 191,         //< Tilera TILE-Gx multicore architecture family
  kCLOUDSHIELD = 192,    //< CloudShield architecture family
  kCOREA_1ST = 193,      //< KIPO-KAIST Core-A 1st generation processor family
  kCOREA_2ND = 194,      //< KIPO-KAIST Core-A 2nd generation processor family
  kARC_COMPACT2 = 195,   //< Synopsys ARCompact V2
  kOPEN8 = 196,          //< Open8 8-bit RISC soft processor core
  kRL78 = 197,           //< Renesas RL78 family
  kVIDEOCORE5 = 198,     //< Broadcom VideoCore V processor
  k78KOR = 199,          //< Renesas 78KOR family
  k56800EX = 200,        //< Freescale 56800EX Digital Signal Controller (DSC)
  kBA1 = 201,            //< Beyond BA1 CPU architecture
  kBA2 = 202,            //< Beyond BA2 CPU architecture
  kXCORE = 203,          //< XMOS xCORE processor family
  kMCHP_PIC = 204,       //< Microchip 8-bit PIC(r) family
  kINTEL205 = 205,       //< Reserved by Intel
  kINTEL206 = 206,       //< Reserved by Intel
  kINTEL207 = 207,       //< Reserved by Intel
  kINTEL208 = 208,       //< Reserved by Intel
  kINTEL209 = 209,       //< Reserved by Intel
  kKM32 = 210,           //< KM211 KM32 32-bit processor
  kKMX32 = 211,          //< KM211 KMX32 32-bit processor
  kKMX16 = 212,          //< KM211 KMX16 16-bit processor
  kKMX8 = 213,           //< KM211 KMX8 8-bit processor
  kKVARC = 214,          //< KM211 KVARC processor
  kCDP = 215,            //< Paneve CDP architecture family
  kCOGE = 216,           //< Cognitive Smart Memory Processor
  kCOOL = 217,           //< iCelero CoolEngine
  kNORC = 218,           //< Nanoradio Optimized RISC
  kCSR_KALIMBA = 219,    //< CSR Kalimba architecture family
  kAMDGPU = 224,         //< AMD GPU architecture
  kRISCV = 243,          //< RISC-V
  kLANAI = 244,          //< Lanai 32-bit processor
  kBPF = 247,            //< Linux kernel bpf virtual machine
  kVE = 251,             //< NEC SX-Aurora VE
  kCSKY = 252,           //< C-SKY 32-bit processor
};

struct ElfHeader {
  byte ident[16];  //< ELF Identification bytes
  u16 type;
  ElfMachineType machine;

  bool checkMagic() const {
    return (memcmp(ident, kElfMagic, strlen(kElfMagic))) == 0;
  }
};
}  // namespace program_loader
