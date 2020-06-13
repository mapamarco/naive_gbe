//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "emulator_app.hpp"

#include "config.hpp"
#include "state_no_rom.hpp"
#include "state_help.hpp"
#include "state_emulating.hpp"

emulator_app::emulator_app(std::string const& assets_dir)
{
	auto& engine = get_engine();

	get_engine().set_assets_dir(assets_dir);

	add_state(std::make_shared<state_no_rom>(engine, data_, emulator_));
	add_state(std::make_shared<state_help>(engine, data_, emulator_));
	add_state(std::make_shared<state_emulating>(engine, data_, emulator_));

	set_state(state_base::state::NO_ROM);
}

bool emulator_app::load_rom(std::string const& rom_path, std::error_code& ec)
{
	if (!emulator_.load_rom(rom_path, ec))
		return false;

	set_state(state_base::state::EMULATING);

	return true;
}

int emulator_app::run()
{
	auto const& ppu = emulator_.get_ppu();
	auto scale = 4;
	auto width = ppu.get_screen_width() * scale;
	auto height = ppu.get_screen_height() * scale;

	init("naive_gbe 0.0.1", width, height);

	auto& engine = get_engine();

	engine.set_icon("app.ico");

	data_.debug_font_ = engine.create_font("debug", "JetBrainsMono-Bold.ttf", 20);
	data_.help_font_ = engine.create_font("help", "JetBrainsMono-Bold.ttf", 30);

	return game::run();
}