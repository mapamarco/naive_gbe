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
	class state_help
		: public base_state
	{
	public:

		state_help(naive_2dge::engine& engine, emulator& emulator);

		void on_enter(std::size_t prev_state) override;

		void on_update() override;

		void on_exit() override;

	private:

		std::size_t on_key_down(SDL_Event const& event);

		naive_2dge::font::ptr	font_ = nullptr;

		std::size_t	prev_state_ = 0;
	};

}