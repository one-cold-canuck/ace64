#include "../code/cpu.h"
#include <gtest/gtest.h>
// TODO: These tests need to be implemented:
//  - TAX, TAY, TXA, TYA
//  - Stack overflow/underflow: going past 0x01FF or below 0x0100 should wrap
//  to the proper location on the other side of the stack
class ace64Test : public testing::Test
{
public:
  CPU cpu;

  virtual void
  SetUp ()
  {
    initialize_memory (&cpu);
    reset (&cpu);
  }
  virtual void
  TearDown ()
  {
  }
};

static void
VerifyUnmodifiedFlags (CPU cpu, CPU cpuCopy)
{
  EXPECT_FALSE ((cpu.P & FLAG_OVERFLOW) ^ (cpuCopy.P & FLAG_OVERFLOW));
  EXPECT_FALSE ((cpu.P & FLAG_DECIMAL_MODE) ^ (cpuCopy.P & FLAG_DECIMAL_MODE));
  EXPECT_FALSE ((cpu.P & FLAG_INTERRUPT_DISABLE)
                ^ (cpuCopy.P & FLAG_INTERRUPT_DISABLE));
  EXPECT_FALSE ((cpu.P & FLAG_CARRY) ^ (cpuCopy.P & FLAG_CARRY));
  EXPECT_FALSE ((cpu.P & FLAG_BREAK) ^ (cpuCopy.P & FLAG_BREAK));
}

TEST_F (ace64Test, StackPointerCheckOverflow)
{

  // given:
  cpu.SP = 0xFF;
  cpu.Memory[0x0101] = 0x77;
  CPU cpuCopy = cpu;

  // when:
  cpu.SP += 2;

  // then:
  EXPECT_EQ (cpu.Memory[0x0100 + cpu.SP], 0x77);
  EXPECT_EQ (cpu.SP, 0x01);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, StackPointerCheckUnderflow)
{

  // given:
  cpu.SP = 0x00;
  cpu.Memory[0x01FE] = 0x77;
  CPU cpuCopy = cpu;

  // when:
  cpu.SP -= 2;

  // then:
  EXPECT_EQ (cpu.Memory[0x0100 + cpu.SP], 0x77);
  EXPECT_EQ (cpu.SP, 0xFE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

/***************************************************
 * Begin Memory Instruction Tests
 */

/* INS_LDA - All modes */
TEST_F (ace64Test, LDAImmediate)
{

  // given:
  cpu.Memory[0xFFFC] = INS_LDA_IM;
  cpu.Memory[0xFFFD] = 0x77;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 2;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDAZeroPage)
{

  // given:
  cpu.Memory[0xFFFC] = INS_LDA_ZP;
  cpu.Memory[0xFFFD] = 0x42;
  cpu.Memory[0x0042] = 0x37;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 3;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.A, 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDAZeroPageX)
{

  // given:
  cpu.X = 0x02;
  cpu.Memory[0xFFFC] = INS_LDA_ZPX;
  cpu.Memory[0xFFFD] = 0x04;
  cpu.Memory[0x0006] = 0x37;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 4;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDAZeroPageXBoundaryCheck)
{

  // given:
  cpu.X = 0xFF;
  cpu.Memory[0xFFFC] = INS_LDA_ZPX;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0x007F] = 0x37;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 4;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDAImmediateZeroValue)
{

  // given:
  cpu.A = 0x44;
  cpu.Memory[0xFFFC] = INS_LDA_IM;
  cpu.Memory[0xFFFD] = 0x0;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 2;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.A, 0x0);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_TRUE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDAAbsolute)
{

  // given:
  cpu.Memory[0xFFFC] = INS_LDA_ABS;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44; // 0x4480
  cpu.Memory[0x4480] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 4;
  CPU cpuCopy = cpu;
  //
  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}
TEST_F (ace64Test, LDAAbsoluteX)
{

  // given:
  cpu.X = 1;
  cpu.Memory[0xFFFC] = INS_LDA_ABX;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44; // 0x4480
  cpu.Memory[0x4481] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 4;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDAAbsoluteXBoundary)
{

  // given:
  cpu.X = 0xFF;
  cpu.Memory[0xFFFC] = INS_LDA_ABX;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44; // 0x4480
  cpu.Memory[0x457F] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 5;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDAAbsoluteY)
{

  // given:
  cpu.Y = 1;
  cpu.Memory[0xFFFC] = INS_LDA_ABY;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44; // 0x4480
  cpu.Memory[0x4481] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 4;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDAAbsoluteYBoundary)
{

  // given:
  cpu.Y = 0xFF;
  cpu.Memory[0xFFFC] = INS_LDA_ABY;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44; // 0x4480
  cpu.Memory[0x457F] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 5;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDAIndirectX)
{

  // given:
  cpu.X = 4;
  cpu.Memory[0xFFFC] = INS_LDA_IDX;
  cpu.Memory[0xFFFD] = 0x02;
  cpu.Memory[0x0006] = 0x00;
  cpu.Memory[0x0007] = 0x80;
  cpu.Memory[0x8000] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 6;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDAIndirectY)
{

  // given:
  cpu.Y = 0x04;
  cpu.Memory[0xFFFC] = INS_LDA_IDY;
  cpu.Memory[0xFFFD] = 0x02;
  cpu.Memory[0x0002] = 0x00; // 0x4480
  cpu.Memory[0x0003] = 0x80;
  cpu.Memory[0x8004] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 5;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDAAIndirectYBoundary)
{

  // given:
  cpu.Y = 0xFF;
  cpu.Memory[0xFFFC] = INS_LDA_IDY;
  cpu.Memory[0xFFFD] = 0x02;
  cpu.Memory[0x0002] = 0x80;
  cpu.Memory[0x0003] = 0x44;
  cpu.Memory[0x457F] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 6;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

/* INS_LDX - All Modes */
TEST_F (ace64Test, LDXImmediate)
{

  // given:
  cpu.Memory[0xFFFC] = INS_LDX_IM;
  cpu.Memory[0xFFFD] = 0x77;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 2;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.X, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDXZeroPage)
{

  // given:
  cpu.Memory[0xFFFC] = INS_LDX_ZP;
  cpu.Memory[0xFFFD] = 0x42;
  cpu.Memory[0x0042] = 0x37;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 3;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.X, 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDXZeroPageY)
{

  // given:
  cpu.Y = 0x04;
  cpu.Memory[0xFFFC] = INS_LDX_ZPY;
  cpu.Memory[0xFFFD] = 0x06;
  cpu.Memory[0x000A] = 0x37;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 4;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.X, 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDXZeroPageYBoundaryCheck)
{

  // given:
  cpu.Y = 0xFF;
  cpu.Memory[0xFFFC] = INS_LDX_ZPY;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0x007F] = 0x37;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 4;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.X, 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDXAbsolute)
{

  // given:
  cpu.Memory[0xFFFC] = INS_LDX_ABS;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44; // 0x4480
  cpu.Memory[0x4480] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 4;
  CPU cpuCopy = cpu;
  //
  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.X, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDXAbsoluteY)
{

  // given:
  cpu.Y = 1;
  cpu.Memory[0xFFFC] = INS_LDX_ABY;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44; // 0x4480
  cpu.Memory[0x4481] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 4;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.X, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDXAbsoluteYBoundary)
{

  // given:
  cpu.Y = 0xFF;
  cpu.Memory[0xFFFC] = INS_LDX_ABY;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44; // 0x4480
  cpu.Memory[0x457F] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 5;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.X, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

/* INS_LDY - All Modes */
TEST_F (ace64Test, LDYImmediate)
{

  // given:
  cpu.Memory[0xFFFC] = INS_LDY_IM;
  cpu.Memory[0xFFFD] = 0x77;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 2;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.Y, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDYZeroPage)
{

  // given:
  cpu.Memory[0xFFFC] = INS_LDY_ZP;
  cpu.Memory[0xFFFD] = 0x42;
  cpu.Memory[0x0042] = 0x37;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 3;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Y, 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDYZeroPageX)
{

  // given:
  cpu.X = 0x04;
  cpu.Memory[0xFFFC] = INS_LDY_ZPX;
  cpu.Memory[0xFFFD] = 0x06;
  cpu.Memory[0x000A] = 0x37;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 4;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.Y, 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDYZeroPageXBoundaryCheck)
{

  // given:
  cpu.X = 0xFF;
  cpu.Memory[0xFFFC] = INS_LDY_ZPX;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0x007F] = 0x37;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 4;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.Y, 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDYAbsolute)
{

  // given:
  cpu.Memory[0xFFFC] = INS_LDY_ABS;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44; // 0x4480
  cpu.Memory[0x4480] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 4;
  CPU cpuCopy = cpu;
  //
  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.Y, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDYAbsoluteX)
{

  // given:
  cpu.X = 1;
  cpu.Memory[0xFFFC] = INS_LDY_ABX;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44; // 0x4480
  cpu.Memory[0x4481] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 4;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Y, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, LDYAbsoluteXBoundary)
{

  // given:
  cpu.X = 0xFF;
  cpu.Memory[0xFFFC] = INS_LDY_ABX;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44; // 0x4480
  cpu.Memory[0x457F] = 0x77;
  constexpr Sint32 EXPECTED_CYCLES = 5;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Y, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

/* INS_STA - All Modes */
TEST_F (ace64Test, STAZPWritesARegisterToMemory)
{
  // given:
  cpu.A = 0x37;
  cpu.Memory[0xFFFC] = INS_STA_ZP;
  cpu.Memory[0xFFFD] = 0x3D;

  constexpr Sint32 EXPECTED_CYCLES = 3;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.A, 0x37);
  EXPECT_EQ (cpu.Memory[0x003D], 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, STAZPXWritesARegisterToMemory)
{
  // given:
  cpu.A = 0x37;
  cpu.X = 0x10;
  cpu.Memory[0xFFFC] = INS_STA_ZPX;
  cpu.Memory[0xFFFD] = 0x3D;

  constexpr Sint32 EXPECTED_CYCLES = 4;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.A, 0x37);
  EXPECT_EQ (cpu.Memory[0x4D], 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, STAAbsolute)
{

  // given:
  cpu.A = 0x77;
  cpu.Memory[0xFFFC] = INS_STA_ABS;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44; // 0x4480
  constexpr Sint32 EXPECTED_CYCLES = 4;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_EQ (cpu.Memory[0x4480], 0x77);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}
TEST_F (ace64Test, STAAbsoluteX)
{

  // given:
  cpu.A = 0x77;
  cpu.X = 0x10;
  cpu.Memory[0xFFFC] = INS_STA_ABX;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44; // 0x4480
  constexpr Sint32 EXPECTED_CYCLES = 5;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x4490], 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, STAAbsoluteXBoundary)
{

  // given:
  cpu.A = 0x77;
  cpu.X = 0xFF;
  cpu.Memory[0xFFFC] = INS_STA_ABX;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44; // 0x4480
  constexpr Sint32 EXPECTED_CYCLES = 5;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x457F], 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, STAAbsoluteY)
{

  // given:
  cpu.A = 0x77;
  cpu.Y = 0x10;
  cpu.Memory[0xFFFC] = INS_STA_ABY;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44; // 0x4480
  constexpr Sint32 EXPECTED_CYCLES = 5;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x4490], 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, STAAbsoluteYBoundary)
{

  // given:
  cpu.A = 0x77;
  cpu.Y = 0xFF;
  cpu.Memory[0xFFFC] = INS_STA_ABY;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44; // 0x4480
  constexpr Sint32 EXPECTED_CYCLES = 5;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x457F], 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, STAIndirectX)
{

  // given:
  cpu.A = 0x77;
  cpu.X = 0x04;
  cpu.Memory[0xFFFC] = INS_STA_IDX;
  cpu.Memory[0xFFFD] = 0x02;
  cpu.Memory[0x0006] = 0x00;
  cpu.Memory[0x0007] = 0x80;
  constexpr Sint32 EXPECTED_CYCLES = 6;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x8000], 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, STAIndirectY)
{

  // given:
  cpu.A = 0x77;
  cpu.Y = 0x04;
  cpu.Memory[0xFFFC] = INS_STA_IDY;
  cpu.Memory[0xFFFD] = 0x02;
  cpu.Memory[0x0002] = 0x00; // 0x4480
  cpu.Memory[0x0003] = 0x80;
  constexpr Sint32 EXPECTED_CYCLES = 6;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x8004], 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, STAAIndirectYBoundary)
{

  // given:
  cpu.A = 0x77;
  cpu.Y = 0xFF;
  cpu.Memory[0xFFFC] = INS_STA_IDY;
  cpu.Memory[0xFFFD] = 0x02;
  cpu.Memory[0x0002] = 0x80;
  cpu.Memory[0x0003] = 0x44;
  constexpr Sint32 EXPECTED_CYCLES = 6;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x457F], 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

/* INS_STX - All Modes */
TEST_F (ace64Test, STX_ZeroPoint_Writes_X_Register_To_Memory)
{
  // given:
  cpu.X = 0x37;
  cpu.Memory[0xFFFC] = INS_STX_ZP;
  cpu.Memory[0xFFFD] = 0x3D;

  constexpr Sint32 EXPECTED_CYCLES = 3;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.X, 0x37);
  EXPECT_EQ (cpu.Memory[0x003D], 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, STXAbsolute)
{

  // given:
  cpu.X = 0x77;
  cpu.Memory[0xFFFC] = INS_STX_ABS;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44; // 0x4480
  constexpr Sint32 EXPECTED_CYCLES = 4;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.X, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_EQ (cpu.Memory[0x4480], 0x77);
}

TEST_F (ace64Test, STX_ZeroPoint_Indexed_Address_Contains_Value)
{
  //
  // given:
  cpu.Y = 0x02;
  cpu.X = 0x37;
  cpu.Memory[0xFFFC] = INS_STX_ZPY;
  cpu.Memory[0xFFFD] = 0x04;
  cpu.Memory[0x0006] = 0x00;
  constexpr Sint32 EXPECTED_CYCLES = 4;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x0006], 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
}

TEST_F (ace64Test, STY_ZerPoint_Address_Contains_Value)
{

  // given:
  cpu.Y = 0x37;
  cpu.Memory[0xFFFC] = INS_STY_ZP;
  cpu.Memory[0xFFFD] = 0x3D;

  constexpr Sint32 EXPECTED_CYCLES = 3;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x003D], 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
}

/* DEC and INC */
TEST_F (ace64Test, DEC_ZeroPage_Decreases_Value)
{
  // given:
  cpu.Memory[0xFFFC] = INS_DEC_ZP;
  cpu.Memory[0xFFFD] = 0x3D;
  cpu.Memory[0x003D] = 0x01;
  constexpr Sint32 EXPECTED_CYCLES = 5;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x003D], 0x00);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, DEC_ZeroPage_Indexed_Decreases_Value)
{
  // given:
  cpu.X = 0x02;
  cpu.Memory[0xFFFC] = INS_DEC_ZPX;
  cpu.Memory[0xFFFD] = 0x04;
  cpu.Memory[0x0006] = 0x01;
  constexpr Sint32 EXPECTED_CYCLES = 6;
  CPU cpuCopy = cpu;
  printf ("Status before: %b\n", cpu.P);
  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x0006], 0x00);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  printf ("Status: %b\n", cpu.P);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, DEC_ZeroPage_ZeroValue)
{
  // given:
  cpu.Memory[0xFFFC] = INS_DEC_ZP;
  cpu.Memory[0xFFFD] = 0x04;
  cpu.Memory[0x0004] = 0x00;
  constexpr Sint32 EXPECTED_CYCLES = 5;
  CPU cpuCopy = cpu;
  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x0004], 0xFF);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  printf ("Status: %b\n", cpu.P);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, INC_ZeroPage_Increases_Value)
{
  // given:
  cpu.Memory[0xFFFC] = INS_INC_ZP;
  cpu.Memory[0xFFFD] = 0x04;
  cpu.Memory[0x0004] = 0x01;
  constexpr Sint32 EXPECTED_CYCLES = 5;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x0004], 0x02);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, INC_ZeroPage_FFValue)
{
  // given:
  cpu.Memory[0xFFFC] = INS_INC_ZP;
  cpu.Memory[0xFFFD] = 0x3D;
  cpu.Memory[0x003D] = 0xFF;
  constexpr Sint32 EXPECTED_CYCLES = 5;
  CPU cpuCopy = cpu;
  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x003D], 0x00);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  printf ("Status: %b\n", cpu.P);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, INC_ZeroPage_Indexed_Increased_Value)
{
  // given:
  cpu.X = 0x02;
  cpu.Memory[0xFFFC] = INS_INC_ZPX;
  cpu.Memory[0xFFFD] = 0x04;
  cpu.Memory[0x0006] = 0x01;
  constexpr Sint32 EXPECTED_CYCLES = 6;
  CPU cpuCopy = cpu;
  printf ("Status before: %b\n", cpu.P);
  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x0006], 0x02);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  printf ("Status: %b\n", cpu.P);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, DEC_Absolute_Decreases_Value)
{
  // given:
  cpu.Memory[0xFFFC] = INS_DEC_ABS;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44;
  cpu.Memory[0x4480] = 0x31;
  constexpr Sint32 EXPECTED_CYCLES = 6;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x4480], 0x30);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  printf ("Status: %b\n", cpu.P);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, INC_Absolute_Decreases_Value)
{
  // given:
  cpu.Memory[0xFFFC] = INS_INC_ABS;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44;
  cpu.Memory[0x4480] = 0x31;
  constexpr Sint32 EXPECTED_CYCLES = 6;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x4480], 0x32);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  printf ("Status: %b\n", cpu.P);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, DEC_Absolute_Indexed_Decreases_Value)
{
  // given:
  cpu.X = 0x32;
  cpu.Memory[0xFFFC] = INS_DEC_ABX;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44;
  cpu.Memory[0x44B2] = 0x31;
  constexpr Sint32 EXPECTED_CYCLES = 7;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x44B2], 0x30);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  printf ("Status: %b\n", cpu.P);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, INC_Absolute_Indexed_Increases_Value)
{
  // given:
  cpu.X = 0x32;
  cpu.Memory[0xFFFC] = INS_INC_ABX;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44;
  cpu.Memory[0x44B2] = 0x31;
  constexpr Sint32 EXPECTED_CYCLES = 7;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x44B2], 0x32);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  printf ("Status: %b\n", cpu.P);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

/* Program flow tests */
TEST_F (ace64Test, JMPAbsolutePCContainsAddress)
{
  // given:
  cpu.Memory[0xFFFC] = INS_JMP_ABS;
  cpu.Memory[0xFFFD] = 0x80;
  cpu.Memory[0xFFFE] = 0x44;
  cpu.Memory[0x4480] = INS_LDA_ABS;

  constexpr Sint32 EXPECTED_CYCLES = 3;
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.PC, 0x4480);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, JSRTestPushAndJump)
{
  // given:
  Word startPC = 0x1000;
  cpu.PC = startPC;
  cpu.Memory[startPC] = 0x20;
  cpu.Memory[startPC + 1] = 0x00;
  cpu.Memory[startPC + 2] = 0x40;

  cpu.SP = 0xFF;

  constexpr Sint32 EXPECTED_CYCLES = 6;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.PC, 0x4000);
  EXPECT_EQ (cpu.SP, 0xFD);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
}
TEST_F (ace64Test, TAXTestValues)
{

  // given:
  cpu.Memory[0xFFFC] = INS_TAX;
  cpu.A = 0x42;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 2;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.X, 0x42);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, TAXTestZeroValue)
{

  // given:
  cpu.Memory[0xFFFC] = INS_TAX;
  cpu.A = 0x00;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 2;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.X, 0x00);
  EXPECT_TRUE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, TAYTestValues)
{

  // given:
  cpu.Memory[0xFFFC] = INS_TAY;
  cpu.A = 0x42;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 2;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.Y, 0x42);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, TAYTestZeroValue)
{

  // given:
  cpu.Memory[0xFFFC] = INS_TAY;
  cpu.A = 0x00;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 2;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.Y, 0x00);
  EXPECT_TRUE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, TXATestValues)
{

  // given:
  cpu.Memory[0xFFFC] = INS_TXA;
  cpu.X = 0x42;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 2;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x42);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, TXATestZeroValue)
{

  // given:
  cpu.Memory[0xFFFC] = INS_TXA;
  cpu.X = 0x00;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 2;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.Y, 0x00);
  EXPECT_TRUE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, TYATestValues)
{

  // given:
  cpu.Memory[0xFFFC] = INS_TYA;
  cpu.Y = 0x42;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 2;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x42);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, TYATestZeroValue)
{

  // given:
  cpu.Memory[0xFFFC] = INS_TYA;
  cpu.Y = 0x00;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 2;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.Y, 0x00);
  EXPECT_TRUE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, TSXTestValues)
{

  // given:
  cpu.Memory[0xFFFC] = INS_TSX;
  cpu.SP = 0x7A;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 2;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.X, 0x7A);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, TSXTestZeroValue)
{

  // given:
  cpu.SP = 0x00;
  cpu.Memory[0xFFFC] = INS_TSX;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 2;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.X, 0x00);
  EXPECT_TRUE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, TXSTestValues)
{

  // given:
  cpu.Memory[0xFFFC] = INS_TXS;
  cpu.X = 0x7A;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 2;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.SP, 0x7A);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, PHATestStackPush)
{

  // given:
  cpu.Memory[0xFFFC] = INS_PHA;
  cpu.A = 0x42;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 3;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.Memory[0x01FF], 0x42);
  EXPECT_EQ (cpu.SP, 0xFE);
}

TEST_F (ace64Test, PHPTestStackPush)
{

  // given:
  cpu.Memory[0xFFFC] = INS_PHP;
  cpu.P |= FLAG_OVERFLOW | FLAG_CARRY;

  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 3;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  Byte expectedValue = FLAG_OVERFLOW | FLAG_CARRY | 0x30;
  EXPECT_EQ (cpu.Memory[0x01FF], expectedValue);
  EXPECT_EQ (cpu.SP, 0xFE);
}

TEST_F (ace64Test, PLATestAccumulatorLoad)
{

  // given:
  cpu.SP = 0xFC;
  cpu.Memory[0xFFFC] = INS_PLA;
  cpu.Memory[0x01FD] = 0x88;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 3;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x88);
  EXPECT_EQ (cpu.SP, 0xFD);
  EXPECT_EQ (0x0100 + cpu.SP, 0x01FD);
}

TEST_F (ace64Test, PLPTestStatusLoad)
{

  // given:
  Byte stackValue = FLAG_NEGATIVE | FLAG_OVERFLOW | 0x10 | FLAG_CARRY;
  cpu.SP = 0xFE;
  cpu.Memory[0x01FF] = stackValue;

  cpu.Memory[0xFFFC] = INS_PLP;
  CPU cpuCopy = cpu;

  constexpr Sint32 EXPECTED_CYCLES = 3;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:

  Byte expected = (stackValue & ~0x10) | 0x20;
  EXPECT_EQ (cpu.P, expected);
  EXPECT_EQ (cpu.SP, 0xFF);
}

TEST_F (ace64Test, ASLAccumulatorMode)
{
  // given:
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x0A;
  cpu.A = 0x81;
  cpu.P = 0x00;
  constexpr Sint32 EXPECTED_CYCLES = 2;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.A, 0x02);
  EXPECT_TRUE (cpu.P & FLAG_CARRY);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
}

TEST_F (ace64Test, ASLZeroPageMode)
{
  // given:
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x06;
  cpu.Memory[0x1001] = 0x80;
  cpu.Memory[0x0080] = 0xC0;
  cpu.P = 0x00;

  constexpr Sint32 EXPECTED_CYCLES = 5;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x0080], 0x80);
  EXPECT_TRUE (cpu.P & FLAG_CARRY);
  EXPECT_TRUE (cpu.P & FLAG_NEGATIVE);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
}

TEST_F (ace64Test, ASLZeroPageXMode)
{
  // given:
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x16;
  cpu.Memory[0x1001] = 0x80;

  cpu.X = 0x05;
  cpu.Memory[0x0085] = 0xC0;
  cpu.P = 0x00;

  constexpr Sint32 EXPECTED_CYCLES = 6;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x0085], 0x80);
  EXPECT_TRUE (cpu.P & FLAG_CARRY);
  EXPECT_TRUE (cpu.P & FLAG_NEGATIVE);

  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
}

TEST_F(ace64Test, ASLZeroPageXWrappingTest)
{
  // given: 
  // Base $FF + X $02 = Effective address $01 (NOT $0101)
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x16; 
  cpu.Memory[0x1001] = 0xFF; 
  
  cpu.X = 0x02;
  cpu.Memory[0x0001] = 0x80; // Target is at $0001

  // when:
  execute(&cpu);

  // then:
  EXPECT_EQ(cpu.Memory[0x0001], 0x00); // $80 << 1 = $00
  EXPECT_TRUE(cpu.P & FLAG_ZERO);
  EXPECT_TRUE(cpu.P & FLAG_CARRY);
}

TEST_F(ace64Test, ASLAbsoluteTest){

  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = INS_ASL_ABS;
  cpu.Memory[0x1001] = 0x80;
  cpu.Memory[0x1002] = 0x44;
  cpu.Memory[0x4480] = 0xC0;
  cpu.P = 0x00;

  constexpr Sint32 EXPECTED_CYCLES = 6;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x4480], 0x80);
  EXPECT_TRUE (cpu.P & FLAG_CARRY);
  EXPECT_TRUE (cpu.P & FLAG_NEGATIVE);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
}

TEST_F(ace64Test, ASLAbsoluteXIndexedTest){

  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = INS_ASL_ABX;
  cpu.Memory[0x1001] = 0x80;
  cpu.Memory[0x1002] = 0x44;
  cpu.X = 0x05;
  cpu.Memory[0x4485] = 0xC0;
  cpu.P = 0x00;

  constexpr Sint32 EXPECTED_CYCLES = 7;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x4485], 0x80);
  EXPECT_TRUE (cpu.P & FLAG_CARRY);
  EXPECT_TRUE (cpu.P & FLAG_NEGATIVE);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
}

TEST_F (ace64Test, LSRAccumulatorTest)
{
  // given:
  // Binary: 0000 0011 ($03)
  // Shift right: Bit 0 is '1', so Carry should be set.
  // Result should be 0000 0001 ($01).
  cpu.PC = 0x2000;
  cpu.Memory[0x2000] = 0x4A; // LSR Accumulator opcode
  cpu.A = 0x03;
  cpu.P = FLAG_NEGATIVE; // Start with Negative set to see if it clears

  // when:
  execute (&cpu);

  // then:
  EXPECT_EQ (cpu.A, 0x01);
  EXPECT_TRUE (cpu.P & FLAG_CARRY);     // Old bit 0 was 1
  EXPECT_FALSE (cpu.P & FLAG_ZERO);     // Result is not 0
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE); // LSR always clears bit 7
}

TEST_F(ace64Test, LSRZeroPageTest) {
    // given: $0080 contains $03 (0000 0011)
    cpu.PC = 0x1000;
    cpu.Memory[0x1000] = 0x46; 
    cpu.Memory[0x1001] = 0x80;
    cpu.Memory[0x0080] = 0x03;
    cpu.P = 0x00;

    // when: $03 >> 1 = $01, Carry = 1
    Sint32 cycles = execute(&cpu);

    // then:
    EXPECT_EQ(cpu.Memory[0x0080], 0x01);
    EXPECT_TRUE(cpu.P & FLAG_CARRY);
    EXPECT_EQ(cycles, 5);
}

TEST_F(ace64Test, LSRZeroPageXTest) {
    // given: Base $FF + X $02 = Address $01 (Wrap)
    cpu.PC = 0x1000;
    cpu.Memory[0x1000] = 0x56; 
    cpu.Memory[0x1001] = 0xFF; 
    cpu.X = 0x02;
    cpu.Memory[0x0001] = 0x01; // Value $01 >> 1 = $00

    // when:
    Sint32 cycles = execute(&cpu);

    // then:
    EXPECT_EQ(cpu.Memory[0x0001], 0x00);
    EXPECT_TRUE(cpu.P & FLAG_CARRY);
    EXPECT_TRUE(cpu.P & FLAG_ZERO);
    EXPECT_EQ(cycles, 6);
}

TEST_F(ace64Test, LSRAbsoluteTest) {
    // given: $2000 contains $40 (0100 0000)
    cpu.PC = 0x1000;
    cpu.Memory[0x1000] = 0x4E; 
    cpu.Memory[0x1001] = 0x00; // Lo
    cpu.Memory[0x1002] = 0x20; // Hi
    cpu.Memory[0x2000] = 0x40;

    // when: $40 >> 1 = $20, Carry = 0
    Sint32 cycles = execute(&cpu);

    // then:
    EXPECT_EQ(cpu.Memory[0x2000], 0x20);
    EXPECT_FALSE(cpu.P & FLAG_CARRY);
    EXPECT_EQ(cycles, 6);
}

TEST_F(ace64Test, LSRAbsoluteXTest) {
    // given: $2000 + X $05 = $2005 (Same page)
    cpu.PC = 0x1000;
    cpu.Memory[0x1000] = 0x5E; 
    cpu.Memory[0x1001] = 0x00;
    cpu.Memory[0x1002] = 0x20;
    cpu.X = 0x05;
    cpu.Memory[0x2005] = 0x01; // Value $01 >> 1 = $00

    // when:
    Sint32 cycles = execute(&cpu);

    // then:
    EXPECT_EQ(cpu.Memory[0x2005], 0x00);
    EXPECT_TRUE(cpu.P & FLAG_ZERO);
    EXPECT_EQ(cycles, 7); // Forced cycle check
}
