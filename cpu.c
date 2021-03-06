#include "inc.h"
// NOT DONE: 0x10, stop 0  0x76, halt 

static void Unimplemented(byte);
static int execute_extended_instruction(void);
static void reset_bit(byte *, int);
static void set_bit(byte *, int);
static int half_carry_add(byte, byte);
static int half_carry_sub(byte, byte);
static void push(word);
static word pop(void);
static void check_interrupts(void);
static void service_interrupt(int);
static void set_zero(void) {
	cpu.flagz = 1;
}

byte n;
int in0233 = 0;
int innum = 0;
int opfound = 0;
int imp = 0;
int instructions[100];
uint8_t o, z, a;
uint16_t pc;
int zero;

void check(byte instr) {
	int found = 0;
	for (int i=0; i<imp; i++) {
		if (instr == instructions[i]) {
			found = 1;
			break;
		}
	}
	if (!found) {
		instructions[imp++] = instr;
	}
}

void cpu_update(void) {
	int cycle_count = 0;
	int cycles;
	
	while (cycle_count < MAX_CYCLES) {
			
		//++innum;
		//check(readmem(cpu->pc));
		o = readmem(cpu.pc);
		pc = cpu.pc;
		z = cpu.flagz;
		a = cpu.a;
		if (a == 0x94)
			cpu.flagz = 1;
		
		/*
		// print states whilst stuck in loop */
		
		if ((o == 0xf0 || o == 0xfe || o == 0x20) && (pc == 0x233 || pc == 0x235 || pc == 0x237)) {
			printf("%d\n", innum);
			printf("\ninside loop\n");
			printf("%.4x: %.2x\n", cpu.pc, readmem(cpu.pc));
			printf("%x\n", cpu.a);
			printf("zero flag: %d\n", cpu.flagz);
			
		}

		cycles = execute_instruction(a,o,z,pc);
		cycle_count += cycles;
		// update_graphics doesn't touch any of the flags 
		update_graphics(cycles);
		
		// we're stuck in infinite loop and interrupt
		// switch is off, so this is never accessed 
		if (cpu.interrupt_switch == true) {
			check_interrupts();
		}

	}
	//printf("limit reached \n");
}

void request_interrupt(int id) {
	byte required = readmem(0xff0f);
	set_bit(&required, id);
	writebyte(0xff0f, id);
}

static void check_interrupts(void) {
	byte required_ints = readmem(0xff0f);
	byte enabled_ints = readmem(0xffff);
	if (required_ints > 0) {
		for (int i=0; i<5; i++) {
			if (testbit(required_ints, i)) {
				if (testbit(enabled_ints, i)) {
					service_interrupt(i);
				}
			}
		}
	}
}

static void service_interrupt(int interrupt) {
	// clear interrupt 
	cpu.interrupt_switch = false;
	byte required_int = readmem(0xff0f);
	reset_bit(&required_int, interrupt);
	writebyte(0xff0f, required_int);
	
	// save pc 
	push(cpu.pc);
	
	// jump to correct place 
	switch (interrupt) {
		// vblank 
		case 0: cpu.pc = 0x40; printf("vram\n"); break;
		// lcd 
		case 1: cpu.pc = 0x48; break;
		//timer
		case 2: cpu.pc = 0x50; break;
		// joypad 
		case 3: cpu.pc = 0x60; break;
	}
}

int execute_instruction(uint8_t a, uint8_t o, uint8_t z, uint16_t pc) {
	byte opcode = readbyte(cpu.pc);
	
	switch (opcode) {
		
		// 8 bit load/store/move
		case 0x02: {
			word bc = (cpu.b << 8) | cpu.c;
			writebyte(bc, cpu.a);
			return 8;
		}
		case 0x06: {
			// ld b, d8 
			cpu.b = readbyte(cpu.pc);
			return 4;
		}
		case 0x0e: {
			//ld c, d8 
			cpu.c = readbyte(cpu.pc);
			return 8;
		}
		case 0x12: {
			// ld (de), a 
			word de = (cpu.d << 8) | cpu.e;
			writebyte(de, cpu.a);
			return 8;
		}
		case 0x16: {
			// ld d, d8 
			cpu.d = readbyte(cpu.pc);
			return 8;
		}
		case 0x1a: {
			// ld a, (de)
			cpu.a = readmem((cpu.d << 8) | cpu.e);
			return 8;
		}
		case 0x1e: {
			// ld e, d8 
			cpu.e = readbyte(cpu.pc);
			return 8;
		}
		case 0x22: {
			// ld (hl+), a
			word hl = (cpu.h << 8) | cpu.l;
			writebyte(hl, cpu.a);
			++hl;
			cpu.h = (hl >> 8) & 0xff;
			cpu.l = hl & 0xff;
			return 8;
		}
		case 0x26: {
			// ld h, d8 
			cpu.h = readbyte(cpu.pc);
			return 8;
		}
		case 0x2a: {
			// ld a, (hl+)
			word hl = (cpu.h << 8) | cpu.l;
			cpu.a = readmem(hl);
			++hl;
			cpu.h = (hl >> 8) & 0xff;
			cpu.l = hl  & 0xff;
			return 8;
		}
		case 0x2e: {
			// ld l, d8 
			cpu.l = readbyte(cpu.pc);
			return 8;
		}
		case 0x32: {
			// ld (hl-), a 
			word hl = (cpu.h << 8) | cpu.l;
			writebyte(hl, cpu.a);
			--hl;
			cpu.h = (hl >> 8) & 0xff;
			cpu.l = hl & 0xff;
			return 8;
		}
		case 0x36: {
			// ld (hl), d8 
			word hl = (cpu.h << 8) | cpu.l;
			byte n = readbyte(cpu.pc);
			writebyte(hl, n);
			return 12;
		}
		case 0x3e: {
			// ld a, d8
			cpu.a = readbyte(cpu.pc);
			//printf("%.2x\n", cpu.a);
			return 8;
		}
		case 0x40: {
			// ld b, b 
			return 4;
		}
		case 0x41: {
			// ld b, c
			cpu.b = cpu.c;
			return 4;
		}
		case 0x42: {
			// ld b, d 
			cpu.b = cpu.d;
			return 4;
		}
		case 0x43: {
			// ld b, e 
			cpu.b = cpu.e;
			return 4;
		}
		case 0x44: {
			// ld b, h 
			cpu.b = cpu.h;
			return 4;
		}
		case 0x45: {
			// ld b, l 
			cpu.b = cpu.l;
			return 4;
		}
		case 0x46: {
			// ld b, (hl)
			word hl = (cpu.h << 8) | cpu.l;
			cpu.b = readmem(hl);
			return 8;
		}
		case 0x4f: {
			// ld c, a
			cpu.c = cpu.a;
			return 4;
		}
		case 0x50: {
			// ld d, b 
			cpu.d = cpu.b;
			return 4;
		}
		case 0x51: {
			// ld d, c 
			cpu.d = cpu.c;
			return 4;
		}
		case 0x52: {
			// ld d, d 
			return 4;
		}
		case 0x53: {
			// ld d, e 
			cpu.d = cpu.e;
			return 4;
		}
		case 0x54: {
			// ld d, h 
			cpu.d = cpu.h;
			return 4;
		}
		case 0x55: {
			// ld d, l 
			cpu.d = cpu.l;
			return 4;
		}
		case 0x56: {
			// ld d, (hl)
			word hl = (cpu.h << 8) | cpu.l;
			cpu.d = readmem(hl);
			return 8;
		}
		case 0x57: {
			// ld d, a 
			cpu.d = cpu.a;
			return 4;
		}
		case 0x60: {
			// ld h, b 
			cpu.h = cpu.b;
			return 4;
		}
		case 0x61: {
			// ld h, c 
			cpu.h = cpu.c;
			return 4;
		}
		case 0x62: {
			// ld h, d 
			cpu.h = cpu.d;
			return 4;
		}
		case 0x63: {
			// ld h, e 
			cpu.h = cpu.e;
			return 4;
		}
		case 0x64: {
			// ld h, h 
			return 4;
		}
		case 0x65: {
			// ld h, l 
			cpu.h = cpu.l;
			return 4;
		}
		case 0x66: {
			// ld h, (hl)
			word hl = (cpu.h << 8) | cpu.l;
			cpu.h = readmem(hl);
			return 8;
		}
		case 0x67: {
			// ld h, a 
			cpu.h = cpu.a;
			return 4;
		}
		case 0x70: {
			// ld (hl), b 
			word hl = (cpu.h << 8) | cpu.l;
			writebyte(hl, cpu.b);
			return 8;
		}
		case 0x71: {
			// ld (hl), c 
			word hl = (cpu.h << 8) | cpu.l;
			writebyte(hl, cpu.c);
			return 8;
		}
		case 0x72: {
			// ld (hl), d 
			word hl = (cpu.h << 8) | cpu.l;
			writebyte(hl, cpu.d);
			return 8;
		}
		case 0x73: {
			// ld (hl), e 
			word hl = (cpu.h << 8) | cpu.l;
			writebyte(hl, cpu.e);
			return 8;
		}
		case 0x74: {
			// ld (hl), h 
			word hl = (cpu.h << 8) | cpu.l;
			writebyte(hl, cpu.h);
			return 8;
		}
		case 0x75: {
			// ld (hl), l 
			word hl = (cpu.h << 8) | cpu.l;
			writebyte(hl, cpu.l);
			return 8;
		}
		case 0x77: {
			// ld (hl), a 
			writebyte(((cpu.h << 8) | cpu.l), cpu.a);
			return 8;
		}
		case 0x78: {
			//ld a, b 
			cpu.a = cpu.b;
			return 4;
		}
		case 0x7b: {
			// ld a, e 
			cpu.a = cpu.e;
			return 4;
		}
		case 0x7c: {
			// ld a, h 
			cpu.a = cpu.h;
			return 4;
		}
		case 0x7d: {
			// ld a, l 
			cpu.a = cpu.l;
			return 4;
		}
		case 0xe0: {
			// ld (a8+0xff00), a 
			byte n = readbyte(cpu.pc);
			writebyte(0xff00+n, cpu.a);
			//printf("%x\n", cpu.pc);
			return 12;
		}
		case 0xe2: {
			// ld (c+0xff00), a 
			writebyte(0xff00+cpu.c, cpu.a);
			return 8;
		}
		case 0xea: {
			// ld (a16), a 
			word addr = readword(cpu.pc);
			writebyte(addr, cpu.a);
			return 16;
		}
		case 0xf0: {
			// ld 
			byte n = readbyte(cpu.pc);
			byte data = readmem(0xff00 + n);
			cpu.a = data;
			return 12;
		}
		case 0xf2: {
			// ld a, (c)
			cpu.a = readmem(0xff00 + cpu.c);
			return 8;
		}
		case 0xfa: {
			// ld a, (a16)
			word addr = readword(cpu.pc);
			cpu.a = readmem(addr);
			return 16;
		}
		
		/////////////*************\\\\\\\\\\\\
		/* 8 bit arithmetic/logical */
		case 0x04: {
			// inc b 
			int res = cpu.b + 1;
			cpu.flagz = (res == 0);
			cpu.flagn = 0;
			cpu.flagh = half_carry_add(cpu.b, 1);
			cpu.b = res & 0xff;
			return 4;
		}
		case 0x05: {
			// dec b 
			int res = cpu.b - 1;
			cpu.flagz = (res == 0);
			cpu.flagn = 1;
			cpu.flagh = half_carry_sub(cpu.b, 1);
			cpu.b = res & 0xff;
			//printf("%x \n",cpu.b);
			return 4;
		}
		case 0x07: {
			// rlca 
			cpu.flagc = (cpu.a >> 7) & 1;
			cpu.a = (cpu.a << 1) & 0xff;
			cpu.flagz = (cpu.a == 0);
			cpu.flagn = cpu.flagh = 0;
			return 4;
		}
		case 0x0c: {
			// inc c 
			sword res = cpu.c + 1;
			cpu.flagz = (res == 0);
			cpu.flagn = 0;
			cpu.flagh = half_carry_add(cpu.c, 1);
			cpu.c = res & 0xff;
			return 4;
		}
		case 0x0d: {
			// dec c 
			sword res = cpu.c - 1;
			cpu.flagz = (res == 0);
			cpu.flagn = 1;
			cpu.flagh = half_carry_sub(cpu.c, 1);
			cpu.c = res & 0xff;
			return 4;
		}
		case 0x2f: {
			//cpl 
			cpu.a = ~(cpu.a);
			cpu.flagn =1 ;
			cpu.flagh = 1;
			return 4;
		}
		case 0x14: {
			// inc d 
			int res = cpu.d + 1;
			cpu.flagz = (res == 0);
			cpu.flagn = 0;
			cpu.flagh = half_carry_add(cpu.d, 1);
			cpu.d = res & 0xff;
			return 4;
		}
		case 0x15: {
			// dec d 
			sword res = cpu.d - 1;
			cpu.flagz = (res == 0);
			cpu.flagn = 1;
			cpu.flagh = half_carry_sub(cpu.d, 1);
			cpu.d = res & 0xff;
			return 4;
		}
		case 0x17: {
			// rla
			byte msb = (cpu.a >> 7) & 1;
			cpu.a = (cpu.a << 1) & 0xff;
			cpu.a |= cpu.flagc;
			cpu.flagc = msb;
			cpu.flagz = cpu.flagn = cpu.flagh = 0;
			return 4;
		}
		case 0x1c: {
			// inc e 
			int res = cpu.e + 1;
			cpu.flagz = (res == 0);
			cpu.flagn = 0;
			cpu.flagh = half_carry_add(cpu.e, 1);
			cpu.e = res & 0xff;
			return 4;
		}
		case 0x1d: {
			// dec e 
			int res = cpu.e - 1;
			cpu.flagz = (res == 0);
			cpu.flagn = 1;
			cpu.flagh = half_carry_sub(cpu.e, 1);
			cpu.e = res & 0xff;
			return 4;
		}
		case 0x24: {
			// inc h 
			int res = cpu.h + 1;
			cpu.flagz = (res == 0);
			cpu.flagn = 0;
			cpu.flagh = half_carry_add(cpu.h, 1);
			cpu.h = res & 0xff;
			return 4;
		}
		case 0x25: {
			// dec h 
			int res = cpu.h - 1;
			cpu.flagz = (res == 0);
			cpu.flagn = 1;
			cpu.flagh = half_carry_sub(cpu.h, 1);
			cpu.h = res & 0xff;
			return 4;
		}
		case 0x34: {
			// inc (hl)
			word hl = (cpu.h << 8) | cpu.l;
			byte data = readmem(hl);
			++data;
			writebyte(hl, data);
			cpu.flagz = (data == 0);
			cpu.flagn = 0;
			cpu.flagh = half_carry_add(data-1, 1);
			return 12;
		}
		case 0x35: {
			word hl = (cpu.h << 8) | cpu.l;
			byte data = readmem(hl);
			--data;
			writebyte(hl, data);
			cpu.flagz = (data == 0);
			cpu.flagn = 1;
			cpu.flagh = half_carry_sub(data+1, 1);
			return 12;
		}
		case 0x37: {
			// scf 
			cpu.flagc = 1;
			cpu.flagn = cpu.flagh = 0;
			return 4;
		}
		case 0x3d: {
			// dec a 
			int res = cpu.a - 1;
			cpu.flagz = (res == 0);
			cpu.flagn = 1;
			cpu.flagh = half_carry_sub(cpu.a, 1);
			cpu.a = res & 0xff;
			return 4;
		}
		case 0x80: {
			// add a, b 
			int res = cpu.a + cpu.b;
			cpu.flagz = (res == 0);
			cpu.flagn = 0;
			cpu.flagh = half_carry_add(cpu.a, cpu.b);
			cpu.flagc = res > 0xff;
			cpu.a = res & 0xff;
			return 4;
		}
		case 0x81: {
			// add a, c 
			int res = cpu.a + cpu.c;
			cpu.flagz = (res == 0);
			cpu.flagn = 0;
			cpu.flagh = half_carry_add(cpu.a, cpu.c);
			cpu.flagc = res > 0xff;
			cpu.a = res & 0xff;
			return 4;
		}
		case 0x82: {
			// add a, d 
			int res = cpu.a + cpu.d;
			cpu.flagz = (res == 0);
			cpu.flagn = 0;
			cpu.flagh = half_carry_add(cpu.a, cpu.d);
			cpu.flagc = res > 0xff;
			cpu.a = res & 0xff;
			return 4;
		}
		case 0x83: {
			// add a, e 
			int res = cpu.a + cpu.e;
			cpu.flagz = (res == 0);
			cpu.flagn = 0;
			cpu.flagh = half_carry_add(cpu.a, cpu.e);
			cpu.flagc = res > 0xff;
			cpu.a = res & 0xff;
			return 4;
		}
		case 0x84: {
			// add a, h 
			int res = cpu.a + cpu.h;
			cpu.flagz = (res == 0);
			cpu.flagn = 0;
			cpu.flagh = half_carry_add(cpu.a, cpu.h);
			cpu.flagc = res > 0xff;
			cpu.a = res & 0xff;
			return 4;
		}
		case 0x85: {
			// add a, l
			int res = cpu.a + cpu.l;
			cpu.flagz = (res == 0);
			cpu.flagn = 0;
			cpu.flagh = half_carry_add(cpu.a, cpu.l);
			cpu.flagc = res > 0xff;
			cpu.a = res & 0xff;
			return 4;
		}
		case 0x86: {
			// add a, (hl)
			sbyte data = readmem(((cpu.h << 8) | cpu.l));
			int res = cpu.a + data;
			cpu.flagz = (res == 0);
			cpu.flagn = 0;
			cpu.flagh = half_carry_add(cpu.a, data);
			cpu.flagc = (res > 0xff);
			cpu.a = res & 0xff;
			return 8;
		}
		case 0x90: {
			// sub b 
			int res = cpu.a - cpu.b;
			cpu.flagz = (res == 0);
			cpu.flagn = 1;
			cpu.flagh = half_carry_sub(cpu.a, cpu.b);
			cpu.flagc = (cpu.a < cpu.b);
			cpu.a = res & 0xff;
			return 4;
		}
		case 0x91: {
			// sub c 
			int res = cpu.a - cpu.c;
			cpu.flagz = res == 0;
			cpu.flagn = 1;
			cpu.flagh = half_carry_sub(cpu.a, cpu.c);
			cpu.flagc = (cpu.a < cpu.c);
			cpu.a = res & 0xff;
			return 4;
		}
		case 0x92: {
			// sub  d
			int res = cpu.a - cpu.d;
			cpu.flagz = res == 0;
			cpu.flagn = 1;
			cpu.flagh = half_carry_sub(cpu.a, cpu.d);
			cpu.flagc = (cpu.a < cpu.d);
			cpu.a = res & 0xff;
			return 4;
		}
		case 0x93: {
			// sub  e
			int res = cpu.a - cpu.e;
			cpu.flagz = res == 0;
			cpu.flagn = 1;
			cpu.flagh = half_carry_sub(cpu.a, cpu.e);
			cpu.flagc = (cpu.a < cpu.e);
			cpu.a = res & 0xff;
			return 4;
		}
		case 0x94: {
			// sub  h
			int res = cpu.a - cpu.h;
			cpu.flagz = res == 0;
			cpu.flagn = 1;
			cpu.flagh = half_carry_sub(cpu.a, cpu.h);
			cpu.flagc = (cpu.a < cpu.h);
			cpu.a = res & 0xff;
			return 4;
		}
		case 0x95: {
			// sub  l
			int res = cpu.a - cpu.l;
			cpu.flagz = res == 0;
			cpu.flagn = 1;
			cpu.flagh = half_carry_sub(cpu.a, cpu.l);
			cpu.flagc = (cpu.a < cpu.l);
			cpu.a = res & 0xff;
			return 4;
		}
		case 0x96: {
			// sub (hl)
			word hl = (cpu.h << 8) | cpu.l;
			byte data = readmem(hl);
			int res = cpu.a - data;
			cpu.flagz = (res == 0);
			cpu.flagn = 1;
			cpu.flagh = half_carry_sub(cpu.a, data);
			cpu.flagc = (cpu.a < data);
			cpu.a = res & 0xff;
			return 8;
		}
		case 0x9d: {
			// sbc a, l 
			int res = cpu.a - (cpu.l + cpu.flagc);
			cpu.flagz = (res == 0);
			cpu.flagn = 1;
			cpu.flagh = half_carry_sub(cpu.a, cpu.l+cpu.flagc);
			cpu.flagc = (cpu.a < (cpu.l + cpu.flagc));
			return 4;
		}
		case 0xa0: {
			cpu.a &= cpu.b;
			cpu.flagz = (cpu.a == 0);
			cpu.flagn = 0;
			cpu.flagh = 1;
			cpu.flagc = 0;
			return 4;
		}
		case 0xa1: {
			// and c 
			cpu.a &= cpu.c;
			cpu.flagz = (cpu.a == 0);
			cpu.flagn = 0;
			cpu.flagh = 1;
			cpu.flagc = 0;
			return 4;
		}
		case 0xa2: {
			// and d
			cpu.a &= cpu.d;
			cpu.flagz = (cpu.a == 0);
			cpu.flagn = 0;
			cpu.flagh = 1;
			cpu.flagc = 0;
			return 4;
		}
		case 0xa3: {
			// and e
			cpu.a &= cpu.e;
			cpu.flagz = (cpu.a == 0);
			cpu.flagn = 0;
			cpu.flagh = 1;
			cpu.flagc = 0;
			return 4;
		}
		case 0xa4: {
			// and h
			cpu.a &= cpu.h;
			cpu.flagz = (cpu.a == 0);
			cpu.flagn = 0;
			cpu.flagh = 1;
			cpu.flagc = 0;
			return 4;
		}
		case 0xa5: {
			// and l
			cpu.a &= cpu.l;
			cpu.flagz = (cpu.a == 0);
			cpu.flagn = 0;
			cpu.flagh = 1;
			cpu.flagc = 0;
			return 4;
		}
		case 0xa6: {
			// and (hl)
			word hl = (cpu.h << 8) | cpu.l;
			byte data = readmem(hl);
			cpu.a &= data;
			cpu.flagz = (cpu.a == 0);
			cpu.flagh = 1;
			cpu.flagn = cpu.flagc = 0;
			return 8;
		}
		case 0xa7: {
			// and a 
			cpu.a &= cpu.a;
			cpu.flagz = (cpu.a == 0);
			cpu.flagh = 1;
			cpu.flagn = cpu.flagc = 0;
			return 4;
		}
		case 0xaf: {
			// xor a 
			cpu.a ^= cpu.a;
			cpu.flagz = (cpu.a == 0);
			cpu.flagn = 0;
			cpu.flagh = 0;
			cpu.flagc = 0;
			return 4;
		}
		case 0xb0: {
			// or b 
			cpu.a |= cpu.b;
			cpu.flagz = (cpu.a == 0);
			cpu.flagn = cpu.flagh = cpu.flagc = 0;
			return 4;
		}
		case 0xb1: {
			// or c 
			cpu.a |= cpu.c;
			cpu.flagz = (cpu.a == 0);
			cpu.flagn = cpu.flagh = cpu.flagc = 0;
			return 4;
		}
		case 0xb2: {
			// or d
			cpu.a |= cpu.d;
			cpu.flagz = (cpu.a == 0);
			cpu.flagn = cpu.flagh = cpu.flagc = 0;
			return 4;
		}
		case 0xb3: {
			// or e
			cpu.a |= cpu.e;
			cpu.flagz = (cpu.a == 0);
			cpu.flagn = cpu.flagh = cpu.flagc = 0;
			return 4;
		}
		case 0xb4: {
			// or h
			cpu.a |= cpu.h;
			cpu.flagz = (cpu.a == 0);
			cpu.flagn = cpu.flagh = cpu.flagc = 0;
			return 4;
		}
		case 0xb5: {
			// or l
			cpu.a |= cpu.l;
			cpu.flagz = (cpu.a == 0);
			cpu.flagn = cpu.flagh = cpu.flagc = 0;
			return 4;
		}
		case 0xb6: {
			// or (hl)
			byte data = readmem((cpu.h << 8) | cpu.l);
			cpu.a |= data;
			cpu.flagz = (cpu.a == 0);
			cpu.flagn = cpu.flagh = cpu.flagc = 0;
			return 8;
		}
		case 0xbe: {
			// cp (hl)
			word hl = (cpu.h << 8) | cpu.l;
			sbyte data = readmem(hl);
			cpu.flagz = (cpu.a == data);
			cpu.flagn = 1;
			cpu.flagh = half_carry_sub(cpu.a, data);
			cpu.flagc = (cpu.a < data);
			return 8;
		}
		case 0xc6: {
			// add a, d8 
			byte data = readbyte(cpu.pc);
			int res = cpu.a + data;
			cpu.flagz = (res == 0);
			cpu.flagn = 0;
			cpu.flagh = half_carry_add(cpu.a, data);
			cpu.flagc = (res > 0xff);
			cpu.a = res & 0xff;
			return 8;
		}
		case 0xd6: {
			// sub d8 
			byte data = readbyte(cpu.pc);
			int res = cpu.a - data;
			cpu.flagz = (res == 0);
			cpu.flagn = 1;
			cpu.flagh = half_carry_sub(cpu.a, data);
			cpu.flagc = (cpu.a < data);
			cpu.a = res & 0xff;
			return 8;
		}
		case 0xe6: {
			// and d8 
			byte data = readbyte(cpu.pc);
			cpu.a &= data;
			cpu.flagz = (cpu.a == 0);
			cpu.flagh = 1;
			cpu.flagn = cpu.flagc = 0;
			return 8;
		}
		case 0xf6: {
			// or d8 
			byte data = readbyte(cpu.pc);
			cpu.a |= data;
			cpu.flagz = (cpu.a == 0);
			cpu.flagn = cpu.flagh = cpu.flagc = 0;
			return 8;
		}
		case 0xfe: {
			// cp d8 
			n = readbyte(cpu.pc);
			/*
			printf("\ninside switch, case 0xfe\n");
			printf("%x\n", cpu.a == n);*/
			if (cpu.a == n) {
				//printf("\n\n==============MATCH FOUND===========\n\n");
				set_zero();
			} else {
				cpu.flagz = 0;
			}
			//printf("zero: %d\n", cpu.flagz);
			//cpu.flagz = ((cpu.a - n) == 0);
			cpu.flagn = 1;
			cpu.flagh = half_carry_sub(cpu.a, n);
			cpu.flagc = (cpu.a < n);
			return 8;
		}
		
		
		////////***********\\\\\\\\\\\\\\\
		// 16 bit load/store/move instructions
		case 0x01: {
			// ld bc, d16 
			word nn = readword(cpu.pc);
			cpu.b = (nn >> 8) & 0xff;
			cpu.c = nn & 0xff;
			return 12;
		}
		case 0x11: {
			// ld de, d16
			word de = readword(cpu.pc);
			cpu.d = (de >> 8) & 0xff;
			cpu.e = de & 0xff;
			return 12;
		}
		case 0x21: {
			//ld hl, d16 
			word hl = readword(cpu.pc);
			cpu.h = (hl >> 8) & 0xff;
			cpu.l = hl & 0xff;
			return 12;
		}
		case 0x31: {
			// ld sp, d16 
			cpu.sp = readword(cpu.pc);
			return 12;
		}
		case 0xc1: {
			// pop bc 
			word bc = pop();
			cpu.b = (bc >> 8) & 0xff;
			cpu.c = bc & 0xff;
			return 12;
		}
		case 0xc5: {
			// push bc 
			push((cpu.b << 8) | cpu.c);
			return 16;
		}
		case 0xd1: {
			// pop de 
			word de = pop();
			cpu.d = (de >> 8) & 0xff;
			cpu.e = de & 0xff;
			return 12;
		}
		case 0xd5: {
			// push de 
			push((cpu.d << 8) | cpu.e);
			return 16;
		}
		case 0xe1: {
			// pop hl 
			word hl = pop();
			cpu.h = (hl >> 8) & 0xff;
			cpu.l = hl & 0xff;
			return 12;
		}
		case 0xe5: {
			// push hl 
			push((cpu.h << 8) | cpu.l);
			return 16;
		}
		case 0xf1: {
			// pop af 
			word af = pop();
			cpu.a = (af >> 8) & 0xff;
			// set flags based on f 
			cpu.flagz = (af >> 7) & 1;
			cpu.flagn = (af >> 6) & 1;
			cpu.flagh = (af >> 5) & 1;
			cpu.flagc = (af >> 4) & 1;
			return 12;
		}
		case 0xf5: {
			// push af 
			byte f = ((cpu.flagz << 7) | (cpu.flagn << 6) |
						(cpu.flagh << 5) | (cpu.flagc << 4));
			push((cpu.a << 8) | f);
			return 16;
		}
		
		/////////************\\\\\\\\\\\\\
		/* 16 bit arithmetic/logical 
		case 0x03: {
			// inc bc 
			word bc = (cpu.b << 8) | cpu.c;
			++bc;
			cpu.b = (bc >> 8) & 0xff;
			cpu.c = bc & 0xff;
			return 8;
		}
		case 0x09: {
			// add hl, bc 
			word hl = (cpu.h << 8) | cpu.l;
			word bc = (cpu.b << 8) | cpu.c;
			int res = hl + bc;
			cpu.flagn = 0;
			cpu.flagh = (((hl ^ bc ^ res) & 0x1000) == 0x1000);
			cpu.c = (res > 0xffff);
			cpu.h = (res >> 8) & 0xff;
			cpu.l = res & 0xff;
			return 8;
		}
		case 0x0b: {
			// dec bc 
			word bc = (cpu.b << 8) | cpu.c;
			--bc;
			cpu.b = (bc >> 8) & 0xff;
			cpu.c = bc & 0xff;
			return 8;
		}
		case 0x13: {
			// inc de 
			word de = (cpu.d << 8) | cpu.e;
			++de;
			cpu.d = (de >> 8) & 0xff;
			cpu.e = de & 0xff;
			return 8;
		}
		case 0x23: {
			// inc hl 
			word hl = (cpu.h << 8) | cpu.l;
			++hl;
			cpu.h = (hl >> 8) & 0xff;
			cpu.l = hl & 0xff;
			return 8;
		}
		case 0x33: {
			// inc sp 
			++cpu.sp;
			return 8;
		}
		
		/////////*************\\\\\\\\\\\\\\\
		/* jumps/calls/returns */
		case 0x18: {
			sbyte n = readbyte(cpu.pc);
			cpu.pc += n;
			return 12;
		}
		case 0x20: {
			//jr nz r8 
			sbyte n = readbyte(cpu.pc);
			int retval;
			// check if zero flag is reset 
			if (cpu.flagz == 0) {
				cpu.pc += n;
				retval = 12;
			} else {
				printf("false\n");
				retval = 9;
			}
			return retval;
		}
		case 0x28: {
			// jr z, r8 
			sbyte n = readbyte(cpu.pc);
			if (cpu.flagz) {
				cpu.pc += n;
				return 12;
			}
			return 8;
		}
		case 0x30: {
			// jr nc 
			sbyte n = readbyte(cpu.pc);
			if (cpu.flagc == 0) {
				cpu.pc += n;
				return 12;
			} else {
				return 8;
			}
		}
		case 0xc0: {
			// ret nc 
			if (cpu.flagz == 0) {
				cpu.pc = pop();
				return 20;
			} else {
				return 8;
			}
		}
		case 0xc2: {
			// jp nz, a16 
			word addr = readword(cpu.pc);
			if (cpu.flagz == 0) {
				cpu.pc = addr;
				return 16;
			} else {
				return 12;
			}
		}
		case 0xc3: {
			// jp a16 
			word addr = readword(cpu.pc);
			cpu.pc = addr;
			return 16;
		}
		case 0xc4: {
			// call nz, a16 
			word addr = readword(cpu.pc);
			if (cpu.flagz == 0) {
				push(cpu.pc);
				cpu.pc = addr;
				return 24;
			} else {
				return 12;
			}
		}
		case 0xc8: {
			// ret z 
			if (cpu.flagz) {
				cpu.pc = pop();
				return 20;
			} else {
				return 8;
			}
		}
		case 0xc9: {
			//ret 
			word addr = pop();
			cpu.pc = addr;
			return 16;
		}
		case 0xcd: {
			// call 
			word addr = readword(cpu.pc);
			push(cpu.pc);
			cpu.pc = addr;
			return 24;
		}
		case 0xd0: {
			// ret nc 
			if (cpu.flagc == 0) {
				cpu.pc = pop();
				return 20;
			} else {
				return 8;
			}
		}
		case 0xd2: {
			// jp nc, a16 
			word addr = readword(cpu.pc);
			if (cpu.flagc == 0) {
				cpu.pc = addr;
				return 16;
			} else {
				return 12;
			}
		}
		case 0xd4: {
			// call nc, a16 
			word addr = readword(cpu.pc);
			if (cpu.flagc == 0) {
				push(cpu.pc);
				cpu.pc = addr;
				return 24;
			} else {
				return 12;
			}
		}
		
		/////////*********\\\\\\\\\\\\
		/* misc/control instructions */
		case 0x00: {
			// nop 
			return 4;
		}
		case 0xcb: {
			// extended prefix
			return execute_extended_instruction();
		}
		case 0xf3: {
			// disable interrupt
			cpu.interrupt_switch = false;
			return 4;
		}
		case 0xfb: {
			// enable interrupts 
			cpu.interrupt_switch = true;
			return 4;
		}
		
		default:
			printf("unimplemented %.2x at %.4x\n", opcode, cpu.pc-1);
			printf("a=%.2x, bc=%.2x%.2x, de=%.2x%.2x,", cpu.a, cpu.b, cpu.c, cpu.d, cpu.e);
			printf(" hl=%.2x%.2x, sp=%.4x\n", cpu.h, cpu.l, cpu.sp);
			printf("flag: %x\n", ((cpu.flagz << 7) | (cpu.flagn << 6) | (cpu.flagh << 5) | (cpu.flagc << 4)));
			printf("%x %x %x %x\n", cpu.flagz, cpu.flagn, cpu.flagh, cpu.flagc);
			printf("%d instructions implemented and %d carried out\n", imp, innum);
			//printf("%x\n", memory[mmu->int_enable]);
			exit(1);
			break;
	}
}

static int execute_extended_instruction(void) {
	byte opcode = readbyte(cpu.pc);
	switch (opcode) {
		// rotate left
		case 0x11: {
			// rl c 
			byte msb = (cpu.c >> 7) & 1;
			cpu.c = (cpu.c << 1) & 0xff;
			cpu.c |= cpu.flagc;
			cpu.flagc = msb;
			cpu.flagz = (cpu.c == 0);
			cpu.flagh = 0;
			cpu.flagn = 0;
			return 8;
		}
		// check bits 
		case 0x7c: {
			//bit 7, h
			testbit(cpu.h, 7);
			return 8;
		}
		default:
			printf("unimplemented extended %.2x %.4x\n", opcode, cpu.pc);
			exit(1);
			return 1;
	}
}

static inline void push(word addr) {
	writebyte(--cpu.sp, ((addr >> 8) & 0xff));
	writebyte(--cpu.sp, (addr & 0xff));
}

static inline word pop(void) {
	byte low = memory[cpu.sp++];
	byte high = memory[cpu.sp++];
	return ((high << 8) | low);
}

static inline int half_carry_sub(byte n, byte m) {
	return (((n & 0xf) - (m & 0xf)) < 0);
}

static inline int half_carry_add(byte n, byte m) {
	return ((((n & 0xf) + (m & 0xf)) & 0x10) == 0x10);
}

byte testbit(byte reg, int bit) {
	byte res = (reg >> bit) & 0x1;
	// set zero flag 
	cpu.flagz = (res == 0);
	// reset sub flag
	cpu.flagn = 0;
	//set hc flag 
	cpu.flagh = 1;
	return res;
}

static inline void set_bit(byte *n, int bit) {
	(*n) |= (1 << bit);
}

static inline void reset_bit(byte *n, int bit) {
	(*n) &= ~(1 << bit);
}

static void Unimplemented(byte opcode) {
	printf("Unimplemented instruction %.2x at %.4x\n", opcode, cpu.pc-1);
	exit(1);
}