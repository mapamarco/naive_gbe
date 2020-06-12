//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "state_base.hpp"

namespace naive_gbe
{
	class state_emulating
		: public base_state
	{
	public:

		state_emulating(naive_2dge::engine& engine, emulator& emulator);

		void on_enter(std::size_t prev_state) override;

		void on_update() override;

		void on_exit() override;

	private:

		using keymap = std::unordered_map<SDL_Keycode, emulator::joypad_input>;

		using pallete = std::unordered_map<std::uint8_t, std::uint32_t>;

		enum class scale_mode
		{
			NO_SCALING,
			SCALED_2x,
			SCALED_3x,
			SCALED_4x,
		};

		std::size_t on_key_down(SDL_Event const& event);

		pallete create_pallete() const;

		keymap get_default_keymap() const;

		void set_scale(scale_mode mode);

		void update_vram();

		naive_2dge::font::ptr		font_		= nullptr;
		naive_2dge::texture::ptr	vram_		= nullptr;
		std::size_t					num_steps_	= 0;
		bool						paused_		= false;
		keymap						keymap_		= {};
		pallete						pallete_	= {};
	};

}