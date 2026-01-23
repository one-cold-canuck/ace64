#include "opcodes.h"

// Helper functions:
Word
get_indirect_indexed_y (CPU *cpu, Sint32 *cycles, bool forcePageBoundary)
{
  Byte pointer = fetch_byte (cpu, cycles);

  Byte loByte = read_byte (cpu, pointer, cycles);
  Byte hiByte = read_byte (cpu, (pointer + 1) & 0xFF, cycles);

  Word baseAddress = get_word_address (loByte, hiByte);
  Word effectiveAddress = baseAddress + cpu->Y;

  if (forcePageBoundary
      || (baseAddress & 0xFF00) != (effectiveAddress & 0xFF00))
    {
      (*cycles)++;
    }

  return effectiveAddress;
}

Word
get_indexed_indirect_x (CPU *cpu, Sint32 *cycles)
{
  Byte pointer = fetch_byte (cpu, cycles);
  pointer = pointer + cpu->X;
  (*cycles)++;

  Byte loByte = read_byte (cpu, pointer, cycles);
  Byte hiByte = read_byte (cpu, (pointer + 1) & 0xFF, cycles);

  Word effectiveAddress = get_word_address (loByte, hiByte);

  return effectiveAddress;
}

Word
get_addr_zpx (CPU *cpu, Sint32 *cycles)
{
  Byte pointer = fetch_byte (cpu, cycles);
  Word address = (pointer + cpu->X) & 0xFF;
  (*cycles)++;

  return (Word)address;
}

Word
get_addr_zpy (CPU *cpu, Sint32 *cycles)
{
  Byte pointer = fetch_byte (cpu, cycles);
  Word address = (pointer + cpu->Y) & 0xFF;
  (*cycles)++;

  return (Word)address;
}

Word
get_addr_abs (CPU *cpu, Sint32 *cycles)
{
  Byte loByte = fetch_byte (cpu, cycles);
  Byte hiByte = fetch_byte (cpu, cycles);

  Word address = get_word_address (loByte, hiByte);
  return address;
}

Word
get_addr_abx (CPU *cpu, Sint32 *cycles, bool forcePageBoundary)
{
  Byte loByte = fetch_byte (cpu, cycles);
  Byte hiByte = fetch_byte (cpu, cycles);

  Word baseAddress = get_word_address (loByte, hiByte);
  Word effectiveAddress = baseAddress + cpu->X;

  bool pageCrossed = (baseAddress & 0xFF00) != (effectiveAddress & 0xFF00);

  if (pageCrossed || forcePageBoundary)
    {
      (*cycles)++;
    }

  return effectiveAddress;
}

Word
get_addr_aby (CPU *cpu, Sint32 *cycles, bool forcePageBoundary)
{

  Byte loByte = fetch_byte (cpu, cycles);
  Byte hiByte = fetch_byte (cpu, cycles);

  Word baseAddress = get_word_address (loByte, hiByte);
  Word effectiveAddress = baseAddress + cpu->Y;

  bool pageCrossed = (baseAddress & 0xFF00) != (effectiveAddress & 0xFF00);

  if (pageCrossed || forcePageBoundary)
    {
      (*cycles)++;
    }

  return effectiveAddress;
}

void
stack_push (CPU *cpu, Byte value, Sint32 *cycles)
{
  Word address = 0x0100 + cpu->SP;
  write_byte (cpu, address, value, cycles);
  cpu->SP--;
}

Byte
stack_pop (CPU *cpu, Sint32 *cycles)
{
  cpu->SP++;
  Word address = 0x0100 + cpu->SP;
  return read_byte (cpu, address, cycles);
}

Byte
perform_asl_logic (CPU *cpu, Byte value)
{
  if (value & 0x80)
    {
      set_flag (&cpu->P, FLAG_CARRY);
    }
  else
    {
      clear_flag (&cpu->P, FLAG_CARRY);
    }

  Byte result = value << 1;

  set_status_flag (&cpu->P, result);

  return result;
}

Byte
perform_lsr_logic (CPU *cpu, Byte value)
{
  if (value & 0x01)
    {
      set_flag (&cpu->P, FLAG_CARRY);
    }
  else
    {
      clear_flag (&cpu->P, FLAG_CARRY);
    }

  Byte result = value >> 1;

  set_status_flag (&cpu->P, result);

  return result;
}

Byte
perform_rol_logic (CPU *cpu, Byte value)
{

  Byte old_carry = get_carry_flag (cpu);

  if (value & 0x80)
    {
      set_flag (&cpu->P, FLAG_CARRY);
    }
  else
    {
      clear_flag (&cpu->P, FLAG_CARRY);
    }

  Byte result = (value << 1) | old_carry;
  set_status_flag (&cpu->P, result);

  return result;
}

Byte
perform_ror_logic (CPU *cpu, Byte value)
{
  Byte old_carry = get_carry_flag (cpu);

  if (value & 0x01)
    {
      set_flag (&cpu->P, FLAG_CARRY);
    }
  else
    {
      clear_flag (&cpu->P, FLAG_CARRY);
    }

  Byte result = (value >> 1) | (old_carry << 7);
  set_status_flag (&cpu->P, result);

  return result;
}

Byte
perform_eor_logic (CPU *cpu, Byte value)
{
  Byte result = cpu->A ^ value;
  set_status_flag (&cpu->P, result);

  return result;
}

Byte
perform_ora_logic (CPU *cpu, Byte value)
{
  Byte result = cpu->A | value;
  set_status_flag (&cpu->P, result);

  return result;
}

void
perform_bit_logic (CPU *cpu, Byte value)
{

  if ((cpu->A & value) == 0)
    {
      set_flag (&cpu->P, FLAG_ZERO);
    }
  else
    {
      clear_flag (&cpu->P, FLAG_ZERO);
    }

  if ((value & 0x80))
    {
      set_flag (&cpu->P, FLAG_NEGATIVE);
    }
  else
    {
      clear_flag (&cpu->P, FLAG_NEGATIVE);
    }

  if ((value & 0x40))
    {
      set_flag (&cpu->P, FLAG_OVERFLOW);
    }
  else
    {
      clear_flag (&cpu->P, FLAG_OVERFLOW);
    }
}

void
perform_cmp_logic (CPU *cpu, Byte registerValue, Byte readValue)
{

  Byte result = (Byte)(registerValue - readValue);

  if (registerValue >= readValue)
    {
      set_flag (&cpu->P, FLAG_CARRY);
    }
  else
    {
      clear_flag (&cpu->P, FLAG_CARRY);
    }

  if (result == 0)
    {
      set_flag (&cpu->P, FLAG_ZERO);
    }
  else
    {
      clear_flag (&cpu->P, FLAG_ZERO);
    }

  if (result & 0x80)
    {
      set_flag (&cpu->P, FLAG_NEGATIVE);
    }
  else
    {
      clear_flag (&cpu->P, FLAG_NEGATIVE);
    }
}

void
perform_adc_binary (CPU *cpu, Byte value)
{

  Byte accValue = cpu->A;
  Byte carry = get_carry_flag (cpu);

  Word result = (Word)accValue + (Word)value + (Word)carry;

  if (result > 0xFF)
    {
      set_flag (&cpu->P, FLAG_CARRY);
    }
  else
    {
      clear_flag (&cpu->P, FLAG_CARRY);
    }

  if (~(accValue ^ value) & (accValue ^ (Byte)result) & 0x80)
    {
      set_flag (&cpu->P, FLAG_OVERFLOW);
    }
  else
    {
      clear_flag (&cpu->P, FLAG_OVERFLOW);
    }

  cpu->A = (Byte)(result & 0xFF);

  set_status_flag (&cpu->P, cpu->A);
}

void
perform_adc_decimal (CPU *cpu, Byte value)
{
  Byte startA = cpu->A;
  Byte carry = get_carry_flag (cpu);

  int lo = (cpu->A & 0x0F) + (value & 0x0F) + carry;
  if (lo > 0x09)
    {
      lo += 0x06;
    }

  int intermediateCarry = (lo > 0x0F) ? 1 : 0;

  int hi = (startA >> 4) + (value >> 4) + intermediateCarry;

  Word binarySum = (Word)startA + (Word)value + (Word)carry;

  if ((~(startA ^ value) & (startA ^ (Byte)binarySum)) & 0x80)
    {
      set_flag (&cpu->P, FLAG_OVERFLOW);
    }
  else
    {
      clear_flag (&cpu->P, FLAG_OVERFLOW);
    }

  if (binarySum & 0x80)
    {
      set_flag (&cpu->P, FLAG_NEGATIVE);
    }
  else
    {
      clear_flag (&cpu->P, FLAG_NEGATIVE);
    }

  if (hi > 0x09)
    {
      hi += 0x06;
    }

  if (hi > 0x0F)
    {
      set_flag (&cpu->P, FLAG_CARRY);
    }
  else
    {
      clear_flag (&cpu->P, FLAG_CARRY);
    }

  cpu->A = ((hi << 4) | (lo & 0x0F)) & 0xFF;

  set_status_flag (&cpu->P, binarySum);
}

void
perform_sbc_decimal (CPU *cpu, Byte value)
{
  Byte carry = get_carry_flag (cpu);

  int loA
      = (int)(cpu->A & 0x0F) - (int)(value & 0x0F) - (1 - carry); // low nibble
  int hiA = (int)(cpu->A >> 4) - (int)(value >> 4); // high nibble

  int binarySum = (int)cpu->A - (int)value - (1 - carry);

  if (loA < 0)
    {
      loA -= 6;
      hiA -= 1;
    }

  if (hiA < 0)
    {
      hiA -= 6;
    }

  if (binarySum >= 0)
    {
      set_flag (&cpu->P, FLAG_CARRY);
    }
  else
    {
      clear_flag (&cpu->P, FLAG_CARRY);
    }

  if ((~(cpu->A ^ value) & (cpu->A ^ (Byte)binarySum)) & 0x80)
    {
      set_flag (&cpu->P, FLAG_OVERFLOW);
    }
  else
    {
      clear_flag (&cpu->P, FLAG_OVERFLOW);
    }

  cpu->A = ((hiA << 4) | (loA & 0x0F)) & 0xFF;

  set_status_flag (&cpu->P, cpu->A);
}

void execute_branch(CPU *cpu, Sint32 *cycles, bool condition) {
  SByte offset = (SByte)fetch_byte(cpu, cycles);

  if(condition) {
    burn_cycle(cpu, cycles);
    Word originalPC = cpu->PC;
    cpu->PC += offset;
    
    if ((originalPC & 0xFF00) != (cpu->PC & 0xFF00)) {
      burn_cycle(cpu, cycles);
    }
  }
}

// Opcode functions:
void
ins_and_im (CPU *cpu, Sint32 *cycles)
{
  Byte value = fetch_byte (cpu, cycles);
  cpu->A = cpu->A & value;
  set_status_flag (&cpu->P, cpu->A);
}

void
ins_and_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = cpu->A & value;
  set_status_flag (&cpu->P, cpu->A);
}

void
ins_and_zpx (CPU *cpu, Sint32 *cycles)
{

  Word address = get_addr_zpx (cpu, cycles);
  cpu->A = cpu->A & read_byte (cpu, address, cycles);
  set_status_flag (&cpu->P, cpu->A);
}

void
ins_and_abs (CPU *cpu, Sint32 *cycles)
{
  Byte loByte = fetch_byte (cpu, cycles);
  Byte hiByte = fetch_byte (cpu, cycles);
  Word address = get_word_address (loByte, hiByte);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = cpu->A & value;
  set_status_flag (&cpu->P, cpu->A);
}

void
ins_and_abx (CPU *cpu, Sint32 *cycles)
{

  Word address = get_addr_abx (cpu, cycles, false);

  cpu->A = cpu->A & read_byte (cpu, address, cycles);
  set_status_flag (&cpu->P, cpu->A);
}

void
ins_and_aby (CPU *cpu, Sint32 *cycles)
{

  Word address = get_addr_aby (cpu, cycles, false);

  cpu->A = cpu->A & read_byte (cpu, address, cycles);
  set_status_flag (&cpu->P, cpu->A);
}

void
ins_and_idx (CPU *cpu, Sint32 *cycles)
{
  Word effectiveAddress = get_indexed_indirect_x (cpu, cycles);
  cpu->A = cpu->A & read_byte (cpu, effectiveAddress, cycles);
  set_status_flag (&cpu->P, cpu->A);
}

void
ins_and_idy (CPU *cpu, Sint32 *cycles)
{
  Word address = get_indirect_indexed_y (cpu, cycles, false);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = cpu->A & value;
  set_status_flag (&cpu->P, cpu->A);
}

// ASL: Arithmetic Shift Left: bits shifted 1 position, 0 shifted into bit 0,
// bit 7 -> Carry Flags: N-----ZC
void
ins_asl_acc (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  cpu->A = perform_asl_logic (cpu, cpu->A);
}
void
ins_asl_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);

  write_byte (cpu, address, value, cycles);

  value = perform_asl_logic (cpu, value);
  write_byte (cpu, address, value, cycles);
}

void
ins_asl_zpx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_zpx (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);

  write_byte (cpu, address, value, cycles);

  value = perform_asl_logic (cpu, value);
  write_byte (cpu, address, value, cycles);
}

void
ins_asl_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);

  write_byte (cpu, address, value, cycles);
  value = perform_asl_logic (cpu, value);
  write_byte (cpu, address, value, cycles);
}

void
ins_asl_abx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abx (cpu, cycles, true);
  Byte value = read_byte (cpu, address, cycles);

  write_byte (cpu, address, value, cycles);
  value = perform_asl_logic (cpu, value);
  write_byte (cpu, address, value, cycles);
}

// EOR: Bitwise exclusive OR with ACC. Add 1 cycle if page boundary is crossed.
// Flags: N-----Z-
void
ins_eor_im (CPU *cpu, Sint32 *cycles)
{
  Byte value = fetch_byte (cpu, cycles);
  cpu->A = perform_eor_logic (cpu, value);
}

void
ins_eor_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = perform_eor_logic (cpu, value);
}

void
ins_eor_zpx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_zpx (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = perform_eor_logic (cpu, value);
}

void
ins_eor_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = perform_eor_logic (cpu, value);
}

void
ins_eor_abx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abx (cpu, cycles, false);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = perform_eor_logic (cpu, value);
}

void
ins_eor_aby (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_aby (cpu, cycles, false);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = perform_eor_logic (cpu, value);
}

void
ins_eor_idx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_indexed_indirect_x (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = perform_eor_logic (cpu, value);
}

void
ins_eor_idy (CPU *cpu, Sint32 *cycles)
{
  Word address = get_indirect_indexed_y (cpu, cycles, false);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = perform_eor_logic (cpu, value);
}

// LSR: Logical Shift Right: bits shifted 1 position.  0 shifted to bit 7, bit
// 0 -> Carry Flags: N-----ZC
void
ins_lsr_acc (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  cpu->A = perform_lsr_logic (cpu, cpu->A);
}
void
ins_lsr_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);

  write_byte (cpu, address, value, cycles);

  value = perform_lsr_logic (cpu, value);
  write_byte (cpu, address, value, cycles);
}
void
ins_lsr_zpx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_zpx (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);

  write_byte (cpu, address, value, cycles);

  value = perform_lsr_logic (cpu, value);
  write_byte (cpu, address, value, cycles);
}
void
ins_lsr_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);

  write_byte (cpu, address, value, cycles);
  value = perform_lsr_logic (cpu, value);
  write_byte (cpu, address, value, cycles);
}
void
ins_lsr_abx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abx (cpu, cycles, true);
  Byte value = read_byte (cpu, address, cycles);

  write_byte (cpu, address, value, cycles);
  value = perform_lsr_logic (cpu, value);
  write_byte (cpu, address, value, cycles);
}

// ORA: Bitwise OR with ACC: add 1 cycle if page boundary is crossed
// Flags: N-----Z-
void
ins_ora_im (CPU *cpu, Sint32 *cycles)
{
  Byte value = fetch_byte (cpu, cycles);
  cpu->A = perform_ora_logic (cpu, value);
}

void
ins_ora_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = perform_ora_logic (cpu, value);
}
void
ins_ora_zpx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_zpx (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = perform_ora_logic (cpu, value);
}

void
ins_ora_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = perform_ora_logic (cpu, value);
}

void
ins_ora_abx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abx (cpu, cycles, false);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = perform_ora_logic (cpu, value);
}

void
ins_ora_aby (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_aby (cpu, cycles, false);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = perform_ora_logic (cpu, value);
}

void
ins_ora_idx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_indexed_indirect_x (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = perform_ora_logic (cpu, value);
}

void
ins_ora_idy (CPU *cpu, Sint32 *cycles)
{
  Word address = get_indirect_indexed_y (cpu, cycles, false);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = perform_ora_logic (cpu, value);
}

// ROL: Shift all bits left 1.  Carry shifted to 0, original 7 shifted to Carry
// Flags: N-----ZC
void
ins_rol_acc (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  cpu->A = perform_rol_logic (cpu, cpu->A);
}
void
ins_rol_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  write_byte (cpu, address, value, cycles);
  value = perform_rol_logic (cpu, value);
  write_byte (cpu, address, value, cycles);
}

void
ins_rol_zpx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_zpx (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  write_byte (cpu, address, value, cycles);
  value = perform_rol_logic (cpu, value);
  write_byte (cpu, address, value, cycles);
}

void
ins_rol_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  write_byte (cpu, address, value, cycles);
  value = perform_rol_logic (cpu, value);
  write_byte (cpu, address, value, cycles);
}

void
ins_rol_abx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abx (cpu, cycles, true);
  Byte value = read_byte (cpu, address, cycles);
  write_byte (cpu, address, value, cycles);
  value = perform_rol_logic (cpu, value);
  write_byte (cpu, address, value, cycles);
}

// ROR: Shift all bits right 1.  Carry to bit 7, original bit 0 to Carry
// Flags: N-----ZC
void
ins_ror_acc (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  cpu->A = perform_ror_logic (cpu, cpu->A);
}
void
ins_ror_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  write_byte (cpu, address, value, cycles);
  value = perform_ror_logic (cpu, value);
  write_byte (cpu, address, value, cycles);
}
void
ins_ror_zpx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_zpx (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  write_byte (cpu, address, value, cycles);
  value = perform_ror_logic (cpu, value);
  write_byte (cpu, address, value, cycles);
}
void
ins_ror_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  write_byte (cpu, address, value, cycles);
  value = perform_ror_logic (cpu, value);
  write_byte (cpu, address, value, cycles);
}
void
ins_ror_abx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abx (cpu, cycles, true);
  Byte value = read_byte (cpu, address, cycles);
  write_byte (cpu, address, value, cycles);
  value = perform_ror_logic (cpu, value);
  write_byte (cpu, address, value, cycles);
}

/* Branch Instructions */
// All branch instructions are relative mode, with a length of 2 bytes.
// Syntax: Bxx Displacement,  or Bxx Label.
// A branch not taken requires 2 machine cycles.  3 if taken, plus 1 if it
// crosses a page boundary. Crossing only occurs if the branch destination is
// on a different page than the instruction AFTER the branch instruction Flags:
// --------
void
ins_bpl (CPU *cpu, Sint32 *cycles)
{
  execute_branch(cpu, cycles, !(cpu->P & FLAG_NEGATIVE));
}
void
ins_bmi (CPU *cpu, Sint32 *cycles)
{
  execute_branch(cpu, cycles, (cpu->P & FLAG_NEGATIVE));
}
void
ins_bvc (CPU *cpu, Sint32 *cycles)
{
  execute_branch(cpu, cycles, !(cpu->P & FLAG_OVERFLOW));
}
void
ins_bvs (CPU *cpu, Sint32 *cycles)
{
  execute_branch(cpu, cycles, (cpu->P & FLAG_OVERFLOW));
}
void
ins_bcc(CPU *cpu, Sint32 *cycles)
{
  execute_branch(cpu, cycles, !(cpu->P & FLAG_CARRY));
}
void
ins_bcs (CPU *cpu, Sint32 *cycles)
{
  execute_branch(cpu, cycles, (cpu->P & FLAG_CARRY));
}
void
ins_bne (CPU *cpu, Sint32 *cycles)
{
  execute_branch(cpu, cycles, !(cpu->P & FLAG_ZERO));
}
void
ins_beq (CPU *cpu, Sint32 *cycles)
{
  execute_branch(cpu, cycles, (cpu->P & FLAG_ZERO));
}

/* Compare Instructions */

// CMP: Compare Accumulator.  Sets the processor flags as if a subtraction
// has been carried out
// Acc == Compared: Zero Flag
// ACC >= Compared: Carry Flag
// Flags: N-----ZC
void
ins_cmp_im (CPU *cpu, Sint32 *cycles)
{
  Byte value = fetch_byte (cpu, cycles);
  perform_cmp_logic (cpu, cpu->A, value);
}

void
ins_cmp_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  perform_cmp_logic (cpu, cpu->A, value);
}

void
ins_cmp_zpx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_zpx (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  perform_cmp_logic (cpu, cpu->A, value);
}

void
ins_cmp_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  perform_cmp_logic (cpu, cpu->A, value);
}

void
ins_cmp_abx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abx (cpu, cycles, false);
  Byte value = read_byte (cpu, address, cycles);
  perform_cmp_logic (cpu, cpu->A, value);
}

void
ins_cmp_aby (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_aby (cpu, cycles, false);
  Byte value = read_byte (cpu, address, cycles);
  perform_cmp_logic (cpu, cpu->A, value);
}

void
ins_cmp_idx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_indexed_indirect_x (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  perform_cmp_logic (cpu, cpu->A, value);
}

void
ins_cmp_idy (CPU *cpu, Sint32 *cycles)
{
  Word address = get_indirect_indexed_y (cpu, cycles, false);
  Byte value = read_byte (cpu, address, cycles);
  perform_cmp_logic (cpu, cpu->A, value);
}

// BIT: Test Bits
// Sets the Z flag as though value in addr tested were ANDed with ACC
// N, V are set equal to bits 7 and 6 of the value in the tested address
// Flags: NV----Z-
void
ins_bit_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  perform_bit_logic (cpu, value);
}

void
ins_bit_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  perform_bit_logic (cpu, value);
}

// CPX, CPY: Compare X, Y register.
// Identical to equivalent CMP operations
// Flags: N-----ZC
void
ins_cpx_im (CPU *cpu, Sint32 *cycles)
{
  Byte value = fetch_byte (cpu, cycles);
  perform_cmp_logic (cpu, cpu->X, value);
}

void
ins_cpx_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  perform_cmp_logic (cpu, cpu->X, value);
}

void
ins_cpx_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  perform_cmp_logic (cpu, cpu->X, value);
}

void
ins_cpy_im (CPU *cpu, Sint32 *cycles)
{
  Byte value = fetch_byte (cpu, cycles);
  perform_cmp_logic (cpu, cpu->Y, value);
}

void
ins_cpy_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  perform_cmp_logic (cpu, cpu->Y, value);
}

void
ins_cpy_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  perform_cmp_logic (cpu, cpu->Y, value);
}

/* Flag Instructions */
// Instructions that change processor status flags
// Carry set/clear
// Flags: -------C
void
ins_clc (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  clear_flag (&cpu->P, FLAG_CARRY);
}

void
ins_sec (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  set_flag (&cpu->P, FLAG_CARRY);
}

// Decimal set/clear
// Flags: ----D---
void
ins_cld (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  clear_flag (&cpu->P, FLAG_DECIMAL_MODE);
}

void
ins_sed (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  set_flag (&cpu->P, FLAG_DECIMAL_MODE);
}

// Interrupt set/clear
// Flags: -----I--
void
ins_cli (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  clear_flag (&cpu->P, FLAG_INTERRUPT_DISABLE);
}

void
ins_sei (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  set_flag (&cpu->P, FLAG_INTERRUPT_DISABLE);
}

// Overflow clear
// Flags: -V------
void
ins_clv (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  clear_flag (&cpu->P, FLAG_OVERFLOW);
}

/* Jump Instructions */
// JMP: No carry associated.  Indirect JMP must never use a vector
// beginning on the last byte of a page
// JMP ($30FF) will read low byte from $30FF, and high byte from $3000
void
ins_jmp_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  cpu->PC = address;
  cycles++;
}

void
ins_jmp_ind (CPU *cpu, Sint32 *cycles)
{
  Word pointerAddress = get_addr_abs (cpu, cycles);

  Byte loByte = read_byte (cpu, pointerAddress, cycles);

  Word hiAddress = (pointerAddress & 0xFF00) | ((pointerAddress + 1) & 0xFF);
  Byte hiByte = read_byte (cpu, hiAddress, cycles);

  cpu->PC = (hiByte << 8) | loByte;
}

void
ins_jsr (CPU *cpu, Sint32 *cycles)
{
  Byte loByte = fetch_byte (cpu, cycles);

  burn_cycle (cpu, cycles);

  stack_push (cpu, (cpu->PC >> 8) & 0xFF, cycles);
  stack_push (cpu, cpu->PC & 0xFF, cycles);

  Byte hiByte = fetch_byte (cpu, cycles);
  cpu->PC = (hiByte << 8) | loByte;
}

void
ins_rts (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  burn_cycle (cpu, cycles);

  Byte loByte = stack_pop (cpu, cycles);
  Byte hiByte = stack_pop (cpu, cycles);

  Word address = get_word_address (loByte, hiByte);
  burn_cycle (cpu, cycles);
  cpu->PC = address;
  cpu->PC++;
}

// RTI: Return from Interrupt.  Retrieves the Processor Status byte and PC from
// stack in that order.  Unlike RTS, return address on the stack is the actual
// address rather than address-1 Flags: NV-BDIZC

void
ins_rti (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  burn_cycle (cpu, cycles);

  Byte pValue = stack_pop (cpu, cycles);
  cpu->P = (pValue & 0xEF) | 0x20;

  Byte loByte = stack_pop (cpu, cycles);
  Byte hiByte = stack_pop (cpu, cycles);

  Word address = get_word_address (loByte, hiByte);
  cpu->PC = address;
}

/* Math Instructions */
// Add 1 cycle if page boundary is crossed ADC, AND, SBC
// Flags: NV----ZC
void
ins_adc_im (CPU *cpu, Sint32 *cycles)
{
  Byte value = fetch_byte (cpu, cycles);

  if (cpu->P & FLAG_DECIMAL_MODE)
    {
      perform_adc_decimal (cpu, value);
    }
  else
    {
      perform_adc_binary (cpu, value);
    }
}

void
ins_adc_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  if (cpu->P & FLAG_DECIMAL_MODE)
    {
      perform_adc_decimal (cpu, value);
    }
  else
    {
      perform_adc_binary (cpu, value);
    }
}

void
ins_adc_zpx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_zpx (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  if (cpu->P & FLAG_DECIMAL_MODE)
    {
      perform_adc_decimal (cpu, value);
    }
  else
    {
      perform_adc_binary (cpu, value);
    }
}

void
ins_adc_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  if (cpu->P & FLAG_DECIMAL_MODE)
    {
      perform_adc_decimal (cpu, value);
    }
  else
    {
      perform_adc_binary (cpu, value);
    }
}

void
ins_adc_abx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abx (cpu, cycles, false);
  Byte value = read_byte (cpu, address, cycles);
  if (cpu->P & FLAG_DECIMAL_MODE)
    {
      perform_adc_decimal (cpu, value);
    }
  else
    {
      perform_adc_binary (cpu, value);
    }
}

void
ins_adc_aby (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_aby (cpu, cycles, false);
  Byte value = read_byte (cpu, address, cycles);
  if (cpu->P & FLAG_DECIMAL_MODE)
    {
      perform_adc_decimal (cpu, value);
    }
  else
    {
      perform_adc_binary (cpu, value);
    }
}

void
ins_adc_idx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_indexed_indirect_x (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  if (cpu->P & FLAG_DECIMAL_MODE)
    {
      perform_adc_decimal (cpu, value);
    }
  else
    {
      perform_adc_binary (cpu, value);
    }
}

void
ins_adc_idy (CPU *cpu, Sint32 *cycles)
{
  Word address = get_indirect_indexed_y (cpu, cycles, false);
  Byte value = read_byte (cpu, address, cycles);
  if (cpu->P & FLAG_DECIMAL_MODE)
    {
      perform_adc_decimal (cpu, value);
    }
  else
    {
      perform_adc_binary (cpu, value);
    }
}

void
ins_sbc_im (CPU *cpu, Sint32 *cycles)
{
  Byte value = fetch_byte (cpu, cycles);
  if (cpu->P & FLAG_DECIMAL_MODE)
    {
      perform_sbc_decimal (cpu, value);
    }
  else
    {
      perform_adc_binary (cpu, ~value);
    }
}

void
ins_sbc_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  if (cpu->P & FLAG_DECIMAL_MODE)
    {
      perform_sbc_decimal (cpu, value);
    }
  else
    {
      perform_adc_binary (cpu, ~value);
    }
}

void
ins_sbc_zpx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_zpx (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  if (cpu->P & FLAG_DECIMAL_MODE)
    {
      perform_sbc_decimal (cpu, value);
    }
  else
    {
      perform_adc_binary (cpu, ~value);
    }
}

void
ins_sbc_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  if (cpu->P & FLAG_DECIMAL_MODE)
    {
      perform_sbc_decimal (cpu, value);
    }
  else
    {
      perform_adc_binary (cpu, ~value);
    }
}

void
ins_sbc_abx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abx (cpu, cycles, false);
  Byte value = read_byte (cpu, address, cycles);
  if (cpu->P & FLAG_DECIMAL_MODE)
    {
      perform_sbc_decimal (cpu, value);
    }
  else
    {
      perform_adc_binary (cpu, ~value);
    }
}

void
ins_sbc_aby (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_aby (cpu, cycles, false);
  Byte value = read_byte (cpu, address, cycles);
  if (cpu->P & FLAG_DECIMAL_MODE)
    {
      perform_sbc_decimal (cpu, value);
    }
  else
    {
      perform_adc_binary (cpu, ~value);
    }
}

void
ins_sbc_idx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_indexed_indirect_x (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  if (cpu->P & FLAG_DECIMAL_MODE)
    {
      perform_sbc_decimal (cpu, value);
    }
  else
    {
      perform_adc_binary (cpu, ~value);
    }
}

void
ins_sbc_idy (CPU *cpu, Sint32 *cycles)
{
  Word address = get_indirect_indexed_y (cpu, cycles, false);
  Byte value = read_byte (cpu, address, cycles);
  if (cpu->P & FLAG_DECIMAL_MODE)
    {
      perform_sbc_decimal (cpu, value);
    }
  else
    {
      perform_adc_binary (cpu, ~value);
    }
}

/* Memory Instructions */

// LDA: Load Accumulator.  Add 1 cycle if page boundary is crossed
// Flags: N-----Z-
void
ins_lda_im (CPU *cpu, Sint32 *cycles)
{
  Byte value = fetch_byte (cpu, cycles);
  cpu->A = value;
  set_status_flag (&cpu->P, cpu->A);
}

void
ins_lda_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  cpu->A = read_byte (cpu, address, cycles);
  set_status_flag (&cpu->P, cpu->A);
}

void
ins_lda_zpx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_zpx (cpu, cycles);
  cpu->A = read_byte (cpu, address, cycles);
  set_status_flag (&cpu->P, cpu->A);
}

void
ins_lda_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  cpu->A = read_byte (cpu, address, cycles);
  set_status_flag (&cpu->P, cpu->A);
}

void
ins_lda_abx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abx (cpu, cycles, false);
  cpu->A = read_byte (cpu, address, cycles);
  set_status_flag (&cpu->P, cpu->A);
}

void
ins_lda_aby (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_aby (cpu, cycles, false);
  cpu->A = read_byte (cpu, address, cycles);
  set_status_flag (&cpu->P, cpu->A);
}

void
ins_lda_idx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_indexed_indirect_x (cpu, cycles);
  cpu->A = read_byte (cpu, address, cycles);
  set_status_flag (&cpu->P, cpu->A);
}
void
ins_lda_idy (CPU *cpu, Sint32 *cycles)
{
  Word address = get_indirect_indexed_y (cpu, cycles, false);
  cpu->A = read_byte (cpu, address, cycles);
  set_status_flag (&cpu->P, cpu->A);
}

// LDX, LDY: Load X Register, Load Y Register
// Flags: N-----Z-
void
ins_ldx_im (CPU *cpu, Sint32 *cycles)
{
  Byte value = fetch_byte (cpu, cycles);
  cpu->X = value;
  set_status_flag (&cpu->P, cpu->X);
}

void
ins_ldx_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  cpu->X = read_byte (cpu, address, cycles);
  set_status_flag (&cpu->P, cpu->X);
}

void
ins_ldx_zpy (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_zpy (cpu, cycles);
  cpu->X = read_byte (cpu, address, cycles);
  set_status_flag (&cpu->P, cpu->X);
}

void
ins_ldx_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  cpu->X = read_byte (cpu, address, cycles);
  set_status_flag (&cpu->P, cpu->X);
}

void
ins_ldx_aby (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_aby (cpu, cycles, false);
  cpu->X = read_byte (cpu, address, cycles);
  set_status_flag (&cpu->P, cpu->X);
}

void
ins_ldy_im (CPU *cpu, Sint32 *cycles)
{
  Byte Value = fetch_byte (cpu, cycles);
  cpu->Y = Value;
  set_status_flag (&cpu->P, cpu->Y);
}

void
ins_ldy_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  cpu->Y = read_byte (cpu, address, cycles);
  set_status_flag (&cpu->P, cpu->Y);
}

void
ins_ldy_zpx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_zpx (cpu, cycles);
  cpu->Y = read_byte (cpu, address, cycles);
}

void
ins_ldy_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  cpu->Y = read_byte (cpu, address, cycles);
  set_status_flag (&cpu->P, cpu->Y);
}

void
ins_ldy_abx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abx (cpu, cycles, false);
  cpu->Y = read_byte (cpu, address, cycles);
  set_status_flag (&cpu->P, cpu->Y);
}
void
ins_lax_im (CPU *cpu, Sint32 *cycles)
{
  Byte imm = fetch_byte (cpu, cycles);
  Byte value = cpu->A;
  Byte result = (cpu->A | MAGIC_VALUE) & imm;
  // The MAGIC_VALUE is used here to simulate the 6510's behaviour when running
  // this illegal opcode
  cpu->A =  result;
  cpu->X = result;
  set_status_flag (&cpu->P, value);
}

void
ins_lax_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = value;
  cpu->X = value;
  set_status_flag (&cpu->P, value);
}

void
ins_lax_zpy (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_zpy(cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = value;
  cpu->X = value;
  set_status_flag (&cpu->P, value);
}

void
ins_lax_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = value;
  cpu->X = value;
  set_status_flag (&cpu->P, value);
}

void
ins_lax_aby (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_aby (cpu, cycles, false);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = value;
  cpu->X = value;
  set_status_flag (&cpu->P, value);
}

void
ins_lax_idx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_indexed_indirect_x (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = value;
  cpu->X = value;
  set_status_flag (&cpu->P, value);
}

void
ins_lax_idy (CPU *cpu, Sint32 *cycles)
{
  Word address = get_indirect_indexed_y (cpu, cycles, false);
  Byte value = read_byte (cpu, address, cycles);
  cpu->A = value;
  cpu->X = value;
  set_status_flag (&cpu->P, value);
}

// STA: Store Accumulator
// Flags: --------
void
ins_sta_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  write_byte (cpu, address, cpu->A, cycles);
}

void
ins_sta_zpx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_zpx (cpu, cycles);
  write_byte (cpu, address, cpu->A, cycles);
}

void
ins_sta_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  write_byte (cpu, address, cpu->A, cycles);
}

void
ins_sta_abx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abx (cpu, cycles, true);
  write_byte (cpu, address, cpu->A, cycles);
}

void
ins_sta_aby (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_aby (cpu, cycles, true);
  write_byte (cpu, address, cpu->A, cycles);
}

void
ins_sta_idx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_indexed_indirect_x (cpu, cycles);
  write_byte (cpu, address, cpu->A, cycles);
}

void
ins_sta_idy (CPU *cpu, Sint32 *cycles)
{
  Word address = get_indirect_indexed_y (cpu, cycles, true);
  write_byte (cpu, address, cpu->A, cycles);
}

// STX, STY: Store X Reg, Store Y Reg
// Flags: --------

void
ins_stx_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  write_byte (cpu, address, cpu->X, cycles);
}

void
ins_stx_zpy (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_zpy (cpu, cycles);
  write_byte (cpu, address, cpu->X, cycles);
}

void
ins_stx_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  write_byte (cpu, address, cpu->X, cycles);
}

void
ins_sty_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  write_byte (cpu, address, cpu->Y, cycles);
}

void
ins_sty_zpx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_zpx (cpu, cycles);
  write_byte (cpu, address, cpu->Y, cycles);
}

void
ins_sty_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  write_byte (cpu, address, cpu->Y, cycles);
}

// DEC: Decrement Memory
// Flags: N-----Z-
void
ins_dec_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);

  write_byte (cpu, address, value, cycles);   // Cycle 4
  write_byte (cpu, address, --value, cycles); // Cycle 5
  set_status_flag (&cpu->P, value);
}
void
ins_dec_zpx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_zpx (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  write_byte (cpu, address, value, cycles);
  write_byte (cpu, address, --value, cycles);
  set_status_flag (&cpu->P, value);
}
void
ins_dec_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  write_byte (cpu, address, value, cycles);
  write_byte (cpu, address, --value, cycles);
  set_status_flag (&cpu->P, value);
}
void
ins_dec_abx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abx (cpu, cycles, true);
  Byte value = read_byte (cpu, address, cycles);
  write_byte (cpu, address, value, cycles);
  write_byte (cpu, address, --value, cycles);
  set_status_flag (&cpu->P, value);
}

// INC: Increment Memory
// Flags: N-----Z-
void
ins_inc_zp (CPU *cpu, Sint32 *cycles)
{
  Word address = fetch_byte (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);

  write_byte (cpu, address, value, cycles);   // Cycle 4
  write_byte (cpu, address, ++value, cycles); // Cycle 5
  set_status_flag (&cpu->P, value);
}
void
ins_inc_zpx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_zpx (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  write_byte (cpu, address, value, cycles);
  write_byte (cpu, address, ++value, cycles);
  set_status_flag (&cpu->P, value);
}
void
ins_inc_abs (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abs (cpu, cycles);
  Byte value = read_byte (cpu, address, cycles);
  write_byte (cpu, address, value, cycles);
  write_byte (cpu, address, ++value, cycles);
  set_status_flag (&cpu->P, value);
}

void
ins_inc_abx (CPU *cpu, Sint32 *cycles)
{
  Word address = get_addr_abx (cpu, cycles, true);
  Byte value = read_byte (cpu, address, cycles);
  write_byte (cpu, address, value, cycles);
  write_byte (cpu, address, ++value, cycles);
  set_status_flag (&cpu->P, value);
}

/* Register Instructions */

// Implied mode, length of one byte and requre 2 machine cycles
// Flags: N-----Z-
void
ins_tax (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  cpu->X = cpu->A;
  set_status_flag (&cpu->P, cpu->X);
}
void
ins_tay (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  cpu->Y = cpu->A;
  set_status_flag (&cpu->P, cpu->Y);
}
void
ins_txa (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  cpu->A = cpu->X;
  set_status_flag (&cpu->P, cpu->A);
}
void
ins_tya (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  cpu->A = cpu->Y;
  set_status_flag (&cpu->P, cpu->A);
}
void
ins_dex (CPU *cpu, Sint32 *cycles)
{
  burn_cycle(cpu, cycles);
  cpu->X--;
  set_status_flag(&cpu->P, cpu->X);
}
void
ins_dey (CPU *cpu, Sint32 *cycles)
{
  burn_cycle(cpu, cycles);
  cpu->Y--;
  set_status_flag(&cpu->P, cpu->Y);
}
void
ins_inx (CPU *cpu, Sint32 *cycles)
{
  burn_cycle(cpu, cycles);
  cpu->X++;
  set_status_flag(&cpu->P, cpu->X);
}
void
ins_iny (CPU *cpu, Sint32 *cycles)
{
  burn_cycle(cpu, cycles);
  cpu->Y++;
  set_status_flag(&cpu->P, cpu->Y);
}

/* Stack Instructions */

// Implied mode, 1 byte.  Stack is on page $01 ($0100-$01FF), and works
// top-down.  Decrement on push, Increment on pull(pop)
// Flags --------
void
ins_pha (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  stack_push (cpu, cpu->A, cycles);
}

void
ins_php (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  stack_push (cpu, cpu->P | 0x30, cycles);
}

void
ins_txs (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  cpu->SP = cpu->X;
}

// Flags: N-----Z-
void
ins_pla (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  burn_cycle (cpu, cycles);
  cpu->A = stack_pop (cpu, cycles);
  set_status_flag (&cpu->P, cpu->A);
}
void
ins_tsx (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  cpu->X = cpu->SP;
  set_status_flag (&cpu->P, cpu->X);
}

// Flags: NV-BDIZC
void
ins_plp (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  burn_cycle (cpu, cycles);

  Byte status = stack_pop (cpu, cycles);

  cpu->P = (status & 0xEF) | 0x20;
}

/* Other Instructions */
// Sets the B flag, then generates a forced interrupt.  Interrupt flag is
// ignored, CPU goes through the normal interrupt process.  B flag can be used
// to distinguish BRK from standard interrupt. Flags: ---B----
void
ins_brk (CPU *cpu, Sint32 *cycles)
{
  burn_cycle (cpu, cycles);
  cpu->PC++;
  stack_push(cpu, (cpu->PC >> 8) & 0xFF, cycles);
  stack_push(cpu, cpu->PC & 0xFF, cycles);
  stack_push(cpu, cpu->P | FLAG_BREAK, cycles);

  Byte loByte = read_byte(cpu, 0xFFFE, cycles);  // Interrupt handler
  Byte hiByte = read_byte(cpu, 0xFFFF, cycles);  //

  Word interruptAddress = get_word_address(loByte, hiByte);
  cpu->PC = interruptAddress;
  cpu->P |= FLAG_INTERRUPT_DISABLE;
}

// 2 machine cycles, does not affect any register or mem location
// Flags: --------
void
ins_nop (CPU *cpu, Sint32 *cycles)
{
  burn_cycle(cpu, cycles);
}
