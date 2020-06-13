//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "state_base.hpp"

class state_help
	: public state_base
{
public:

	state_help(naive_2dge::engine& engine, emulator_data& data, naive_gbe::emulator& emulator);

	void on_create() override;

	void on_enter(std::size_t prev_state) override;

	void on_update() override;

private:

	std::size_t on_key_down(SDL_Event const& event);
};
