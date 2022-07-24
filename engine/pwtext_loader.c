//Lithio (The developer's pseudonym)
//June 10, 2022

#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "pwtext_loader.h"

void pwtextloader_init(PWTextLoader *t){
	t->glyph_maps = hashtable_init();
	t->glyph_advances = hashtable_init();
}
	
void pwtextloader_load(PWTextLoader *t, const char *filename, PWTexture **texture, float **advances){
	//variables for loading the ttf and rendering it to a glyph map
	TTF_Font *font;
	int font_skip;
	float loaded_size;
	int glyph_map_size = 1024;
	SDL_Surface *glyph_surface;
	SDL_Surface *glyph_map;
	int i;
	int advance;
	SDL_Color color = {255, 255, 255};
	char cc[2] = {0, 0};
	SDL_Rect rect;
	
	//this code is a bit messy, but it is to check if a texture (glyph map)
	//is loaded already, and load one if not
	*texture = (PWTexture*) hashtable_at_str(t->glyph_maps, filename);
	*advances = (float*) hashtable_at_str(t->glyph_advances, filename);
	if(!*texture){
		//initialize the texture
		glyph_map = SDL_CreateRGBSurface(0, glyph_map_size, glyph_map_size, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
		if(!glyph_map){
			printf("glyph_map surface creation failed: %s\n", SDL_GetError());
		}
		
		loaded_size = (float)(glyph_map_size >> 4);
		font = TTF_OpenFont(filename, (int)(loaded_size));
		if(!font){
			printf("Could not open ttf file: %s\n", TTF_GetError());
			return;
		}
		
		*texture = (PWTexture*) malloc(sizeof(PWTexture));
		*advances = (float*) malloc(sizeof(float) * 128);
		
		//adjust the size of the text so that it is exactly 1/16th of
		//the texture's height
		font_skip = TTF_FontLineSkip(font);
		TTF_CloseFont(font);
		loaded_size = loaded_size * loaded_size / font_skip;
		font = TTF_OpenFont(filename, (int)(loaded_size));
		
		//render the ASCII characters to a 16x16 sprite map
		for(i = 32; i < 127; ++i){
			TTF_GlyphMetrics(font, (Uint16)i, NULL, NULL, NULL, NULL, &advance);
			(*advances)[i] = advance / (float)(glyph_map_size);
			//create surface of each glyph, blit to glyph_map
			cc[0] = i;
			glyph_surface = TTF_RenderText_Solid(font, cc, color);
			rect.w = advance;
			rect.h = glyph_map_size >> 4;
			rect.x = (i & 0x0f) * (glyph_map_size >> 4);
			rect.y = (i >> 4) * (glyph_map_size >> 4);
			SDL_BlitSurface(glyph_surface, NULL, glyph_map, &rect);
			SDL_FreeSurface(glyph_surface);
		}
		TTF_CloseFont(font);
		
		//create a texture with the surface pixels
		SDL_LockSurface(glyph_map);
		pwtexture_init_pixels(*texture, glyph_map_size, glyph_map_size, glyph_map->pixels);
		SDL_UnlockSurface(glyph_map);
		
		//SDL_SaveBMP(glyph_map, "glyph_map.bmp");
		SDL_FreeSurface(glyph_map);
		
		//add to the tables
		hashtable_set_at_str(t->glyph_maps, filename, *texture);
		hashtable_set_at_str(t->glyph_advances, filename, *advances);
	}
}


void pwtextloader_close(PWTextLoader *t){
	float *advance = NULL;
	PWTexture *font_map = NULL;
	
	advance = (float*) hashtable_begin(t->glyph_advances);
	while(advance){
		free(advance);
		advance = (float*) hashtable_next(t->glyph_advances);
	}
	hashtable_clear(t->glyph_advances);
	
	font_map = (PWTexture*) hashtable_begin(t->glyph_maps);
	while(font_map){
		pwtexture_close(font_map);
		free(font_map);
		font_map = (PWTexture*) hashtable_next(t->glyph_maps);
	}
	hashtable_clear(t->glyph_maps);
}

