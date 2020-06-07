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
	{
		keymap_ = get_default_keymap();
	}

	emulator_gui::~emulator_gui()
	{
		destroy_vram();
		deinit_sdl();
	}

	void emulator_gui::run()
	{
		init_sdl();
		create_vram();
		load_icon();
		load_font();
		set_scale(scale_mode::SCALED_4x);

		if (state_ == state::READY)
		{
			SDL_ShowCursor(SDL_DISABLE);
			state_ = state::PLAYING;
		}

		while (state_ != state::FINISHED)
		{
			process_input();
			process_emulation();
			process_output();
		}
	}

	bool emulator_gui::load_rom(std::string const& rom_path, std::error_code& ec)
	{
		if (!emulator_.load_rom(rom_path, ec))
			return false;

		state_ = state::READY;

		return true;
	}

	void emulator_gui::set_window_size(std::uint16_t width, std::uint16_t height)
	{
		SDL_SetWindowSize(window_, width, height);
		SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

		width_ = width;
		height_ = height;
	}

	void emulator_gui::set_scale(scale_mode mode)
	{
		auto window = emulator_.get_ppu().get_window();

		switch (mode)
		{
		case scale_mode::NO_SCALING:
			set_window_size(window.width, window.height);
			scale_mode_ = mode;
			break;
		case scale_mode::SCALED_2x:
			set_window_size(window.width * 2, window.height * 2);
			scale_mode_ = mode;
			break;
		case scale_mode::SCALED_3x:
			set_window_size(window.width * 3, window.height * 3);
			scale_mode_ = mode;
			break;
		case scale_mode::SCALED_4x:
			set_window_size(window.width * 4, window.height * 4);
			scale_mode_ = mode;
			break;
		case scale_mode::FULLSCREEN:
			toggle_fullscreen();
			break;
		case scale_mode::CUSTOM:
			break;
		default:
			break;
		}
	}

	void emulator_gui::init_sdl()
	{
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
			throw_sdl_error("Could not initialise SDL2");

		if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
			throw_img_error("Could not initialise SDL2_image");

		if (Mix_OpenAudio(
			MIX_DEFAULT_FREQUENCY,
			MIX_DEFAULT_FORMAT,
			MIX_DEFAULT_CHANNELS, 4096) == -1)
			throw_mix_error("Could not initialise SDL2_mixer");

		int flags = MIX_INIT_FLAC | MIX_INIT_MP3 | MIX_INIT_MID | MIX_INIT_MOD;
		if (Mix_Init(flags) != flags)
			throw_mix_error("Could not initialise SDL2_mixer");

		if (TTF_Init() == -1)
			throw_ttf_error("Could not initialise SDL2_ttf");

		std::string title = app_name_ + " " + app_version_;
		window_ = SDL_CreateWindow(
			title.c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			width_,
			height_,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

		if (!window_)
			throw_sdl_error("Could not create window");

		renderer_ = SDL_CreateRenderer(
			window_,
			-1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

		if (!renderer_)
			throw_sdl_error("Could not create renderer");

		if (SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND))
			throw_sdl_error("Could not set blender mode");
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

	void emulator_gui::create_pallete()
	{
		SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
		if (!format)
			throw_sdl_error("Could not allocate format");

		pallete_ =
		{
			{ 0, SDL_MapRGBA(format,   0,   0,   0, 255) },
			{ 1, SDL_MapRGBA(format,  63,  63,  63, 255) },
			{ 2, SDL_MapRGBA(format, 127, 127, 127, 255) },
			{ 3, SDL_MapRGBA(format, 255, 255, 255, 255) },
		};

		SDL_FreeFormat(format);
	}

	void emulator_gui::create_vram()
	{
		auto window = emulator_.get_ppu().get_window();

		create_pallete();

		vram_ =
			SDL_CreateTexture(
				renderer_,
				SDL_PIXELFORMAT_RGBA32,
				SDL_TEXTUREACCESS_STREAMING,
				window.width,
				window.height);
	}

	void emulator_gui::update_vram()
	{
		if (!vram_)
			return;

		std::uint32_t* pixels = nullptr;
		int pitch = 0;

		SDL_LockTexture(vram_, nullptr, reinterpret_cast<void**>(&pixels), &pitch);

		auto ppu = emulator_.get_ppu();
		auto vram = ppu.get_video_ram();

		auto screen = ppu.get_screen();
		auto window = ppu.get_window();

		std::size_t total = 0;
		for (auto row = 0; row < window.height; ++row)
		{
			std::size_t offset = window.x_pos + (row + window.y_pos) * screen.width;

			for (auto col = 0; col < window.width; ++col)
			{
				*pixels++ = pallete_[vram[offset + col]];
				total++;
			}
		}

		SDL_UnlockTexture(vram_);
	}

	void emulator_gui::destroy_vram()
	{
		if (vram_)
		{
			SDL_DestroyTexture(vram_);
			vram_ = nullptr;
		}
	}

	void emulator_gui::load_icon()
	{
		std::string icon_path = assets_dir_ + "/app.ico";
		SDL_Surface* surface = IMG_Load(icon_path.c_str());

		if (!surface)
			throw_sdl_error("Unable to load icon '" + icon_path + "'");

		SDL_SetWindowIcon(window_, surface);
		SDL_FreeSurface(surface);
	}

	void emulator_gui::load_font()
	{
		std::string font_path = assets_dir_ + "/JetBrainsMono-Bold.ttf";
		font_ = TTF_OpenFont(font_path.c_str(), 36);

		if (!font_)
			throw_ttf_error("Failed to load font '" + font_path + "'");
	}

	bool emulator_gui::is_fullscreen()
	{
		return SDL_GetWindowFlags(window_) & SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	void emulator_gui::throw_sdl_error(std::string const& description)
	{
		throw std::runtime_error(description + ". Error: " + SDL_GetError());
	}

	void emulator_gui::throw_mix_error(std::string const& description)
	{
		throw std::runtime_error(description + ". Error: " + Mix_GetError());
	}

	void emulator_gui::throw_img_error(std::string const& description)
	{
		throw std::runtime_error(description + ". Error: " + IMG_GetError());
	}

	void emulator_gui::throw_ttf_error(std::string const& description)
	{
		throw std::runtime_error(description + ". Error: " + TTF_GetError());
	}

	void emulator_gui::toggle_fullscreen()
	{
		std::uint32_t flags = 0;

		if (!is_fullscreen())
			flags = SDL_WINDOW_FULLSCREEN_DESKTOP;

		if (SDL_SetWindowFullscreen(window_, flags) < 0)
			throw_sdl_error("Unable to set fullscreen");

		if (!flags)
			set_scale(scale_mode_);
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
		keys[SDLK_RIGHT] = input::RIGHT;

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

			if (event.type == SDL_DROPFILE)
			{
				std::error_code ec;
				std::string rom_path = event.drop.file;

				if (!emulator_.load_rom(rom_path, ec))
					throw std::system_error(ec);

				SDL_ShowCursor(SDL_DISABLE);
				state_ = state::PLAYING;

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

				if (event.key.keysym.sym == SDLK_p)
				{
					if (state_ == state::PAUSED)
					{
						SDL_ShowCursor(SDL_DISABLE);
						state_ = state::PLAYING;
					}
					else
					{
						SDL_ShowCursor(SDL_ENABLE);
						state_ = state::PAUSED;
					}
					continue;
				}

				if (event.key.keysym.sym == SDLK_f)
				{
					set_scale(scale_mode::FULLSCREEN);
					continue;
				}

				if (event.key.keysym.sym == SDLK_3)
				{
					set_scale(scale_mode::NO_SCALING);
					continue;
				}

				if (event.key.keysym.sym == SDLK_4)
				{
					set_scale(scale_mode::SCALED_2x);
					continue;
				}

				if (event.key.keysym.sym == SDLK_5)
				{
					set_scale(scale_mode::SCALED_3x);
					continue;
				}

				if (event.key.keysym.sym == SDLK_6)
				{
					set_scale(scale_mode::SCALED_4x);
					continue;
				}

				if (event.key.keysym.sym == SDLK_6)
				{
					stretch_ = !stretch_;
					continue;
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
		if (state_ == state::PLAYING)
			num_steps_ += emulator_.run();
	}

	void emulator_gui::process_output()
	{
		SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);

		SDL_RenderClear(renderer_);

		int width, height;
		SDL_GetWindowSize(window_, &width, &height);

		if (state_ == state::PLAYING)
		{
			update_vram();
			render_display();
		}
		else if (state_ == state::PAUSED)
		{
			SDL_Rect rect{ 0, 0, width, height };

			render_display();
			SDL_SetRenderDrawColor(renderer_, 0, 0, 128, 128);
			SDL_RenderFillRect(renderer_, &rect);
		}

		render_text(0, 0, std::to_string(get_fps()));

		SDL_RenderPresent(renderer_);
	}

	void emulator_gui::render_display()
	{
		int width, height;
		SDL_GetWindowSize(window_, &width, &height);

		auto window = emulator_.get_ppu().get_window();

		int max_width = width / window.width;
		int max_height = height / window.height;
		int factor = std::min(max_width, max_height);

		SDL_Rect src{ 0, 0, window.width, window.height };

		int w = window.width * factor;
		int h = window.height * factor;
		SDL_Rect dst{ (width - w) / 2, (height - h) / 2, w, h };

		render_texture(vram_, &src, stretch_ ? nullptr : &dst);
	}

	void emulator_gui::render_text(std::uint16_t x, std::uint16_t y, std::string const& text)
	{
		SDL_Surface* surface = TTF_RenderText_Solid(font_, text.c_str(), SDL_Color{ 0, 0, 255, 0 });

		if (!surface)
			throw_sdl_error("Could not render text '" + text + "'");

		SDL_Rect src = { 0, 0, surface->w, surface->h };
		SDL_Rect dst = { x, y, surface->w, surface->h };

		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);

		if (!texture)
		{
			SDL_FreeSurface(surface);

			throw_sdl_error("Could not create texture for text '" + text + "'");
		}

		SDL_FreeSurface(surface);

		render_texture(texture, &src, &dst);

		SDL_DestroyTexture(texture);
	}

	void emulator_gui::render_texture(SDL_Texture* texture, SDL_Rect* src, SDL_Rect* dst)
	{
		if (SDL_RenderCopy(renderer_, texture, src, dst) == -1)
			throw_sdl_error("Could not render texture");
	}

	float emulator_gui::get_fps() const
	{
		using namespace std::chrono;
		static auto last = high_resolution_clock::now();
		static std::size_t frames = 0;
		static float fps_rate = 0.0;

		auto now = high_resolution_clock::now();
		auto elapsed = duration_cast<milliseconds>(now - last).count();

		frames++;

		if (elapsed >= 100)
		{
			fps_rate = frames * 1000.0f / elapsed;
			last = now;
			frames = 0;
		}

		return fps_rate;
	}
}
