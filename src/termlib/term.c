#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

#include "term.h"

Terminal* term_init(unsigned int w, unsigned int h, const char* fn) {
	SDL_Surface* temp = IMG_Load(fn);
	if (temp == NULL) {
		printf("Unable to load image: %s\n", fn);
		exit(1);
	}

	Terminal* newterm = calloc(1, sizeof(Terminal));
	char* textbuff = calloc(w*h, sizeof(char));

	newterm->font = temp;
	newterm->width = w;
	newterm->height = h;
	newterm->text = textbuff;

	term_clear(newterm);

	return newterm;
}

void term_clear(Terminal* t) {
	t->x = 0;
	t->y = 0;

	for(int i=0; i < t->width*t->height; i++)
		t->text[i] = ' ';
}

void term_move(Terminal* t, unsigned int x, unsigned int y) {
	if(x < t->width && y < t->height) {
		t->x = x;
		t->y = y;
	}
}

void term_next(Terminal* t) {
	t->x++;

	if(t->x >= t->width) {
		t->x = 0;
		t->y++;
	}

	if(t->y >= t->height) {
		t->x = 0;
		t->y = 0;
	}
}

void term_putc(Terminal* t, char c) {
	switch(c) {
		case '\n':
			t->y++;
			break;
		case '\r':
			t->x = 0;
			break;
		default:
			t->text[t->y*t->width+t->x] = c;
			term_next(t);
	}
}

void term_puts(Terminal* t, const char* str) {
	char c; while((c = *(str++))) term_putc(t, c);
}

void term_render(Terminal* t, struct SDL_Surface* canvas) {
	SDL_Rect src, dest;

	src.w = 11;
	src.h = 13;
	dest.w = src.w;
	dest.h = src.h;

	for(int y=0; y < t->height; y++) {
		for(int x=0; x < t->width; x++) {
			int index = t->text[y*t->width+x] - ' ';

			src.x = (index%10)*src.w;
			src.y = (index/10)*src.h;

			dest.x = x*dest.w;
			dest.y = y*dest.h;

			SDL_BlitSurface(t->font, &src, canvas, &dest);
			//printf("r");
		}
	}
}
