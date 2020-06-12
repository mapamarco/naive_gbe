//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "state_base.hpp"

class state_no_rom
	: public state_base
{
public:

	state_no_rom(naive_2dge::engine& engine, naive_gbe::emulator& emulator);

	void on_create() override;

	void on_enter(std::size_t prev_state) override;

	void on_update() override;

private:

	std::size_t on_drop_file(SDL_Event const& event);

	naive_2dge::font::ptr	font_ = nullptr;
};
