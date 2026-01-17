#include "cpu.h"
#include <malloc.h>
#include <stdio.h>

int
main (int argc, char *argv[])
{
  CPU *cpu = (CPU *)malloc (sizeof (CPU));
  reset (cpu);

  printf (
      "CPU Initialized\nP = %x, PC = %x, SP = %x, Y = %x, X = %x, A = %x\n",
      cpu->P, cpu->PC, cpu->SP, cpu->Y, cpu->X, cpu->A);
  printf ("Data - first 2 bytes: %X, %X\n", cpu->Memory[0x00],
          cpu->Memory[0x01]);
  Sint32 Cycles = 0;

  cpu->Y = 0x02;
  cpu->X = 0x37;
  cpu->Memory[0xFFFC] = 0x96;
  cpu->Memory[0xFFFD] = 0x04;
  cpu->Memory[0x0006] = 0x00;

  Cycles = execute (cpu);

  printf("Cycles used: %d, Value at 0x0006: %d\n", Cycles, cpu->Memory[0x0006]);
  free (cpu);
  return (0);
}
