
#include <SDL.h>
#include <tinyutf8.h>
#include <iostream>
#include <functional>
#include <memory>
#include <stb_image.h>

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

int main(int argc, char **argv) {

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		SDL_PrintError(SDL_Init);
		return EXIT_FAILURE;
	}

	auto window = make_window(
		"wizlike",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		640,
		480,
		SDL_WINDOW_RESIZABLE
	);
	if (!window) {
		SDL_PrintError(SDL_CreateWindow);
		SDL_Quit();
		return EXIT_FAILURE;
	}

	auto renderer = make_renderer(window.get(), -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		SDL_PrintError(SDL_CreateRenderer);
		window.reset();
		SDL_Quit();
		return EXIT_FAILURE;
	}

	auto bmp = SDL_LoadBMP("test.bmp");
	auto tex = make_texture_from_surface(renderer.get(), bmp);
	if (bmp) SDL_FreeSurface(bmp);

	SDL_RenderSetLogicalSize(renderer.get(), 320, 240);
	SDL_RenderSetIntegerScale(renderer.get(), SDL_TRUE);

	SDL_Rect rect{ 0, 0, 320, 240 };

	SDL_Event event;
	bool running = true;
	while (running) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				running = false;
				break;
			}
		}

		SDL_RenderClear(renderer.get());
		if (tex) SDL_RenderCopy(renderer.get(), tex.get(), nullptr, &rect);

		SDL_RenderPresent(renderer.get());

		SDL_Delay(1000 / 60);
	}

	tex.reset();
	renderer.reset();
	window.reset();

	SDL_Quit();

	return 0;
}
