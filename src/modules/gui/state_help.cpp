//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "state_help.hpp"

namespace naive_gbe
{
	state_help::state_help(naive_2dge::engine& engine, emulator& emulator)
		: base_state(engine, emulator, state::HELP)
	{
		using namespace std::placeholders;

		remove_event_handler(SDL_KEYDOWN);
		add_event_handler(SDL_KEYDOWN, std::bind(&state_help::on_key_down, this, _1));
	}

	void state_help::on_enter(std::size_t prev_state)
	{
		prev_state_ = prev_state;
		font_ = engine_.create_font("help", "JetBrainsMono-Bold.ttf", 20);
		SDL_ShowCursor(SDL_ENABLE);
	}

	void state_help::on_exit()
	{
	}

	void state_help::on_update()
	{
		engine_.draw_text("HELP", font_, 0, 0, 0, 0, 255);
	}

	std::size_t state_help::on_key_down(SDL_Event const& event)
	{
		switch (event.key.keysym.sym)
		{
		case SDLK_ESCAPE:
		case SDLK_F1:
			next_state_ = prev_state_;
			break;
		}

		return next_state_;
	}
}