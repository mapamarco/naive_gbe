//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <memory>
#include <string>

#include <naive_2dge/state.hpp>

namespace naive_2dge
{
	class game
	{
	public:

		game();

		virtual ~game();

		virtual int run();

		void init(const std::string& title, std::uint32_t width, std::uint32_t height);

	protected:

		engine& get_engine();

		std::size_t add_state(state::ptr state);

		void set_state(std::size_t indx);

		state::ptr get_state(std::size_t indx);

		state::ptr get_curr_state();

	private:

		struct impl;

		std::unique_ptr<impl>   impl_;
	};
}
