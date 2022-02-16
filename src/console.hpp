#ifndef CONSOLE_HPP_
#define CONSOLE_HPP_

#include <SDL.h>

#include <memory>
#include <algorithm>
#include <vector>
#include <string>
#include <tinyutf8.h>

#include "util.hpp"
#include "font.hpp"

class cursor {
public:
	cursor() : _rect{} {}

	inline void x(int n) { _rect.x = n; }
	inline void y(int n) { _rect.y = n; }
	inline void w(int n) { _rect.w = n; }
	inline void h(int n) { _rect.h = n; }
	inline void pos(int x, int y) { _rect.x = x; _rect.y = y; }
	inline void size(int w, int h) { _rect.w = w; _rect.h = h; }
	inline void coord(int x, int y) { _rect.x = x * _rect.w; _rect.y = y * _rect.h; }

	inline const SDL_Rect& rect() const { return _rect; }

	inline int x() const { return _rect.x; }
	inline int y() const { return _rect.y; }
	inline int w() const { return _rect.w; }
	inline int h() const { return _rect.h; }
	inline SDL_Point pos() const { return { x(), y() }; }

	inline int coord_x() const { return _rect.x / _rect.w; }
	inline int coord_y() const { return _rect.y / _rect.h; }
	inline SDL_Point coord() const { return { coord_x(), coord_y() }; }

	inline SDL_Point to_pos(int x, int y) const { return { x * _rect.w, y * _rect.h }; }
	inline SDL_Point to_coord(int x, int y) const { return { x / _rect.w, y / _rect.h }; }

	inline void advance(int x = 1, int y = 0) {
		auto [adv_x, adv_y] = to_pos(x, y);
		_rect.x += adv_x;
		_rect.y += adv_y;
	}
	inline void advance_x(int x = 1) {
		advance(x, 0);
	}
	inline void advance_y(int y = 1) {
		advance(0, y);
	}
	inline void next_line(int left) {
		x(left);
		advance_y();
	}

private:
	SDL_Rect _rect{};
};

class console {
public:
	console() {}

	enum option {
		none = 0,
		inverse = 1 << 0,
		fill_cell_bg = 1 << 1,
	};

	inline auto current_font() {
		return _font_ptr.lock();
	}

	void print(SDL_Renderer* renderer, const tiny_utf8::utf8_string &string, option opt = option::none) {
		auto p = current_font();
		if (!p) return;

		bool inverse = ((opt & option::inverse) != 0);
		bool fill_cell_bg = ((opt & option::fill_cell_bg) != 0);
		auto& put_color = inverse ? _bg_color : _fg_color;

		for (char32_t codepoint : string) {
			if (codepoint == '\n') {
				next_line();

			} else {
				if (_cursor.x() >= right()) {
					next_line();
				}
				if (_cursor.y() >= bottom()) {
					break;
				}

				if (fill_cell_bg || inverse) fill_cell(renderer, inverse);
				p->put_char(renderer, _cursor.x(), _cursor.y(), codepoint, &put_color);
				_cursor.advance();
			}
		}
	}

	void print(SDL_Renderer* renderer, const tiny_utf8::utf8_string& string, int x, int y, option opt = option::none) {
		SDL_Rect render_rect = _rect;
		render_rect.x += x;
		render_rect.y += y;
		print(renderer, string, render_rect, opt);
	}

	void print(SDL_Renderer* renderer, const tiny_utf8::utf8_string& string, const SDL_Rect &rect, option opt = option::none) {
		auto p = current_font();
		if (!p) return;

		bool inverse = ((opt & option::inverse) != 0);
		bool fill_cell_bg = ((opt & option::fill_cell_bg) != 0);
		auto& put_color = inverse ? _bg_color : _fg_color;

		// 描画範囲
		SDL_Rect render_rect = rect;
		render_rect.x += _rect.x;
		render_rect.y += _rect.y;
		{
			auto temp_right = render_rect.x + render_rect.w;
			if (temp_right >= right()) {
				render_rect.w -= (temp_right - right());
			}
		}
		auto render_right = render_rect.x + render_rect.w;
		{
			auto temp_bottom = render_rect.y + render_rect.h;
			if (temp_bottom >= bottom()) {
				render_rect.h -= (temp_bottom - bottom());
			}
		}
		auto render_bottom = render_rect.y + render_rect.h;

		auto local_cursor = _cursor;
		local_cursor.pos(render_rect.x, render_rect.y);

		if ((local_cursor.x() + local_cursor.w()) > render_right) return;

		for (char32_t codepoint : string) {
			if (codepoint == '\n') {
				local_cursor.next_line(render_rect.x);

			} else {
				if ((local_cursor.x() + local_cursor.w()) > render_right) {
					local_cursor.next_line(render_rect.x);
				}
				if ((local_cursor.y() + local_cursor.h()) > render_bottom) {
					break;
				}

				if (fill_cell_bg || inverse) {
					fill_cell(renderer, { local_cursor.x(), local_cursor.y(), local_cursor.w(), local_cursor.h() }, inverse);
				}
				p->put_char(renderer, local_cursor.x(), local_cursor.y(), codepoint, &_fg_color);
				local_cursor.advance();
			}
		}
	}

	void fill(SDL_Renderer *renderer, bool inverse = false) {
		auto& color = inverse ? _fg_color : _bg_color;
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0xFF);
		SDL_RenderFillRect(renderer, &_rect);
	}

	void fill_cell(SDL_Renderer* renderer, bool inverse = false) {
		auto& color = inverse ? _fg_color : _bg_color;
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0xFF);
		SDL_RenderFillRect(renderer, &_cursor.rect());
	}

	void fill_cell(SDL_Renderer* renderer, const SDL_Rect& rect, bool inverse = false) {
		auto& color = inverse ? _fg_color : _bg_color;
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0xFF);
		SDL_RenderFillRect(renderer, &rect);
	}

	inline void current_font(const std::shared_ptr<font_set> &font_ptr) {
		_font_ptr = font_ptr;
	}

	inline void pos(int x, int y) { _rect.x = x; _rect.y = y; }
	inline void cell(int w, int h) { _cell.w = w; _cell.h = h; _cursor.size(w, h); }
	inline void size(int w, int h) { _rect.w = w; _rect.h = h; }
	inline void geom(int cols, int rows) { size(_cell.w * cols, _cell.h * rows); }

	inline void fg_color(const SDL_Color& color) { _fg_color = color; }
	inline void bg_color(const SDL_Color &color) { _bg_color = color; }

	inline const SDL_Rect &rect() const { return _rect; }
	inline const SDL_Rect& cell() const { return _cell; }

	inline int left() const { return _rect.x; }
	inline int top() const { return _rect.y; }
	inline int right() const { return _rect.x + _rect.w; }
	inline int bottom() const { return _rect.y + _rect.h; }

	inline SDL_Point grid_to_real(int x, int y) const { return { x * _cell.w, y * _cell.h }; }
	inline SDL_Point real_to_grid(int x, int y) const { return { x / _cell.w, y / _cell.h }; }

	inline void coord(int coord_x = 0, int corrd_y = 0) {
		_cursor.coord(coord_x, corrd_y);
		_cursor.x(_cursor.x() + left());
		_cursor.y(_cursor.y() + top());
	}

	inline void cr() { _cursor.x(left()); }
	inline void lf() { _cursor.advance_y(); }
	inline void next_line() { cr(); lf(); }

	inline void print(const tiny_utf8::utf8_string &string, const SDL_Rect &rect, option opt = option::none) {
		_entries.push_back({ SDL_Rect{ rect.x * _cell.w, rect.y * _cell.h, rect.w * _cell.w, rect.h * _cell.h }, string, opt });
	}

	inline void print(const tiny_utf8::utf8_string& string, int x, int y, option opt = option::none) {
		_entries.push_back({ SDL_Rect{ x, y, -1, -1 }, string, opt });
	}

	inline void print(const tiny_utf8::utf8_string& string, option opt = option::none) {
		_entries.push_back({ SDL_Rect{ -1, -1, -1, -1 }, string, opt });
	}

	inline void cls() {
		_entries.clear();
	}

	inline void flush(SDL_Renderer* renderer) {
		fill(renderer);
		for (const auto &entry : _entries) {
			if (entry.rect.x < 0) {
				print(renderer, entry.str, entry.opt);

			} else if (entry.rect.w < 0) {
				coord(entry.rect.x, entry.rect.y);
				print(renderer, entry.str, entry.opt);

			} else {
				print(renderer, entry.str, entry.rect, entry.opt);
			}
		}
	}

protected:
	template<typename... Args>
	inline auto put_char(Args&&... args) {
		if (auto p = font()) {
			p->put_char(std::forward<Args>(args)...);
		}
	}

	struct entry {
		SDL_Rect rect;
		tiny_utf8::utf8_string str;
		option opt;
	};

private:
	cursor _cursor;
	SDL_Rect _rect{ 0, 0, 320, 200 };
	SDL_Rect _cell{ 0, 0, 8, 8 };
	std::weak_ptr<font_set> _font_ptr;
	SDL_Color _fg_color{ 0xFF, 0xFF, 0xFF, 0xFF };
	SDL_Color _bg_color{ 0, 0, 0, 0xFF };

	std::vector<entry> _entries;
};

#endif // CONSOLE_HPP_
