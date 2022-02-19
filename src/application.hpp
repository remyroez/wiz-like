#ifndef APPLICATION_HPP_
#define APPLICATION_HPP_

#include <SDL.h>
#include <iostream>
#include <memory>

#include "util.hpp"

class application {
public:
	application() {}
	virtual ~application() { finalize(); }

	inline bool initialized() const { return _initialized; }
	operator bool() {
		return initialized();
	}

protected:
	bool initialize(const char *title, int width, int height, Uint32 window_flags, Uint32 renderer_flags) {
		if (_initialized) {

		} else if (!_context.initialize(SDL_INIT_EVERYTHING)) {
			SDL_PrintError(SDL_Init);

		} else if (_window = make_window(
			title,
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			width,
			height,
			window_flags
		); !_window) {
			SDL_PrintError(SDL_CreateWindow);

		} else if (_renderer = make_renderer(window(), -1, renderer_flags); !_renderer) {
			SDL_PrintError(SDL_CreateRenderer);

		} else {
			_initialized = true;
		}

		return _initialized;
	}

	void finalize() {
		_renderer.reset();
		_window.reset();
		_context.finalize();
	}

	virtual void update(float deltatime = 0.f) {}
	virtual void draw() {}
	virtual void poll_event() {}

public:
	int boot() {
		const int ms_frame = 1000 / 60;
		Uint32 initial_ms = 0, elapsed_ms = 0;
		while (_running) {
			initial_ms = SDL_GetTicks();

			while (SDL_PollEvent(&_event)) {
				poll_event();
				switch (_event.type) {
				case SDL_QUIT:
					_running = false;
					break;
				case SDL_WINDOWEVENT:
					{
						switch (_event.window.event) {
						case SDL_WINDOWEVENT_CLOSE:
							if (_event.window.windowID == SDL_GetWindowID(window())) {
								_running = false;
							}
							break;
						}
					}
					break;
				}
			}

			update();

			draw();
			SDL_RenderPresent(renderer());

			elapsed_ms = SDL_GetTicks() - initial_ms;
			if (elapsed_ms < ms_frame) SDL_Delay(ms_frame - elapsed_ms);
		}

		return 0;
	}

protected:
	inline SDL_Window*window() { return _window ? _window.get() : nullptr; }
	inline SDL_Renderer*renderer() { return _renderer ? _renderer.get() : nullptr; }
	inline auto *event() { return &_event; }

private:
	SDL_Context _context;
	SDL_Pointer<SDL_Window> _window;
	SDL_Pointer<SDL_Renderer> _renderer;
	SDL_Event _event{};

	bool _running = true;
	bool _initialized = false;
};

#endif // APPLICATION_HPP_
