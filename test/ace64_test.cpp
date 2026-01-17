#include "../code/cpu.c"
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
  constexpr Sint32 EXPECTED_CYCLES = 4;
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
  constexpr Sint32 EXPECTED_CYCLES = 4;
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
  constexpr Sint32 EXPECTED_CYCLES = 5;
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
  CPU cpuCopy = cpu;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.X, 0x77);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
  EXPECT_EQ (cpu.Memory[0x4480], 0x77);
  VerifyUnmodifiedFlags (cpu, cpuCopy);
}

TEST_F (ace64Test, STX_ZeroPoint_Indexed_Address_Contains_Value){
  //
  // given:
  cpu.Y = 0x02;
  cpu.X = 0x37;
  cpu.Memory[0xFFFC] = INS_STX_ZPY;
  cpu.Memory[0xFFFD] = 0x04;
  cpu.Memory[0x0006] = 0x00;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 4;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  // then:
  EXPECT_EQ (cpu.Memory[0x0006], 0x37);
  EXPECT_EQ (CyclesUsed, EXPECTED_CYCLES);
}

TEST_F (ace64Test, STY_ZerPoint_Address_Contains_Value){
  
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
  cpu.P |= FLAG_OVERFLOW | FLAG_BREAK | FLAG_CARRY;

  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 3;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.Memory[0x01FF], FLAG_OVERFLOW | FLAG_BREAK | FLAG_CARRY);
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
  cpu.SP = 0xFC;
  cpu.Memory[0xFFFC] = INS_PLP;
  cpu.Memory[0x01FD] = FLAG_OVERFLOW | FLAG_BREAK | FLAG_CARRY;
  CPU cpuCopy = cpu;
  constexpr Sint32 EXPECTED_CYCLES = 3;

  // when:
  Sint32 CyclesUsed = execute (&cpu);

  EXPECT_EQ (cpu.P, FLAG_OVERFLOW | FLAG_BREAK | FLAG_CARRY);
  EXPECT_EQ (cpu.SP, 0xFD);
  EXPECT_EQ (0x0100 + cpu.SP, 0x01FD);
}
