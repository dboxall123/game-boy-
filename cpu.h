typedef struct {
	uint8_t a, b, c, d, e, h, l;
	uint8_t flagz, flagn, flagh, flagc;
	uint16_t pc, sp; 
	uint8_t interrupt_switch;
} CPU;

CPU cpu;

#define MAX_CYCLES 69905
