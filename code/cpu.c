#include "cpu.h"
#include <stdio.h>

void InitializeMemory(CPU *cpu) {
  for (Uint32 i = 0; i < MAX_MEMORY; i++) {
    cpu->Memory[i] = 0;
  }
  cpu->Memory[0x00] = 0xFF;
  cpu->Memory[0x01] = 0x07;
}

void Reset(CPU *cpu) {
  cpu->PC = 0xFFFC;
  cpu->SP = 0xFF;

  cpu->A = 0x0;
  cpu->X = 0x0;
  cpu->Y = 0x0;

  cpu->P = 0b00000000;

  InitializeMemory(cpu);
}

Byte FetchByte(CPU *cpu, Sint32 *Cycles) {
  Byte Data = cpu->Memory[cpu->PC];
  cpu->PC++;
  *Cycles += 1;
  return (Data);
}

// TODO: This reads the current byte from the PC, and ignores the result.
// Would it be better to just increment the Cycle count and the PC register?
//  Pros: don't need this function, just increment
//  Cons: This more accurately describes what is happening.
void BurnByte(CPU *cpu, Sint32 *Cycles) {
  Byte Data = cpu->Memory[cpu->PC];
  cpu->PC++;
  *Cycles += 1;
}

Word FetchWord(CPU *cpu, Sint32 *Cycles) {
  Word Data = cpu->Memory[cpu->PC];
  cpu->PC++;

  *Cycles += 1;
  Data |= (cpu->Memory[cpu->PC] << 8);
  cpu->PC++;

  *Cycles += 1;
  return (Data);
}

// TODO: Ensure a Byte passed to this function has only the Low Byte set.
Byte ReadByte(CPU *cpu, Word Address, Sint32 *Cycles) {
  Byte Data = cpu->Memory[Address];
  *Cycles += 1;
  return (Data);
}

Word ReadWord(CPU *cpu, Byte Address, Sint32 *Cycles) {
  Word Data = cpu->Memory[Address];
  *Cycles += 1;
  Data = cpu->Memory[Address + 1] << 8;
  *Cycles += 1;

  return (Data);
}
void WriteByte(CPU *cpu, Word Address, Byte Value, Sint32 *Cycles) {
  cpu->Memory[Address] = Value;
  *Cycles += 1;
}

void WriteWord(CPU *cpu, Word Value, Byte Address, Sint32 *Cycles) {
  cpu->Memory[Address] = Value & 0xFF;
  cpu->Memory[Address - 1] = (Value >> 8);
  *Cycles += 2;
}

void SetStatusFlag(Byte *flags, Byte *Value) {
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
  if (*Value == 0) {
    *flags |= FLAG_ZERO;
  }
  *flags |= (*Value & FLAG_NEGATIVE);
}

Word GetWordAddress(Byte LoByte, Byte HiByte) {
  Word Address = LoByte + (HiByte << 8);
  return Address;
}

// TODO:  If we want to emulate something cycle exact, we would want these instructions
// to be designed in a way that there are discrete steps:
//        - Cycle 1: Get instruction from the Program Counter
//        - Cycle 2..# Perform the next single action (Read a byte, transfer a register, increment, etc)
//
//       In order to do this, we will need to figure out a way to either queue the individual actions after
//       the instruction is pulled, or a way to tell the Instruction case which cycle we are on.
//
//       Other considerations:
//        - We have a fixed number of addressing modes.  With the exception of a few details, every
//          instruction takes the same steps in each addressing mode.
Sint32 Execute(CPU *cpu) {

  Sint32 Cycles = 0;

  Byte instruction = FetchByte(cpu, &Cycles); // One cycle
  switch (instruction) {
    /****************************************
     * Implied Addressing
     ****************************************
     *
     * Transfer from one register to the other
     */

  case INS_TAX: {
    BurnByte(cpu, &Cycles);
    cpu->X = cpu->A;
    SetStatusFlag(&cpu->P, &cpu->X);
  } break;
  case INS_TAY: {
    BurnByte(cpu, &Cycles);
    cpu->Y = cpu->A;
    SetStatusFlag(&cpu->P, &cpu->Y);
  } break;
  case INS_TXA: {
    BurnByte(cpu, &Cycles);
    cpu->A = cpu->X;
    SetStatusFlag(&cpu->P, &cpu->A);
  } break;
  case INS_TYA: {
    BurnByte(cpu, &Cycles);
    cpu->A = cpu->Y;
    SetStatusFlag(&cpu->P, &cpu->A);
  } break;
  case INS_TSX: {
    BurnByte(cpu, &Cycles);
    cpu->X = cpu->SP;
    SetStatusFlag(&cpu->P, &cpu->X);
  } break;
  case INS_TXS: {
    BurnByte(cpu, &Cycles);
    cpu->SP = cpu->X;
  } break;
  case INS_PHA: {
    BurnByte(cpu, &Cycles);
    Word Address = 0x0100 + cpu->SP;
    WriteByte(cpu, Address, cpu->A, &Cycles);
    cpu->SP--;
  } break;
  case INS_PHP: {
    BurnByte(cpu, &Cycles);
    Word Address = 0x0100 + cpu->SP;
    WriteByte(cpu, Address, cpu->P, &Cycles);
    cpu->SP--;
  } break;
  case INS_PLP: {
    BurnByte(cpu, &Cycles);
    cpu->SP++;
    Cycles++;
    Word Address = 0x0100 + cpu->SP;
    cpu->P = ReadByte(cpu, Address, &Cycles);
  } break;
  case INS_PLA: {
    BurnByte(cpu, &Cycles);
    cpu->SP++;
    Cycles++;
    Word Address = 0x0100 + cpu->SP;
    cpu->A = ReadByte(cpu, Address, &Cycles);
    SetStatusFlag(&cpu->P, &cpu->A);
  } break;
  /****************************************
   * Immediate Addressing
   ***************************************

    #  address R/W description
   --- ------- --- ------------------------------------------
    1    PC     R  fetch opcode, increment PC
    2    PC     R  fetch value, increment PC
   */
  case INS_LDA_IM: {
    Byte Value = FetchByte(cpu, &Cycles);
    cpu->A = Value;
    SetStatusFlag(&cpu->P, &cpu->A);
  } break;
  case INS_LDX_IM: {
    Byte Value = FetchByte(cpu, &Cycles);
    cpu->X = Value;
    SetStatusFlag(&cpu->P, &cpu->X);
  } break;
  case INS_LDY_IM: {
    Byte Value = FetchByte(cpu, &Cycles);
    cpu->Y = Value;
    SetStatusFlag(&cpu->P, &cpu->Y);
  } break;

    /****************************************
     * Absolute Addressing
     ****************************************
    JMP

      #  address R/W description
     --- ------- --- -------------------------------------------------
      1    PC     R  fetch opcode, increment PC
      2    PC     R  fetch low address byte, increment PC
      3    PC     R  copy low address byte to PCL, fetch high address
                     byte to PCH

    Read instructions (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT,
                      LAX, NOP)

      #  address R/W description
     --- ------- --- ------------------------------------------
      1    PC     R  fetch opcode, increment PC
      2    PC     R  fetch low byte of address, increment PC
      3    PC     R  fetch high byte of address, increment PC
      4  address  R  read from effective address

    Read-Modify-Write instructions (ASL, LSR, ROL, ROR, INC, DEC,
                                   SLO, SRE, RLA, RRA, ISB, DCP)

      #  address R/W description
     --- ------- --- ------------------------------------------
      1    PC     R  fetch opcode, increment PC
      2    PC     R  fetch low byte of address, increment PC
      3    PC     R  fetch high byte of address, increment PC
      4  address  R  read from effective address
      5  address  W  write the value back to effective address,
                     and do the operation on it
      6  address  W  write the new value to effective address

    Write instructions (STA, STX, STY, SAX)

      #  address R/W description
     --- ------- --- ------------------------------------------
      1    PC     R  fetch opcode, increment PC
      2    PC     R  fetch low byte of address, increment PC
      3    PC     R  fetch high byte of address, increment PC
      4  address  W  write register to effective address
     */
  case INS_STX_ABS: {
    Byte LoByte = FetchByte(cpu, &Cycles);
    Byte HiByte = FetchByte(cpu, &Cycles);

    Word Address = GetWordAddress(LoByte, HiByte);
    WriteByte(cpu, Address, cpu->X, &Cycles);
  } break;
  case INS_STY_ABS: {
    Byte LoByte = FetchByte(cpu, &Cycles);
    Byte HiByte = FetchByte(cpu, &Cycles);

    Word Address = GetWordAddress(LoByte, HiByte);

    WriteByte(cpu, Address, cpu->Y, &Cycles);
  } break;
  case INS_STA_ABS: {
    Byte LoByte = FetchByte(cpu, &Cycles);
    Byte HiByte = FetchByte(cpu, &Cycles);

    Word Address = GetWordAddress(LoByte, HiByte);

    WriteByte(cpu, Address, cpu->A, &Cycles);
  } break;
  case INS_LDA_ABS: {
    Byte LoByte = FetchByte(cpu, &Cycles);
    Byte HiByte = FetchByte(cpu, &Cycles);

    Word Address = GetWordAddress(LoByte, HiByte);

    cpu->A = ReadByte(cpu, Address, &Cycles);

    SetStatusFlag(&cpu->P, &cpu->A);
  } break;
  case INS_LDX_ABS: {
    Byte LoByte = FetchByte(cpu, &Cycles);
    Byte HiByte = FetchByte(cpu, &Cycles);

    Word Address = GetWordAddress(LoByte, HiByte);
    cpu->X = ReadByte(cpu, Address, &Cycles);
    SetStatusFlag(&cpu->P, &cpu->X);
  } break;
  case INS_LDY_ABS: {
    Byte LoByte = FetchByte(cpu, &Cycles);
    Byte HiByte = FetchByte(cpu, &Cycles);

    Word Address = GetWordAddress(LoByte, HiByte);
    cpu->Y = ReadByte(cpu, Address, &Cycles);
    SetStatusFlag(&cpu->P, &cpu->Y);
  } break;
    /*
            #  address R/W description
           --- ------- ---
       ------------------------------------------------- 1    PC     R
       fetch opcode, increment PC 2    PC     R  fetch low address byte,
       increment PC 3    PC     R  copy low address byte to PCL, fetch high
       address byte to PCH
    */
  case INS_JMP_ABS: {
    Byte LoByte = FetchByte(cpu, &Cycles);
    Byte HiByte = FetchByte(cpu, &Cycles);

    cpu->PC = GetWordAddress(LoByte, HiByte);
    Cycles++;
  } break;
  /****************************************
   * Absolute Indexed Addressing
   ***************************************
   Read instructions (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT,
                    LAX, LAE, SHS, NOP)

    #   address  R/W description
   --- --------- --- ------------------------------------------
    1     PC      R  fetch opcode, increment PC
    2     PC      R  fetch low byte of address, increment PC
    3     PC      R  fetch high byte of address,
                     add index register to low address byte,
                     increment PC
    4  address+I* R  read from effective address,
                     fix the high byte of effective address
    5+ address+I  R  re-read from effective address

    Notes: I denotes either index register (X or Y).

          * The high byte of the effective address may be invalid
            at this time, i.e. it may be smaller by $100.

          + This cycle will be executed only if the effective address
            was invalid during cycle #4, i.e. page boundary was crossed.

  Read-Modify-Write instructions (ASL, LSR, ROL, ROR, INC, DEC,
                                 SLO, SRE, RLA, RRA, ISB, DCP)

    #   address  R/W description
   --- --------- --- ------------------------------------------
    1    PC       R  fetch opcode, increment PC
    2    PC       R  fetch low byte of address, increment PC
    3    PC       R  fetch high byte of address,
                     add index register X to low address byte,
                     increment PC
    4  address+X* R  read from effective address,
                     fix the high byte of effective address
    5  address+X  R  re-read from effective address
    6  address+X  W  write the value back to effective address,
                     and do the operation on it
    7  address+X  W  write the new value to effective address

   Notes: * The high byte of the effective address may be invalid
            at this time, i.e. it may be smaller by $100.

  Write instructions (STA, STX, STY, SHA, SHX, SHY)

    #   address  R/W description
   --- --------- --- ------------------------------------------
    1     PC      R  fetch opcode, increment PC
    2     PC      R  fetch low byte of address, increment PC
    3     PC      R  fetch high byte of address,
                     add index register to low address byte,
                     increment PC
    4  address+I* R  read from effective address,
                     fix the high byte of effective address
    5  address+I  W  write to effective address

    Notes: I denotes either index register (X or Y).

          * The high byte of the effective address may be invalid
            at this time, i.e. it may be smaller by $100. Because
            the processor cannot undo a write to an invalid
            address, it always reads from the address first.
   */
  case INS_STA_ABX: {
    Byte LoByte = FetchByte(cpu, &Cycles);
    Byte HiByte = FetchByte(cpu, &Cycles);

    Word Address = GetWordAddress(LoByte, HiByte);
    Byte HiByteBefore = Address >> 8;

    Address += cpu->X;
    Byte HiByteAfter = Address >> 8;

    WriteByte(cpu, Address, cpu->A, &Cycles);

    if (HiByteBefore != HiByteAfter) {
      Cycles++;
    }
  } break;
  case INS_STA_ABY: {

    Byte LoByte = FetchByte(cpu, &Cycles);
    Byte HiByte = FetchByte(cpu, &Cycles);

    Word Address = GetWordAddress(LoByte, HiByte);
    Byte AddrHiByte = Address >> 8;

    Address += cpu->Y;
    Byte AddrAfterHiByte = Address >> 8;

    WriteByte(cpu, Address, cpu->A, &Cycles);
    SetStatusFlag(&cpu->P, &cpu->A);
    if (AddrHiByte != AddrAfterHiByte) {
      Cycles++;
    }
  } break;
  case INS_LDA_ABX: {
    Byte LoByte = FetchByte(cpu, &Cycles);
    Byte HiByte = FetchByte(cpu, &Cycles);

    Word Address = GetWordAddress(LoByte, HiByte);
    Byte HiByteBefore = Address >> 8;

    Address += cpu->X;
    Byte HiByteAfter = Address >> 8;

    cpu->A = ReadByte(cpu, Address, &Cycles);
    SetStatusFlag(&cpu->P, &cpu->A);
    if (HiByteBefore != HiByteAfter) {
      Cycles++;
    }
  } break;
  case INS_LDA_ABY: {

    Byte LoByte = FetchByte(cpu, &Cycles);
    Byte HiByte = FetchByte(cpu, &Cycles);

    Word Address = GetWordAddress(LoByte, HiByte);
    Byte AddrHiByte = Address >> 8;

    Address += cpu->Y;
    Byte AddrAfterHiByte = Address >> 8;

    cpu->A = ReadByte(cpu, Address, &Cycles);
    SetStatusFlag(&cpu->P, &cpu->A);
    if (AddrHiByte != AddrAfterHiByte) {
      Cycles++;
    }
  } break;
  case INS_LDX_ABY: {

    Byte LoByte = FetchByte(cpu, &Cycles);
    Byte HiByte = FetchByte(cpu, &Cycles);

    Word Address = GetWordAddress(LoByte, HiByte);
    Byte AddrHiByte = Address >> 8;

    Address += cpu->Y;
    Byte AddrAfterHiByte = Address >> 8;

    cpu->X = ReadByte(cpu, Address, &Cycles);
    SetStatusFlag(&cpu->P, &cpu->X);
    if (AddrHiByte != AddrAfterHiByte) {
      Cycles++;
    }
  } break;
  case INS_LDY_ABX: {
    Byte LoByte = FetchByte(cpu, &Cycles);
    Byte HiByte = FetchByte(cpu, &Cycles);

    Word Address = GetWordAddress(LoByte, HiByte);
    Byte AddrHiByte = Address >> 8;

    Address += cpu->X;
    Byte AddrAfterHiByte = Address >> 8;

    cpu->Y = ReadByte(cpu, Address, &Cycles);
    SetStatusFlag(&cpu->P, &cpu->Y);
    if (AddrHiByte != AddrAfterHiByte) {
      Cycles++;
    }
  } break;

  /****************************************
   * Zero Page Addressing
   ***************************************

   Read instructions (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT,
                    LAX, NOP)

    #  address R/W description
   --- ------- --- ------------------------------------------
    1    PC     R  fetch opcode, increment PC
    2    PC     R  fetch address, increment PC
    3  address  R  read from effective address

  Read-Modify-Write instructions (ASL, LSR, ROL, ROR, INC, DEC,
                                 SLO, SRE, RLA, RRA, ISB, DCP)

    #  address R/W description
   --- ------- --- ------------------------------------------
    1    PC     R  fetch opcode, increment PC
    2    PC     R  fetch address, increment PC
    3  address  R  read from effective address
    4  address  W  write the value back to effective address,
                   and do the operation on it
    5  address  W  write the new value to effective address

  Write instructions (STA, STX, STY, SAX)

    #  address R/W description
   --- ------- --- ------------------------------------------
    1    PC     R  fetch opcode, increment PC
    2    PC     R  fetch address, increment PC
    3  address  W  write register to effective address
   */
  case INS_STX_ZP: {
    Byte Address = FetchByte(cpu, &Cycles);
    WriteByte(cpu, Address, cpu->X, &Cycles);
  } break;
  case INS_STY_ZP: {
    Byte Address = FetchByte(cpu, &Cycles);
    WriteByte(cpu, Address, cpu->Y, &Cycles);
  } break;
  case INS_STA_ZP: {
    Byte Address = FetchByte(cpu, &Cycles);
    WriteByte(cpu, Address, cpu->A, &Cycles);
  } break;
  case INS_LDA_ZP: {
    Byte Address = FetchByte(cpu, &Cycles);
    cpu->A = ReadByte(cpu, Address, &Cycles);
    SetStatusFlag(&cpu->P, &cpu->A);
  } break;
  case INS_LDX_ZP: {
    Byte Address = FetchByte(cpu, &Cycles);
    cpu->X = ReadByte(cpu, Address, &Cycles);
    SetStatusFlag(&cpu->P, &cpu->X);
  } break;
  case INS_LDY_ZP: {
    Byte Address = FetchByte(cpu, &Cycles);
    cpu->Y = ReadByte(cpu, Address, &Cycles);
    SetStatusFlag(&cpu->P, &cpu->Y);
  } break;

    /****************************************
     * Zero Page Indexed Addressing
     ***************************************
     Read instructions (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT,
                      LAX, NOP)

      #   address  R/W description
     --- --------- --- ------------------------------------------
      1     PC      R  fetch opcode, increment PC
      2     PC      R  fetch address, increment PC
      3   address   R  read from address, add index register to it
      4  address+I* R  read from effective address

      Notes: I denotes either index register (X or Y).

            * The high byte of the effective address is always zero,
              i.e. page boundary crossings are not handled.

    Read-Modify-Write instructions (ASL, LSR, ROL, ROR, INC, DEC,
                                   SLO, SRE, RLA, RRA, ISB, DCP)

      #   address  R/W description
     --- --------- --- ---------------------------------------------
      1     PC      R  fetch opcode, increment PC
      2     PC      R  fetch address, increment PC
      3   address   R  read from address, add index register X to it
      4  address+X* R  read from effective address
      5  address+X* W  write the value back to effective address,
                       and do the operation on it
      6  address+X* W  write the new value to effective address

      Note: * The high byte of the effective address is always zero,
             i.e. page boundary crossings are not handled.

    Write instructions (STA, STX, STY, SAX)

      #   address  R/W description
     --- --------- --- -------------------------------------------
      1     PC      R  fetch opcode, increment PC
      2     PC      R  fetch address, increment PC
      3   address   R  read from address, add index register to it
      4  address+I* W  write to effective address

      Notes: I denotes either index register (X or Y).

            * The high byte of the effective address is always zero,
              i.e. page boundary crossings are not handled.
    */
  case INS_STY_ZPX: {
    Byte Address = FetchByte(cpu, &Cycles);
    Address = (Address + cpu->X) & 0x00FF;
    Cycles++;
    WriteByte(cpu, Address, cpu->Y, &Cycles);
  } break;
  case INS_STA_ZPX: {
    Byte Address = FetchByte(cpu, &Cycles);
    Address = (Address + cpu->X) & 0x00FF;
    Cycles++;
    WriteByte(cpu, Address, cpu->A, &Cycles);
  } break;
  case INS_STX_ZPY: {
    Byte Address = FetchByte(cpu, &Cycles);
    Address = (Address + cpu->Y) & 0x00FF;
    Cycles++;
    WriteByte(cpu, Address, cpu->X, &Cycles);
  };
  case INS_LDA_ZPX: {
    Byte Address = FetchByte(cpu, &Cycles);
    Address = (Address + cpu->X) & 0x00FF;
    Cycles++;
    cpu->A = ReadByte(cpu, Address, &Cycles);
    SetStatusFlag(&cpu->P, &cpu->A);
  } break;
  case INS_LDX_ZPY: {
    Byte Address = FetchByte(cpu, &Cycles);
    Address += cpu->Y;
    Cycles++;
    cpu->X = ReadByte(cpu, Address, &Cycles);
    SetStatusFlag(&cpu->P, &cpu->X);
  } break;
  case INS_LDY_ZPX: {
    Byte Address = FetchByte(cpu, &Cycles);
    Address += cpu->X;
    Cycles++;
    cpu->Y = ReadByte(cpu, Address, &Cycles);
    SetStatusFlag(&cpu->P, &cpu->Y);
  } break;

    /*************************************************************
     *Relative addressing (BCC, BCS, BNE, BEQ, BPL, BMI, BVC, BVS)
     *************************************************************

      #   address  R/W description
     --- --------- --- ---------------------------------------------
      1     PC      R  fetch opcode, increment PC
      2     PC      R  fetch operand, increment PC
      3     PC      R  Fetch opcode of next instruction,
                       If branch is taken, add operand to PCL.
                       Otherwise increment PC.
      4+    PC*     R  Fetch opcode of next instruction.
                       Fix PCH. If it did not change, increment PC.
      5!    PC      R  Fetch opcode of next instruction,
                       increment PC.

      Notes: The opcode fetch of the next instruction is included to
            this diagram for illustration purposes. When determining
            real execution times, remember to subtract the last
            cycle.

            * The high byte of Program Counter (PCH) may be invalid
              at this time, i.e. it may be smaller or bigger by $100.

            + If branch is taken, this cycle will be executed.

            ! If branch occurs to different page, this cycle will be
              executed. */

    /****************************************
     * Indexed Indirect Addressing
     ***************************************
    Read instructions (LDA, ORA, EOR, AND, ADC, CMP, SBC, LAX)

      #    address   R/W description
     --- ----------- --- ------------------------------------------
      1      PC       R  fetch opcode, increment PC
      2      PC       R  fetch pointer address, increment PC
      3    pointer    R  read from the address, add X to it
      4   pointer+X   R  fetch effective address low
      5  pointer+X+1  R  fetch effective address high
      6    address    R  read from effective address

     Note: The effective address is always fetched from zero page,
           i.e. the zero page boundary crossing is not handled.

    Read-Modify-Write instructions (SLO, SRE, RLA, RRA, ISB, DCP)

      #    address   R/W description
     --- ----------- --- ------------------------------------------
      1      PC       R  fetch opcode, increment PC
      2      PC       R  fetch pointer address, increment PC
      3    pointer    R  read from the address, add X to it
      4   pointer+X   R  fetch effective address low
      5  pointer+X+1  R  fetch effective address high
      6    address    R  read from effective address
      7    address    W  write the value back to effective address,
                         and do the operation on it
      8    address    W  write the new value to effective address

      Note: The effective address is always fetched from zero page,
           i.e. the zero page boundary crossing is not handled.

    Write instructions (STA, SAX)

      #    address   R/W description
     --- ----------- --- ------------------------------------------
      1      PC       R  fetch opcode, increment PC
      2      PC       R  fetch pointer address, increment PC
      3    pointer    R  read from the address, add X to it
      4   pointer+X   R  fetch effective address low
      5  pointer+X+1  R  fetch effective address high
      6    address    W  write to effective address

      Note: The effective address is always fetched from zero page,
           i.e. the zero page boundary crossing is not handled. */

  case INS_STA_IDX: {
    Byte Address = FetchByte(cpu, &Cycles);
    Address += cpu->X;

    Byte LoByte = ReadByte(cpu, Address, &Cycles);
    Address += 1;
    Byte HiByte = ReadByte(cpu, Address, &Cycles);

    Word TargetAddress = GetWordAddress(LoByte, HiByte);
    Cycles += 1;
    WriteByte(cpu, TargetAddress, cpu->A, &Cycles);
  } break;
  case INS_LDA_IDX: {
    Byte Address = FetchByte(cpu, &Cycles);
    Address += cpu->X;

    Byte LoByte = ReadByte(cpu, Address, &Cycles);
    Address += 1;
    Byte HiByte = ReadByte(cpu, Address, &Cycles);

    Word TargetAddress = GetWordAddress(LoByte, HiByte);
    Cycles += 1;
    cpu->A = ReadByte(cpu, TargetAddress, &Cycles);
    SetStatusFlag(&cpu->P, &cpu->A);
  } break;

    /****************************************
     * Indirect Indexed Addressing
     ***************************************

    Read instructions (LDA, EOR, AND, ORA, ADC, SBC, CMP)

      #    address   R/W description
     --- ----------- --- ------------------------------------------
      1      PC       R  fetch opcode, increment PC
      2      PC       R  fetch pointer address, increment PC
      3    pointer    R  fetch effective address low
      4   pointer+1   R  fetch effective address high,
                         add Y to low byte of effective address
      5   address+Y*  R  read from effective address,
                         fix high byte of effective address
      6+  address+Y   R  read from effective address

      Notes: The effective address is always fetched from zero page,
            i.e. the zero page boundary crossing is not handled.

            * The high byte of the effective address may be invalid
              at this time, i.e. it may be smaller by $100.

            + This cycle will be executed only if the effective address
              was invalid during cycle #5, i.e. page boundary was crossed.

    Read-Modify-Write instructions (SLO, SRE, RLA, RRA, ISB, DCP)

      #    address   R/W description
     --- ----------- --- ------------------------------------------
      1      PC       R  fetch opcode, increment PC
      2      PC       R  fetch pointer address, increment PC
      3    pointer    R  fetch effective address low
      4   pointer+1   R  fetch effective address high,
                         add Y to low byte of effective address
      5   address+Y*  R  read from effective address,
                         fix high byte of effective address
      6   address+Y   R  read from effective address
      7   address+Y   W  write the value back to effective address,
                         and do the operation on it
      8   address+Y   W  write the new value to effective address

      Notes: The effective address is always fetched from zero page,
            i.e. the zero page boundary crossing is not handled.

            * The high byte of the effective address may be invalid
              at this time, i.e. it may be smaller by $100.

     Write instructions (STA, SHA)

      #    address   R/W description
     --- ----------- --- ------------------------------------------
      1      PC       R  fetch opcode, increment PC
      2      PC       R  fetch pointer address, increment PC
      3    pointer    R  fetch effective address low
      4   pointer+1   R  fetch effective address high,
                         add Y to low byte of effective address
      5   address+Y*  R  read from effective address,
                         fix high byte of effective address
      6   address+Y   W  write to effective address

      Notes: The effective address is always fetched from zero page,
            i.e. the zero page boundary crossing is not handled.

            * The high byte of the effective address may be invalid
              at this time, i.e. it may be smaller by $100.
     */
  case INS_STA_IDY: {

    Byte Address = FetchByte(cpu, &Cycles);

    Byte LoByte = ReadByte(cpu, Address, &Cycles);
    Address += 1;
    Byte HiByte = ReadByte(cpu, Address, &Cycles);

    Word TargetAddress = GetWordAddress(LoByte, HiByte);
    TargetAddress += cpu->Y;

    Byte AddrAfterHiByte = TargetAddress >> 8;

    WriteByte(cpu, TargetAddress, cpu->A, &Cycles);
    if (AddrAfterHiByte != HiByte) {
      Cycles++;
    }
  } break;
  case INS_LDA_IDY: {
    Byte Address = FetchByte(cpu, &Cycles);

    Byte LoByte = ReadByte(cpu, Address, &Cycles);
    Address += 1;
    Byte HiByte = ReadByte(cpu, Address, &Cycles);

    Word TargetAddress = GetWordAddress(LoByte, HiByte);
    TargetAddress += cpu->Y;

    Byte AddrAfterHiByte = TargetAddress >> 8;

    cpu->A = ReadByte(cpu, TargetAddress, &Cycles);
    SetStatusFlag(&cpu->P, &cpu->A);
    if (AddrAfterHiByte != HiByte) {
      Cycles++;
    }
  } break;

  /**************************************************
   * Program flow / Stack Instructions
   * ***********************************************/
  case INS_JSR: {
    Byte LoByte = FetchByte(cpu, &Cycles);
    Byte HiByte = FetchByte(cpu, &Cycles);

    Word Address = GetWordAddress(LoByte, HiByte);
    WriteWord(cpu, cpu->PC - 1, cpu->SP, &Cycles);
    cpu->SP -= 1;
    cpu->PC = Address;
    Cycles++;
  } break;

  /****************************************
   * Absolute Indirect Addressing (JMP)
   ****************************************

    #   address  R/W description
   --- --------- --- ------------------------------------------
    1     PC      R  fetch opcode, increment PC
    2     PC      R  fetch pointer address low, increment PC
    3     PC      R  fetch pointer address high, increment PC
    4   pointer   R  fetch low address to latch
    5  pointer+1* R  fetch PCH, copy latch to PCL

     Note: * The PCH will always be fetched from the same page
           than PCL, i.e. page boundary crossing is not handled.
  */
  case INS_JMP_IND: {
  } break;
  default: {
    printf("Operation not handled %d\n", instruction);
  };
  }

  return Cycles;
}
