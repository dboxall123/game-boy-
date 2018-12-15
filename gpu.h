typedef struct {
	SDL_Window *window;
	SDL_Renderer *renderer;
} App;

App app;

typedef struct {
	SDL_Rect rect;
	uint8_t *color;
} Pixel;

int line_counter;