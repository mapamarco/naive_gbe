//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <naive_2dge/state.hpp>
#include <naive_2dge/engine.hpp>
#include <naive_2dge/types.hpp>
#include <naive_gbe/emulator.hpp>

class state_base
	: public naive_2dge::state
{
public:

	enum state
	{
		NO_ROM		= 0,
		HELP,
		EMULATING,
	};

	enum flags
	{
		STRETCH		= 1 << 0,
		DEBUG		= 1 << 1,
	};

	enum class scale_mode
	{
		NO_SCALING,
		SCALED_2X,
		SCALED_3X,
		SCALED_4X,
	};

	state_base(naive_2dge::engine& engine, naive_gbe::emulator& emulator, std::size_t next_state);

	virtual void on_create() override;

	virtual void on_enter(std::size_t prev_state) override;

	virtual void on_update() override;

protected:

	std::size_t on_quit();

	std::size_t on_key_down(SDL_Event const& event);

	void on_update_debug();

	void set_scale(scale_mode mode);

	std::string fps_fmt(float fps);

	std::string state_fmt(std::size_t state);

	std::string reg_fmt(naive_gbe::lr35902::r16 reg);

	void throw_error(std::string const& description, std::string const& detail) const;

	naive_gbe::emulator&		emulator_;

	naive_2dge::font::ptr		debug_fnt_;

	std::size_t					prev_state_;

	std::size_t					next_state_;

	static std::uint32_t		flags_;
};
