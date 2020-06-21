//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "state_base.hpp"

class state_emulating
	: public state_base
{
public:

	state_emulating(naive_2dge::engine& engine, emulator_data& data, naive_gbe::emulator& emulator);

	void on_create() override;

	void on_enter(std::size_t prev_state) override;

	void on_update() override;

private:

	using keymap = std::unordered_map<SDL_Keycode, naive_gbe::emulator::joypad_input>;

	using pallete = std::unordered_map<std::uint8_t, std::uint32_t>;

	std::size_t on_key_down(SDL_Event const& event);

	std::size_t on_key_up(SDL_Event const& event);

	void toggle_pause();

	pallete create_pallete() const;

	void update_vram();

	naive_2dge::texture::ptr	vram_			= nullptr;

	std::size_t					steps_to_run_	= 0;

	std::size_t					steps_			= 0;

	bool						paused_			= false;

	pallete						pallete_		= {};
};
