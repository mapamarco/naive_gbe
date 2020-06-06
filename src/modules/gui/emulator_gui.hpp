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
#else
	#include <SDL2/SDL.h>
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

		emulator_gui(std::uint16_t width = 640, std::uint16_t height = 320);

		virtual ~emulator_gui();

		bool load_rom(std::string const& rom_path, std::error_code& ec);

		void run();

	private:

		using keymap = std::unordered_map<SDL_Keycode, emulator::joypad_input>;

		void init_sdl();

		void deinit_sdl();

		keymap get_default_keymap() const;

		void process_input();

		void process_emulation();

		void process_output();

		emulator		emulator_;
		keymap			keymap_;
		std::size_t		num_steps_	= 0;
		state			state_		= state::NO_CARTRIDGE;
		uint16_t		width_		= 0;
		uint16_t		height_		= 0;
		SDL_Window*		window_		= nullptr;
		SDL_Renderer*	renderer_	= nullptr;
	};
}
