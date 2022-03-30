// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <base/arch.h>
#include <base/win/minwin.h>
#include <program_loader/file_classifier.h>
// need some portable windoze headers...

namespace program_loader {

namespace {
// On 64bit versions of Windows the smallest 32bit executable is 268 bytes
constexpr size_t kMinimumPEExecutableSize = 268;

struct dos_header {
  char magic[2];
  u16 UsedBytesInTheLastPage;
  u16 FileSizeInPages;
  u16 NumberOfRelocationItems;
  u16 HeaderSizeInParagraphs;
  u16 MinimumExtraParagraphs;
  u16 MaximumExtraParagraphs;
  u16 InitialRelativeSS;
  u16 InitialSP;
  u16 Checksum;
  u16 InitialIP;
  u16 InitialRelativeCS;
  u16 AddressOfRelocationTable;
  u16 OverlayNumber;
  u16 Reserved[4];
  u16 OEMid;
  u16 OEMinfo;
  u16 Reserved2[10];
  u32 farnew;
};

struct pe32_header {
  u16 Magic;
  u8 MajorLinkerVersion;
  u8 MinorLinkerVersion;
  u32 SizeOfCode;
  u32 SizeOfInitializedData;
  u32 SizeOfUninitializedData;
  u32 AddressOfEntryPoint;
  u32 BaseOfCode;
  u32 BaseOfData;
  u32 ImageBase;
  u32 SectionAlignment;
  u32 FileAlignment;
  u16 MajorOperatingSystemVersion;
  u16 MinorOperatingSystemVersion;
  u16 MajorImageVersion;
  u16 MinorImageVersion;
  u16 MajorSubsystemVersion;
  u16 MinorSubsystemVersion;
  u32 Win32VersionValue;
  u32 SizeOfImage;
  u32 SizeOfHeaders;
  u32 CheckSum;
  u16 Subsystem;
  // FIXME: This should be DllCharacteristics.
  u16 DLLCharacteristics;
  u32 SizeOfStackReserve;
  u32 SizeOfStackCommit;
  u32 SizeOfHeapReserve;
  u32 SizeOfHeapCommit;
  u32 LoaderFlags;
  // FIXME: This should be NumberOfRvaAndSizes.
  u32 NumberOfRvaAndSize;
};

/// The 64-bit PE header that follows the COFF header.
struct pe32plus_header {
  u16 Magic;
  u8 MajorLinkerVersion;
  u8 MinorLinkerVersion;
  u32 SizeOfCode;
  u32 SizeOfInitializedData;
  u32 SizeOfUninitializedData;
  u32 AddressOfEntryPoint;
  u32 BaseOfCode;
  u64 ImageBase;
  u32 SectionAlignment;
  u32 FileAlignment;
  u16 MajorOperatingSystemVersion;
  u16 MinorOperatingSystemVersion;
  u16 MajorImageVersion;
  u16 MinorImageVersion;
  u16 MajorSubsystemVersion;
  u16 MinorSubsystemVersion;
  u32 Win32VersionValue;
  u32 SizeOfImage;
  u32 SizeOfHeaders;
  u32 CheckSum;
  u16 Subsystem;
  u16 DLLCharacteristics;
  u64 SizeOfStackReserve;
  u64 SizeOfStackCommit;
  u64 SizeOfHeapReserve;
  u64 SizeOfHeapCommit;
  u32 LoaderFlags;
  u32 NumberOfRvaAndSize;
};

enum class CoffMachineType : u16 { kIntel86 = 0x14c, kAmd64 = 0x8664 };

struct coff_file_header {
  CoffMachineType Machine;
  u16 NumberOfSections;
  u32 TimeDateStamp;
  u32 PointerToSymbolTable;
  u32 NumberOfSymbols;
  u16 SizeOfOptionalHeader;
  u16 Characteristics;

  bool isImportLibrary() const { return NumberOfSections == 0xffff; }
};

static Arch TranslateCoffMachineType(CoffMachineType machine_type) {
  switch (machine_type) {
    case CoffMachineType::kIntel86:
      return Arch::kX86;
    case CoffMachineType::kAmd64:
      return Arch::kX64;
  }
  return Arch::kNone;
}

// https://github.com/llvm/llvm-project/blob/d480f968ad8b56d3ee4a6b6df5532d485b0ad01e/llvm/include/llvm/Object/ELFTypes.h

// https://github.com/llvm/llvm-project/blob/d480f968ad8b56d3ee4a6b6df5532d485b0ad01e/llvm/include/llvm/Object/COFF.h

// https://github.com/llvm/llvm-project/blob/d480f968ad8b56d3ee4a6b6df5532d485b0ad01e/llvm/include/llvm/Object/ELFTypes.h

static constexpr const char kElfMagic[] = {0x7f, 'E', 'L', 'F', '\0'};

struct Elf_Ehdr {
  byte ident[16];  // ELF Identification bytes
  u16 type;
  u16 machine;

  bool checkMagic() const {
    return (memcmp(ident, kElfMagic, strlen(kElfMagic))) == 0;
  }
};

enum {
  EM_NONE = 0,             // No machine
  EM_M32 = 1,              // AT&T WE 32100
  EM_SPARC = 2,            // SPARC
  EM_386 = 3,              // Intel 386
  EM_68K = 4,              // Motorola 68000
  EM_88K = 5,              // Motorola 88000
  EM_IAMCU = 6,            // Intel MCU
  EM_860 = 7,              // Intel 80860
  EM_MIPS = 8,             // MIPS R3000
  EM_S370 = 9,             // IBM System/370
  EM_MIPS_RS3_LE = 10,     // MIPS RS3000 Little-endian
  EM_PARISC = 15,          // Hewlett-Packard PA-RISC
  EM_VPP500 = 17,          // Fujitsu VPP500
  EM_SPARC32PLUS = 18,     // Enhanced instruction set SPARC
  EM_960 = 19,             // Intel 80960
  EM_PPC = 20,             // PowerPC
  EM_PPC64 = 21,           // PowerPC64
  EM_S390 = 22,            // IBM System/390
  EM_SPU = 23,             // IBM SPU/SPC
  EM_V800 = 36,            // NEC V800
  EM_FR20 = 37,            // Fujitsu FR20
  EM_RH32 = 38,            // TRW RH-32
  EM_RCE = 39,             // Motorola RCE
  EM_ARM = 40,             // ARM
  EM_ALPHA = 41,           // DEC Alpha
  EM_SH = 42,              // Hitachi SH
  EM_SPARCV9 = 43,         // SPARC V9
  EM_TRICORE = 44,         // Siemens TriCore
  EM_ARC = 45,             // Argonaut RISC Core
  EM_H8_300 = 46,          // Hitachi H8/300
  EM_H8_300H = 47,         // Hitachi H8/300H
  EM_H8S = 48,             // Hitachi H8S
  EM_H8_500 = 49,          // Hitachi H8/500
  EM_IA_64 = 50,           // Intel IA-64 processor architecture
  EM_MIPS_X = 51,          // Stanford MIPS-X
  EM_COLDFIRE = 52,        // Motorola ColdFire
  EM_68HC12 = 53,          // Motorola M68HC12
  EM_MMA = 54,             // Fujitsu MMA Multimedia Accelerator
  EM_PCP = 55,             // Siemens PCP
  EM_NCPU = 56,            // Sony nCPU embedded RISC processor
  EM_NDR1 = 57,            // Denso NDR1 microprocessor
  EM_STARCORE = 58,        // Motorola Star*Core processor
  EM_ME16 = 59,            // Toyota ME16 processor
  EM_ST100 = 60,           // STMicroelectronics ST100 processor
  EM_TINYJ = 61,           // Advanced Logic Corp. TinyJ embedded processor family
  EM_X86_64 = 62,          // AMD x86-64 architecture
  EM_PDSP = 63,            // Sony DSP Processor
  EM_PDP10 = 64,           // Digital Equipment Corp. PDP-10
  EM_PDP11 = 65,           // Digital Equipment Corp. PDP-11
  EM_FX66 = 66,            // Siemens FX66 microcontroller
  EM_ST9PLUS = 67,         // STMicroelectronics ST9+ 8/16 bit microcontroller
  EM_ST7 = 68,             // STMicroelectronics ST7 8-bit microcontroller
  EM_68HC16 = 69,          // Motorola MC68HC16 Microcontroller
  EM_68HC11 = 70,          // Motorola MC68HC11 Microcontroller
  EM_68HC08 = 71,          // Motorola MC68HC08 Microcontroller
  EM_68HC05 = 72,          // Motorola MC68HC05 Microcontroller
  EM_SVX = 73,             // Silicon Graphics SVx
  EM_ST19 = 74,            // STMicroelectronics ST19 8-bit microcontroller
  EM_VAX = 75,             // Digital VAX
  EM_CRIS = 76,            // Axis Communications 32-bit embedded processor
  EM_JAVELIN = 77,         // Infineon Technologies 32-bit embedded processor
  EM_FIREPATH = 78,        // Element 14 64-bit DSP Processor
  EM_ZSP = 79,             // LSI Logic 16-bit DSP Processor
  EM_MMIX = 80,            // Donald Knuth's educational 64-bit processor
  EM_HUANY = 81,           // Harvard University machine-independent object files
  EM_PRISM = 82,           // SiTera Prism
  EM_AVR = 83,             // Atmel AVR 8-bit microcontroller
  EM_FR30 = 84,            // Fujitsu FR30
  EM_D10V = 85,            // Mitsubishi D10V
  EM_D30V = 86,            // Mitsubishi D30V
  EM_V850 = 87,            // NEC v850
  EM_M32R = 88,            // Mitsubishi M32R
  EM_MN10300 = 89,         // Matsushita MN10300
  EM_MN10200 = 90,         // Matsushita MN10200
  EM_PJ = 91,              // picoJava
  EM_OPENRISC = 92,        // OpenRISC 32-bit embedded processor
  EM_ARC_COMPACT = 93,     // ARC International ARCompact processor (old
                           // spelling/synonym: EM_ARC_A5)
  EM_XTENSA = 94,          // Tensilica Xtensa Architecture
  EM_VIDEOCORE = 95,       // Alphamosaic VideoCore processor
  EM_TMM_GPP = 96,         // Thompson Multimedia General Purpose Processor
  EM_NS32K = 97,           // National Semiconductor 32000 series
  EM_TPC = 98,             // Tenor Network TPC processor
  EM_SNP1K = 99,           // Trebia SNP 1000 processor
  EM_ST200 = 100,          // STMicroelectronics (www.st.com) ST200
  EM_IP2K = 101,           // Ubicom IP2xxx microcontroller family
  EM_MAX = 102,            // MAX Processor
  EM_CR = 103,             // National Semiconductor CompactRISC microprocessor
  EM_F2MC16 = 104,         // Fujitsu F2MC16
  EM_MSP430 = 105,         // Texas Instruments embedded microcontroller msp430
  EM_BLACKFIN = 106,       // Analog Devices Blackfin (DSP) processor
  EM_SE_C33 = 107,         // S1C33 Family of Seiko Epson processors
  EM_SEP = 108,            // Sharp embedded microprocessor
  EM_ARCA = 109,           // Arca RISC Microprocessor
  EM_UNICORE = 110,        // Microprocessor series from PKU-Unity Ltd. and MPRC
                           // of Peking University
  EM_EXCESS = 111,         // eXcess: 16/32/64-bit configurable embedded CPU
  EM_DXP = 112,            // Icera Semiconductor Inc. Deep Execution Processor
  EM_ALTERA_NIOS2 = 113,   // Altera Nios II soft-core processor
  EM_CRX = 114,            // National Semiconductor CompactRISC CRX
  EM_XGATE = 115,          // Motorola XGATE embedded processor
  EM_C166 = 116,           // Infineon C16x/XC16x processor
  EM_M16C = 117,           // Renesas M16C series microprocessors
  EM_DSPIC30F = 118,       // Microchip Technology dsPIC30F Digital Signal
                           // Controller
  EM_CE = 119,             // Freescale Communication Engine RISC core
  EM_M32C = 120,           // Renesas M32C series microprocessors
  EM_TSK3000 = 131,        // Altium TSK3000 core
  EM_RS08 = 132,           // Freescale RS08 embedded processor
  EM_SHARC = 133,          // Analog Devices SHARC family of 32-bit DSP
                           // processors
  EM_ECOG2 = 134,          // Cyan Technology eCOG2 microprocessor
  EM_SCORE7 = 135,         // Sunplus S+core7 RISC processor
  EM_DSP24 = 136,          // New Japan Radio (NJR) 24-bit DSP Processor
  EM_VIDEOCORE3 = 137,     // Broadcom VideoCore III processor
  EM_LATTICEMICO32 = 138,  // RISC processor for Lattice FPGA architecture
  EM_SE_C17 = 139,         // Seiko Epson C17 family
  EM_TI_C6000 = 140,       // The Texas Instruments TMS320C6000 DSP family
  EM_TI_C2000 = 141,       // The Texas Instruments TMS320C2000 DSP family
  EM_TI_C5500 = 142,       // The Texas Instruments TMS320C55x DSP family
  EM_MMDSP_PLUS = 160,     // STMicroelectronics 64bit VLIW Data Signal Processor
  EM_CYPRESS_M8C = 161,    // Cypress M8C microprocessor
  EM_R32C = 162,           // Renesas R32C series microprocessors
  EM_TRIMEDIA = 163,       // NXP Semiconductors TriMedia architecture family
  EM_HEXAGON = 164,        // Qualcomm Hexagon processor
  EM_8051 = 165,           // Intel 8051 and variants
  EM_STXP7X = 166,         // STMicroelectronics STxP7x family of configurable
                           // and extensible RISC processors
  EM_NDS32 = 167,          // Andes Technology compact code size embedded RISC
                           // processor family
  EM_ECOG1 = 168,          // Cyan Technology eCOG1X family
  EM_ECOG1X = 168,         // Cyan Technology eCOG1X family
  EM_MAXQ30 = 169,         // Dallas Semiconductor MAXQ30 Core Micro-controllers
  EM_XIMO16 = 170,         // New Japan Radio (NJR) 16-bit DSP Processor
  EM_MANIK = 171,          // M2000 Reconfigurable RISC Microprocessor
  EM_CRAYNV2 = 172,        // Cray Inc. NV2 vector architecture
  EM_RX = 173,             // Renesas RX family
  EM_METAG = 174,          // Imagination Technologies META processor
                           // architecture
  EM_MCST_ELBRUS = 175,    // MCST Elbrus general purpose hardware architecture
  EM_ECOG16 = 176,         // Cyan Technology eCOG16 family
  EM_CR16 = 177,           // National Semiconductor CompactRISC CR16 16-bit
                           // microprocessor
  EM_ETPU = 178,           // Freescale Extended Time Processing Unit
  EM_SLE9X = 179,          // Infineon Technologies SLE9X core
  EM_L10M = 180,           // Intel L10M
  EM_K10M = 181,           // Intel K10M
  EM_AARCH64 = 183,        // ARM AArch64
  EM_AVR32 = 185,          // Atmel Corporation 32-bit microprocessor family
  EM_STM8 = 186,           // STMicroeletronics STM8 8-bit microcontroller
  EM_TILE64 = 187,         // Tilera TILE64 multicore architecture family
  EM_TILEPRO = 188,        // Tilera TILEPro multicore architecture family
  EM_CUDA = 190,           // NVIDIA CUDA architecture
  EM_TILEGX = 191,         // Tilera TILE-Gx multicore architecture family
  EM_CLOUDSHIELD = 192,    // CloudShield architecture family
  EM_COREA_1ST = 193,      // KIPO-KAIST Core-A 1st generation processor family
  EM_COREA_2ND = 194,      // KIPO-KAIST Core-A 2nd generation processor family
  EM_ARC_COMPACT2 = 195,   // Synopsys ARCompact V2
  EM_OPEN8 = 196,          // Open8 8-bit RISC soft processor core
  EM_RL78 = 197,           // Renesas RL78 family
  EM_VIDEOCORE5 = 198,     // Broadcom VideoCore V processor
  EM_78KOR = 199,          // Renesas 78KOR family
  EM_56800EX = 200,        // Freescale 56800EX Digital Signal Controller (DSC)
  EM_BA1 = 201,            // Beyond BA1 CPU architecture
  EM_BA2 = 202,            // Beyond BA2 CPU architecture
  EM_XCORE = 203,          // XMOS xCORE processor family
  EM_MCHP_PIC = 204,       // Microchip 8-bit PIC(r) family
  EM_INTEL205 = 205,       // Reserved by Intel
  EM_INTEL206 = 206,       // Reserved by Intel
  EM_INTEL207 = 207,       // Reserved by Intel
  EM_INTEL208 = 208,       // Reserved by Intel
  EM_INTEL209 = 209,       // Reserved by Intel
  EM_KM32 = 210,           // KM211 KM32 32-bit processor
  EM_KMX32 = 211,          // KM211 KMX32 32-bit processor
  EM_KMX16 = 212,          // KM211 KMX16 16-bit processor
  EM_KMX8 = 213,           // KM211 KMX8 8-bit processor
  EM_KVARC = 214,          // KM211 KVARC processor
  EM_CDP = 215,            // Paneve CDP architecture family
  EM_COGE = 216,           // Cognitive Smart Memory Processor
  EM_COOL = 217,           // iCelero CoolEngine
  EM_NORC = 218,           // Nanoradio Optimized RISC
  EM_CSR_KALIMBA = 219,    // CSR Kalimba architecture family
  EM_AMDGPU = 224,         // AMD GPU architecture
  EM_RISCV = 243,          // RISC-V
  EM_LANAI = 244,          // Lanai 32-bit processor
  EM_BPF = 247,            // Linux kernel bpf virtual machine
  EM_VE = 251,             // NEC SX-Aurora VE
  EM_CSKY = 252,           // C-SKY 32-bit processor
};

static Arch TranslateElfMachineType(u16 type) {
  switch (type) {
    case EM_386:
      return Arch::kX86;
    case EM_X86_64:
      return Arch::kX64;
  }

  return Arch::kNone;
}

enum CPUType {
  CPU_TYPE_ANY = -1,
  CPU_TYPE_X86 = 7,
  CPU_TYPE_I386 = CPU_TYPE_X86,
  CPU_TYPE_X86_64 = CPU_TYPE_X86 | CPU_ARCH_ABI64,
  /* CPU_TYPE_MIPS      = 8, */
  CPU_TYPE_MC98000 = 10,  // Old Motorola PowerPC
  CPU_TYPE_ARM = 12,
  CPU_TYPE_ARM64 = CPU_TYPE_ARM | CPU_ARCH_ABI64,
  CPU_TYPE_ARM64_32 = CPU_TYPE_ARM | CPU_ARCH_ABI64_32,
  CPU_TYPE_SPARC = 14,
  CPU_TYPE_POWERPC = 18,
  CPU_TYPE_POWERPC64 = CPU_TYPE_POWERPC | CPU_ARCH_ABI64
};

enum : uint32_t {
  // Constants for the "magic" field in llvm::MachO::mach_header and
  // llvm::MachO::mach_header_64
  MH_MAGIC = 0xFEEDFACEu,
  MH_CIGAM = 0xCEFAEDFEu,
  MH_MAGIC_64 = 0xFEEDFACFu,
  MH_CIGAM_64 = 0xCFFAEDFEu,
  FAT_MAGIC = 0xCAFEBABEu,
  FAT_CIGAM = 0xBEBAFECAu,
  FAT_MAGIC_64 = 0xCAFEBABFu,
  FAT_CIGAM_64 = 0xBFBAFECAu
};

struct mach_header {
  u32 magic;
  u32 cputype;
  u32 cpusubtype;
  u32 filetype;
  u32 ncmds;
  u32 sizeofcmds;
  u32 flags;

  bool CheckMagic() const {
    switch (magic) {
      case MH_MAGIC:
      case MH_CIGAM:
      case MH_MAGIC_64:
      case MH_CIGAM_64:
      case FAT_MAGIC:
      case FAT_CIGAM:
      case FAT_MAGIC_64:
      case FAT_CIGAM_64:
        return true;
      default:
        return false;
    }
  }
};

static Arch TranslateMachMachineType(u32 type) {
  switch (type) {
    case CPUType::CPU_TYPE_X86:
      return Arch::kX86;
    case CPUType::CPU_TYPE_X86_64:
      return Arch::kX64;
  }

  return Arch::kNone;
}

// https://github.com/llvm/llvm-project/blob/d480f968ad8b56d3ee4a6b6df5532d485b0ad01e/llvm/include/llvm/BinaryFormat/ELF.h
}  // namespace

class FileReader {
 public:
  FileReader(base::Span<byte> buffer) : buffer_(buffer) {}

  template <typename T>
  T* Fetch(size_t at_offset = 0) {
    T* head = nullptr;
    return Read(reinterpret_cast<void*>(head), sizeof(T)) ? head : nullptr;
  }

  template< typename T>
  T* FetchRewind(size_t at_offset = 0) {
    T* head = nullptr;
    pos_ = 0;
    return Read(reinterpret_cast<void*>(head), sizeof(T)) ? head : nullptr;
  }

 private:
  bool Read(byte* new_head, size_t n) {
    pos += n;
    if (pos > buffer_.size())
      return false;

    new_head = &buffer_[pos];
    return true;
  }

 private:
  size_t pos = 0;
  base::Span<byte> buffer_;
};

bool ClassifyFile(base::Span<byte> buffer, FileClassificationInfo* info) {
  *info = {};

  FileReader reader(buffer);
  if (buffer.length() >= kMinimumPEExecutableSize) {
    auto* as_dos = reader.FetchRewind<dos_header>();
    if (as_dos->magic[0] == 'M' && as_dos->magic[1] == 'Z') {
      info->format_type = Format::kPE;

      auto* coff = reader.Fetch<coff_file_header>();
      auto arch = TranslateCoffMachineType(coff->Machine);
      if (arch == Arch::kNone)
        return false;

      info->architecture = arch;
      return true;
    }
  }

  auto* as_ehdr = reader.FetchRewind<Elf_Ehdr>();
  if (as_ehdr->checkMagic()) {
    info->format_type = Format::kELF;

    const auto arch = TranslateElfMachineType(as_ehdr->machine);
    if (arch == Arch::kNone)
      return false;

    info->architecture = arch;
    return true;
  }

  auto* as_mach = reader.FetchRewind<mach_header>();
  if (as_mach->CheckMagic()) {
    info->format_type = Format::kMachO;

    const auto arch = TranslateMachMachineType(as_mach->cputype);
    if (arch == Arch::kNone)
      return false;

    info->architecture = arch;
    return true;
  }

  return false;
}
}  // namespace program_loader