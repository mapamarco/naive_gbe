// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "emulator_gui.hpp"

#include <stdexcept>
#include <sstream>

namespace naive_gbe
{
	emulator_gui::emulator_gui(std::string const& assets_dir)
		: assets_dir_(assets_dir)
		, state_(state::NO_CARTRIDGE)
		, width_(emulator::SCREEN_WIDTH * 3)
		, height_(emulator::SCREEN_HEIGHT * 3)
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

	void emulator_gui::set_window_size(std::uint16_t width, std::uint16_t height)
	{
		SDL_SetWindowSize(window_, width, height);
		width_  = width;
		height_ = height;
	}

	void emulator_gui::init_sdl()
	{
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			std::stringstream err;
			err << "Could not initialise SDL2. Error: " << SDL_GetError();
			throw std::runtime_error(err.str());
		}

		if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
		{
			std::stringstream err;
			err << "Could not initialise SDL2_image. Error: " << IMG_GetError();
			throw std::runtime_error(err.str());
		}

		if (Mix_OpenAudio(
			MIX_DEFAULT_FREQUENCY,
			MIX_DEFAULT_FORMAT,
			MIX_DEFAULT_CHANNELS, 4096) == -1)
		{
			std::stringstream err;
			err << "Could not initialise SDL2_mixer: Error " << Mix_GetError();
			throw std::runtime_error(err.str());
		}

		int flags = MIX_INIT_FLAC | MIX_INIT_MP3 | MIX_INIT_MID | MIX_INIT_MOD;
		if (Mix_Init(flags) != flags)
		{
			std::stringstream err;
			err << "Could not initialise SDL2_mixer: Error: " << Mix_GetError();
			throw std::runtime_error(err.str());
		}

		if (TTF_Init() == -1)
		{
			std::stringstream err;
			err << "Could not initialise SDL2_ttf: Error: " << TTF_GetError();
			throw std::runtime_error(err.str());
		}

		std::string title = app_name_ + " " + app_version_;
		window_ = SDL_CreateWindow(
			title.c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			width_,
			height_,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

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

		SDL_ShowCursor(SDL_DISABLE);

		set_icon();
		load_font();
	}

	void emulator_gui::deinit_sdl()
	{
		if (font_)
		{
			TTF_CloseFont(font_);
			font_ = nullptr;
		}

		TTF_Quit();

		Mix_CloseAudio();
		Mix_Quit();

		IMG_Quit();

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

	void emulator_gui::set_icon()
	{
		std::string icon_path = assets_dir_ + "/app.ico";
		SDL_Surface* surface = IMG_Load(icon_path.c_str());

		if (!surface)
		{
			std::stringstream err;
			err << "Unable to load icon " << icon_path <<  ". Error: " << SDL_GetError();
			throw std::runtime_error(err.str());
		}

		SDL_SetWindowIcon(window_, surface);
		SDL_FreeSurface(surface);
	}

	void emulator_gui::load_font()
	{
		std::string font_path = assets_dir_ + "/JetBrainsMono-Bold.ttf";
		font_ = TTF_OpenFont(font_path.c_str(), static_cast<int>(24));

		if (!font_)
		{
			std::stringstream err;
			err << "Failed to load font '" << font_path << "'. Error: " << TTF_GetError();
			throw std::runtime_error(err.str());
		}
	}

	void emulator_gui::toggle_fullscreen()
	{
		std::uint32_t flags = SDL_GetWindowFlags(window_);

		if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
			flags = 0;
		else
			flags = SDL_WINDOW_FULLSCREEN_DESKTOP;

		if (SDL_SetWindowFullscreen(window_, flags) < 0)
		{
			std::stringstream err;
			err << "Unable to set fullscreen. Error: " << SDL_GetError();
			throw std::runtime_error(err.str());
		}

		if (SDL_GetWindowDisplayMode(window_, &display_) < 0)
		{
			std::stringstream err;
			err << "Unable to get display information! Error: " << SDL_GetError();
			throw std::runtime_error(err.str());
		}
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

				if (event.key.keysym.sym == SDLK_f)
				{
					toggle_fullscreen();
					break;
				}

				if (event.key.keysym.sym == SDLK_3)
					set_window_size(emulator::SCREEN_WIDTH, emulator::SCREEN_HEIGHT);

				if (event.key.keysym.sym == SDLK_4)
					set_window_size(emulator::SCREEN_WIDTH * 2, emulator::SCREEN_HEIGHT * 2);

				if (event.key.keysym.sym == SDLK_5)
					set_window_size(emulator::SCREEN_WIDTH * 3, emulator::SCREEN_HEIGHT * 3);

				if (event.key.keysym.sym == SDLK_6)
					set_window_size(emulator::SCREEN_WIDTH * 4, emulator::SCREEN_HEIGHT * 4);

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

		std::string text = "Press ESC to quit";
		SDL_Surface* surface = TTF_RenderText_Solid(font_, text.c_str(), SDL_Color{ 0, 0, 255, 0 });

		if (!surface)
		{
			std::stringstream err;
			err << "failed to render text '" << text << "'! Error: " << IMG_GetError();
			throw std::runtime_error(err.str());
		}

		int w = surface->w;
		int h = surface->h;

		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);

		SDL_FreeSurface(surface);

		SDL_Rect src = { 0, 0, w, h };
		SDL_Rect dst = { width_ / 2 - w / 2, height_ / 2 - h /2, w, h };

		if (SDL_RenderCopy(renderer_, texture, &src, nullptr) == -1)
		{
			std::stringstream err;
			err << "render copy error: " << SDL_GetError();
			throw std::runtime_error(err.str());
		}

		int point_w = width_ / 64;
		int point_h = height_ / 32;

		SDL_Rect point;
		point.h = point_h;
		point.w = point_w;

		SDL_SetRenderDrawColor(renderer_, 100, 100, 150, 255);
		SDL_RenderPresent(renderer_);
	}
}
