#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "mmu.h"
#include "cpu.h"
#include "gpu.h"

typedef uint8_t byte;
typedef int8_t sbyte;
typedef uint16_t word;
typedef int16_t sword;

//init.c 
void init(char *);

//cpu.c 
int execute_instruction(uint8_t, uint8_t, uint8_t, uint16_t);
void cpu_update(void);
void request_interrupt(int);
byte testbit(byte, int);

//mmu.c 
byte readbyte(word);
word readword(word);
void writebyte(word, byte);
byte readmem(word);
void reset(void);

//gpu.c 
void update_graphics(int);