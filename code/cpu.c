#include "cpu.h"
#include "opcodes.h"
#include <stdbool.h>
#include <stdio.h>

static const OpcodeFunction opcodes[256] = {
  ins_brk,     ins_ora_idx, NULL,        NULL,        NULL,        ins_ora_zp,
  ins_asl_zp,  NULL,        ins_php,     ins_ora_im,  ins_asl_acc, NULL,
  NULL,        ins_ora_abs, ins_asl_abs, NULL,        ins_bpl,     ins_ora_idy,
  NULL,        NULL,        NULL,        ins_ora_zpx, ins_asl_zpx, NULL,
  ins_clc,     ins_ora_aby, NULL,        NULL,        NULL,        ins_ora_abx,
  ins_asl_abx, NULL,        ins_jsr,     ins_and_idx, NULL,        NULL,
  ins_bit_zp,  ins_and_zp,  ins_rol_zp,  NULL,        ins_plp,     ins_and_im,
  ins_rol_acc, NULL,        ins_bit_abs, ins_and_abs, ins_rol_abs, NULL,
  ins_bmi,     ins_and_idy, NULL,        NULL,        NULL,        ins_and_zpx,
  ins_rol_zpx, NULL,        ins_sec,     ins_and_aby, NULL,        NULL,
  NULL,        ins_and_abx, ins_rol_abx, NULL,        ins_rti,     ins_eor_idx,
  NULL,        NULL,        NULL,        ins_eor_zp,  ins_lsr_zp,  NULL,
  ins_pha,     ins_eor_im,  ins_lsr_acc, NULL,        ins_jmp_abs, ins_eor_abs,
  ins_lsr_abs, NULL,        ins_bvc,     ins_eor_idy, NULL,        NULL,
  NULL,        ins_eor_zpx, ins_lsr_zpx, NULL,        ins_cli,     ins_eor_aby,
  NULL,        NULL,        NULL,        ins_eor_abx, ins_lsr_abx, NULL,
  ins_rts,     ins_adc_idx, NULL,        NULL,        NULL,        ins_adc_zp,
  ins_ror_zp,  NULL,        ins_pla,     ins_adc_im,  ins_ror_acc, NULL,
  ins_jmp_ind, ins_adc_abs, ins_ror_abs, NULL,        ins_bvs,     ins_adc_idy,
  NULL,        NULL,        NULL,        ins_adc_zpx, ins_ror_zpx, NULL,
  ins_sei,     ins_adc_aby, NULL,        NULL,        NULL,        ins_adc_abx,
  ins_ror_abx, NULL,        NULL,        ins_sta_idx, NULL,        NULL,
  ins_sty_zp,  ins_sta_zp,  ins_stx_zp,  NULL,        ins_dey,     NULL,
  ins_txa,     NULL,        ins_sty_abs, ins_sta_abs, ins_stx_abs, NULL,
  ins_bcc,     ins_sta_idy, NULL,        NULL,        ins_sty_zpx, ins_sta_zpx,
  ins_stx_zpy, NULL,        ins_tya,     ins_sta_aby, ins_txs,     NULL,
  NULL,        ins_sta_abx, NULL,        NULL,        ins_ldy_im,  ins_lda_idx,
  ins_ldx_im,  NULL,        ins_ldy_zp,  ins_lda_zp,  ins_ldx_zp,  NULL,
  ins_tay,     ins_lda_im,  ins_tax,     NULL,        ins_ldy_abs, ins_lda_abs,
  ins_ldx_abs, NULL,        ins_bcs,     ins_lda_idy, NULL,        NULL,
  ins_ldy_zpx, ins_lda_zpx, ins_ldx_zpy, NULL,        ins_clv,     ins_lda_aby,
  ins_tsx,     NULL,        ins_ldy_abx, ins_lda_abx, ins_ldx_aby, NULL,
  ins_cpy_im,  ins_cmp_idx, NULL,        NULL,        ins_cpy_zp,  ins_cmp_zp,
  ins_dec_zp,  NULL,        ins_iny,     ins_cmp_im,  ins_dex,     NULL,
  ins_cpy_abs, ins_cmp_abs, ins_dec_abs, NULL,        ins_bne,     ins_cmp_idy,
  NULL,        NULL,        NULL,        ins_cmp_zpx, ins_dec_zpx, NULL,
  ins_cld,     ins_cmp_aby, NULL,        NULL,        NULL,        ins_cmp_abx,
  ins_dec_abx, NULL,        ins_cpx_im,  ins_sbc_idx, NULL,        NULL,
  ins_cpx_zp,  ins_sbc_zp,  ins_inc_zp,  NULL,        ins_inx,     ins_sbc_im,
  ins_nop,     NULL,        ins_cpx_abs, ins_sbc_abs, ins_inc_abs, NULL,
  ins_beq,     ins_sbc_idy, NULL,        NULL,        NULL,        ins_sbc_zpx,
  ins_inc_zpx, NULL,        ins_sed,     ins_sbc_aby, NULL,        NULL,
  NULL,        ins_sbc_abx, ins_inc_abx, NULL
};

void
initialize_memory (CPU *cpu)
{
  for (Uint32 i = 0; i < MAX_MEMORY; i++)
    {
      cpu->Memory[i] = 0;
    }
  cpu->Memory[0x00] = 0xFF;
  cpu->Memory[0x01] = 0x07;
}

void
reset (CPU *cpu)
{
  /* TODO: Implement MOS 6510 System Reset routine
   * From documentation:
   * ; Reset vector (Kernel address $FFFC) points here.
   * ;
   * ; If cartridge is detected then cartridge cold start routine is
   * activated. ; If no cartridge is detected then I/O and memory are
   * initialised and BASIC ; cold start routine is activated
   *
   * FCE2  A2 FF     LDX #$FF        ;
   * FCE4  78        SEI             ; set interrupt disable
   * FCE5  9A        TXS             ; transfer .X to stack
   * FCE6  D8        CLD             ; clear decimal flag
   * FCE7  20 02 FD  JSR $FD02       ; check for cart
   * FCEA  D0 03     BNE $FCEF       ; .Z=0? then no cart detected
   * FCEC  6C 00 80  JMP ($8000)     ; direct to cartridge cold start via
   * vector FCEF  8E 16 D0  STX $D016       ; sets bit 5 (MCM) off, bit 3 (38
   * cols) off FCF2  20 A3 FD  JSR $FDA3       ; initialise I/O FCF5  20 50
   * FD  JSR $FD50       ; initialise memory FCF8  20 15 FD  JSR $FD15 ; set
   * I/O vectors ($0314..$0333) to kernel defaults FCFB  20 5B FF  JSR $FF5B
   * ; more initialising... mostly set system IRQ to correct value and start
   * FCFE  58        CLI             ; clear interrupt flag FCFF  6C 00 A0
   * JMP ($A000)                     ; direct to BASIC cold start via vector
   */

  cpu->PC = 0xFFFC;
  cpu->SP = 0xFF;

  cpu->A = 0x0;
  cpu->X = 0x0;
  cpu->Y = 0x0;

  cpu->P = FLAG_UNDEFINED | FLAG_INTERRUPT_DISABLE;

  initialize_memory (cpu);
}

Byte
fetch_byte (CPU *cpu, Sint32 *cycles)
{
  Byte Data = cpu->Memory[cpu->PC];
  cpu->PC++;
  *cycles += 1;
  return (Data);
}

// TODO: This reads the current byte from the PC, and ignores the result.
// Would it be better to just increment the Cycle count and the PC register?
//  Pros: don't need this function, just increment
//  Cons: This more accurately describes what is happening.
void
burn_cycle (CPU *cpu, Sint32 *cycles)
{
  Byte data = read_byte (cpu, cpu->PC, cycles);
}

Word
fetch_word (CPU *cpu, Sint32 *cycles)
{
  Word data = cpu->Memory[cpu->PC];
  cpu->PC++;

  *cycles += 1;
  data |= (cpu->Memory[cpu->PC] << 8);
  cpu->PC++;

  *cycles += 1;
  return (data);
}

// TODO: Ensure a Byte passed to this function has only the Low Byte set.
Byte
read_byte (CPU *cpu, Word address, Sint32 *cycles)
{
  Byte data = cpu->Memory[address];
  *cycles += 1;
  return (data);
}

Word
read_word (CPU *cpu, Byte address, Sint32 *cycles)
{
  Word Data = cpu->Memory[address];
  *cycles += 1;
  Data = cpu->Memory[address + 1] << 8;
  *cycles += 1;

  return (Data);
}
void
write_byte (CPU *cpu, Word address, Byte value, Sint32 *cycles)
{
  cpu->Memory[address] = value;
  *cycles += 1;
}

void
write_word (CPU *cpu, Word value, Byte address, Sint32 *cycles)
{
  cpu->Memory[address] = value & 0xFF;
  cpu->Memory[address - 1] = (value >> 8);
  *cycles += 2;
}

void
set_status_flag (Byte *flags, Byte value)
{
  // If value is 0, apply FLAG_ZERO mask; otherwise, apply an empty mask (0)
  // Then clear the bit and OR the result back in.
  *flags = (*flags & ~FLAG_ZERO) | (value == 0 ? FLAG_ZERO : 0);

  // If bit 7 is set, apply FLAG_NEGATIVE mask; otherwise, apply 0
  *flags = (*flags & ~FLAG_NEGATIVE) | (value & 0x80 ? FLAG_NEGATIVE : 0);
}

void set_flag(Byte *flags, Byte flagToSet) {
  *flags |= flagToSet;
}

void clear_flag(Byte *flags, Byte flagToClear) {
  *flags &= ~flagToClear;
}

Byte get_carry_flag (CPU *cpu)
{
  return (cpu->P & FLAG_CARRY) ? 1 : 0;
}

Word
get_word_address (Byte loByte, Byte hiByte)
{
  Word address = loByte + (hiByte << 8);
  return address;
}

// TODO:  If we want to emulate something cycle exact, we would want these
// instructions to be designed in a way that there are discrete steps:
//        - Cycle 1: Get instruction from the Program Counter
//        - Cycle 2..# Perform the next single action (Read a byte, transfer a
//        register, increment, etc)
//
//       In order to do this, we will need to figure out a way to either queue
//       the individual actions after the instruction is pulled, or a way to
//       tell the Instruction case which cycle we are on.
//
//       Other considerations:
//        - We have a fixed number of addressing modes.  With the exception of
//        a few details, every
//          instruction takes the same steps in each addressing mode.
Sint32
execute (CPU *cpu)
{

  Sint32 cycles = 0;

  Byte instruction = fetch_byte (cpu, &cycles); // One cycle

  if (opcodes[instruction] != NULL)
    {
      opcodes[instruction](cpu, &cycles);
    }
  else
    {
      printf ("Operation not handled");
    }

  return cycles;
}
