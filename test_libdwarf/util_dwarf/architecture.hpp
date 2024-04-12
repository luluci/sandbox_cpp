#pragma once

#include <cstdint>

#include "elf.hpp"

namespace util_dwarf {

namespace arch {
// アーキテクチャ依存情報を定義する
struct arch_info
{
    uint8_t *address_class;
};

namespace addr_cls {
static constexpr size_t size = 10;
uint8_t Renesas_RL78[size]   = {
    0,  // 0: 未定義
    0,  // 1: 未定義
    0,  // 2: 未定義
    4,  // 3: far pointer アドレスサイズ=4byte
    2,  // 4: near pointer アドレスサイズ=2byte
};
}  // namespace addr_cls

arch_info arch_info_tbl[elf::EM_MAX_SIZE] = {
    {nullptr},                 // EM_NONE
    {nullptr},                 // EM_M32
    {nullptr},                 // EM_SPARC
    {nullptr},                 // EM_386
    {nullptr},                 // EM_68K
    {nullptr},                 // EM_88K
    {nullptr},                 // EM_IAMCU
    {nullptr},                 // EM_860
    {nullptr},                 // EM_MIPS
    {nullptr},                 // EM_S370
    {nullptr},                 // EM_MIPS_RS3_LE
    {nullptr},                 // reserved_11
    {nullptr},                 // reserved_12
    {nullptr},                 // reserved_13
    {nullptr},                 // reserved_14
    {nullptr},                 // EM_PARISC
    {nullptr},                 // reserved
    {nullptr},                 // EM_VPP500
    {nullptr},                 // EM_SPARC32PLUS
    {nullptr},                 // EM_960
    {nullptr},                 // EM_PPC
    {nullptr},                 // EM_PPC64
    {nullptr},                 // EM_S390
    {nullptr},                 // EM_SPU
    {nullptr},                 // reserved_24
    {nullptr},                 // reserved_25
    {nullptr},                 // reserved_26
    {nullptr},                 // reserved_27
    {nullptr},                 // reserved_28
    {nullptr},                 // reserved_29
    {nullptr},                 // reserved_30
    {nullptr},                 // reserved_31
    {nullptr},                 // reserved_32
    {nullptr},                 // reserved_33
    {nullptr},                 // reserved_34
    {nullptr},                 // reserved_35
    {nullptr},                 // EM_V800
    {nullptr},                 // EM_FR20
    {nullptr},                 // EM_RH32
    {nullptr},                 // EM_RCE
    {nullptr},                 // EM_ARM
    {nullptr},                 // EM_ALPHA
    {nullptr},                 // EM_SH
    {nullptr},                 // EM_SPARCV9
    {nullptr},                 // EM_TRICORE
    {nullptr},                 // EM_ARC
    {nullptr},                 // EM_H8_300
    {nullptr},                 // EM_H8_300H
    {nullptr},                 // EM_H8S
    {nullptr},                 // EM_H8_500
    {nullptr},                 // EM_IA_64
    {nullptr},                 // EM_MIPS_X
    {nullptr},                 // EM_COLDFIRE
    {nullptr},                 // EM_68HC12
    {nullptr},                 // EM_MMA
    {nullptr},                 // EM_PCP
    {nullptr},                 // EM_NCPU
    {nullptr},                 // EM_NDR1
    {nullptr},                 // EM_STARCORE
    {nullptr},                 // EM_ME16
    {nullptr},                 // EM_ST100
    {nullptr},                 // EM_TINYJ
    {nullptr},                 // EM_X86_64
    {nullptr},                 // EM_PDSP
    {nullptr},                 // EM_PDP10
    {nullptr},                 // EM_PDP11
    {nullptr},                 // EM_FX66
    {nullptr},                 // EM_ST9PLUS
    {nullptr},                 // EM_ST7
    {nullptr},                 // EM_68HC16
    {nullptr},                 // EM_68HC11
    {nullptr},                 // EM_68HC08
    {nullptr},                 // EM_68HC05
    {nullptr},                 // EM_SVX
    {nullptr},                 // EM_ST19
    {nullptr},                 // EM_VAX
    {nullptr},                 // EM_CRIS
    {nullptr},                 // EM_JAVELIN
    {nullptr},                 // EM_FIREPATH
    {nullptr},                 // EM_ZSP
    {nullptr},                 // EM_MMIX
    {nullptr},                 // EM_HUANY
    {nullptr},                 // EM_PRISM
    {nullptr},                 // EM_AVR
    {nullptr},                 // EM_FR30
    {nullptr},                 // EM_D10V
    {nullptr},                 // EM_D30V
    {nullptr},                 // EM_V850
    {nullptr},                 // EM_M32R
    {nullptr},                 // EM_MN10300
    {nullptr},                 // EM_MN10200
    {nullptr},                 // EM_PJ
    {nullptr},                 // EM_OPENRISC
    {nullptr},                 // EM_ARC_COMPACT
    {nullptr},                 // EM_XTENSA
    {nullptr},                 // EM_VIDEOCORE
    {nullptr},                 // EM_TMM_GPP
    {nullptr},                 // EM_NS32K
    {nullptr},                 // EM_TPC
    {nullptr},                 // EM_SNP1K
    {nullptr},                 // EM_ST200
    {nullptr},                 // EM_IP2K
    {nullptr},                 // EM_MAX
    {nullptr},                 // EM_CR
    {nullptr},                 // EM_F2MC16
    {nullptr},                 // EM_MSP430
    {nullptr},                 // EM_BLACKFIN
    {nullptr},                 // EM_SE_C33
    {nullptr},                 // EM_SEP
    {nullptr},                 // EM_ARCA
    {nullptr},                 // EM_UNICORE
    {nullptr},                 // EM_EXCESS
    {nullptr},                 // EM_DXP
    {nullptr},                 // EM_ALTERA_NIOS2
    {nullptr},                 // EM_CRX
    {nullptr},                 // EM_XGATE
    {nullptr},                 // EM_C166
    {nullptr},                 // EM_M16C
    {nullptr},                 // EM_DSPIC30F
    {nullptr},                 // EM_CE
    {nullptr},                 // EM_M32C
    {nullptr},                 // reserved_121
    {nullptr},                 // reserved_122
    {nullptr},                 // reserved_123
    {nullptr},                 // reserved_124
    {nullptr},                 // reserved_125
    {nullptr},                 // reserved_126
    {nullptr},                 // reserved_127
    {nullptr},                 // reserved_128
    {nullptr},                 // reserved_129
    {nullptr},                 // reserved_130
    {nullptr},                 // EM_TSK3000
    {nullptr},                 // EM_RS08
    {nullptr},                 // EM_SHARC
    {nullptr},                 // EM_ECOG2
    {nullptr},                 // EM_SCORE7
    {nullptr},                 // EM_DSP24
    {nullptr},                 // EM_VIDEOCORE3
    {nullptr},                 // EM_LATTICEMICO32
    {nullptr},                 // EM_SE_C17
    {nullptr},                 // EM_TI_C6000
    {nullptr},                 // EM_TI_C2000
    {nullptr},                 // EM_TI_C5500
    {nullptr},                 // EM_TI_ARP32
    {nullptr},                 // EM_TI_PRU
    {nullptr},                 // reserved_145
    {nullptr},                 // reserved_146
    {nullptr},                 // reserved_147
    {nullptr},                 // reserved_148
    {nullptr},                 // reserved_149
    {nullptr},                 // reserved_150
    {nullptr},                 // reserved_151
    {nullptr},                 // reserved_152
    {nullptr},                 // reserved_153
    {nullptr},                 // reserved_154
    {nullptr},                 // reserved_155
    {nullptr},                 // reserved_156
    {nullptr},                 // reserved_157
    {nullptr},                 // reserved_158
    {nullptr},                 // reserved_159
    {nullptr},                 // EM_MMDSP_PLUS
    {nullptr},                 // EM_CYPRESS_M8C
    {nullptr},                 // EM_R32C
    {nullptr},                 // EM_TRIMEDIA
    {nullptr},                 // EM_QDSP6
    {nullptr},                 // EM_8051
    {nullptr},                 // EM_STXP7X
    {nullptr},                 // EM_NDS32
    {nullptr},                 // EM_ECOG1, EM_ECOG1X
    {nullptr},                 // EM_MAXQ30
    {nullptr},                 // EM_XIMO16
    {nullptr},                 // EM_MANIK
    {nullptr},                 // EM_CRAYNV2
    {nullptr},                 // EM_RX
    {nullptr},                 // EM_METAG
    {nullptr},                 // EM_MCST_ELBRUS
    {nullptr},                 // EM_ECOG16
    {nullptr},                 // EM_CR16
    {nullptr},                 // EM_ETPU
    {nullptr},                 // EM_SLE9X
    {nullptr},                 // EM_L10M
    {nullptr},                 // EM_K10M
    {nullptr},                 // reserved_182
    {nullptr},                 // EM_AARCH64
    {nullptr},                 // reserved_184
    {nullptr},                 // EM_AVR32
    {nullptr},                 // EM_STM8
    {nullptr},                 // EM_TILE64
    {nullptr},                 // EM_TILEPRO
    {nullptr},                 // EM_MICROBLAZE
    {nullptr},                 // EM_CUDA
    {nullptr},                 // EM_TILEGX
    {nullptr},                 // EM_CLOUDSHIELD
    {nullptr},                 // EM_COREA_1ST
    {nullptr},                 // EM_COREA_2ND
    {nullptr},                 // EM_ARC_COMPACT2
    {nullptr},                 // EM_OPEN8
    {addr_cls::Renesas_RL78},  // EM_RL78
    {nullptr},                 // EM_VIDEOCORE5
    {nullptr},                 // EM_78KOR
    {nullptr},                 // EM_56800EX
    {nullptr},                 // EM_BA1
    {nullptr},                 // EM_BA2
    {nullptr},                 // EM_XCORE
    {nullptr},                 // EM_MCHP_PIC
    {nullptr},                 // EM_INTEL205
    {nullptr},                 // EM_INTEL206
    {nullptr},                 // EM_INTEL207
    {nullptr},                 // EM_INTEL208
    {nullptr},                 // EM_INTEL209
    {nullptr},                 // EM_KM32
    {nullptr},                 // EM_KMX32
    {nullptr},                 // EM_KMX16
    {nullptr},                 // EM_KMX8
    {nullptr},                 // EM_KVARC
    {nullptr},                 // EM_CDP
    {nullptr},                 // EM_COGE
    {nullptr},                 // EM_COOL
    {nullptr},                 // EM_NORC
    {nullptr},                 // EM_CSR_KALIMBA
    {nullptr},                 // EM_Z80
    {nullptr},                 // EM_VISIUM
    {nullptr},                 // EM_FT32
    {nullptr},                 // EM_MOXIE
    {nullptr},                 // EM_AMDGPU
    {nullptr},                 // reserve_225
    {nullptr},                 // reserve_226
    {nullptr},                 // reserve_227
    {nullptr},                 // reserve_228
    {nullptr},                 // reserve_229
    {nullptr},                 // reserve_230
    {nullptr},                 // reserve_231
    {nullptr},                 // reserve_232
    {nullptr},                 // reserve_233
    {nullptr},                 // reserve_234
    {nullptr},                 // reserve_235
    {nullptr},                 // reserve_236
    {nullptr},                 // reserve_237
    {nullptr},                 // reserve_238
    {nullptr},                 // reserve_239
    {nullptr},                 // reserve_240
    {nullptr},                 // reserve_241
    {nullptr},                 // reserve_242
    {nullptr},                 // EM_RISCV
};
}  // namespace arch

}  // namespace util_dwarf