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
  // We set specific flags (Negative, Overflow, and Carry)
  // Binary: 1100 0001 ($C1)
  cpu.PC = 0xFFFC;
  cpu.Memory[0xFFFC] = 0x08; // PHP Opcode
  cpu.P = FLAG_NEGATIVE | FLAG_OVERFLOW | FLAG_CARRY;
  cpu.SP = 0xFF; // Start of stack

  // when:
  execute (&cpu);

  // then:
  // 1. Stack Pointer should decrement
  EXPECT_EQ (cpu.SP, 0xFE);

  /* 2. Verify the Pushed Value at $01FF
     The pushed value MUST be: (Original P) OR 0x30
     - Bit 5 is always 1 on the stack ($20)
     - Bit 4 is the Break flag, set to 1 for PHP ($10)
     Original ($C1) | $30 = $F1 (1111 0001)
  */
  Byte expectedValue = FLAG_NEGATIVE | FLAG_OVERFLOW | FLAG_CARRY | 0x30;
  EXPECT_EQ (cpu.Memory[0x01FF], expectedValue);
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

TEST_F (ace64Test, ASLZeroPageXWrappingTest)
{
  // given:
  // Base $FF + X $02 = Effective address $01 (NOT $0101)
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x16;
  cpu.Memory[0x1001] = 0xFF;

  cpu.X = 0x02;
  cpu.Memory[0x0001] = 0x80; // Target is at $0001

  // when:
  execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x0001], 0x00); // $80 << 1 = $00
  EXPECT_TRUE (cpu.P & FLAG_ZERO);
  EXPECT_TRUE (cpu.P & FLAG_CARRY);
}

TEST_F (ace64Test, ASLAbsoluteTest)
{

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

TEST_F (ace64Test, ASLAbsoluteXIndexedTest)
{

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

TEST_F (ace64Test, LSRZeroPageTest)
{
  // given: $0080 contains $03 (0000 0011)
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x46;
  cpu.Memory[0x1001] = 0x80;
  cpu.Memory[0x0080] = 0x03;
  cpu.P = 0x00;

  // when: $03 >> 1 = $01, Carry = 1
  Sint32 cycles = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x0080], 0x01);
  EXPECT_TRUE (cpu.P & FLAG_CARRY);
  EXPECT_EQ (cycles, 5);
}

TEST_F (ace64Test, LSRZeroPageXTest)
{
  // given: Base $FF + X $02 = Address $01 (Wrap)
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x56;
  cpu.Memory[0x1001] = 0xFF;
  cpu.X = 0x02;
  cpu.Memory[0x0001] = 0x01; // Value $01 >> 1 = $00

  // when:
  Sint32 cycles = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x0001], 0x00);
  EXPECT_TRUE (cpu.P & FLAG_CARRY);
  EXPECT_TRUE (cpu.P & FLAG_ZERO);
  EXPECT_EQ (cycles, 6);
}

TEST_F (ace64Test, LSRAbsoluteTest)
{
  // given: $2000 contains $40 (0100 0000)
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x4E;
  cpu.Memory[0x1001] = 0x00; // Lo
  cpu.Memory[0x1002] = 0x20; // Hi
  cpu.Memory[0x2000] = 0x40;

  // when: $40 >> 1 = $20, Carry = 0
  Sint32 cycles = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x2000], 0x20);
  EXPECT_FALSE (cpu.P & FLAG_CARRY);
  EXPECT_EQ (cycles, 6);
}

TEST_F (ace64Test, LSRAbsoluteXTest)
{
  // given: $2000 + X $05 = $2005 (Same page)
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x5E;
  cpu.Memory[0x1001] = 0x00;
  cpu.Memory[0x1002] = 0x20;
  cpu.X = 0x05;
  cpu.Memory[0x2005] = 0x01; // Value $01 >> 1 = $00

  // when:
  Sint32 cycles = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x2005], 0x00);
  EXPECT_TRUE (cpu.P & FLAG_ZERO);
  EXPECT_EQ (cycles, 7); // Forced cycle check
}

TEST_F (ace64Test, EORImmediate)
{
  cpu.A = 0xFF;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x49; // EOR #$AA
  cpu.Memory[0x1001] = 0xAA;

  Sint32 cyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x55);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  EXPECT_EQ (cyclesUsed, 2);
}

TEST_F (ace64Test, EORZeroPage)
{
  cpu.A = 0xFF;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x45; // EOR $80
  cpu.Memory[0x1001] = 0x80;
  cpu.Memory[0x0080] = 0xAA;

  Sint32 cyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x55);
  EXPECT_EQ (cyclesUsed, 3);
}

TEST_F (ace64Test, EORZeroPageX)
{
  cpu.A = 0xAA;
  cpu.X = 0x05;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x55; // EOR $10, X
  cpu.Memory[0x1001] = 0x10;
  cpu.Memory[0x0015] = 0xAA; // Address: $10 + $05

  Sint32 cyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x00);
  EXPECT_TRUE (cpu.P & FLAG_ZERO); // Result is 0
  EXPECT_EQ (cyclesUsed, 4);
}

TEST_F (ace64Test, EORAbsolute)
{
  cpu.A = 0x00;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x4D;
  cpu.Memory[0x1001] = 0x00; // Lo
  cpu.Memory[0x1002] = 0x20; // Hi
  cpu.Memory[0x2000] = 0x80;

  Sint32 cyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x80);
  EXPECT_TRUE (cpu.P & FLAG_NEGATIVE); // Bit 7 is 1
  EXPECT_EQ (cyclesUsed, 4);
}

TEST_F (ace64Test, EORAbsoluteXPagePenalty)
{
  cpu.A = 0xFF;
  cpu.X = 0xFF; // Forces page cross
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x5D;
  cpu.Memory[0x1001] = 0x01; // Base $2001
  cpu.Memory[0x1002] = 0x20;
  cpu.Memory[0x2100] = 0xAA; // Effective: $2001 + $FF = $2100

  Sint32 cyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x55);
  EXPECT_EQ (cyclesUsed, 5); // 4 + 1 for page cross
}

TEST_F (ace64Test, EORIndirectX)
{
  cpu.A = 0xFF;
  cpu.X = 0x01;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x41;
  cpu.Memory[0x1001] = 0x10; // Pointer address base
  // Pointer at $11 ($10+1): $2000
  cpu.Memory[0x0011] = 0x00;
  cpu.Memory[0x0012] = 0x20;
  cpu.Memory[0x2000] = 0xAA;

  Sint32 cyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x55);
  EXPECT_EQ (cyclesUsed, 6);
}

TEST_F (ace64Test, EORIndirectY)
{
  cpu.A = 0xFF;
  cpu.Y = 0x01;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x51;
  cpu.Memory[0x1001] = 0x10; // ZP Pointer
  cpu.Memory[0x0010] = 0x00; // Pointer base $2000
  cpu.Memory[0x0011] = 0x20;
  cpu.Memory[0x2001] = 0xAA; // Effective: $2000 + $01

  Sint32 cyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x55);
  EXPECT_EQ (cyclesUsed, 5);
}

TEST_F (ace64Test, ORAImmediate)
{
  cpu.A = 0xF0;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x09; // ORA #$0F
  cpu.Memory[0x1001] = 0x0F;

  Sint32 cyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0xFF);
  EXPECT_TRUE (cpu.P & FLAG_NEGATIVE);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_EQ (cyclesUsed, 2);
}

TEST_F (ace64Test, ORAZeroPage)
{
  cpu.A = 0xAA;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x05; // ORA $80
  cpu.Memory[0x1001] = 0x80;
  cpu.Memory[0x0080] = 0x55;

  Sint32 cyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0xFF);
  EXPECT_EQ (cyclesUsed, 3);
}

TEST_F (ace64Test, ORAZeroPageX)
{
  cpu.A = 0x00;
  cpu.X = 0x05;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x15; // ORA $10, X
  cpu.Memory[0x1001] = 0x10;
  cpu.Memory[0x0015] = 0x00; // $00 | $00

  Sint32 cyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x00);
  EXPECT_TRUE (cpu.P & FLAG_ZERO);
  EXPECT_EQ (cyclesUsed, 4);
}

TEST_F (ace64Test, ORAAbsolute)
{
  cpu.A = 0x01;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x0D;
  cpu.Memory[0x1001] = 0x00;
  cpu.Memory[0x1002] = 0x20;
  cpu.Memory[0x2000] = 0x02;

  Sint32 cyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x03);
  EXPECT_EQ (cyclesUsed, 4);
}

TEST_F (ace64Test, ORAAbsoluteYPagePenalty)
{
  cpu.A = 0x00;
  cpu.Y = 0xFF; // Forces page cross
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x19; // ORA $2001, Y
  cpu.Memory[0x1001] = 0x01;
  cpu.Memory[0x1002] = 0x20;
  cpu.Memory[0x2100] = 0x80;

  Sint32 cyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x80);
  EXPECT_TRUE (cpu.P & FLAG_NEGATIVE);
  EXPECT_EQ (cyclesUsed, 5); // 4 + 1 penalty
}

TEST_F (ace64Test, ORAAbsoluteXPagePenalty)
{
  cpu.A = 0x00;
  cpu.X = 0xFF; // Forces page cross
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x1D; // ORA $2001, Y
  cpu.Memory[0x1001] = 0x01;
  cpu.Memory[0x1002] = 0x20;
  cpu.Memory[0x2100] = 0x80;

  Sint32 cyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x80);
  EXPECT_TRUE (cpu.P & FLAG_NEGATIVE);
  EXPECT_EQ (cyclesUsed, 5); // 4 + 1 penalty
}

TEST_F (ace64Test, ORAIndirectX)
{
  cpu.A = 0x01;
  cpu.X = 0x01;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x01;
  cpu.Memory[0x1001] = 0x10;
  cpu.Memory[0x0011] = 0x00; // Pointer Lo
  cpu.Memory[0x0012] = 0x20; // Pointer Hi
  cpu.Memory[0x2000] = 0x80;

  Sint32 cyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x81);
  EXPECT_EQ (cyclesUsed, 6);
}

TEST_F (ace64Test, ORAIndirectY)
{
  cpu.A = 0x10;
  cpu.Y = 0x01;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x11;
  cpu.Memory[0x1001] = 0x20; // ZP Pointer
  cpu.Memory[0x0020] = 0x00; // $3000 base
  cpu.Memory[0x0021] = 0x30;
  cpu.Memory[0x3001] = 0x20;

  Sint32 cyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x30);
  EXPECT_EQ (cyclesUsed, 5);
}

TEST_F (ace64Test, ROLAccumulatorTest)
{
  // given: A = $80 (1000 0000), Carry = 1
  // ROL: Bit 7 ($80) goes to Carry. Old Carry (1) goes to Bit 0.
  // Result: 0000 0001 ($01), Carry = 1
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x2A; // ROL A
  cpu.A = 0x80;
  cpu.P = FLAG_CARRY; // Set Carry to 1

  Sint32 cycles = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x01);
  EXPECT_TRUE (cpu.P & FLAG_CARRY); // New Carry from bit 7
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
  EXPECT_EQ (cycles, 2);
}

TEST_F (ace64Test, RORAccumulatorTest)
{
  // given: A = $01 (0000 0001), Carry = 1
  // ROR: Bit 0 ($01) goes to Carry. Old Carry (1) goes to Bit 7.
  // Result: 1000 0000 ($80), Carry = 1
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x6A; // ROR A
  cpu.A = 0x01;
  cpu.P = FLAG_CARRY;

  Sint32 cycles = execute (&cpu);

  EXPECT_EQ (cpu.A, 0x80);
  EXPECT_TRUE (cpu.P & FLAG_CARRY);    // New Carry from bit 0
  EXPECT_TRUE (cpu.P & FLAG_NEGATIVE); // Bit 7 is now 1
  EXPECT_EQ (cycles, 2);
}

TEST_F (ace64Test, ROLZeroPageTest)
{
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x26;
  cpu.Memory[0x1001] = 0x80;
  cpu.Memory[0x0080] = 0x40; // 0100 0000
  cpu.P = FLAG_CARRY;        // Carry is 1

  execute (&cpu);

  // 0x40 shifted left is 0x80. Bit 0 becomes the OLD carry (1).
  EXPECT_EQ (cpu.Memory[0x0080], 0x81); // Should be 129
  EXPECT_FALSE (cpu.P & FLAG_CARRY);    // New carry is 0 (old bit 7)
  EXPECT_TRUE (cpu.P & FLAG_NEGATIVE);  // Bit 7 is set
}

TEST_F (ace64Test, RORAbsoluteXTest)
{
  // given: $2000 + X($01) = $2001, contains $01, Carry = 0
  // Result: $00, Carry = 1
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x7E;
  cpu.Memory[0x1001] = 0x00;
  cpu.Memory[0x1002] = 0x20;
  cpu.X = 0x01;
  cpu.Memory[0x2001] = 0x01;
  cpu.P = 0x00; // Carry = 0

  Sint32 cycles = execute (&cpu);

  EXPECT_EQ (cpu.Memory[0x2001], 0x00);
  EXPECT_TRUE (cpu.P & FLAG_CARRY); // Bit 0 moved to Carry
  EXPECT_TRUE (cpu.P & FLAG_ZERO);  // Result is 0
  EXPECT_EQ (cycles, 7);            // Mandatory 7 cycles
}

TEST_F (ace64Test, ROLMultiValueTest)
{
  struct TestData
  {
    Byte initialValue;
    Byte initialCarry;
    Byte expectedValue;
    bool expectedCarry;
    bool expectedNegative;
    bool expectedZero;
  };

  std::vector<TestData> cases
      = { // 1. No bits moving to Carry, No Carry moving in
          { 0x01, 0, 0x02, false, false, false },

          // 2. Bit 7 moves to Carry, No Carry moving in
          { 0x80, 0, 0x00, true, false, true },

          // 3. No bits moving to Carry, Carry moves into Bit 0
          { 0x40, 1, 0x81, false, true, false },

          // 4. Bit 7 moves to Carry, Carry moves into Bit 0
          { 0x81, 1, 0x03, true, false, false }
        };

  for (const auto &t : cases)
    {
      // Reset CPU for each sub-test
      cpu.PC = 0x1000;
      cpu.Memory[0x1000] = 0x26;
      cpu.Memory[0x1001] = 0x80;
      cpu.Memory[0x0080] = t.initialValue;
      cpu.P = t.initialCarry ? FLAG_CARRY : 0;

      execute (&cpu);

      EXPECT_EQ (cpu.Memory[0x0080], t.expectedValue)
          << "Failed on value: " << (int)t.initialValue;
      EXPECT_EQ ((bool)(cpu.P & FLAG_CARRY), t.expectedCarry);
      EXPECT_EQ ((bool)(cpu.P & FLAG_NEGATIVE), t.expectedNegative);
      EXPECT_EQ ((bool)(cpu.P & FLAG_ZERO), t.expectedZero);
    }
}

TEST_F (ace64Test, RORMultiValueTest)
{
  struct TestData
  {
    Byte initialValue;
    Byte initialCarry;
    Byte expectedValue;
    bool expectedCarry;
    bool expectedNegative;
    bool expectedZero;
  };

  std::vector<TestData> cases
      = { // 1. Simple shift right (No carry in, no carry out)
          // $02 (0000 0010) >> 1 = $01 (0000 0001)
          { 0x02, 0, 0x01, false, false, false },

          // 2. Bit 0 moves out to Carry (No carry in)
          // $01 (0000 0001) >> 1 = $00, Carry = 1
          { 0x01, 0, 0x00, true, false, true },

          // 3. Carry moves into Bit 7 (No carry out)
          // $02 >> 1 = $01 | bit 7 = $81 (1000 0001)
          { 0x02, 1, 0x81, false, true, false },

          // 4. Circular: Bit 0 to Carry, Carry to Bit 7
          // $81 (1000 0001) >> 1 = $40 | bit 7 = $C0 (1100 0000)
          { 0x81, 1, 0xC0, true, true, false }
        };

  for (const auto &t : cases)
    {
      // Reset CPU State
      cpu.PC = 0x1000;
      cpu.Memory[0x1000] = 0x66; // ROR Zero Page opcode
      cpu.Memory[0x1001] = 0x80; // ZP Address $80
      cpu.Memory[0x0080] = t.initialValue;
      cpu.P = t.initialCarry ? FLAG_CARRY : 0;

      execute (&cpu);

      EXPECT_EQ (cpu.Memory[0x0080], t.expectedValue)
          << "Failed on value: " << (int)t.initialValue;
      EXPECT_EQ ((bool)(cpu.P & FLAG_CARRY), t.expectedCarry)
          << "Carry mismatch on: " << (int)t.initialValue;
      EXPECT_EQ ((bool)(cpu.P & FLAG_NEGATIVE), t.expectedNegative)
          << "Negative mismatch on: " << (int)t.initialValue;
      EXPECT_EQ ((bool)(cpu.P & FLAG_ZERO), t.expectedZero)
          << "Zero mismatch on: " << (int)t.initialValue;
    }
}

TEST_F (ace64Test, BITAbsoluteTest)
{
  // given: A = $0F, Memory at $2000 = $C0 (1100 0000)
  cpu.A = 0x0F;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x2C; // BIT Absolute
  cpu.Memory[0x1001] = 0x00;
  cpu.Memory[0x1002] = 0x20;
  cpu.Memory[0x2000] = 0xC0;
  cpu.P = 0x00;

  // when:
  // 1. (0x0F & 0xC0) is 0 -> Zero Flag = 1
  // 2. Bit 7 of 0xC0 is 1 -> Negative Flag = 1
  // 3. Bit 6 of 0xC0 is 1 -> Overflow Flag = 1
  execute (&cpu);

  // then:
  EXPECT_EQ (cpu.A, 0x0F); // Accumulator MUST NOT change
  EXPECT_TRUE (cpu.P & FLAG_ZERO);
  EXPECT_TRUE (cpu.P & FLAG_NEGATIVE);
  EXPECT_TRUE (cpu.P & FLAG_OVERFLOW);
}

TEST_F (ace64Test, CMPImmediateGreater)
{
  cpu.A = 0x50;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0xC9; // CMP #$10
  cpu.Memory[0x1001] = 0x10;

  execute (&cpu);

  EXPECT_TRUE (cpu.P & FLAG_CARRY); // 0x50 > 0x10
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
}

TEST_F (ace64Test, CMPZeroPageEqual)
{
  cpu.A = 0x10;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0xC5;
  cpu.Memory[0x1001] = 0x80;
  cpu.Memory[0x0080] = 0x10; // Match

  execute (&cpu);

  EXPECT_TRUE (cpu.P & FLAG_CARRY);
  EXPECT_TRUE (cpu.P & FLAG_ZERO); // Result is 0
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
}

TEST_F (ace64Test, CMPZeroPageXLess)
{
  cpu.A = 0x05;
  cpu.X = 0x05;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0xD5;
  cpu.Memory[0x1001] = 0x10; // Address $15
  cpu.Memory[0x0015] = 0x10; // 0x05 < 0x10

  execute (&cpu);

  EXPECT_FALSE (cpu.P & FLAG_CARRY);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_TRUE (cpu.P & FLAG_NEGATIVE); // 0x05 - 0x10 = 0xF5 (Bit 7 set)
}

TEST_F (ace64Test, CMPAbsolute)
{
  cpu.A = 0x05;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0xCD;
  cpu.Memory[0x1001] = 0x00;
  cpu.Memory[0x1002] = 0x20;
  cpu.Memory[0x2000] = 0x01; // 0xFF > 0x01

  execute (&cpu);

  EXPECT_TRUE (cpu.P & FLAG_CARRY);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
}

TEST_F (ace64Test, CMPAbsoluteXPagePenalty)
{
  cpu.A = 0x10;
  cpu.X = 0xFF; // Crosses page
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0xDD;
  cpu.Memory[0x1001] = 0x01; // Base $2001 -> Effective $2100
  cpu.Memory[0x1002] = 0x20;
  cpu.Memory[0x2100] = 0x10;

  Sint32 cycles = execute (&cpu);

  EXPECT_EQ (cycles, 5); // 4 + 1
  EXPECT_TRUE (cpu.P & FLAG_ZERO);
}

TEST_F (ace64Test, CMPIndirectX)
{
  cpu.A = 0x40;
  cpu.X = 0x01;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0xC1;
  cpu.Memory[0x1001] = 0x10; // Ptr $11 -> $2000
  cpu.Memory[0x0011] = 0x00;
  cpu.Memory[0x0012] = 0x20;
  cpu.Memory[0x2000] = 0x80; // 0x40 < 0x80

  execute (&cpu);

  EXPECT_FALSE (cpu.P & FLAG_CARRY);
  EXPECT_TRUE (cpu.P & FLAG_NEGATIVE); // 0x40 - 0x80 = 0xC0
}

TEST_F (ace64Test, CMPIndirectY)
{
  cpu.A = 0x10;
  cpu.Y = 0x00;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0xD1;
  cpu.Memory[0x1001] = 0x20; // Ptr $20 -> $3000
  cpu.Memory[0x0020] = 0x00;
  cpu.Memory[0x0021] = 0x30;
  cpu.Memory[0x3000] = 0x10;

  execute (&cpu);

  EXPECT_TRUE (cpu.P & FLAG_ZERO);
  EXPECT_TRUE (cpu.P & FLAG_CARRY);
}

TEST_F (ace64Test, CPXImmediateGreater)
{
  cpu.X = 0x50;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = INS_CPX_IM;
  cpu.Memory[0x1001] = 0x10;

  execute (&cpu);

  EXPECT_TRUE (cpu.P & FLAG_CARRY); // 0x50 > 0x10
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
}

TEST_F (ace64Test, CPXZeroPageEqual)
{
  cpu.X = 0x10;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = INS_CPX_ZP;
  cpu.Memory[0x1001] = 0x80;
  cpu.Memory[0x0080] = 0x10; // Match

  execute (&cpu);

  EXPECT_TRUE (cpu.P & FLAG_CARRY);
  EXPECT_TRUE (cpu.P & FLAG_ZERO); // Result is 0
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
}

TEST_F (ace64Test, CPXAbsolute)
{
  cpu.X = 0x05;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = INS_CPX_ABS;
  cpu.Memory[0x1001] = 0x00;
  cpu.Memory[0x1002] = 0x20;
  cpu.Memory[0x2000] = 0x01; // 0xFF > 0x01

  execute (&cpu);

  EXPECT_TRUE (cpu.P & FLAG_CARRY);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
}

TEST_F (ace64Test, CPYImmediateGreater)
{
  cpu.Y = 0x50;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = INS_CPY_IM;
  cpu.Memory[0x1001] = 0x10;

  execute (&cpu);

  EXPECT_TRUE (cpu.P & FLAG_CARRY); // 0x50 > 0x10
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
}

TEST_F (ace64Test, CPYZeroPageEqual)
{
  cpu.Y = 0x10;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = INS_CPY_ZP;
  cpu.Memory[0x1001] = 0x80;
  cpu.Memory[0x0080] = 0x10; // Match

  execute (&cpu);

  EXPECT_TRUE (cpu.P & FLAG_CARRY);
  EXPECT_TRUE (cpu.P & FLAG_ZERO); // Result is 0
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
}

TEST_F (ace64Test, CPYAbsolute)
{
  cpu.Y = 0x05;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = INS_CPY_ABS;
  cpu.Memory[0x1001] = 0x00;
  cpu.Memory[0x1002] = 0x20;
  cpu.Memory[0x2000] = 0x01; // 0xFF > 0x01

  execute (&cpu);

  EXPECT_TRUE (cpu.P & FLAG_CARRY);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
}

TEST_F (ace64Test, JMPIndirectPageBoundaryBug)
{
  // JMP ($10FF)
  cpu.PC = 0x0000;
  cpu.Memory[0x0000] = 0x6C; // JMP Indirect
  cpu.Memory[0x0001] = 0xFF; // Lo
  cpu.Memory[0x0002] = 0x10; // Hi

  cpu.Memory[0x10FF] = 0xAD; // Target Lo
  cpu.Memory[0x1000] = 0xDE; // Target Hi (This is the bug!)
  // If it worked "correctly", it would look at $1100

  execute (&cpu);

  EXPECT_EQ (cpu.PC, 0xDEAD);
}

TEST_F (ace64Test, JSR_RTS_RoundTrip)
{
  // --- GIVEN ---
  // Program starts at $1000
  // $1000: JSR $2000 (Opcode $20, Lo $00, Hi $20)
  // $1003: LDX #$42   (The instruction we expect to return to)
  // $2000: RTS        (Opcode $60)

  cpu.PC = 0x1000;
  cpu.SP = 0xFF; // Top of stack

  // Main Code
  cpu.Memory[0x1000] = 0x20; // JSR
  cpu.Memory[0x1001] = 0x00;
  cpu.Memory[0x1002] = 0x20;
  cpu.Memory[0x1003] = 0xA2; // LDX #$42 (Opcode)
  cpu.Memory[0x1004] = 0x42; // LDX operand

  // Subroutine Code
  cpu.Memory[0x2000] = 0x60; // RTS

  // --- WHEN ---

  // 1. Execute JSR
  Sint32 cyclesJSR = execute (&cpu);

  // Check JSR state
  EXPECT_EQ (cpu.PC, 0x2000);
  EXPECT_EQ (cpu.SP, 0xFD); // Pushed two bytes
  EXPECT_EQ (cpu.Memory[0x01FF],
             0x10); // High byte of last byte of JSR ($1002)
  EXPECT_EQ (cpu.Memory[0x01FE], 0x02); // Low byte of last byte of JSR ($1002)
  EXPECT_EQ (cyclesJSR, 6);

  // 2. Execute RTS
  Sint32 cyclesRTS = execute (&cpu);

  // --- THEN ---

  // 1. PC should be at $1003 (The instruction after JSR)
  EXPECT_EQ (cpu.PC, 0x1003);

  // 2. Stack Pointer should be back to $FF
  EXPECT_EQ (cpu.SP, 0xFF);

  // 3. Timing Check
  EXPECT_EQ (cyclesRTS, 6);

  // 4. Verify we can continue executing
  execute (&cpu); // Executes LDX #$42
  EXPECT_EQ (cpu.X, 0x42);
}

TEST_F (ace64Test, RTIReturnTest)
{
  // --- GIVEN ---
  // We want to return to $4000 with Negative and Carry flags set.
  cpu.SP = 0xFC;
  cpu.Memory[0x01FD] = FLAG_NEGATIVE | FLAG_CARRY; // Status (P) at $01FD
  cpu.Memory[0x01FE] = 0x00;                       // PCL at $01FE
  cpu.Memory[0x01FF] = 0x40;                       // PCH at $01FF

  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x40; // RTI Opcode

  // --- WHEN ---
  Sint32 cyclesUsed = execute (&cpu);

  // --- THEN ---
  // 1. PC should be exactly $4000
  EXPECT_EQ (cpu.PC, 0x4000);

  // 2. Flags should be restored (including the mandatory bit 5)
  EXPECT_EQ (cpu.P, FLAG_NEGATIVE | FLAG_CARRY | 0x20);

  // 3. Stack Pointer should be back to $FF
  EXPECT_EQ (cpu.SP, 0xFF);

  // 4. Timing: RTI is 6 cycles
  EXPECT_EQ (cyclesUsed, 6);
}

TEST_F (ace64Test, ADC_Simple)
{
  // Case A: 10 + 10 = 20
  cpu.A = 10;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x69; // ADC Immediate
  cpu.Memory[0x1001] = 10;
  cpu.P = 0; // Carry is 0

  execute (&cpu);
  EXPECT_EQ (cpu.A, 20);
  EXPECT_FALSE (cpu.P & FLAG_CARRY);
}

TEST_F (ace64Test, ADC_Carry)
{

  // Case B: 255 + 1 = 0 (Carry set)
  cpu.PC = 0x1000;
  cpu.A = 0xFF;
  cpu.Memory[0x1000] = 0x69;
  cpu.Memory[0x1001] = 0x01;

  execute (&cpu);

  EXPECT_EQ (cpu.A, 0x00);
  EXPECT_TRUE (cpu.P & FLAG_CARRY);
  EXPECT_TRUE (cpu.P & FLAG_ZERO);
}

TEST_F (ace64Test, ADC_OverflowPositive)
{
  cpu.A = 0x40; // 64
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x69;
  cpu.Memory[0x1001] = 0x40; // 64
  cpu.P = 0;

  execute (&cpu);

  EXPECT_EQ (cpu.A, 0x80);
  EXPECT_TRUE (cpu.P & FLAG_OVERFLOW); // V set because result sign flipped
  EXPECT_TRUE (cpu.P & FLAG_NEGATIVE); // Result bit 7 is 1
  EXPECT_FALSE (cpu.P & FLAG_CARRY);   // No unsigned carry
}

TEST_F (ace64Test, ADC_OverflowNegative)
{
  cpu.A = 0x80; // -128
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x69;
  cpu.Memory[0x1001] = 0xFF; // -1
  cpu.P = 0;

  execute (&cpu);

  EXPECT_EQ (cpu.A, 0x7F);              // Result wrapped
  EXPECT_TRUE (cpu.P & FLAG_OVERFLOW);  // V set
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE); // Result bit 7 is 0
  EXPECT_TRUE (cpu.P & FLAG_CARRY);     // Unsigned carry occurred
}

TEST_F (ace64Test, ADC_WithCarryIn)
{
  cpu.A = 10;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x69;
  cpu.Memory[0x1001] = 10;
  cpu.P = 0; // Initial carry is 1
  set_flag (&cpu.P, FLAG_CARRY);

  execute (&cpu);
  EXPECT_EQ (cpu.A, 21);
}

TEST_F (ace64Test, ADCDecimalBasic)
{
  // Case A: 9 + 1 = 10 (BCD)
  cpu.P = FLAG_DECIMAL_MODE; // Carry is 0
  cpu.A = 0x09;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x69; // ADC Immediate
  cpu.Memory[0x1001] = 0x01;

  execute (&cpu);
  EXPECT_EQ (cpu.A, 0x10);
  EXPECT_FALSE (cpu.P & FLAG_CARRY);

  // Case B: 99 + 1 = 00 (Carry 1)
  cpu.PC = 0x1000;
  cpu.A = 0x99;
  cpu.Memory[0x1001] = 0x01;
  execute (&cpu);
  EXPECT_EQ (cpu.A, 0x00);
  EXPECT_TRUE (cpu.P & FLAG_CARRY);
}

TEST_F (ace64Test, SBCDecimalBasic)
{
  // Case A: 10 - 1 = 09
  cpu.P = FLAG_DECIMAL_MODE | FLAG_CARRY; // C=1 means NO Borrow
  cpu.A = 0x10;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0xE9; // SBC Immediate
  cpu.Memory[0x1001] = 0x01;

  execute (&cpu);
  EXPECT_EQ (cpu.A, 0x09);
  EXPECT_TRUE (cpu.P & FLAG_CARRY); // No borrow occurred

  // Case B: 00 - 1 = 99 (Borrow)
  cpu.PC = 0x1000;
  cpu.A = 0x00;
  cpu.Memory[0x1001] = 0x01;
  cpu.P = FLAG_DECIMAL_MODE | FLAG_CARRY;

  execute (&cpu);
  EXPECT_EQ (cpu.A, 0x99);
  EXPECT_FALSE (cpu.P & FLAG_CARRY); // Borrow occurred
}

TEST_F (ace64Test, ADCDecimalFlagQuirk)
{
  // Math: $50 + $50 = $100 (BCD)
  // Binary Math: $50 + $50 = $A0 (Negative bit set)
  cpu.P = FLAG_DECIMAL_MODE;
  cpu.A = 0x50;
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x69;
  cpu.Memory[0x1001] = 0x50;

  execute (&cpu);

  EXPECT_EQ (cpu.A, 0x00); // BCD result $50+$50 = $00 (carry 1)
  EXPECT_TRUE (cpu.P & FLAG_CARRY);
  EXPECT_TRUE (cpu.P
               & FLAG_OVERFLOW); // Binary $50+$50 = $A0 (Overflow from + to -)
  EXPECT_TRUE (cpu.P & FLAG_NEGATIVE); // Binary result $A0 has bit 7 set
}

TEST_F (ace64Test, BRKAndRTIRoundTrip)
{
  // given
  // 1. Setup the Vector ($FFFE/$FFFF points to our "handler" at $9000)
  cpu.Memory[0xFFFE] = 0x00; // Lo
  cpu.Memory[0xFFFF] = 0x90; // Hi

  // 2. Setup the Main Program at $1000
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0x00; // BRK
  cpu.Memory[0x1001] = 0xEA; // NOP (The "padding" byte BRK skips)
  cpu.Memory[0x1002] = 0x38; // SEC (The instruction we want to return to)

  // 3. Setup the Interrupt Handler at $9000
  cpu.Memory[0x9000] = 0x40; // RTI

  // cpu.SP = 0xFF;
  // cpu.P = 0x00; // Clear flags

  // when
  Sint32 cyclesBRK = execute (&cpu);

  // Then
  // Verify BRK results
  EXPECT_EQ (cpu.PC, 0x9000);                   // Jumped to handler
  EXPECT_EQ (cpu.SP, 0xFC);                     // Pushed 3 bytes (PCH, PCL, P)
  EXPECT_TRUE (cpu.P & FLAG_INTERRUPT_DISABLE); // I flag set automatically
  EXPECT_EQ (cyclesBRK, 7);                     // BRK is exactly 7 cycles

  // Verify Stack contents (PC+2 was pushed: $1002)
  EXPECT_EQ (cpu.Memory[0x01FF], 0x10); // PCH
  EXPECT_EQ (cpu.Memory[0x01FE], 0x02); // PCL
  // P on stack should have Break bit ($10) and Unused bit ($20) set
  EXPECT_TRUE (cpu.Memory[0x01FD] & 0x30);

  // --- EXECUTE RTI ---
  Sint32 cyclesRTI = execute (&cpu);

  // Verify RTI results
  EXPECT_EQ (cpu.PC, 0x1002); // Returned to SEC instruction
  EXPECT_EQ (cpu.SP, 0xFF);   // Stack cleared
  EXPECT_EQ (cyclesRTI, 6);   // RTI is exactly 6 cycles
}

TEST_F (ace64Test, RegisterIncDecTests)
{
  // 1. Test INX ($E8): 0x00 -> 0x01
  cpu.PC = 0x1000;
  cpu.Memory[0x1000] = 0xE8;
  cpu.X = 0x00;
  execute (&cpu);
  EXPECT_EQ (cpu.X, 0x01);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);

  // 2. Test DEX ($CA): 0x01 -> 0x00 (Verify Zero Flag)
  cpu.Memory[cpu.PC] = 0xCA;
  execute (&cpu);
  EXPECT_EQ (cpu.X, 0x00);
  EXPECT_TRUE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);

  // 3. Test INY ($C8): 0xFF -> 0x00 (Verify Overflow Wrap & Zero Flag)
  cpu.Memory[cpu.PC] = 0xC8;
  cpu.Y = 0xFF;
  execute (&cpu);
  EXPECT_EQ (cpu.Y, 0x00);
  EXPECT_TRUE (cpu.P & FLAG_ZERO);
  EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);

  // 4. Test DEY ($88): 0x00 -> 0xFF (Verify Underflow Wrap & Negative Flag)
  cpu.Memory[cpu.PC] = 0x88;
  cpu.Y = 0x00;
  execute (&cpu);
  EXPECT_EQ (cpu.Y, 0xFF);
  EXPECT_FALSE (cpu.P & FLAG_ZERO);
  EXPECT_TRUE (cpu.P & FLAG_NEGATIVE); // 0xFF has bit 7 set
}

TEST_F (ace64Test, AllBranchOpcodesLogic)
{
  // Utility values
  const Word startPC = 0x2000;
  const Byte offset = 0x05;                   // Branch forward 5 bytes
  const Word targetPC = startPC + 2 + offset; // PC after fetch + offset

  struct BranchTest
  {
    Byte opcode;
    Byte flag;
    bool shouldBranchIfSet;
    std::string name;
  };

  std::vector<BranchTest> tests = {
    { 0x90, FLAG_CARRY, false, "BCC" },    // Branch if Carry Clear
    { 0xB0, FLAG_CARRY, true, "BCS" },     // Branch if Carry Set
    { 0xD0, FLAG_ZERO, false, "BNE" },     // Branch if Zero Clear
    { 0xF0, FLAG_ZERO, true, "BEQ" },      // Branch if Zero Set
    { 0x10, FLAG_NEGATIVE, false, "BPL" }, // Branch if Negative Clear (Plus)
    { 0x30, FLAG_NEGATIVE, true, "BMI" },  // Branch if Negative Set (Minus)
    { 0x50, FLAG_OVERFLOW, false, "BVC" }, // Branch if Overflow Clear
    { 0x70, FLAG_OVERFLOW, true, "BVS" }   // Branch if Overflow Set
  };

  for (auto &t : tests)
    {
      // --- Test 1: Branch Taken ---
      cpu.PC = startPC;
      cpu.Memory[startPC] = t.opcode;
      cpu.Memory[startPC + 1] = offset;
      cpu.P = t.shouldBranchIfSet ? t.flag : 0x00; // Setup P to trigger branch

      Sint32 cyclesTaken = execute (&cpu);
      EXPECT_EQ (cpu.PC, targetPC) << "Failed Taken logic for " << t.name;
      EXPECT_EQ (cyclesTaken, 3) << "Failed Taken timing for " << t.name;

      // --- Test 2: Branch NOT Taken ---
      cpu.PC = startPC;
      cpu.P = t.shouldBranchIfSet ? 0x00 : t.flag; // Setup P to FAIL branch

      Sint32 cyclesNotTaken = execute (&cpu);
      EXPECT_EQ (cpu.PC, startPC + 2)
          << "Failed Not Taken logic for " << t.name;
      EXPECT_EQ (cyclesNotTaken, 2)
          << "Failed Not Taken timing for " << t.name;
    }
}

TEST_F (ace64Test, LAXAllAddressingModes)
{
  struct LAXTest
  {
    Byte opcode;
    Word address;
    Byte value;
    Sint32 expectedCycles;
    std::string name;
    bool isImmediate;
  };

  // Note: $B7 and $B3 are unique because they use Y for indexing
  // where almost all other 'X' register instructions use X.
  std::vector<LAXTest> tests = {
    { 0xA7, 0x0080, 0x42, 3, "Zero Page", false },
    { 0xB7, 0x0080, 0x42, 4, "Zero Page, Y", false }, // Note: Uses Y!
    { 0xAF, 0x2000, 0x42, 4, "Absolute", false },
    { 0xBF, 0x2000, 0x42, 4, "Absolute, Y", false },
    { 0xA3, 0x0080, 0x42, 6, "Indexed Indirect", false },
    { 0xB3, 0x0080, 0x42, 5, "Indirect Indexed", false },
    { 0xAB, 0x0000, 0x42, 2, "Immediate", true } // Magic logic applied
  };

  for (auto &t : tests)
    {
      // --- Reset CPU State ---
      reset (&cpu);
      cpu.PC = 0x1000;
      cpu.A = 0x00;
      cpu.X = 0x00;
      cpu.Y = 0x05; // Offset for indexed modes
      cpu.Memory[0x1000] = t.opcode;

      // Setup addressing based on mode
      if (t.isImmediate)
        {
          cpu.Memory[0x1001] = t.value;
          // Setup A for the Magic Logic test: A = (A | 0xEE) & Imm
          cpu.A = 0x11;
        }
      else if (t.name == "Indexed Indirect")
        {
          cpu.X = 0x05; // Must use X for $A3
          Byte pointer = 0x20;
          cpu.Memory[0x1001] = pointer; // Operand is the pointer address

          // The "Pointer" in Zero Page points to $2000
          cpu.Memory[(pointer + cpu.X) & 0xFF] = 0x00;     // Lo
          cpu.Memory[(pointer + cpu.X + 1) & 0xFF] = 0x20; // Hi

          // The actual value is at $2000
          cpu.Memory[0x2000] = t.value;
        }
      else if (t.name == "Indirect Indexed")
        {
          cpu.Y = 0x05; // Must use Y for $B3
          Byte pointer = 0x20;
          cpu.Memory[0x1001] = pointer;

          // The "Pointer" in Zero Page points to $2000
          cpu.Memory[pointer] = 0x00;     // Lo
          cpu.Memory[pointer + 1] = 0x20; // Hi

          // Target = $2000 + Y = $2005
          cpu.Memory[0x2000 + cpu.Y] = t.value;
        }
      else
        {
          // Setup memory at target
          cpu.Memory[t.address
                     + (t.name.find (", Y") != std::string::npos ? cpu.Y : 0)]
              = t.value;
          // Simplified: putting operand for address fetch
          cpu.Memory[0x1001] = (t.address & 0xFF);
          cpu.Memory[0x1002] = (t.address >> 8);
        }

      // --- Execute ---
      Sint32 cycles = execute (&cpu);

      // --- Assertions ---
      if (t.isImmediate)
        {
          // Logic: A = X = (0x11 | 0xEE) & 0x42
          // 0x11 (0001 0001) | 0xEE (1110 1110) = 0xFF
          // 0xFF & 0x42 = 0x42
          EXPECT_EQ (cpu.A, 0x42) << "Failed logic for " << t.name;
          EXPECT_EQ (cpu.X, 0x42) << "Failed logic for " << t.name;
        }
      else
        {
          EXPECT_EQ (cpu.A, t.value) << "Failed value A for " << t.name;
          EXPECT_EQ (cpu.X, t.value) << "Failed value X for " << t.name;
        }
      EXPECT_EQ (cycles, t.expectedCycles) << "Failed timing for " << t.name;
      EXPECT_FALSE (cpu.P & FLAG_ZERO);
      EXPECT_FALSE (cpu.P & FLAG_NEGATIVE);
    }
}
