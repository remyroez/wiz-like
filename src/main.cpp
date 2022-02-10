
#include <SDL.h>

#include "application.hpp"
#include "util.hpp"
#include "font.hpp"
#include "console.hpp"

#define SDL_STB_IMAGE_IMPLEMENTATION
#include "SDL_stb_image.hpp"

namespace {

class game : public application {
public:
	game() {}
	virtual ~game() { finalize(); }

	static const int cell_width = 8;
	static const int cell_height = 8;
	static const int framebuffer_width = cell_width * 40;
	static const int framebuffer_height = cell_height * 25;
	static const int window_width = framebuffer_width * 2;
	static const int window_height = framebuffer_height * 2;

	bool initialize() {
		if (initialized()) {

		} else if (application::initialize("wizlike", window_width, window_height, SDL_WINDOW_RESIZABLE, SDL_RENDERER_ACCELERATED)) {
			if (auto bmp = SDL_LoadBMP("assets/test.bmp")) {
				_tex = make_texture_from_surface(renderer(), bmp);
				SDL_FreeSurface(bmp);
			}

			SDL_SetWindowMinimumSize(window(), framebuffer_width, framebuffer_height);
			SDL_RenderSetLogicalSize(renderer(), framebuffer_width, framebuffer_height);
			SDL_RenderSetIntegerScale(renderer(), SDL_TRUE);

			_font = std::make_shared<font_set>();
			_font->load_font(renderer(), "assets/font/unscii.fnt");
			_font->load_font(renderer(), "assets/font/misaki_gothic_2nd.fnt");

			_console.current_font(_font);
			_console.pos(cell_width, cell_height);
			_console.size(framebuffer_width - cell_width * 2, framebuffer_height - cell_height * 2);
			_console.cell(cell_width, cell_height);
			_console.fg_color({0xFF, 0xFF, 0});
			_console.bg_color({0, 0xFF, 0xFF});
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
		static const SDL_Rect src_rect{ 0, 0, framebuffer_width, framebuffer_height };
		static SDL_Rect src_rect2{ 0, 0, 32, 32 };
		static SDL_Rect target_rect{ 0, 0, 32, 32 };

		SDL_GetMouseLogicalState(window(), renderer(), &target_rect.x, &target_rect.y);

		SDL_SetRenderDrawColor(renderer(), 0x80, 0x80, 0x80, 0);
		SDL_RenderClear(renderer());

		if (_tex) {
			SDL_RenderCopy(renderer(), _tex.get(), &src_rect, nullptr);
			SDL_RenderCopy(renderer(), _tex.get(), &src_rect2, &target_rect);
		}

		_console.fill(renderer());
		_console.print(renderer(), target_rect.x, target_rect.y, u8"あいうえおかきくけこ\nハローワールドAAAテスト\nÅǢÅ");

		static const SDL_Rect print_rect{
			framebuffer_width - 6 * cell_width - 1, 5 * cell_height - 1,
			framebuffer_width, framebuffer_height
		};
		_console.print(renderer(), print_rect, u8"1234567890ABCDEFG");

		_console.coord();
		_console.print(renderer(), u8"01234567890123456789012345678901234567890123456789");
		_console.print(renderer(), u8"ABCDE", console::option::inverse);
	}

private:
	SDL_Pointer<SDL_Texture> _tex;
	std::shared_ptr<font_set> _font;
	console _console;
};

} // namespace game

int main(int argc, char **argv) {
	int result = 0;
	if (::game app{}; app.initialize()) {
		result = app.boot();
	}
	return result;
}
