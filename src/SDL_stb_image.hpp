
#ifndef SDL_STB_IMAGE_HPP_
#define SDL_STB_IMAGE_HPP_

/**
*  Load a surface from a file.
*
*  \return the new surface, or nullptr if there was an error.
*/
SDL_Surface* STB_IMG_Load(const char* file);

/**
*  Allocate surface.
*
*  \return the new surface, or nullptr if there was an error.
*/
SDL_Surface* STB_IMG_CreateSurface(void* data, int width, int height, int comp, bool free);

#endif // SDL_STB_IMAGE_HPP_

#ifdef SDL_STB_IMAGE_IMPLEMENTATION

#define STBI_MALLOC SDL_malloc
#define STBI_REALLOC SDL_realloc
#define STBI_FREE SDL_free

#define STB_IMAGE_IMPLEMENTATION

#ifdef SDL_STB_IMAGE_NO_STDIO
#define STBI_NO_STDIO
#endif

#include "stb_image.h"

static SDL_Surface* STB_IMG_CreateSurface(void* data, int width, int height, int comp, bool free) {
	SDL_Surface* surface = nullptr;

	unsigned int rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	int shift = (comp == STBI_rgb) ? 8 : 0;
	rmask = 0xff000000 >> shift;
	gmask = 0x00ff0000 >> shift;
	bmask = 0x0000ff00 >> shift;
	amask = 0x000000ff >> shift;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = (comp == STBI_rgb) ? 0 : 0xff000000;
#endif

	surface = SDL_CreateRGBSurfaceFrom(data, width, height, comp * 8, comp * width, rmask, gmask, bmask, amask);
	if (surface == nullptr) {
		//SDL_SetError("STB_IMG_CreateSurface: Error on SDL_CreateRGBSurfaceFrom");

	} else if (free) {
		surface->flags &= ~SDL_PREALLOC;
	}

	return surface;
}

static SDL_Surface* STB_IMG_Load(const char* file) {
	SDL_Surface* surface = nullptr;

	int width;
	int height;
	int comp;

	auto info = stbi_info(file, &width, &height, &comp);
	int req_comp = (comp == STBI_grey || comp == STBI_rgb) ? STBI_rgb : STBI_rgb_alpha;

	auto data = stbi_load(file, &width, &height, &comp, req_comp);
	if (data == nullptr) {
		SDL_SetError("STB_IMG_LoadTexture: can't load.");

	} else {
		surface = STB_IMG_CreateSurface(data, width, height, req_comp, true);
	}

	return surface;
}

#endif // SDL_STB_IMAGE_IMPLEMENTATION
