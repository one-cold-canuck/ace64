#ifndef CPU_H
/* cpu.h
* Emulation of the MOS6502 processor.
*/
#include <stdint.h>

#define MAX_MEMORY 1024*64
/* Instructions */

/* Read instructions */
#define INS_LDA_IM 0xA9
#define INS_LDA_ZP 0xA5
#define INS_LDA_ZPX 0xB5
#define INS_LDA_ABS 0xAD
#define INS_LDA_ABX 0xBD
#define INS_LDA_ABY 0xB9
#define INS_LDA_IDX 0xA1
#define INS_LDA_IDY 0xB1
#define INS_LDX_IM 0xA2
#define INS_LDX_ZP 0xA6
#define INS_LDX_ZPY 0xB6
#define INS_LDX_ABS 0xAE
#define INS_LDX_ABY 0xBE
#define INS_LDY_IM 0xA0
#define INS_LDY_ZP 0xA4
#define INS_LDY_ZPX 0xB4
#define INS_LDY_ABS 0xAC
#define INS_LDY_ABX 0xBC

/* Write Instructions */
#define INS_STA_ZP 0x85
#define INS_STA_ZPX 0x95
#define INS_STA_ABS 0x8D
#define INS_STA_ABX 0x9D
#define INS_STA_ABY 0x99
#define INS_STA_IDX 0x81
#define INS_STA_IDY 0x91
#define INS_STX_ZP 0x86
#define INS_STX_ZPY 0x96
#define INS_STX_ABS 0x8E
#define INS_STY_ZP 0x84
#define INS_STY_ZPX 0x94
#define INS_STY_ABS 0x8C

/* Program Flow */
#define INS_JSR 0x20
#define INS_JMP_ABS 0x4C
#define INS_JMP_IND 0x6C

/* Register Transfers */
#define INS_TAX 0xAA
#define INS_TAY 0xA8
#define INS_TXA 0x8A
#define INS_TYA 0x98
#define INS_TSX 0xBA
#define INS_TXS 0x9A
#define INS_PHA 0x48
#define INS_PHP 0x08
#define INS_PLA 0x68 
#define INS_PLP 0x28

/* Processor Status Flags */
#define FLAG_NEGATIVE 0x80
#define FLAG_OVERFLOW 0x40
#define FLAG_UNDEFINED 0x20
#define FLAG_BREAK 0x10
#define FLAG_DECIMAL_MODE 0x08
#define FLAG_INTERRUPT_DISABLE 0x04
#define FLAG_ZERO 0x02
#define FLAG_CARRY 0x01

typedef unsigned char Byte;
typedef unsigned short Word;

typedef uint32_t Uint32;
typedef int32_t Sint32;

typedef struct {
  Word PC; // Program Counter
  Byte SP; // Stack Pointer

  Byte P;  // Processor status: bits are NV-BDIZC

  Byte A;  // Accumulator
  Byte X;  // X Index Register
  Byte Y;  // Y Index Register
  Byte Memory[MAX_MEMORY];
} CPU;

void InitializeMemory(CPU *cpu);
void Reset(CPU *cpu);
void WriteWord (CPU *cpu, Word Value, Byte Address, Sint32 *Cycles);
Byte FetchByte (CPU *cpu, Sint32 *Cycles);
Word FetchWord (CPU *cpu, Sint32 *Cycles);
Byte ReadByte (CPU *cpu, Word Address, Sint32 *Cycles);
void WriteByte (CPU *cpu, Word Address, Byte Value, Sint32 *Cycles);
Word ReadWord (CPU *cpu, Byte Address, Sint32 *Cycles);
void SetStatusFlag (Byte *flags, Byte *Value);
Word GetWordAddress (Byte LoByte, Byte HiByte);
Sint32 Execute (CPU *cpu);

#define CPU_H
#endif // !CPU_H

