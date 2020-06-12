//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "state_help.hpp"

state_help::state_help(naive_2dge::engine& engine, naive_gbe::emulator& emulator)
	: state_base(engine, emulator, state::HELP)
{
	using namespace std::placeholders;

	add_event_handler(SDL_KEYDOWN, std::bind(&state_help::on_key_down, this, _1));
}

void state_help::on_create()
{
	font_ = engine_.create_font("help", "JetBrainsMono-Bold.ttf", 24);
	state_base::on_create();
}

void state_help::on_enter(std::size_t prev_state)
{
	next_state_ = state::HELP;
	engine_.show_cursor(true);
	state_base::on_enter(prev_state);
}

void state_help::on_update()
{
	auto [win_w, win_h] = engine_.get_window_size();
	std::string text = "Help: TODO";
	std::uint32_t text_w, text_h;

	engine_.get_text_size(text, font_, text_w, text_h);
	engine_.draw(text, font_, win_w / 2 - text_w / 2, win_h / 2 - text_h / 2);

	state_base::on_update();
}

std::size_t state_help::on_key_down(SDL_Event const& event)
{
	switch (event.key.keysym.sym)
	{
	case SDLK_ESCAPE:
		engine_.cancel_exit();
		next_state_ = prev_state_;
		break;
	case SDLK_F1:
		next_state_ = prev_state_;
		break;
	}

	return next_state_;
}
