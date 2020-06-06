//
//            Copyright (c) Marco Amorim 2020.
// This project can not be copied and/or distributed without the
// express permission of his author.
//
#pragma once

#include <cstdint>
#include <unordered_map>

#if _WIN32
	#include <SDL.h>
	#include <SDL_mixer.h>
	#include <SDL_ttf.h>
	#include <SDL_image.h>
#else
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_mixer.h>
	#include <SDL2/SDL_ttf.h>
	#include <SDL2/SDL_image.h>
#endif

#include <naive_gbe/emulator.hpp>

namespace naive_gbe
{
	class emulator_gui
	{
	public:

		enum class state
		{
			NO_CARTRIDGE,
			READY,
			PLAYING,
			PAUSED,
			DEBUGGING,
			FINISHED
		};

		emulator_gui(std::string const& assets_dir);

		virtual ~emulator_gui();

		bool load_rom(std::string const& rom_path, std::error_code& ec);

		void run();

	private:

		using keymap = std::unordered_map<SDL_Keycode, emulator::joypad_input>;

		void set_window_size(std::uint16_t width, std::uint16_t height);

		void init_sdl();

		void deinit_sdl();

		void set_icon();

		void load_font();

		void toggle_fullscreen();

		keymap get_default_keymap() const;

		void process_input();

		void process_emulation();

		void process_output();

		emulator		emulator_;
		keymap			keymap_;
		std::string		assets_dir_;
		std::size_t		num_steps_	= 0;
		state			state_		= state::NO_CARTRIDGE;
		std::uint16_t	width_		= 0;
		std::uint16_t	height_		= 0;
		SDL_Window*		window_		= nullptr;
		SDL_Renderer*	renderer_	= nullptr;
		TTF_Font*		font_		= nullptr;
		SDL_DisplayMode display_	= {};
	};
}
