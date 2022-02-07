#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <SDL.h>
#include <memory>

template<typename T>
using SDL_Pointer = std::shared_ptr<T>;

template<typename T, typename Creator, typename Deleter, typename... Args>
inline auto SDL_Make(Creator* creator, Deleter* deleter, Args&&... args) {
	return SDL_Pointer<T>(creator(std::forward<Args>(args)...), deleter);
}

#define DEFINE_MAKE(NAME, T, CREATOR, DESTROYER) \
template<typename... Args> \
inline auto NAME(Args&&... args) { \
	return SDL_Make<T>(CREATOR, DESTROYER, std::forward<Args>(args)...); \
}

DEFINE_MAKE(make_window, SDL_Window, SDL_CreateWindow, SDL_DestroyWindow);
DEFINE_MAKE(make_renderer, SDL_Renderer, SDL_CreateRenderer, SDL_DestroyRenderer);
DEFINE_MAKE(make_texture_from_surface, SDL_Texture, SDL_CreateTextureFromSurface, SDL_DestroyTexture);

DEFINE_MAKE(load_bmp, SDL_Surface, SDL_LoadBMP, SDL_FreeSurface);

#define SDL_PrintError(NAME) { std::cerr << #NAME << ": " << SDL_GetError() << std::endl; }

struct SDL_Context {
	SDL_Context() {}

	SDL_Context(Uint32 flags) {
		initialize(flags);
	}
	~SDL_Context() {
		finalize();
	}

	bool initialize(Uint32 flags) {
		_initialized = SDL_Init(flags) == 0;
		return _initialized;
	}

	void finalize() {
		if (_initialized) {
			SDL_Quit();
			_initialized = false;
		}
	}

	operator bool() {
		return _initialized;
	}

	bool _initialized = false;
};

#endif // UTIL_HPP_
