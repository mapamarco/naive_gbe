//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>

#include <naive_2dge/engine.hpp>

namespace naive_2dge
{
    class state
    {
	public:

        using ptr			= std::shared_ptr<state>;
		using event_handler	= std::function<std::size_t(SDL_Event const&)>;
		using handler_list	= std::vector<event_handler>;
		using handlers_map	= std::unordered_map<std::uint32_t, handler_list>;

		state(engine& engine);

		virtual void on_enter(std::size_t prev_state);
        
        virtual void on_update();
        
		virtual void on_exit();

		void remove_event_handler(std::uint32_t event);

		void add_event_handler(std::uint32_t event, event_handler handler);

		handlers_map const& get_handlers_map();

	protected:

		engine&			engine_;
		handlers_map	handler_map_;
	};
}
