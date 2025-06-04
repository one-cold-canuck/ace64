#include "cpu.h"
#include <malloc.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  Memory *memory = (Memory *)malloc(sizeof(Memory));
  CPU *cpu = (CPU *)malloc(sizeof(CPU));
  cpu->Memory = memory;
  Reset(cpu);

  printf("CPU Initialized\nP = %x, PC = %x, SP = %x, Y = %x, X = %x, A = %x\n",
         cpu->P, cpu->PC, cpu->SP, cpu->Y, cpu->X, cpu->A);
  printf("Data - first 2 bytes: %X, %X\n", memory->Data[0x00], memory->Data[0x01]);
  Sint32 Cycles = 0;

  WriteWord(memory, 0x691F, 0x3C, &Cycles);
  printf("Cycles consumed: %d\n", Cycles);
  printf("Value in [0x3B] [0x3C] = %X %X\n", memory->Data[0x3B], memory->Data[0x3C]);

  memory->Data[0xFFFC] = 0x88;
  Byte Data = FetchByte(memory, &Cycles);

  printf("Data in PC's address: %x\nPC Current Value: %x\n", Data, cpu->PC);
  free(memory);
  free(cpu);
  return (0);
}
