uint8_t cartridge[0x200000];
uint8_t memory[0x10000];
uint8_t ram_bank[0x8000];
uint8_t current_ram_bank;

typedef struct {
	int inbios;
	int rom_bank0;
	int rom_bank1;
	int vram;
	int ext_ram;
	int wram0;
	int wram1;
	int echo_ram;
	int oam;
	int io;
	int hram;
	int int_enable;
} MemoryUnit;

MemoryUnit *mmu;
	