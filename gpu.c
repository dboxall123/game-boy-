#include "inc.h"

const uint8_t white[]      = {255,255,255}; // 0
const uint8_t light_grey[] = {192,192,192}; // 1
const uint8_t dark_grey[]  = {96,96,96};	// 2
const uint8_t black[]      = {0,0,0};		// 3

const uint8_t *colors[] = {white, light_grey, dark_grey, black};


void update_graphics(int prev_cycles) {
	//set_lcd() 
	if (testbit(readmem(0xff40), 7)) {
		line_counter -= prev_cycles;
	} else {
		return;
	}
	
	if (line_counter <= 0) {
		memory[0xff44]++;
		//printf("%x\n", memory[0xff44]);
		byte current_line = readmem(0xff44);
		line_counter = 456;
		
		// entered vblank
		if (current_line == 144) 
			request_interrupt(0);
		
		// reset to 0 
		else if (current_line > 153)
			memory[0xff44] = 0;
		
		// draw current line 
		else if (current_line < 144)
			;
	}
}


