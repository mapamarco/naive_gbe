//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "state_no_cartridge.hpp"

namespace naive_gbe
{
	state_no_cartridge::state_no_cartridge(naive_2dge::engine& engine, emulator& emulator)
		: base_state(engine, emulator, state::NO_CARTRIDGE)
	{
		using namespace std::placeholders;

		add_event_handler(SDL_DROPFILE, std::bind(&state_no_cartridge::on_drop_file, this, _1));
	}

	void state_no_cartridge::on_enter(std::size_t prev_state)
	{
		font_ = engine_.create_font("fps", "JetBrainsMono-Bold.ttf", 24);
		SDL_ShowCursor(SDL_ENABLE);
		engine_.set_icon("app.ico");
		next_state_ = state_;
	}

	void state_no_cartridge::on_update()
	{
		engine_.draw_text("NO CARTRIDGE", font_, 0, 0);
	}

	void state_no_cartridge::on_exit()
	{
	}

	std::size_t state_no_cartridge::on_drop_file(SDL_Event const& event)
	{
		std::error_code ec;
		std::string rom_path = event.drop.file;

		if (!emulator_.load_rom(rom_path, ec))
			throw std::system_error(ec);

		return state::EMULATING;
	}
}