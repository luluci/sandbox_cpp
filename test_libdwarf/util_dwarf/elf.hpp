#pragma once

#include <libdwarf.h>

namespace util_dwarf {

namespace elf {

struct machine_architecture
{
    Dwarf_Small ftype;
    Dwarf_Small obj_pointersize;
    Dwarf_Bool obj_is_big_endian;
    Dwarf_Unsigned obj_machine; /*architecture*/
    Dwarf_Unsigned obj_flags;
    Dwarf_Small path_source;
    Dwarf_Unsigned ub_offset;
    Dwarf_Unsigned ub_count;
    Dwarf_Unsigned ub_index;
    Dwarf_Unsigned comdat_groupnumber;
};

enum architecture
{
    EM_NONE        = 0,      // No machine
    EM_M32         = 1,      // AT&T WE 32100
    EM_SPARC       = 2,      // SPARC
    EM_386         = 3,      // Intel 80386
    EM_68K         = 4,      // Motorola 68000
    EM_88K         = 5,      // Motorola 88000
    EM_IAMCU       = 6,      // Intel MCU
    EM_860         = 7,      // Intel 80860
    EM_MIPS        = 8,      // MIPS I Architecture
    EM_S370        = 9,      // IBM System/370 Processor
    EM_MIPS_RS3_LE = 10,     // MIPS RS3000 Little-endian
    reserved_11,             // 11-14	Reserved for future use
    reserved_12,             // 11-14	Reserved for future use
    reserved_13,             // 11-14	Reserved for future use
    reserved_14,             // 11-14	Reserved for future use
    EM_PARISC      = 15,     // Hewlett-Packard PA-RISC
    reserved       = 16,     // Reserved for future use
    EM_VPP500      = 17,     // Fujitsu VPP500
    EM_SPARC32PLUS = 18,     // Enhanced instruction set SPARC
    EM_960         = 19,     // Intel 80960
    EM_PPC         = 20,     // PowerPC
    EM_PPC64       = 21,     // 64-bit PowerPC
    EM_S390        = 22,     // IBM System/390 Processor
    EM_SPU         = 23,     // IBM SPU/SPC
    reserved_24,             // 24-35	Reserved for future use
    reserved_25,             // 24-35	Reserved for future use
    reserved_26,             // 24-35	Reserved for future use
    reserved_27,             // 24-35	Reserved for future use
    reserved_28,             // 24-35	Reserved for future use
    reserved_29,             // 24-35	Reserved for future use
    reserved_30,             // 24-35	Reserved for future use
    reserved_31,             // 24-35	Reserved for future use
    reserved_32,             // 24-35	Reserved for future use
    reserved_33,             // 24-35	Reserved for future use
    reserved_34,             // 24-35	Reserved for future use
    reserved_35,             // 24-35	Reserved for future use
    EM_V800         = 36,    // NEC V800
    EM_FR20         = 37,    // Fujitsu FR20
    EM_RH32         = 38,    // TRW RH-32
    EM_RCE          = 39,    // Motorola RCE
    EM_ARM          = 40,    // ARM 32-bit architecture (AARCH32)
    EM_ALPHA        = 41,    // Digital Alpha
    EM_SH           = 42,    // Hitachi SH
    EM_SPARCV9      = 43,    // SPARC Version 9
    EM_TRICORE      = 44,    // Siemens TriCore embedded processor
    EM_ARC          = 45,    // Argonaut RISC Core, Argonaut Technologies Inc.
    EM_H8_300       = 46,    // Hitachi H8/300
    EM_H8_300H      = 47,    // Hitachi H8/300H
    EM_H8S          = 48,    // Hitachi H8S
    EM_H8_500       = 49,    // Hitachi H8/500
    EM_IA_64        = 50,    // Intel IA-64 processor architecture
    EM_MIPS_X       = 51,    // Stanford MIPS-X
    EM_COLDFIRE     = 52,    // Motorola ColdFire
    EM_68HC12       = 53,    // Motorola M68HC12
    EM_MMA          = 54,    // Fujitsu MMA Multimedia Accelerator
    EM_PCP          = 55,    // Siemens PCP
    EM_NCPU         = 56,    // Sony nCPU embedded RISC processor
    EM_NDR1         = 57,    // Denso NDR1 microprocessor
    EM_STARCORE     = 58,    // Motorola Star*Core processor
    EM_ME16         = 59,    // Toyota ME16 processor
    EM_ST100        = 60,    // STMicroelectronics ST100 processor
    EM_TINYJ        = 61,    // Advanced Logic Corp. TinyJ embedded processor family
    EM_X86_64       = 62,    // AMD x86-64 architecture
    EM_PDSP         = 63,    // Sony DSP Processor
    EM_PDP10        = 64,    // Digital Equipment Corp. PDP-10
    EM_PDP11        = 65,    // Digital Equipment Corp. PDP-11
    EM_FX66         = 66,    // Siemens FX66 microcontroller
    EM_ST9PLUS      = 67,    // STMicroelectronics ST9+ 8/16 bit microcontroller
    EM_ST7          = 68,    // STMicroelectronics ST7 8-bit microcontroller
    EM_68HC16       = 69,    // Motorola MC68HC16 Microcontroller
    EM_68HC11       = 70,    // Motorola MC68HC11 Microcontroller
    EM_68HC08       = 71,    // Motorola MC68HC08 Microcontroller
    EM_68HC05       = 72,    // Motorola MC68HC05 Microcontroller
    EM_SVX          = 73,    // Silicon Graphics SVx
    EM_ST19         = 74,    // STMicroelectronics ST19 8-bit microcontroller
    EM_VAX          = 75,    // Digital VAX
    EM_CRIS         = 76,    // Axis Communications 32-bit embedded processor
    EM_JAVELIN      = 77,    // Infineon Technologies 32-bit embedded processor
    EM_FIREPATH     = 78,    // Element 14 64-bit DSP Processor
    EM_ZSP          = 79,    // LSI Logic 16-bit DSP Processor
    EM_MMIX         = 80,    // Donald Knuth's educational 64-bit processor
    EM_HUANY        = 81,    // Harvard University machine-independent object files
    EM_PRISM        = 82,    // SiTera Prism
    EM_AVR          = 83,    // Atmel AVR 8-bit microcontroller
    EM_FR30         = 84,    // Fujitsu FR30
    EM_D10V         = 85,    // Mitsubishi D10V
    EM_D30V         = 86,    // Mitsubishi D30V
    EM_V850         = 87,    // NEC v850
    EM_M32R         = 88,    // Mitsubishi M32R
    EM_MN10300      = 89,    // Matsushita MN10300
    EM_MN10200      = 90,    // Matsushita MN10200
    EM_PJ           = 91,    // picoJava
    EM_OPENRISC     = 92,    // OpenRISC 32-bit embedded processor
    EM_ARC_COMPACT  = 93,    // ARC International ARCompact processor (old spelling/synonym: EM_ARC_A5)
    EM_XTENSA       = 94,    // Tensilica Xtensa Architecture
    EM_VIDEOCORE    = 95,    // Alphamosaic VideoCore processor
    EM_TMM_GPP      = 96,    // Thompson Multimedia General Purpose Processor
    EM_NS32K        = 97,    // National Semiconductor 32000 series
    EM_TPC          = 98,    // Tenor Network TPC processor
    EM_SNP1K        = 99,    // Trebia SNP 1000 processor
    EM_ST200        = 100,   // STMicroelectronics (www.st.com) ST200 microcontroller
    EM_IP2K         = 101,   // Ubicom IP2xxx microcontroller family
    EM_MAX          = 102,   // MAX Processor
    EM_CR           = 103,   // National Semiconductor CompactRISC microprocessor
    EM_F2MC16       = 104,   // Fujitsu F2MC16
    EM_MSP430       = 105,   // Texas Instruments embedded microcontroller msp430
    EM_BLACKFIN     = 106,   // Analog Devices Blackfin (DSP) processor
    EM_SE_C33       = 107,   // S1C33 Family of Seiko Epson processors
    EM_SEP          = 108,   // Sharp embedded microprocessor
    EM_ARCA         = 109,   // Arca RISC Microprocessor
    EM_UNICORE      = 110,   // Microprocessor series from PKU-Unity Ltd. and MPRC of Peking University
    EM_EXCESS       = 111,   // eXcess: 16/32/64-bit configurable embedded CPU
    EM_DXP          = 112,   // Icera Semiconductor Inc. Deep Execution Processor
    EM_ALTERA_NIOS2 = 113,   // Altera Nios II soft-core processor
    EM_CRX          = 114,   // National Semiconductor CompactRISC CRX microprocessor
    EM_XGATE        = 115,   // Motorola XGATE embedded processor
    EM_C166         = 116,   // Infineon C16x/XC16x processor
    EM_M16C         = 117,   // Renesas M16C series microprocessors
    EM_DSPIC30F     = 118,   // Microchip Technology dsPIC30F Digital Signal Controller
    EM_CE           = 119,   // Freescale Communication Engine RISC core
    EM_M32C         = 120,   // Renesas M32C series microprocessors
    reserved_121,            // 121-130	Reserved for future use
    reserved_122,            // 121-130	Reserved for future use
    reserved_123,            // 121-130	Reserved for future use
    reserved_124,            // 121-130	Reserved for future use
    reserved_125,            // 121-130	Reserved for future use
    reserved_126,            // 121-130	Reserved for future use
    reserved_127,            // 121-130	Reserved for future use
    reserved_128,            // 121-130	Reserved for future use
    reserved_129,            // 121-130	Reserved for future use
    reserved_130,            // 121-130	Reserved for future use
    EM_TSK3000       = 131,  // Altium TSK3000 core
    EM_RS08          = 132,  // Freescale RS08 embedded processor
    EM_SHARC         = 133,  // Analog Devices SHARC family of 32-bit DSP processors
    EM_ECOG2         = 134,  // Cyan Technology eCOG2 microprocessor
    EM_SCORE7        = 135,  // Sunplus S+core7 RISC processor
    EM_DSP24         = 136,  // New Japan Radio (NJR) 24-bit DSP Processor
    EM_VIDEOCORE3    = 137,  // Broadcom VideoCore III processor
    EM_LATTICEMICO32 = 138,  // RISC processor for Lattice FPGA architecture
    EM_SE_C17        = 139,  // Seiko Epson C17 family
    EM_TI_C6000      = 140,  // The Texas Instruments TMS320C6000 DSP family
    EM_TI_C2000      = 141,  // The Texas Instruments TMS320C2000 DSP family
    EM_TI_C5500      = 142,  // The Texas Instruments TMS320C55x DSP family
    EM_TI_ARP32      = 143,  // Texas Instruments Application Specific RISC Processor, 32bit fetch
    EM_TI_PRU        = 144,  // Texas Instruments Programmable Realtime Unit
    reserved_145,            // 145-159	Reserved for future use
    reserved_146,            // 145-159	Reserved for future use
    reserved_147,            // 145-159	Reserved for future use
    reserved_148,            // 145-159	Reserved for future use
    reserved_149,            // 145-159	Reserved for future use
    reserved_150,            // 145-159	Reserved for future use
    reserved_151,            // 145-159	Reserved for future use
    reserved_152,            // 145-159	Reserved for future use
    reserved_153,            // 145-159	Reserved for future use
    reserved_154,            // 145-159	Reserved for future use
    reserved_155,            // 145-159	Reserved for future use
    reserved_156,            // 145-159	Reserved for future use
    reserved_157,            // 145-159	Reserved for future use
    reserved_158,            // 145-159	Reserved for future use
    reserved_159,            // 145-159	Reserved for future use
    EM_MMDSP_PLUS   = 160,   // STMicroelectronics 64bit VLIW Data Signal Processor
    EM_CYPRESS_M8C  = 161,   // Cypress M8C microprocessor
    EM_R32C         = 162,   // Renesas R32C series microprocessors
    EM_TRIMEDIA     = 163,   // NXP Semiconductors TriMedia architecture family
    EM_QDSP6        = 164,   // QUALCOMM DSP6 Processor
    EM_8051         = 165,   // Intel 8051 and variants
    EM_STXP7X       = 166,   // STMicroelectronics STxP7x family of configurable and extensible RISC processors
    EM_NDS32        = 167,   // Andes Technology compact code size embedded RISC processor family
    EM_ECOG1        = 168,   // Cyan Technology eCOG1X family
    EM_ECOG1X       = 168,   // Cyan Technology eCOG1X family
    EM_MAXQ30       = 169,   // Dallas Semiconductor MAXQ30 Core Micro-controllers
    EM_XIMO16       = 170,   // New Japan Radio (NJR) 16-bit DSP Processor
    EM_MANIK        = 171,   // M2000 Reconfigurable RISC Microprocessor
    EM_CRAYNV2      = 172,   // Cray Inc. NV2 vector architecture
    EM_RX           = 173,   // Renesas RX family
    EM_METAG        = 174,   // Imagination Technologies META processor architecture
    EM_MCST_ELBRUS  = 175,   // MCST Elbrus general purpose hardware architecture
    EM_ECOG16       = 176,   // Cyan Technology eCOG16 family
    EM_CR16         = 177,   // National Semiconductor CompactRISC CR16 16-bit microprocessor
    EM_ETPU         = 178,   // Freescale Extended Time Processing Unit
    EM_SLE9X        = 179,   // Infineon Technologies SLE9X core
    EM_L10M         = 180,   // Intel L10M
    EM_K10M         = 181,   // Intel K10M
    reserved_182    = 182,   // Reserved for future Intel use
    EM_AARCH64      = 183,   // ARM 64-bit architecture (AARCH64)
    reserved_184    = 184,   // Reserved for future ARM use
    EM_AVR32        = 185,   // Atmel Corporation 32-bit microprocessor family
    EM_STM8         = 186,   // STMicroeletronics STM8 8-bit microcontroller
    EM_TILE64       = 187,   // Tilera TILE64 multicore architecture family
    EM_TILEPRO      = 188,   // Tilera TILEPro multicore architecture family
    EM_MICROBLAZE   = 189,   // Xilinx MicroBlaze 32-bit RISC soft processor core
    EM_CUDA         = 190,   // NVIDIA CUDA architecture
    EM_TILEGX       = 191,   // Tilera TILE-Gx multicore architecture family
    EM_CLOUDSHIELD  = 192,   // CloudShield architecture family
    EM_COREA_1ST    = 193,   // KIPO-KAIST Core-A 1st generation processor family
    EM_COREA_2ND    = 194,   // KIPO-KAIST Core-A 2nd generation processor family
    EM_ARC_COMPACT2 = 195,   // Synopsys ARCompact V2
    EM_OPEN8        = 196,   // Open8 8-bit RISC soft processor core
    EM_RL78         = 197,   // Renesas RL78 family
    EM_VIDEOCORE5   = 198,   // Broadcom VideoCore V processor
    EM_78KOR        = 199,   // Renesas 78KOR family
    EM_56800EX      = 200,   // Freescale 56800EX Digital Signal Controller (DSC)
    EM_BA1          = 201,   // Beyond BA1 CPU architecture
    EM_BA2          = 202,   // Beyond BA2 CPU architecture
    EM_XCORE        = 203,   // XMOS xCORE processor family
    EM_MCHP_PIC     = 204,   // Microchip 8-bit PIC(r) family
    EM_INTEL205     = 205,   // Reserved by Intel
    EM_INTEL206     = 206,   // Reserved by Intel
    EM_INTEL207     = 207,   // Reserved by Intel
    EM_INTEL208     = 208,   // Reserved by Intel
    EM_INTEL209     = 209,   // Reserved by Intel
    EM_KM32         = 210,   // KM211 KM32 32-bit processor
    EM_KMX32        = 211,   // KM211 KMX32 32-bit processor
    EM_KMX16        = 212,   // KM211 KMX16 16-bit processor
    EM_KMX8         = 213,   // KM211 KMX8 8-bit processor
    EM_KVARC        = 214,   // KM211 KVARC processor
    EM_CDP          = 215,   // Paneve CDP architecture family
    EM_COGE         = 216,   // Cognitive Smart Memory Processor
    EM_COOL         = 217,   // Bluechip Systems CoolEngine
    EM_NORC         = 218,   // Nanoradio Optimized RISC
    EM_CSR_KALIMBA  = 219,   // CSR Kalimba architecture family
    EM_Z80          = 220,   // Zilog Z80
    EM_VISIUM       = 221,   // Controls and Data Services VISIUMcore processor
    EM_FT32         = 222,   // FTDI Chip FT32 high performance 32-bit RISC architecture
    EM_MOXIE        = 223,   // Moxie processor family
    EM_AMDGPU       = 224,   // AMD GPU architecture
    reserve_225,             // 225 - 242
    reserve_226,             // 225 - 242
    reserve_227,             // 225 - 242
    reserve_228,             // 225 - 242
    reserve_229,             // 225 - 242
    reserve_230,             // 225 - 242
    reserve_231,             // 225 - 242
    reserve_232,             // 225 - 242
    reserve_233,             // 225 - 242
    reserve_234,             // 225 - 242
    reserve_235,             // 225 - 242
    reserve_236,             // 225 - 242
    reserve_237,             // 225 - 242
    reserve_238,             // 225 - 242
    reserve_239,             // 225 - 242
    reserve_240,             // 225 - 242
    reserve_241,             // 225 - 242
    reserve_242,             // 225 - 242
    EM_RISCV = 243,          // RISC-V

    EM_MAX_SIZE,
};
}  // namespace elf

}  // namespace util_dwarf
