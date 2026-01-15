#include "cpu.h"
#include <stdio.h>

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
  cpu->PC = 0xFFFC;
  cpu->SP = 0xFF;

  cpu->A = 0x0;
  cpu->X = 0x0;
  cpu->Y = 0x0;

  cpu->P = 0b00000000;

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
burn_byte (CPU *cpu, Sint32 *cycles)
{
  Byte data = cpu->Memory[cpu->PC];
  cpu->PC++;
  *cycles += 1;
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
set_status_flag (Byte *flags, Byte *value)
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
  if (*value == 0)
    {
      *flags |= FLAG_ZERO;
    }
  *flags |= (*value & FLAG_NEGATIVE);
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
  switch (instruction)
    {

    /* Jump Instructions */
    case INS_JMP_ABS:
      {
        Byte loByte = fetch_byte (cpu, &cycles);
        Byte hiByte = fetch_byte (cpu, &cycles);

        cpu->PC = get_word_address (loByte, hiByte);
        cycles++;
      }
      break;
    case INS_JMP_IND:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_JSR_ABS:
      {
        Byte loByte = fetch_byte (cpu, &cycles);
        Byte hiByte = fetch_byte (cpu, &cycles);

        Word address = get_word_address (loByte, hiByte);
        write_word (cpu, cpu->PC - 1, cpu->SP, &cycles);
        cpu->SP -= 1;
        cpu->PC = address;
        cycles++;
      }
      break;
    case INS_RTS:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_RTI:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;

    /* Math Instructions */
    case INS_ADC_IM:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_ADC_ZP:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_ADC_ZPX:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_ADC_ABS:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_ADC_ABX:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_ADC_ABY:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_ADC_IDX:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_ADC_IDY:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_SBC_IM:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_SBC_ZP:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_SBC_ZPX:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_SBC_ABS:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_SBC_ABX:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_SBC_ABY:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_SBC_IDX:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_SBC_IDY:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;

    /* Memory Instructions */
    case INS_LDA_IM:
      {
        Byte Value = fetch_byte (cpu, &cycles);
        cpu->A = Value;
        set_status_flag (&cpu->P, &cpu->A);
      }
      break;
    case INS_LDA_ZP:
      {
        Byte address = fetch_byte (cpu, &cycles);
        cpu->A = read_byte (cpu, address, &cycles);
        set_status_flag (&cpu->P, &cpu->A);
      }
      break;
    case INS_LDA_ZPX:
      {
        Byte address = fetch_byte (cpu, &cycles);
        address = (address + cpu->X) & 0x00FF;
        cycles++;
        cpu->A = read_byte (cpu, address, &cycles);
        set_status_flag (&cpu->P, &cpu->A);
      }
      break;
    case INS_LDA_ABS:
      {
        Byte loByte = fetch_byte (cpu, &cycles);
        Byte hiByte = fetch_byte (cpu, &cycles);

        Word address = get_word_address (loByte, hiByte);

        cpu->A = read_byte (cpu, address, &cycles);

        set_status_flag (&cpu->P, &cpu->A);
      }
      break;
    case INS_LDA_ABX:
      {
        Byte loByte = fetch_byte (cpu, &cycles);
        Byte hiByte = fetch_byte (cpu, &cycles);

        Word address = get_word_address (loByte, hiByte);
        Byte hiByteBefore = address >> 8;

        address += cpu->X;
        Byte hiByteAfter = address >> 8;

        cpu->A = read_byte (cpu, address, &cycles);
        set_status_flag (&cpu->P, &cpu->A);
        if (hiByteBefore != hiByteAfter)
          {
            cycles++;
          }
      }
      break;
    case INS_LDA_ABY:
      {

        Byte loByte = fetch_byte (cpu, &cycles);
        Byte hiByte = fetch_byte (cpu, &cycles);

        Word address = get_word_address (loByte, hiByte);
        Byte addrHiByte = address >> 8;

        address += cpu->Y;
        Byte addrAfterhiByte = address >> 8;

        cpu->A = read_byte (cpu, address, &cycles);
        set_status_flag (&cpu->P, &cpu->A);
        if (addrHiByte != addrAfterhiByte)
          {
            cycles++;
          }
      }
      break;
    case INS_LDA_IDX:
      {
        Byte address = fetch_byte (cpu, &cycles);
        address += cpu->X;

        Byte loByte = read_byte (cpu, address, &cycles);
        address += 1;
        Byte hiByte = read_byte (cpu, address, &cycles);

        Word targetAddress = get_word_address (loByte, hiByte);
        cycles += 1;
        cpu->A = read_byte (cpu, targetAddress, &cycles);
        set_status_flag (&cpu->P, &cpu->A);
      }
      break;
    case INS_LDA_IDY:
      {
        Byte address = fetch_byte (cpu, &cycles);

        Byte loByte = read_byte (cpu, address, &cycles);
        address += 1;
        Byte hiByte = read_byte (cpu, address, &cycles);

        Word targetAddress = get_word_address (loByte, hiByte);
        targetAddress += cpu->Y;

        Byte addrAfterhiByte = targetAddress >> 8;

        cpu->A = read_byte (cpu, targetAddress, &cycles);
        set_status_flag (&cpu->P, &cpu->A);
        if (addrAfterhiByte != hiByte)
          {
            cycles++;
          }
      }
      break;
    case INS_LDX_IM:
      {
        Byte Value = fetch_byte (cpu, &cycles);
        cpu->X = Value;
        set_status_flag (&cpu->P, &cpu->X);
      }
      break;
    case INS_LDX_ZP:
      {
        Byte address = fetch_byte (cpu, &cycles);
        cpu->X = read_byte (cpu, address, &cycles);
        set_status_flag (&cpu->P, &cpu->X);
      }
      break;
    case INS_LDX_ZPY:
      {
        Byte address = fetch_byte (cpu, &cycles);
        address += cpu->Y;
        cycles++;
        cpu->X = read_byte (cpu, address, &cycles);
        set_status_flag (&cpu->P, &cpu->X);
      }
      break;
    case INS_LDX_ABS:
      {
        Byte loByte = fetch_byte (cpu, &cycles);
        Byte hiByte = fetch_byte (cpu, &cycles);

        Word address = get_word_address (loByte, hiByte);
        cpu->X = read_byte (cpu, address, &cycles);
        set_status_flag (&cpu->P, &cpu->X);
      }
      break;
    case INS_LDX_ABY:
      {

        Byte loByte = fetch_byte (cpu, &cycles);
        Byte hiByte = fetch_byte (cpu, &cycles);

        Word address = get_word_address (loByte, hiByte);
        Byte AddrhiByte = address >> 8;

        address += cpu->Y;
        Byte AddrAfterhiByte = address >> 8;

        cpu->X = read_byte (cpu, address, &cycles);
        set_status_flag (&cpu->P, &cpu->X);
        if (AddrhiByte != AddrAfterhiByte)
          {
            cycles++;
          }
      }
      break;
    case INS_LDY_IM:
      {
        Byte Value = fetch_byte (cpu, &cycles);
        cpu->Y = Value;
        set_status_flag (&cpu->P, &cpu->Y);
      }
      break;
    case INS_LDY_ZP:
      {
        Byte address = fetch_byte (cpu, &cycles);
        cpu->Y = read_byte (cpu, address, &cycles);
        set_status_flag (&cpu->P, &cpu->Y);
      }
      break;
    case INS_LDY_ZPX:
      {
        Byte address = fetch_byte (cpu, &cycles);
        address += cpu->X;
        cycles++;
        cpu->Y = read_byte (cpu, address, &cycles);
        set_status_flag (&cpu->P, &cpu->Y);
      }
      break;
    case INS_LDY_ABS:
      {
        Byte loByte = fetch_byte (cpu, &cycles);
        Byte hiByte = fetch_byte (cpu, &cycles);

        Word address = get_word_address (loByte, hiByte);
        cpu->Y = read_byte (cpu, address, &cycles);
        set_status_flag (&cpu->P, &cpu->Y);
      }
      break;
    case INS_LDY_ABX:
      {
        Byte loByte = fetch_byte (cpu, &cycles);
        Byte hiByte = fetch_byte (cpu, &cycles);

        Word address = get_word_address (loByte, hiByte);
        Byte AddrhiByte = address >> 8;

        address += cpu->X;
        Byte AddrAfterhiByte = address >> 8;

        cpu->Y = read_byte (cpu, address, &cycles);
        set_status_flag (&cpu->P, &cpu->Y);
        if (AddrhiByte != AddrAfterhiByte)
          {
            cycles++;
          }
      }
      break;
    case INS_STA_ZP:
      {
        Byte address = fetch_byte (cpu, &cycles);
        write_byte (cpu, address, cpu->A, &cycles);
      }
      break;
    case INS_STA_ZPX:
      {
        Byte address = fetch_byte (cpu, &cycles);
        address = (address + cpu->X) & 0x00FF;
        cycles++;
        write_byte (cpu, address, cpu->A, &cycles);
      }
      break;
    case INS_STA_ABS:
      {
        Byte loByte = fetch_byte (cpu, &cycles);
        Byte hiByte = fetch_byte (cpu, &cycles);

        Word address = get_word_address (loByte, hiByte);

        write_byte (cpu, address, cpu->A, &cycles);
      }
      break;
    case INS_STA_ABX:
      {
        Byte loByte = fetch_byte (cpu, &cycles);
        Byte hiByte = fetch_byte (cpu, &cycles);

        Word address = get_word_address (loByte, hiByte);
        Byte hiByteBefore = address >> 8;

        address += cpu->X;
        Byte hiByteAfter = address >> 8;

        write_byte (cpu, address, cpu->A, &cycles);

        if (hiByteBefore != hiByteAfter)
          {
            cycles++;
          }
      }
      break;
    case INS_STA_ABY:
      {

        Byte loByte = fetch_byte (cpu, &cycles);
        Byte hiByte = fetch_byte (cpu, &cycles);

        Word address = get_word_address (loByte, hiByte);
        Byte AddrhiByte = address >> 8;

        address += cpu->Y;
        Byte AddrAfterhiByte = address >> 8;

        write_byte (cpu, address, cpu->A, &cycles);
        set_status_flag (&cpu->P, &cpu->A);
        if (AddrhiByte != AddrAfterhiByte)
          {
            cycles++;
          }
      }
      break;
    case INS_STA_IDX:
      {
        Byte address = fetch_byte (cpu, &cycles);
        address += cpu->X;

        Byte loByte = read_byte (cpu, address, &cycles);
        address += 1;
        Byte hiByte = read_byte (cpu, address, &cycles);

        Word targetAddress = get_word_address (loByte, hiByte);
        cycles += 1;
        write_byte (cpu, targetAddress, cpu->A, &cycles);
      }
      break;
    case INS_STA_IDY:
      {

        Byte address = fetch_byte (cpu, &cycles);

        Byte loByte = read_byte (cpu, address, &cycles);
        address += 1;
        Byte hiByte = read_byte (cpu, address, &cycles);

        Word targetAddress = get_word_address (loByte, hiByte);
        targetAddress += cpu->Y;

        Byte addrAfterhiByte = targetAddress >> 8;

        write_byte (cpu, targetAddress, cpu->A, &cycles);
        if (addrAfterhiByte != hiByte)
          {
            cycles++;
          }
      }
      break;
    case INS_STX_ZP:
      {
        Byte address = fetch_byte (cpu, &cycles);
        write_byte (cpu, address, cpu->X, &cycles);
      }
      break;
    case INS_STX_ZPY:
      {
        Byte address = fetch_byte (cpu, &cycles);
        address = (address + cpu->Y) & 0x00FF;
        cycles++;
        write_byte (cpu, address, cpu->X, &cycles);
      };
    case INS_STX_ABS:
      {
        Byte loByte = fetch_byte (cpu, &cycles);
        Byte hiByte = fetch_byte (cpu, &cycles);

        Word address = get_word_address (loByte, hiByte);
        write_byte (cpu, address, cpu->X, &cycles);
      }
      break;
    case INS_STY_ZP:
      {
        Byte address = fetch_byte (cpu, &cycles);
        write_byte (cpu, address, cpu->Y, &cycles);
      }
      break;
    case INS_STY_ZPX:
      {
        Byte address = fetch_byte (cpu, &cycles);
        address = (address + cpu->X) & 0x00FF;
        cycles++;
        write_byte (cpu, address, cpu->Y, &cycles);
      }
      break;
    case INS_STY_ABS:
      {
        Byte loByte = fetch_byte (cpu, &cycles);
        Byte hiByte = fetch_byte (cpu, &cycles);

        Word address = get_word_address (loByte, hiByte);

        write_byte (cpu, address, cpu->Y, &cycles);
      }
      break;
    case INS_DEC_ZP:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_DEC_ZPX:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_DEC_ABS:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_DEC_ABX:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;

    case INS_INC_ZP:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_INC_ZPX:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_INC_ABS:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_INC_ABX:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;

    /* Register Instructions */
    case INS_TAX:
      {
        burn_byte (cpu, &cycles);
        cpu->X = cpu->A;
        set_status_flag (&cpu->P, &cpu->X);
      }
      break;
    case INS_TAY:
      {
        burn_byte (cpu, &cycles);
        cpu->Y = cpu->A;
        set_status_flag (&cpu->P, &cpu->Y);
      }
      break;
    case INS_TXA:
      {
        burn_byte (cpu, &cycles);
        cpu->A = cpu->X;
        set_status_flag (&cpu->P, &cpu->A);
      }
      break;
    case INS_TYA:
      {
        burn_byte (cpu, &cycles);
        cpu->A = cpu->Y;
        set_status_flag (&cpu->P, &cpu->A);
      }
      break;
    case INS_DEX:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_DEY:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_INX:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_INY:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;

    /* Stack Instructions */
    case INS_PHA:
      {
        burn_byte (cpu, &cycles);
        Word address = 0x0100 + cpu->SP;
        write_byte (cpu, address, cpu->A, &cycles);
        cpu->SP--;
      }
      break; 
    case INS_PHP:
      {
        burn_byte (cpu, &cycles);
        Word address = 0x0100 + cpu->SP;
        write_byte (cpu, address, cpu->P, &cycles);
        cpu->SP--;
      }
      break;
    case INS_TXS:
      {
        burn_byte (cpu, &cycles);
        cpu->SP = cpu->X;
      }
      break;
    case INS_PLA:
      {
        burn_byte (cpu, &cycles);
        cpu->SP++;
        cycles++;
        Word address = 0x0100 + cpu->SP;
        cpu->A = read_byte (cpu, address, &cycles);
        set_status_flag (&cpu->P, &cpu->A);
      }
      break;
    case INS_TSX:
      {
        burn_byte (cpu, &cycles);
        cpu->X = cpu->SP;
        set_status_flag (&cpu->P, &cpu->X);
      }
      break;
    case INS_PLP:
      {
        burn_byte (cpu, &cycles);
        cpu->SP++;
        cycles++;
        Word address = 0x0100 + cpu->SP;
        cpu->P = read_byte (cpu, address, &cycles);
      }
      break;

    /* Other Instructions */
    case INS_BRK:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;
    case INS_NOP:
      {
        // TODO: Implement this
        printf ("Operation not handled %d\n", instruction);
      }
      break;

    default:
      {
        printf ("Operation not handled %d\n", instruction);
      };
    }

  return cycles;
}
