#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <SDL.h>
#include <memory>
#include <string>
#include <vector>
#include <charconv>
#include <iostream>

#include "SDL_stb_image.hpp"

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

DEFINE_MAKE(load_tex, SDL_Texture, SDL_LoadTexture, SDL_DestroyTexture);

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

int SDL_GetMouseLogicalState(SDL_Window *window, SDL_Renderer *renderer, int* x, int* y) {
	int mouse_x, mouse_y;
	int state = SDL_GetMouseState(&mouse_x, &mouse_y);

	int window_w, window_h;
	SDL_GetWindowSize(window, &window_w, &window_h);
	int center_x = window_w / 2, center_y = window_h;

	int logical_w, logical_h;
	SDL_RenderGetLogicalSize(renderer, &logical_w, &logical_h);

	float scale_x, scale_y;
	SDL_RenderGetScale(renderer, &scale_x, &scale_y);

	int offset_x = (window_w - logical_w * scale_x) / 2;
	int offset_y = (window_h - logical_h * scale_y) / 2;

	int logical_mouse_x = (mouse_x - offset_x) / scale_x;
	int logical_mouse_y = (mouse_y - offset_y) / scale_y;

	if (x) *x = logical_mouse_x;
	if (y) *y = logical_mouse_y;

	return state;
}

SDL_Texture *SDL_LoadTexture(SDL_Renderer *renderer, const char *path) {
	SDL_Texture* ptr = nullptr;
	if (auto *surface = STB_IMG_Load(path)) {
		ptr = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_FreeSurface(surface);
	}
	return ptr;
}

std::vector<std::string> split(const std::string& s, char delim) {
	std::vector<std::string> elems;
	std::string item;
	for (char ch : s) {
		if (ch == delim) {
			if (!item.empty())
				elems.push_back(item);
			item.clear();

		} else {
			item += ch;
		}
	}
	if (!item.empty())
		elems.push_back(item);
	return elems;
}

int to_int(const std::string& str) {
	int value = 0;
	if (auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value); ec == std::errc{}) {
	}
	return value;
}

#endif // UTIL_HPP_
