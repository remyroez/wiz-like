﻿
#include <SDL.h>
#include <tinyutf8.h>
#include <iostream>
#include <functional>
#include <memory>
#include <stb_image.h>

#include "application.hpp"
#include "util.hpp"

namespace {

class game : public application {
public:
	game() {}
	virtual ~game() { finalize(); }

	bool initialize() {
		if (initialized()) {

		} else if (application::initialize("wizlike", 640, 400, SDL_WINDOW_RESIZABLE, SDL_RENDERER_ACCELERATED)) {
			if (auto bmp = SDL_LoadBMP("test.bmp")) {
				_tex = make_texture_from_surface(renderer(), bmp);
				SDL_FreeSurface(bmp);
			}

			SDL_SetWindowMinimumSize(window(), 320, 200);
			SDL_RenderSetLogicalSize(renderer(), 320, 200);
			SDL_RenderSetIntegerScale(renderer(), SDL_TRUE);
		}
		return initialized();
	}

protected:
	void finalize() {
		_tex.reset();
	}

	virtual void update(float deltatime = 0.f) override {

	}

	virtual void draw() override {
		static const SDL_Rect rect{ 0, 0, 320, 240 };

		SDL_RenderClear(renderer());
		if (_tex) SDL_RenderCopy(renderer(), _tex.get(), nullptr, &rect);
	}

private:
	SDL_Pointer<SDL_Texture> _tex;
};

} // namespace game

int main(int argc, char **argv) {
	int result = 0;
	if (::game app{}; app.initialize()) {
		result = app.boot();
	}
	return result;
}
