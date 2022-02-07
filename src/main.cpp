
#include <SDL.h>
#include <tinyutf8.h>
#include <iostream>
#include <functional>
#include <memory>
#include <stb_image.h>
#include <pugixml.hpp>
#include <string>
#include <vector>

#include "application.hpp"
#include "util.hpp"

namespace {

struct bmf_font {
	struct bmf_info {
		std::string face;
		int size = 0;
		bool bold = false;
		bool italic = false;
		std::string charset;
		bool unicode = false;
		float stretchH = 100.f;
		bool smooth = false;
		bool aa = false;
		struct bmf_padding {
			int up, right, down, left;
		} padding{0};
		struct bmf_spacing {
			int horizontal, vertical;
		} spacing{0};
		int outline = 0;
	} info;
	struct bmf_common {
		int line_height;
		int base;
		int scale_width;
		int scale_height;
		int pages;
		bool packed;
		enum bmf_channel {
			glyph = 0,
			outline,
			encoded_glyph_and_outline,
			zero,
			one,
		};
		bmf_channel alpha_channel;
		bmf_channel red_channel;
		bmf_channel green_channel;
		bmf_channel blue_channel;
	} common;
	struct bmf_page {
		int id;
		std::string file;
	};
	std::vector<bmf_page> pages;
	struct bmf_char {
		int id;
		int x, y;
		int width, height;
		int x_offset, y_offset;
		int x_advance;
		int page;
		enum bmf_texture_channel {
			blue = 1 << 0,
			green = 1 << 1,
			red = 1 << 2,
			alpha = 1 << 3,
		};
		int channel;
	};
	std::vector<bmf_char> chars;
};

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

			{
				pugi::xml_document doc;
				pugi::xml_parse_result result = doc.load_file("assets/font/misaki_gothic_2nd.fnt");
				if (result) {
					auto font = doc.child("font");
					{
						auto info = font.child("info");
						_font.info.face = info.attribute("face").as_string();
						_font.info.size = info.attribute("size").as_int();
						_font.info.bold = info.attribute("bold").as_int() != 0;
						_font.info.italic = info.attribute("italic").as_int() != 0;
						_font.info.charset = info.attribute("charset").as_string();
						_font.info.unicode = info.attribute("unicode").as_int() != 0;
						_font.info.stretchH = info.attribute("stretchH").as_int();
						_font.info.smooth = info.attribute("smooth").as_int() != 0;
						_font.info.aa = info.attribute("aa").as_int() != 0;
						{
							auto padding = info.attribute("padding").as_string();
						}
						{
							auto spacing = info.attribute("spacing").as_string();
						}
						_font.info.outline = info.attribute("outline").as_int();
					}
					{
						auto common = font.child("common");
						_font.common.line_height = common.attribute("lineHeight").as_int();
						_font.common.base = common.attribute("base").as_int();
						_font.common.scale_width = common.attribute("scaleW").as_int();
						_font.common.scale_height = common.attribute("scaleH").as_int();
						_font.common.pages = common.attribute("pages").as_int();
						_font.common.packed = common.attribute("packed").as_int() != 0;
						_font.common.alpha_channel = decltype(_font.common.alpha_channel)(common.attribute("alphaChnl").as_int());
						_font.common.red_channel = decltype(_font.common.red_channel)(common.attribute("redChnl").as_int());
						_font.common.green_channel = decltype(_font.common.green_channel)(common.attribute("greenChnl").as_int());
						_font.common.blue_channel = decltype(_font.common.blue_channel)(common.attribute("blueChnl").as_int());
					}
					auto pages = font.child("pages").children("page");
					for (auto& page : pages) {
						_font.pages.push_back({
							page.attribute("id").as_int(),
							page.attribute("file").as_string()
						});
					}
					auto chars = font.child("chars").children("char");
					for (auto& chara : chars) {
						_font.chars.push_back({
							chara.attribute("id").as_int(),
							chara.attribute("x").as_int(),
							chara.attribute("y").as_int(),
							chara.attribute("width").as_int(),
							chara.attribute("height").as_int(),
							chara.attribute("xoffset").as_int(),
							chara.attribute("yoffset").as_int(),
							chara.attribute("xadvance").as_int(),
							chara.attribute("page").as_int(),
							chara.attribute("chnl").as_int()
						});
					}
				}
			}
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

		SDL_RenderClear(renderer());
		if (_tex) {
			SDL_RenderCopy(renderer(), _tex.get(), &src_rect, nullptr);
			SDL_RenderCopy(renderer(), _tex.get(), &src_rect2, &target_rect);
		}
	}

private:
	SDL_Pointer<SDL_Texture> _tex;
	bmf_font _font;
};

} // namespace game

int main(int argc, char **argv) {
	int result = 0;
	if (::game app{}; app.initialize()) {
		result = app.boot();
	}
	return result;
}
