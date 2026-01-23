#ifndef OPCODES_H_
#define OPCODES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "cpu.h"

typedef void (*OpcodeFunction)(CPU *cpu, Sint32 *cycles);

/* -------------------------------------------------------------------
 * Helper functions
 * -------------------------------------------------------------------*/

// Address functions
Word get_indirect_indexed_y (CPU *cpu, Sint32 *cycles, bool forcePageBoundary);
Word get_indexed_indirect_x (CPU *cpu, Sint32 *cycles);
Word get_addr_zpx (CPU *cpu, Sint32 *cycles);
Word get_addr_zpy (CPU *cpu, Sint32 *cycles);
Word get_addr_abs (CPU *cpu, Sint32 *cycles);
Word get_addr_abx (CPU *cpu, Sint32 *cycles, bool forcePageBoundary);
Word get_addr_aby (CPU *cpu, Sint32 *cycles, bool forcePageBoundary);

// Stack functions
void stack_push (CPU *cpu, Byte value, Sint32 *cycles);
Byte stack_pop (CPU *cpu, Sint32 *cycles);

// Logic functions
Byte perform_asl_logic (CPU *cpu, Byte value);
Byte perform_lsr_logic (CPU *cpu, Byte value);
Byte perform_rol_logic (CPU *cpu, Byte value);
Byte perform_ror_logic (CPU *cpu, Byte value);
Byte perform_eor_logic (CPU *cpu, Byte value);
Byte perform_ora_logic (CPU *cpu, Byte value);
void perform_bit_logic (CPU *cpu, Byte value);

// Math functions
void perform_adc_binary (CPU *cpu, Byte value);
void perform_adc_decimal (CPU *cpu, Byte value);
void perform_sbc_decimal (CPU *cpu, Byte value);

// Branch helpers
void execute_branch(CPU* cpu, Sint32 *cycles, bool condition);

/* -------------------------------------------------------------------
 * Operator functions
 * -------------------------------------------------------------------*/
// AND: Bitwise AND with Accumulator
void ins_and_im(CPU *cpu, Sint32 *cycles);
void ins_and_zp(CPU *cpu, Sint32 *cycles);
void ins_and_zpx(CPU *cpu, Sint32 *cycles);
void ins_and_abs (CPU *cpu, Sint32 *cycles);   // implemented
void ins_and_abx (CPU *cpu, Sint32 *cycles);   // implemented
void ins_and_aby (CPU *cpu, Sint32 *cycles);   // implemented
void ins_and_idx (CPU *cpu, Sint32 *cycles);   // implemented
void ins_and_idy (CPU *cpu, Sint32 *cycles);   // implemented


// ASL: Arithmetic Shift Left: bits shifted 1 position, 0 shifted into bit 0,
// bit 7 -> Carry Flags: N-----ZC
void ins_asl_acc (CPU *cpu, Sint32 *cycles);
void ins_asl_zp (CPU *cpu, Sint32 *cycles);
void ins_asl_zpx (CPU *cpu, Sint32 *cycles);
void ins_asl_abs (CPU *cpu, Sint32 *cycles);
void ins_asl_abx (CPU *cpu, Sint32 *cycles);

// EOR: Bitwise exclusive OR with ACC. Add 1 cycle if page boundary is crossed.
// Flags: N-----Z-
void ins_eor_im (CPU *cpu, Sint32 *cycles);
void ins_eor_zp (CPU *cpu, Sint32 *cycles);
void ins_eor_zpx (CPU *cpu, Sint32 *cycles);
void ins_eor_abs (CPU *cpu, Sint32 *cycles);
void ins_eor_abx (CPU *cpu, Sint32 *cycles);
void ins_eor_aby (CPU *cpu, Sint32 *cycles);
void ins_eor_idx (CPU *cpu, Sint32 *cycles);
void ins_eor_idy (CPU *cpu, Sint32 *cycles);

// LSR: Logical Shift Right: bits shifted 1 position.  0 shifted to bit 7, bit
// 0 -> Carry Flags: N-----ZC
void ins_lsr_acc (CPU *cpu, Sint32 *cycles);
void ins_lsr_zp (CPU *cpu, Sint32 *cycles);
void ins_lsr_zpx (CPU *cpu, Sint32 *cycles);
void ins_lsr_abs (CPU *cpu, Sint32 *cycles);
void ins_lsr_abx (CPU *cpu, Sint32 *cycles);

// ORA: Bitwise OR with ACC: add 1 cycle if page boundary is crossed
// Flags: N-----Z-
void ins_ora_im (CPU *cpu, Sint32 *cycles);
void ins_ora_zp (CPU *cpu, Sint32 *cycles);
void ins_ora_zpx (CPU *cpu, Sint32 *cycles);
void ins_ora_abs (CPU *cpu, Sint32 *cycles);
void ins_ora_abx (CPU *cpu, Sint32 *cycles);
void ins_ora_aby (CPU *cpu, Sint32 *cycles);
void ins_ora_idx (CPU *cpu, Sint32 *cycles);
void ins_ora_idy (CPU *cpu, Sint32 *cycles);

// ROL: Shift all bits left 1.  Carry shifted to 0, original 7 shifted to Carry
// Flags: N-----ZC
void ins_rol_acc (CPU *cpu, Sint32 *cycles);
void ins_rol_zp (CPU *cpu, Sint32 *cycles);
void ins_rol_zpx (CPU *cpu, Sint32 *cycles);
void ins_rol_abs (CPU *cpu, Sint32 *cycles);
void ins_rol_abx (CPU *cpu, Sint32 *cycles);

// ROR: Shift all bits right 1.  Carry to bit 7, original bit 0 to Carry
// Flags: N-----ZC
void ins_ror_acc (CPU *cpu, Sint32 *cycles);
void ins_ror_zp (CPU *cpu, Sint32 *cycles);
void ins_ror_zpx (CPU *cpu, Sint32 *cycles);
void ins_ror_abs (CPU *cpu, Sint32 *cycles);
void ins_ror_abx (CPU *cpu, Sint32 *cycles);

/* Branch Instructions */
// All branch instructions are relative mode, with a length of 2 bytes.
// Syntax: Bxx Displacement,  or Bxx Label.
// A branch not taken requires 2 machine cycles.  3 if taken, plus 1 if it
// crosses a page boundary. Crossing only occurs if the branch destination is
// on a different page than the instruction AFTER the branch instruction Flags:
// --------
void ins_bpl (CPU *cpu, Sint32 *cycles);
void ins_bmi (CPU *cpu, Sint32 *cycles);
void ins_bvc (CPU *cpu, Sint32 *cycles);
void ins_bvs (CPU *cpu, Sint32 *cycles);
void ins_bcc (CPU *cpu, Sint32 *cycles);
void ins_bcs (CPU *cpu, Sint32 *cycles);
void ins_bne (CPU *cpu, Sint32 *cycles);
void ins_beq (CPU *cpu, Sint32 *cycles);

/* Compare Instructions */

// CMP: Compare Accumulator.  Sets the processor flags as if a subtraction
// has been carried out
// Acc == Compared: Zero Flag
// ACC >= Compared: Carry Flag
// Flags: N-----ZC
void ins_cmp_im (CPU *cpu, Sint32 *cycles);
void ins_cmp_zp (CPU *cpu, Sint32 *cycles);
void ins_cmp_zpx (CPU *cpu, Sint32 *cycles);
void ins_cmp_abs (CPU *cpu, Sint32 *cycles);
void ins_cmp_abx (CPU *cpu, Sint32 *cycles);
void ins_cmp_aby (CPU *cpu, Sint32 *cycles);
void ins_cmp_idx (CPU *cpu, Sint32 *cycles);
void ins_cmp_idy (CPU *cpu, Sint32 *cycles);

// BIT: Test Bits
// Sets the Z flag as though value in addr tested were ANDed with ACC
// N, V are set equal to bits 7 and 6 of the value in the tested address
// Flags: NV----Z-
void ins_bit_zp (CPU *cpu, Sint32 *cycles);
void ins_bit_abs (CPU *cpu, Sint32 *cycles);

// CPX, CPY: Compare X, Y register.
// Identical to equivalent CMP operations
// Flags: N-----ZC
void ins_cpx_im (CPU *cpu, Sint32 *cycles);
void ins_cpx_zp (CPU *cpu, Sint32 *cycles);
void ins_cpx_abs (CPU *cpu, Sint32 *cycles);
void ins_cpy_im (CPU *cpu, Sint32 *cycles);
void ins_cpy_zp (CPU *cpu, Sint32 *cycles);
void ins_cpy_abs (CPU *cpu, Sint32 *cycles);

/* Flag Instructions */
// Instructions that change processor status flags
// Carry set/clear
// Flags: -------C
void ins_clc (CPU *cpu, Sint32 *cycles);
void ins_sec (CPU *cpu, Sint32 *cycles);

// Decimal set/clear
// Flags: ----D---
void ins_cld (CPU *cpu, Sint32 *cycles);
void ins_sed (CPU *cpu, Sint32 *cycles);

// Interrupt set/clear
// Flags: -----I--
void ins_cli (CPU *cpu, Sint32 *cycles);
void ins_sei (CPU *cpu, Sint32 *cycles);

// Overflow set/clear
// Flags: -V------
void ins_clv (CPU *cpu, Sint32 *cycles);

/* Jump Instructions */
// JMP: No carry associated.  Indirect JMP must never use a vector
// beginning on the last byte of a page
// JMP ($30FF) will read low byte from $30FF, and high byte from $3000
void ins_jsr (CPU *cpu, Sint32 *cycles); 
void ins_jmp_ind (CPU *cpu, Sint32 *cycles);
void ins_jmp_abs (CPU *cpu, Sint32 *cycles);
void ins_jsr_abs (CPU *cpu, Sint32 *cycles); 
void ins_rts (CPU *cpu, Sint32 *cycles);

// RTI: Return from Interrupt.  Retrieves the Processor Status byte and PC from
// stack in that order.  Unlike RTS, return address on the stack is the actual
// address rather than address-1 Flags: NV-BDIZC
void ins_rti (CPU *cpu, Sint32 *cycles);

/* Math Instructions */
// Add 1 cycle if page boundary is crossed ADC, AND, SBC
// Flags: NV----ZC
void ins_adc_im (CPU *cpu, Sint32 *cycles);
void ins_adc_zp (CPU *cpu, Sint32 *cycles);
void ins_adc_zpx (CPU *cpu, Sint32 *cycles);
void ins_adc_abs (CPU *cpu, Sint32 *cycles);
void ins_adc_abx (CPU *cpu, Sint32 *cycles);
void ins_adc_aby (CPU *cpu, Sint32 *cycles);
void ins_adc_idx (CPU *cpu, Sint32 *cycles);
void ins_adc_idy (CPU *cpu, Sint32 *cycles);

void ins_sbc_im (CPU *cpu, Sint32 *cycles);
void ins_sbc_zp (CPU *cpu, Sint32 *cycles);
void ins_sbc_zpx (CPU *cpu, Sint32 *cycles);
void ins_sbc_abs (CPU *cpu, Sint32 *cycles);
void ins_sbc_abx (CPU *cpu, Sint32 *cycles);
void ins_sbc_aby (CPU *cpu, Sint32 *cycles);
void ins_sbc_idx (CPU *cpu, Sint32 *cycles);
void ins_sbc_idy (CPU *cpu, Sint32 *cycles);

/* Memory Instructions */

// LDA: Load Accumulator.  Add 1 cycle if page boundary is crossed
// Flags: N-----Z-
void ins_lda_im (CPU *cpu, Sint32 *cycles);  
void ins_lda_zp (CPU *cpu, Sint32 *cycles);  
void ins_lda_zpx (CPU *cpu, Sint32 *cycles); 
void ins_lda_abs (CPU *cpu, Sint32 *cycles); 
void ins_lda_abx (CPU *cpu, Sint32 *cycles); 
void ins_lda_aby (CPU *cpu, Sint32 *cycles); 
void ins_lda_idx (CPU *cpu, Sint32 *cycles); 
void ins_lda_idy (CPU *cpu, Sint32 *cycles); 

// LDX, LDY: Load X Register, Load Y Register
// Flags: N-----Z-
void ins_ldx_im (CPU *cpu, Sint32 *cycles);  
void ins_ldx_zp (CPU *cpu, Sint32 *cycles);  
void ins_ldx_zpy (CPU *cpu, Sint32 *cycles); 
void ins_ldx_abs (CPU *cpu, Sint32 *cycles); 
void ins_ldx_aby (CPU *cpu, Sint32 *cycles); 
void ins_ldy_im (CPU *cpu, Sint32 *cycles);  
void ins_ldy_zp (CPU *cpu, Sint32 *cycles);  
void ins_ldy_zpx (CPU *cpu, Sint32 *cycles); 
void ins_ldy_abs (CPU *cpu, Sint32 *cycles); 
void ins_ldy_abx (CPU *cpu, Sint32 *cycles); 

// Illegal LDs

void ins_lax_im (CPU *cpu, Sint32 *cycles);  
void ins_lax_zp (CPU *cpu, Sint32 *cycles);  
void ins_lax_zpy (CPU *cpu, Sint32 *cycles); 
void ins_lax_abs (CPU *cpu, Sint32 *cycles); 
void ins_lax_aby (CPU *cpu, Sint32 *cycles); 
void ins_lax_idx (CPU *cpu, Sint32 *cycles); 
void ins_lax_idy (CPU *cpu, Sint32 *cycles); 

// STA: Store Accumulator
// Flags: --------
void ins_sta_zp (CPU *cpu, Sint32 *cycles);  
void ins_sta_zpx (CPU *cpu, Sint32 *cycles); 
void ins_sta_abs (CPU *cpu, Sint32 *cycles); 
void ins_sta_abx (CPU *cpu, Sint32 *cycles); 
void ins_sta_aby (CPU *cpu, Sint32 *cycles); 
void ins_sta_idx (CPU *cpu, Sint32 *cycles); 
void ins_sta_idy (CPU *cpu, Sint32 *cycles); 

// STX, STY: Store X Reg, Store Y Reg
// Flags: --------
void ins_stx_zp (CPU *cpu, Sint32 *cycles);  
void ins_stx_zpy (CPU *cpu, Sint32 *cycles); 
void ins_stx_abs (CPU *cpu, Sint32 *cycles); 
void ins_sty_zp (CPU *cpu, Sint32 *cycles);  
void ins_sty_zpx (CPU *cpu, Sint32 *cycles); 
void ins_sty_abs (CPU *cpu, Sint32 *cycles); 

// DEC: Decrement Memory
// Flags: N-----Z-
void ins_dec_zp (CPU *cpu, Sint32 *cycles);  
void ins_dec_zpx (CPU *cpu, Sint32 *cycles); 
void ins_dec_abs (CPU *cpu, Sint32 *cycles); 
void ins_dec_abx (CPU *cpu, Sint32 *cycles); 

// INC: Increment Memory
// Flags: N-----Z-
void ins_inc_zp (CPU *cpu, Sint32 *cycles); 
void ins_inc_zpx (CPU *cpu, Sint32 *cycles); 
void ins_inc_abs (CPU *cpu, Sint32 *cycles); 
void ins_inc_abx (CPU *cpu, Sint32 *cycles); 

/* Register Instructions */

// Implied mode, length of one byte and requre 2 machine cycles
// Flags: N-----Z-
void ins_tax (CPU *cpu, Sint32 *cycles); 
void ins_tay (CPU *cpu, Sint32 *cycles); 
void ins_txa (CPU *cpu, Sint32 *cycles); 
void ins_tya (CPU *cpu, Sint32 *cycles); 
void ins_dex (CPU *cpu, Sint32 *cycles);
void ins_dey (CPU *cpu, Sint32 *cycles);
void ins_inx (CPU *cpu, Sint32 *cycles);
void ins_iny (CPU *cpu, Sint32 *cycles);

/* Stack Instructions */

// Implied mode, 1 byte.  Stack is on page $01 ($0100-$01FF), and works
// top-down.  Decrement on push, Increment on pull(pop)
// Flags --------
void ins_pha (CPU *cpu, Sint32 *cycles); 
void ins_php (CPU *cpu, Sint32 *cycles); 
void ins_txs (CPU *cpu, Sint32 *cycles); 

// Flags: N-----Z-
void ins_pla (CPU *cpu, Sint32 *cycles); 
void ins_tsx (CPU *cpu, Sint32 *cycles); 

// Flags: NV-BDIZC
void ins_plp (CPU *cpu, Sint32 *cycles); 

/* Other Instructions */
// Sets the B flag, then generates a forced interrupt.  Interrupt flag is
// ignored, CPU goes through the normal interrupt process.  B flag can be used
// to distinguish BRK from standard interrupt. Flags: ---B----
void ins_brk (CPU *cpu, Sint32 *cycles);

// 2 machine cycles, does not affect any register or mem location
// Flags: --------
void ins_nop (CPU *cpu, Sint32 *cycles);
void ins_illegal (CPU *cpu, Sint32 *cycles);
#ifdef __cplusplus
}
#endif

#endif
