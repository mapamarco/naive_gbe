// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "emulator_gui.hpp"

#include <stdexcept>
#include <sstream>

namespace naive_gbe
{
	emulator_gui::emulator_gui(std::uint16_t width, std::uint16_t height)
		: state_(state::NO_CARTRIDGE)
		, width_(width)
		, height_(height)
	{
		keymap_ = get_default_keymap();
	}

	emulator_gui::~emulator_gui()
	{
		deinit_sdl();
	}

	void emulator_gui::run()
	{
		init_sdl();

		while (state_ != state::FINISHED)
		{
			process_input();
			process_emulation();
			process_output();
		}
	}

	bool emulator_gui::load_rom(std::string const& rom_path, std::error_code& ec)
	{
		return emulator_.load_rom(rom_path, ec);
	}

	void emulator_gui::init_sdl()
	{
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			std::stringstream err;
			err << "Could not initialise SDL. Error: " << SDL_GetError();
			throw std::runtime_error(err.str());
		}

		window_ = SDL_CreateWindow(
			"Naive Game Boy",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			width_,
			height_,
			SDL_WINDOW_SHOWN);

		if (!window_)
		{
			std::stringstream err;
			err << "Window could not be created! SDL Error: " << SDL_GetError();
			throw std::runtime_error(err.str());
		}

		renderer_ = SDL_CreateRenderer(
			window_,
			-1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

		if (!renderer_)
		{
			std::stringstream err;
			err << "Renderer could not be created! SDL Error: " << SDL_GetError();
			throw std::runtime_error(err.str());
		}
	}

	void emulator_gui::deinit_sdl()
	{
		if (renderer_)
		{
			SDL_DestroyRenderer(renderer_);
			renderer_ = nullptr;
		}

		if (window_)
		{
			SDL_DestroyWindow(window_);
			window_ = nullptr;
		}

		SDL_Quit();
	}

	emulator_gui::keymap emulator_gui::get_default_keymap() const
	{
		using input = emulator::joypad_input;

		keymap keys;

		keys[SDLK_1] = input::SELECT;
		keys[SDLK_2] = input::START;
		keys[SDLK_a] = input::A;
		keys[SDLK_s] = input::B;
		keys[SDLK_UP] = input::UP;
		keys[SDLK_DOWN] = input::DOWN;
		keys[SDLK_LEFT] = input::LEFT;
		keys[SDLK_RIGHT]= input::RIGHT;

		return keys;
	}

	void emulator_gui::process_input()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				state_ = state::FINISHED;
				break;
			}
			
			if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_ESCAPE
					|| event.key.keysym.sym == SDLK_q)
				{
					state_ = state::FINISHED;
					break;
				}

				if (event.key.keysym.sym == SDLK_RETURN)
				{
					switch (state_)
					{
					case state::PAUSED:
						state_ = state::PLAYING;
						break;
					case state::PLAYING:
						state_ = state::PAUSED;
						break;
					default:
						break;
					}
				}

				auto it = keymap_.find(event.key.keysym.sym);
				if (it != std::end(keymap_))
				{
				}
			}
			else if (event.type == SDL_KEYUP)
			{
				auto it = keymap_.find(event.key.keysym.sym);
				if (it != std::end(keymap_))
				{
				}
			}
		}
	}

	void emulator_gui::process_emulation()
	{
		num_steps_ += emulator_.run();
	}

	void emulator_gui::process_output()
	{
		SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
		SDL_RenderClear(renderer_);

		int point_w = width_ / 64;
		int point_h = height_ / 32;

		SDL_Rect point;
		point.h = point_h;
		point.w = point_w;

		SDL_SetRenderDrawColor(renderer_, 100, 100, 150, 255);
		SDL_RenderPresent(renderer_);
	}
}
