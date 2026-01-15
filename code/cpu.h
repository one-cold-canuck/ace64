#ifndef CPU_H
/* cpu.h
 * Emulation of the MOS6502 processor.
 */
#include <stdint.h>

#define MAX_MEMORY 1024 * 64

/* Instructions */

/* Bitwise Instructions */

// AND: add 1 cycle if page boundary is crossed
// Flags: N----Z-
#define INS_AND_IM  0x29
#define INS_AND_ZP  0x25
#define INS_AND_ZPX 0x35
#define INS_AND_ABS 0x2D
#define INS_AND_ABX 0x3D
#define INS_AND_ABY 0x39
#define INS_AND_IDX 0x21
#define INS_AND_IDY 0x31

// ASL: Arithmetic Shift Left: bits shifted 1 position, 0 shifted into bit 0, bit 7 -> Carry
// Flags: N-----ZC
#define INS_ASL_ACC 0x0A
#define INS_ASL_ZP  0x06
#define INS_ASL_ZPX 0x16
#define INS_ASL_ABS 0x0E
#define INS_ASL_ABX 0x1E

// EOR: Bitwise exclusive OR with ACC. Add 1 cycle if page boundary is crossed.
// Flags: N-----Z-
#define INS_EOR_IM  0x49
#define INS_EOR_ZP  0x45
#define INS_EOR_ZPX 0x55
#define INS_EOR_ABS 0x4D
#define INS_EOR_ABX 0x5D
#define INS_EOR_ABY 0x59
#define INS_EOR_IDX 0x41
#define INS_EOR_IDY 0x51

// LSR: Logical Shift Right: bits shifted 1 position.  0 shifted to bit 7, bit 0 -> Carry 
// Flags: N-----ZC
#define INS_LSR_ACC 0x4A
#define INS_LSR_ZP  0x46
#define INS_LSR_ZPX 0x56
#define INS_LSR_ABS 0x4E
#define INS_LSR_ABX 0x5E

// ORA: Bitwise OR with ACC: add 1 cycle if page boundary is crossed
// Flags: N-----Z-
#define INS_ORA_IM  0x09
#define INS_ORA_ZP  0x05
#define INS_ORA_ZPX 0x15
#define INS_ORA_ABS 0x0D
#define INS_ORA_ABX 0x1D
#define INS_ORA_ABY 0x19
#define INS_ORA_IDX 0x01
#define INS_ORA_IDY 0x11

// ROL: Shift all bits left 1.  Carry shifted to 0, original 7 shifted to Carry
// Flags: N-----ZC
#define INS_ROL_ACC 0x2A
#define INS_ROL_ZP  0x26
#define INS_ROL_ZPX 0x36
#define INS_ROL_ABS 0x2E
#define INS_ROL_ABX 0x3E

// ROR: Shift all bits right 1.  Carry to bit 7, original bit 0 to Carry
// Flags: N-----ZC
#define INS_ROR_ACC 0x6A
#define INS_ROR_ZP  0x66
#define INS_ROR_ZPX 0x76
#define INS_ROR_ABS 0x6E
#define INS_ROR_ABX 0x7E

/* Branch Instructions */
// All branch instructions are relative mode, with a length of 2 bytes.
// Syntax: Bxx Displacement,  or Bxx Label.
// A branch not taken requires 2 machine cycles.  3 if taken, plus 1 if it crosses
// a page boundary. Crossing only occurs if the branch destination is on a 
// different page than the instruction AFTER the branch instruction
// Flags: --------
#define INS_BPL 0x10
#define INS_BMI 0x30
#define INS_BVC 0x50
#define INS_BVS 0x70
#define INS_BCC 0x90
#define INS_BCS 0xB0
#define INS_BNE 0xD0
#define INS_BEQ 0xF0

/* Compare Instructions */

// CMP: Compare Accumulator.  Sets the processor flags as if a subtraction 
// has been carried out
// Acc == Compared: Zero Flag
// ACC >= Compared: Carry Flag
// Flags: N-----ZC
#define INS_CMP_IM  0xC9
#define INS_CMP_ZP  0xC5
#define INS_CMP_ZPX 0xD5
#define INS_CMP_ABS 0xCD
#define INS_CMP_ABX 0xDD
#define INS_CMP_ABY 0xD9
#define INS_CMP_IDX 0xC1
#define INS_CMP_IDY 0xD1

// BIT: Test Bits
// Sets the Z flag as though value in addr tested were ANDed with ACC
// N, V are set equal to bits 7 and 6 of the value in the tested address
// Flags: NV----Z-
#define INS_BIT_ZP  0x24
#define INS_BIT_ABS 0x2C

// CPX, CPY: Compare X, Y register.
// Identical to equivalent CMP operations
// Flags: N-----ZC
#define INS_CPX_IM  0xE0
#define INS_CPX_ZP  0xE4
#define INS_CPX_ABS 0xEC
#define INS_CPY_IM  0xC0
#define INS_CPY_ZP  0xC4
#define INS_CPY_ABS 0xCC

/* Flag Instructions */
// Instructions that change processor status flags
// Carry set/clear
// Flags: -------C
#define INS_CLC 0x18
#define INS_SEC 0x38

// Decimal set/clear
// Flags: ----D---
#define INS_CLD 0xD8
#define INS_SED 0xF8

// Interrupt set/clear
// Flags: -----I--
#define INS_CLI 0x58
#define INS_SEI 0x78

// Overflow set/clear
// Flags: -V------
#define INS_CLV 0xB8

/* Jump Instructions */
// JMP: No carry associated.  Indirect JMP must never use a vector
// beginning on the last byte of a page
// JMP ($30FF) will read low byte from $30FF, and high byte from $3000
#define INS_JMP_ABS 0x4C // Implemented
#define INS_JMP_IND 0x6C

#define INS_JSR_ABS 0x20 // Implemented
#define INS_RTS     0x60

// RTI: Return from Interrupt.  Retrieves the Processor Status byte and PC from
// stack in that order.  Unlike RTS, return address on the stack is the actual address
// rather than address-1
// Flags: NV-BDIZC
#define INS_RTI     0x40

/* Math Instructions */
// Add 1 cycle if page boundary is crossed ADC, AND, SBC
// Flags: NV----ZC
#define INS_ADC_IM  0x69
#define INS_ADC_ZP  0x65
#define INS_ADC_ZPX 0x75
#define INS_ADC_ABS 0x6D
#define INS_ADC_ABX 0x7D
#define INS_ADC_ABY 0x79
#define INS_ADC_IDX 0x61
#define INS_ADC_IDY 0x71

#define INS_SBC_IM  0xE9
#define INS_SBC_ZP  0xE5
#define INS_SBC_ZPX 0xF5
#define INS_SBC_ABS 0xED
#define INS_SBC_ABX 0xFD
#define INS_SBC_ABY 0xF9
#define INS_SBC_IDX 0xE1
#define INS_SBC_IDY 0xF1

/* Memory Instructions */

// LDA: Load Accumulator.  Add 1 cycle if page boundary is crossed
// Flags: N-----Z-
#define INS_LDA_IM  0xA9 // Implemented
#define INS_LDA_ZP  0xA5 // Implemented
#define INS_LDA_ZPX 0xB5 // Implemented
#define INS_LDA_ABS 0xAD // Implemented
#define INS_LDA_ABX 0xBD // Implemented
#define INS_LDA_ABY 0xB9 // Implemented
#define INS_LDA_IDX 0xA1 // Implemented
#define INS_LDA_IDY 0xB1 // Implemented

// LDX, LDY: Load X Register, Load Y Register
// Flags: N-----Z-
#define INS_LDX_IM  0xA2 // Implemented
#define INS_LDX_ZP  0xA6 // Implemented
#define INS_LDX_ZPY 0xB6 // Implemented
#define INS_LDX_ABS 0xAE // Implemented
#define INS_LDX_ABY 0xBE // Implemented
#define INS_LDY_IM  0xA0 // Implemented
#define INS_LDY_ZP  0xA4 // Implemented
#define INS_LDY_ZPX 0xB4 // Implemented
#define INS_LDY_ABS 0xAC // Implemented
#define INS_LDY_ABX 0xBC // Implemented

// STA: Store Accumulator
// Flags: --------
#define INS_STA_ZP  0x85 // Implemented
#define INS_STA_ZPX 0x95 // Implemented
#define INS_STA_ABS 0x8D // Implemented
#define INS_STA_ABX 0x9D // Implemented
#define INS_STA_ABY 0x99 // Implemented
#define INS_STA_IDX 0x81 // Implemented
#define INS_STA_IDY 0x91 // Implemented

// STX, STY: Store X Reg, Store Y Reg
// Flags: --------
#define INS_STX_ZP  0x86 // Implemented
#define INS_STX_ZPY 0x96 // Implemented
#define INS_STX_ABS 0x8E // Implemented
#define INS_STY_ZP  0x84 // Implemented
#define INS_STY_ZPX 0x94 // Implemented
#define INS_STY_ABS 0x8C // Implemented

// DEC: Decrement Memory
// Flags: N-----Z-
#define INS_DEC_ZP  0xC6
#define INS_DEC_ZPX 0xD6
#define INS_DEC_ABS 0xCE
#define INS_DEC_ABX 0xDE

// INC: Increment Memory
// Flags: N-----Z-
#define INS_INC_ZP  0xE6
#define INS_INC_ZPX 0xF6
#define INS_INC_ABS 0xEE
#define INS_INC_ABX 0xFE

/* Register Instructions */

// Implied mode, length of one byte and requre 2 machine cycles
// Flags: N-----Z-
#define INS_TAX 0xAA // Implemented
#define INS_TAY 0xA8 // Implemented
#define INS_TXA 0x8A // Implemented
#define INS_TYA 0x98 // Implemented
#define INS_DEX 0xCA
#define INS_DEY 0x88
#define INS_INX 0xE8
#define INS_INY 0xC8

/* Stack Instructions */

// Implied mode, 1 byte.  Stack is on page $01 ($0100-$01FF), and works
// top-down.  Decrement on push, Increment on pull(pop)
// Flags --------
#define INS_PHA 0x48 // Implemented
#define INS_PHP 0x08 // Implemented
#define INS_TXS 0x9A // Implemented

// Flags: N-----Z-
#define INS_PLA 0x68 // Implemented
#define INS_TSX 0xBA // Implemented

// Flags: NV-BDIZC
#define INS_PLP 0x28 // Implemented

/* Other Instructions */
// Sets the B flag, then generates a forced interrupt.  Interrupt flag is ignored, 
// CPU goes through the normal interrupt process.  B flag can be used to distinguish
// BRK from standard interrupt.
// Flags: ---B----
#define INS_BRK 0x00

// 2 machine cycles, does not affect any register or mem location
// Flags: --------
#define INS_NOP 0xEA

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

typedef struct
{
  Word PC; // Program Counter
  Byte SP; // Stack Pointer

  Byte P; // Processor status: bits are NV-BDIZC

  Byte A; // Accumulator
  Byte X; // X Index Register
  Byte Y; // Y Index Register
  Byte Memory[MAX_MEMORY];
} CPU;

void initialize_memory (CPU *cpu);
void reset (CPU *cpu);
Byte fetch_byte (CPU *cpu, Sint32 *cycles);
void burn_byte(CPU *cpu, Sint32 *cycles);
Word fetch_word (CPU *cpu, Sint32 *cycles);
Byte read_byte (CPU *cpu, Word address, Sint32 *cycles);
Word read_word (CPU *cpu, Byte address, Sint32 *cycles);
void write_byte (CPU *cpu, Word address, Byte value, Sint32 *cycles);
void write_word (CPU *cpu, Word value, Byte address, Sint32 *cycles);
void set_status_flag (Byte *flags, Byte *value);
Word get_word_address (Byte loByte, Byte hiByte);
Sint32 execute (CPU *cpu);

#define CPU_H
#endif // !CPU_H
