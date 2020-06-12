//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "state_base.hpp"

namespace naive_gbe
{
	base_state::base_state(naive_2dge::engine& engine, emulator& emulator, std::size_t next_state)
		: naive_2dge::state(engine)
		, emulator_(emulator)
		, state_(next_state)
		, next_state_(next_state)
	{
		using namespace std::placeholders;

		add_event_handler(SDL_QUIT, std::bind(&base_state::on_quit, this));
		add_event_handler(SDL_KEYDOWN, std::bind(&base_state::on_key_down, this, _1));
	}

	std::size_t base_state::on_quit()
	{
		engine_.exit(EXIT_SUCCESS);
		return next_state_;
	}

	std::size_t base_state::on_key_down(SDL_Event const& event)
	{
		switch (event.key.keysym.sym)
		{
		case SDLK_ESCAPE:
		case SDLK_q:
			next_state_ = on_quit();
			break;
		case SDLK_F1:
			next_state_ = state::HELP;
			break;
		}

		return next_state_;
	}

	void base_state::throw_error(std::string const& description, std::string const& detail) const
	{
		throw std::runtime_error(description + ". Error: " + detail);
	}
}