//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <naive_2dge/state.hpp>
#include <naive_2dge/engine.hpp>
#include <naive_gbe/emulator.hpp>

namespace naive_gbe
{
	class base_state
		: public naive_2dge::state
	{
	public:

		enum state
		{
			NO_CARTRIDGE,
			HELP,
			EMULATING,
		};

		base_state(naive_2dge::engine& engine, emulator& emulator, std::size_t next_state);

	protected:

		std::size_t on_quit();

		std::size_t on_key_down(SDL_Event const& event);

		void throw_error(std::string const& description, std::string const& detail) const;

		emulator&		emulator_;
		std::size_t		state_;
		std::size_t		next_state_;
	};
}