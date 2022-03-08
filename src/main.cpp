
#include <SDL.h>

#include "application.hpp"
#include "util.hpp"
#include "font.hpp"
#include "console.hpp"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"

#define SDL_STB_IMAGE_IMPLEMENTATION
#include "SDL_stb_image.hpp"

#include "generated/Silver.cpp"

namespace {

class game : public application {
public:
	game() {}
	virtual ~game() { finalize(); }

	static const int cell_width = 8;
	static const int cell_height = 8;
	static const int console_columns = 40;
	static const int console_rows = 25;
	static const int framebuffer_width = cell_width * 40;
	static const int framebuffer_height = cell_height * 25;
	static const int window_width = framebuffer_width * 2;
	static const int window_height = framebuffer_height * 2;

	bool initialize() {
		if (initialized()) {

		} else if (application::initialize(
			"wizlike",
			window_width,
			window_height,
			SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI,
			SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED
		)
		) {
			if (auto bmp = SDL_LoadBMP("assets/test.bmp")) {
				_tex = make_texture_from_surface(renderer(), bmp);
				SDL_FreeSurface(bmp);
			}

			SDL_SetWindowMinimumSize(window(), framebuffer_width, framebuffer_height);
			//SDL_RenderSetLogicalSize(renderer(), framebuffer_width, framebuffer_height);
			//SDL_RenderSetIntegerScale(renderer(), SDL_TRUE);

			_font = std::make_shared<font_set>();
			_font->load_font(renderer(), "assets/font/modern_dos.fnt");
			_font->load_font(renderer(), "assets/font/unscii.fnt");
			_font->load_font(renderer(), "assets/font/misaki_gothic_2nd.fnt");

			_console.current_font(_font);
			_console.pos(cell_width*5, cell_height*3);
			_console.cell(cell_width, cell_height);
			_console.geom(console_columns, console_rows);
			_console.fg_color({0xFF, 0xFF, 0});
			_console.bg_color({0, 0xFF, 0xFF});

			_console.cls();
			static const SDL_Rect print_rect{
				1, 20,
				5, 5
			};
			_console.print(u8"1234567890ABCDEFG", print_rect);

			_console.print(u8"01234567890123456789012345678901234567890123456789", 0, 0);
			_console.print(u8"ABCDE", console::option::inverse);

			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGui::StyleColorsDark();
			ImGui_ImplSDL2_InitForSDLRenderer(window(), renderer());
			ImGui_ImplSDLRenderer_Init(renderer());

			ImGuiIO& io = ImGui::GetIO();
			io.Fonts->AddFontFromMemoryCompressedTTF(
				Silver_compressed_data,
				Silver_compressed_size,
				21,
				nullptr,
				io.Fonts->GetGlyphRangesJapanese()
			);
		}
		return initialized();
	}

protected:
	void finalize() {
		ImGui_ImplSDLRenderer_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		_tex.reset();
	}

	virtual void update(float deltatime = 0.f) override {

	}

	virtual void draw() override {
		static const SDL_Rect src_rect{ 0, 0, framebuffer_width, framebuffer_height };
		static SDL_Rect src_rect2{ 0, 0, 32, 32 };
		static SDL_Rect target_rect{ 0, 0, 32, 32 };

		//SDL_GetMouseLogicalState(window(), renderer(), &target_rect.x, &target_rect.y);

		int state = SDL_GetMouseState(&target_rect.x, &target_rect.y);

		SDL_SetRenderDrawColor(renderer(), 0x80, 0x80, 0x80, 0);
		SDL_RenderClear(renderer());

		if (_tex) {
			SDL_RenderCopy(renderer(), _tex.get(), &src_rect, nullptr);
			SDL_RenderCopy(renderer(), _tex.get(), &src_rect2, &target_rect);
		}
		{
			int window_w, window_h;
			SDL_GetWindowSize(window(), &window_w, &window_h);
			_console.scale(std::min(window_w / _console.w(), window_h / _console.h()));
			_console.pos(
				(window_w - _console.w() * _console.scale()) / 2,
				(window_h - _console.h() * _console.scale()) / 2
			);
		}
		_console.begin(renderer());
		_console.flush(renderer());
		_console.print(renderer(), u8"あいうえおかきくけこ\nハローワールドAAAテスト\nÅǢÅ", (target_rect.x - _console.x()) / _console.scale(), (target_rect.y - _console.y()) / _console.scale());
		_console.end(renderer());

		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		static bool show_demo_window = true;
		if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

		ImGui::Begin(u8"Test Window");
		ImGui::Checkbox(u8"Demo Window", &show_demo_window);
		ImGui::Text(u8"Hello, World!");
		ImGui::Text(u8"X = %d\nY = %d", target_rect.x - _console.x(), target_rect.y - _console.y());
		ImGui::End();

		ImGui::Render();
		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
	}

	virtual void poll_event() override {
		ImGui_ImplSDL2_ProcessEvent(event());
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
