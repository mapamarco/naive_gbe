//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <naive_2dge/game.hpp>
#include <naive_2dge/detail/sdl.hpp>

#include <algorithm>
#include <vector>

namespace naive_2dge
{
	struct game::impl
	{
		using state_list = std::vector<state::ptr>;

		std::size_t		curr_indx_	= 0;
		state_list		states_;
		engine			engine_;
	};

	game::game()
		: impl_(std::make_unique<game::impl>())
	{
	}

	game::~game()
	{
	}

	void game::init(const std::string& title, std::uint32_t width, std::uint32_t height)
	{
		get_engine().init(title, width, height);
	}

	int game::run()
	{
		auto& engine = get_engine();
		state::ptr curr = get_curr_state();
		std::size_t next_state = impl_->curr_indx_;
		std::size_t prev_state = next_state;
		SDL_Event event;

		for (auto state : impl_->states_)
			state->on_create();

		curr->on_enter(prev_state);

		while (engine.keep_running())
		{
			curr->on_update();

			auto& handlers_map = curr->get_handlers_map();
			while (SDL_PollEvent(&event) != 0)
			{
				auto it = handlers_map.find(event.type);
				if (it != std::end(handlers_map))
				{
					auto& handler_list = it->second;
					for (auto& handler : handler_list)
					{
						next_state = handler(event);
					}
				}
			}

			engine.render();

			if (prev_state != next_state)
			{
				curr->on_exit();

				set_state(next_state);

				curr = get_curr_state();

				curr->on_enter(prev_state);

				prev_state = next_state;
			}
		}

		curr->on_exit();

		for (auto state : impl_->states_)
			state->on_destroy();

		return engine.get_exit_code();
	}

	engine& game::get_engine()
	{
		return impl_->engine_;
	}

	std::size_t game::add_state(state::ptr state)
	{
		impl_->states_.emplace_back(state);

		return impl_->states_.size() - 1;
	}

	void game::set_state(std::size_t indx)
	{
		impl_->curr_indx_ = indx;
	}

	state::ptr game::get_state(std::size_t indx)
	{
		return impl_->states_.at(indx);
	}

	state::ptr game::get_curr_state()
	{
		return get_state(impl_->curr_indx_);
	}
}
