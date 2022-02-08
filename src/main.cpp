
#include <SDL.h>
#include <tinyutf8.h>
#include <iostream>
#include <functional>
#include <memory>
#include <stb_image.h>
#include <pugixml.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <filesystem>

#include "application.hpp"
#include "util.hpp"

#define SDL_STB_IMAGE_IMPLEMENTATION
#include "SDL_stb_image.hpp"

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
		char32_t id;
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
	std::unordered_map<char32_t, bmf_char> chars;
};

class font {
public:
	font() {}

	using character = bmf_font::bmf_char;

	void load_font(SDL_Renderer* renderer, const std::filesystem::path &path) {
		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_file(path.string().c_str());
		if (result) {
			bmf_font &bmfont = _bmfont;
			auto font = doc.child("font");
			{
				auto info = font.child("info");
				bmfont.info.face = info.attribute("face").as_string();
				bmfont.info.size = info.attribute("size").as_int();
				bmfont.info.bold = info.attribute("bold").as_int() != 0;
				bmfont.info.italic = info.attribute("italic").as_int() != 0;
				bmfont.info.charset = info.attribute("charset").as_string();
				bmfont.info.unicode = info.attribute("unicode").as_int() != 0;
				bmfont.info.stretchH = info.attribute("stretchH").as_int();
				bmfont.info.smooth = info.attribute("smooth").as_int() != 0;
				bmfont.info.aa = info.attribute("aa").as_int() != 0;
				{
					auto padding = info.attribute("padding").as_string();
					auto list = split(padding, ',');
					if (list.size() > 0) bmfont.info.padding.up = to_int(list[0]);
					if (list.size() > 1) bmfont.info.padding.right = to_int(list[1]);
					if (list.size() > 2) bmfont.info.padding.down = to_int(list[2]);
					if (list.size() > 3) bmfont.info.padding.left = to_int(list[3]);
				}
				{
					auto spacing = info.attribute("spacing").as_string();
					auto list = split(spacing, ',');
					if (list.size() > 0) bmfont.info.spacing.horizontal = to_int(list[0]);
					if (list.size() > 1) bmfont.info.spacing.vertical = to_int(list[1]);
				}
				bmfont.info.outline = info.attribute("outline").as_int();
			}
			{
				auto common = font.child("common");
				bmfont.common.line_height = common.attribute("lineHeight").as_int();
				bmfont.common.base = common.attribute("base").as_int();
				bmfont.common.scale_width = common.attribute("scaleW").as_int();
				bmfont.common.scale_height = common.attribute("scaleH").as_int();
				bmfont.common.pages = common.attribute("pages").as_int();
				bmfont.common.packed = common.attribute("packed").as_int() != 0;
				bmfont.common.alpha_channel = decltype(bmfont.common.alpha_channel)(common.attribute("alphaChnl").as_int());
				bmfont.common.red_channel = decltype(bmfont.common.red_channel)(common.attribute("redChnl").as_int());
				bmfont.common.green_channel = decltype(bmfont.common.green_channel)(common.attribute("greenChnl").as_int());
				bmfont.common.blue_channel = decltype(bmfont.common.blue_channel)(common.attribute("blueChnl").as_int());
			}
			auto pages = font.child("pages").children("page");
			for (auto& page : pages) {
				bmfont.pages.push_back({
					page.attribute("id").as_int(),
					page.attribute("file").as_string()
					});
			}
			auto chars = font.child("chars").children("char");
			for (auto& chara : chars) {
				char32_t id = static_cast<char32_t>(chara.attribute("id").as_int());
				bmfont.chars[id] = {
					id,
					chara.attribute("x").as_int(),
					chara.attribute("y").as_int(),
					chara.attribute("width").as_int(),
					chara.attribute("height").as_int(),
					chara.attribute("xoffset").as_int(),
					chara.attribute("yoffset").as_int(),
					chara.attribute("xadvance").as_int(),
					chara.attribute("page").as_int(),
					chara.attribute("chnl").as_int()
				};
			}
			load_page_textures(bmfont, renderer, path.parent_path().string());
		}
	}

	void load_page_textures(const bmf_font &font, SDL_Renderer *renderer, const std::filesystem::path& dir) {
		for (auto &page : font.pages) {
			if (auto* surface = STB_IMG_Load((dir / page.file).string().c_str())) {
				SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0, 0, 0));
				_pages.push_back(make_texture_from_surface(renderer, surface));
				SDL_FreeSurface(surface);
			}
		}
	}

	bmf_font::bmf_char* get_char(char32_t codepoint) {
		bmf_font::bmf_char* chara = nullptr;
		if (auto it = _bmfont.chars.find(codepoint); it != _bmfont.chars.end()) {
			chara = &it->second;
		}
		return chara;
	}

	void put_char(SDL_Renderer* renderer, int x, int y, char32_t codepoint) {
		if (auto *chara = get_char(codepoint)) {
			put_char(renderer, x, y, *chara);
		}
	}

	void put_char(SDL_Renderer* renderer, int x, int y, const character &chara) {
		if (auto page = find_page(chara.page)) {
			SDL_Rect src_rect{ chara.x, chara.y, chara.width, chara.height };
			SDL_Rect dst_rect{ x, y, chara.width, chara.height };
			SDL_RenderCopy(renderer, page.get(), &src_rect, &dst_rect);
		}
	}

	void print(SDL_Renderer* renderer, int x, int y, tiny_utf8::utf8_string string) {
		int begin_x = x;
		for (char32_t codepoint : string) {
			if (codepoint == '\n') {
				x = begin_x;
				y += 8;
			} else {
				put_char(renderer, x, y, codepoint);
				x += 8;
			}
		}
	}

	SDL_Pointer<SDL_Texture> find_page(int page) {
		return (page < _pages.size()) ? _pages[page] : SDL_Pointer<SDL_Texture>{};
	}

private:
	bmf_font _bmfont;
	std::vector<SDL_Pointer<SDL_Texture>> _pages;
};

class font_set {
public:
	font_set() {}

	using character = bmf_font::bmf_char;

	void load_font(SDL_Renderer* renderer, const std::filesystem::path& path) {
		font newfont;
		newfont.load_font(renderer, path);
		_fonts.push_back(newfont);
	}

	void put_char(SDL_Renderer* renderer, int x, int y, char32_t codepoint) {
		font* target_font = nullptr;
		character* chara = nullptr;
		if (find_font(codepoint, target_font, chara)) {
			target_font->put_char(renderer, x, y, *chara);
		}
	}

	void print(SDL_Renderer* renderer, int x, int y, tiny_utf8::utf8_string string) {
		int begin_x = x;
		for (char32_t codepoint : string) {
			if (codepoint == '\n') {
				x = begin_x;
				y += 8;
			} else {
				put_char(renderer, x, y, codepoint);
				x += 8;
			}
		}
	}

	bool find_font(char32_t codepoint, font*& out_font, character*& out_char) {
		bool found = false;
		for (auto& font : _fonts) {
			if (auto* chara = font.get_char(codepoint)) {
				out_font = &font;
				out_char = chara;
				found = true;
				break;
			}
		}
		return found;
	}

private:
	std::vector<font> _fonts;
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
				_font.load_font(renderer(), "assets/font/unscii.fnt");
				_font.load_font(renderer(), "assets/font/misaki_gothic_2nd.fnt");
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
#if 0
		if (_tex) {
			SDL_RenderCopy(renderer(), _tex.get(), &src_rect, nullptr);
			SDL_RenderCopy(renderer(), _tex.get(), &src_rect2, &target_rect);
		}
#endif
		_font.print(renderer(), target_rect.x, target_rect.y, u8"あいうえおかきくけこ\nハローワールドAAAテスト☀");
	}

private:
	SDL_Pointer<SDL_Texture> _tex;
	font_set _font;
};

} // namespace game

int main(int argc, char **argv) {
	int result = 0;
	if (::game app{}; app.initialize()) {
		result = app.boot();
	}
	return result;
}
