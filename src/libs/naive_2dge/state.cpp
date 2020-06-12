//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <naive_2dge/state.hpp>
namespace naive_2dge
{
	state::state(engine& engine)
		: engine_(engine)
	{
	}

	void state::on_create()
	{
	}

	void state::on_enter(std::size_t next_state)
	{
	}

	void state::on_update()
	{
	}

	void state::on_exit()
	{
	}

	void state::on_destroy()
	{
	}

	void state::remove_event_handler(std::uint32_t event)
	{
		handler_map_[event].clear();
	}

	void state::add_event_handler(std::uint32_t event, event_handler handler)
	{
		handler_map_[event].emplace_back(handler);
	}

	state::handlers_map const& state::get_handlers_map()
	{
		return handler_map_;
	}
}
