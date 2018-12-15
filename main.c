#include "inc.h"

#define fps 60.0
#define tpf 1000.0/fps

int main(int argc, char *argv[]) {
	init(*++argv);
	
	int st, et, rt;
	
	for (;;) {
		st = SDL_GetTicks();
		cpu_update();
		//event_loop();
		rt = tpf - (SDL_GetTicks() - st);
		//printf("ticks required \n%d\n\n", rt);
		if (rt > 0)
			SDL_Delay(rt);
		
	}
}