#ifndef CONSOLE_HPP_
#define CONSOLE_HPP_

#include <memory>
#include <algorithm>
#include <vector>
#include <string>
#include <tinyutf8.h>

#include "font.hpp"

class console {
public:
	console() {}
	console(std::shared_ptr<font_set> font_ptr, const SDL_Rect &rect, const SDL_Rect& cell)
		: _font_ptr(font_ptr), _rect(rect), _cell(cell) {}

	inline auto current_font() {
		return _font_ptr.lock();
	}

	void print(SDL_Renderer* renderer, int x, int y, tiny_utf8::utf8_string string) {
		SDL_Rect render_rect = _rect;
		render_rect.x += x;
		render_rect.y += y;
		print(renderer, render_rect, string);
	}

	void print(SDL_Renderer* renderer, tiny_utf8::utf8_string string) {
		auto p = current_font();
		if (!p) return;

		SDL_Rect current_rect = _rect;
		int begin_x = _rect.x;
		for (char32_t codepoint : string) {
			if (codepoint == '\n') {
				current_rect.x = begin_x;
				current_rect.y += _cell.h;

			} else {
				p->put_char(renderer, current_rect.x, current_rect.y, codepoint);
				current_rect.x += _cell.w;
			}

			if (current_rect.x >= (_rect.x + _rect.w)) {
				current_rect.x = begin_x;
				current_rect.y += _cell.h;
			}
			if (current_rect.y >= (_rect.y + _rect.h)) {
				break;
			}
		}
	}

	void print(SDL_Renderer* renderer, const SDL_Rect &rect, tiny_utf8::utf8_string string) {
		auto p = current_font();
		if (!p) return;

		// 描画範囲
		SDL_Rect render_rect = rect;
		render_rect.x += _rect.x;
		render_rect.y += _rect.y;
		{
			auto base_right = _rect.x + _rect.w;
			auto render_right = render_rect.x + render_rect.w;
			if (render_right > base_right) {
				render_rect.w -= (render_right - base_right);
			}
		}
		{
			auto base_bottom = _rect.y + _rect.h;
			auto render_bottom = render_rect.y + render_rect.h;
			if (render_bottom > base_bottom) {
				render_rect.h -= (render_bottom - base_bottom);
			}
		}

		SDL_Rect current_rect = render_rect;
		int begin_x = render_rect.x;
		for (char32_t codepoint : string) {
			if (codepoint == '\n') {
				current_rect.x = begin_x;
				current_rect.y += _cell.h;

			} else {
				p->put_char(renderer, current_rect.x, current_rect.y, codepoint);
				current_rect.x += _cell.w;
			}

			if ((current_rect.x + _cell.w) >= (render_rect.x + render_rect.w)) {
				current_rect.x = begin_x;
				current_rect.y += _cell.h;
			}
			if ((current_rect.y + _cell.h) >= (render_rect.y + render_rect.h)) {
				break;
			}
		}
	}

	inline void current_font(const std::shared_ptr<font_set> &font_ptr) {
		_font_ptr = font_ptr;
	}

	inline void pos(int x, int y) { _rect.x = x; _rect.y = y; }
	inline void size(int w, int h) { _rect.w = w; _rect.h = h; }
	inline void cell(int w, int h) { _cell.w = w; _cell.h = h; }

protected:
	template<typename... Args>
	inline auto put_char(Args&&... args) {
		if (auto p = font()) {
			p->put_char(std::forward<Args>(args)...);
		}
	}

private:
	SDL_Rect _rect{ 0, 0, 320, 200 };
	SDL_Rect _cell{ 0, 0, 8, 8 };
	std::weak_ptr<font_set> _font_ptr;
};

#endif // CONSOLE_HPP_
