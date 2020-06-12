//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "state_no_rom.hpp"

state_no_rom::state_no_rom(naive_2dge::engine& engine, naive_gbe::emulator& emulator)
	: state_base(engine, emulator, state::NO_ROM)
{
	using namespace std::placeholders;

	add_event_handler(SDL_DROPFILE, std::bind(&state_no_rom::on_drop_file, this, _1));
}

void state_no_rom::on_create()
{
	state_base::on_create();
}

void state_no_rom::on_enter(std::size_t prev_state)
{
	next_state_ = state::NO_ROM;
	engine_.show_cursor(true);
	state_base::on_enter(prev_state);
}

void state_no_rom::on_update()
{
	state_base::on_update();
}

std::size_t state_no_rom::on_drop_file(SDL_Event const& event)
{
	std::error_code ec;
	std::string rom_path = event.drop.file;

	if (!emulator_.load_rom(rom_path, ec))
		throw std::system_error(ec);

	return state::EMULATING;
}
