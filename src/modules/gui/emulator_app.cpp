//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "emulator_app.hpp"

#include "config.hpp"
#include "state_no_cartridge.hpp"
#include "state_help.hpp"
#include "state_emulating.hpp"
using namespace naive_gbe;

emulator_app::emulator_app(std::string const& assets_dir)
	: game("naive_gbe 0.0.1", 640, 576)
	, fps_{ 500 }
{
	auto& engine = get_engine();

	engine.set_assets_dir(assets_dir);

	add_state(std::make_shared<state_no_cartridge>(engine, emulator_));
	add_state(std::make_shared<state_help>(engine, emulator_));
	add_state(std::make_shared<state_emulating>(engine, emulator_));

	set_state(base_state::NO_CARTRIDGE);
}

bool emulator_app::load_rom(std::string const& rom_path, std::error_code& ec)
{
	if (!emulator_.load_rom(rom_path, ec))
		return false;

	set_state(base_state::EMULATING);

	return true;
}
