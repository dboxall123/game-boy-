#include "inc.h"

static int init_mmu(FILE *);
static void load_cart_to_mem(int);
static void init_cpu(void);
static void init_gpu(void);

void init(char *fname) {
	FILE *f;
	char *title;
	if (fname)
		title = fname;
	else	// default title 
		title = "tetris.gb";
	
	if ((f = fopen(title, "rb")) == NULL) {
		printf("file %s not found\n", fname);
		exit(1);
	} else {
		init_cpu();
		int fsize = init_mmu(f);
		load_cart_to_mem(fsize);
	}
}

static void init_gpu(void) {
	line_counter = 0;
}

static void init_cpu(void) {
	//cpu = (CPU *) malloc(sizeof(CPU));
	cpu.a = 0;
	cpu.b = 0;
	cpu.c = 0;
	cpu.d = 0;
	cpu.e = 0;
	cpu.h = 0;
	cpu.l = 0;
	cpu.pc = 0;
	cpu.sp = 0;
	cpu.flagz = 0;
	cpu.flagn = 0;cpu.flagh = cpu.flagc = 0;
	cpu.interrupt_switch = true;
}

static void load_cart_to_mem(int size) {
	memcpy(memory, cartridge, size);
	reset();
	//printf("%x\n", memory[0x148]);
}

static int init_mmu(FILE *f) {
	// initialise mmu 
	mmu = (MemoryUnit *) malloc(sizeof(MemoryUnit));
	mmu->rom_bank1 = 0x4000;
	mmu->vram = 0x8000;
	mmu->ext_ram = 0xa000;
	mmu->wram0 = 0xc000;
	mmu->wram1 = 0xd000;
	mmu->echo_ram = 0xe000;
	mmu->oam = 0xfe00;
	mmu->io = 0xff00;
	mmu->hram = 0xff80;
	mmu->int_enable = 0xffff;
	mmu->inbios = true;
	
	//reset cartridge, memory & rambank arrays 
	memset(cartridge, 0, sizeof(cartridge));
	memset(memory, 0, sizeof(memory));
	memset(ram_bank, 0, sizeof(ram_bank));
	current_ram_bank = 0;
	
	// read cartridge 
	int fsize;
	fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	fread(cartridge, 1, fsize, f);
	fclose(f);
	
	//reset();
	return fsize;
}